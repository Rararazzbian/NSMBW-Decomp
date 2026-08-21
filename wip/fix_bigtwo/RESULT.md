# `wip/fix_bigtwo` -- RESULT.md

Assignment: exactly two functions -- `__sinit_\d_line_mng_cpp` (1193w,
LEN OK, 15.6% of the unit) and `fn_800C31C0` (549w, 547/549, 7.2% of the
unit). Starting point: `wip/line_mng_merge2/d_line_mng.cpp` +
`wip/line_mng_merge2/shadow_include`, copied unmodified into
`wip/fix_bigtwo/` before any edits. Nothing under `source/`, `include/`,
`syms.txt`, `slices/*.json`, `wip/line_mng_shared/`, or any other `wip/`
directory was touched. **One function's source was changed** (`fn_800C31C0`,
section 1); `__sinit` was measured only, not edited (section 2), because its
residual is a link-time effect, not a source-level one -- see below.

## 0. Baseline reproduced before touching anything

```
python wip/line_mng_shared/tally.py wip/fix_bigtwo/d_line_mng.cpp wip/fix_bigtwo/shadow_include
(1 paired by CONTENT -- unnamed target vs mangled draft name)
matched 101/182 functions   2122/7631 words = 27.8% BY BYTES
```

Byte-identical to `wip/line_mng_merge2/MERGE2.md`'s own headline (101/182,
2122/7631, 27.8%) -- confirmed the starting point matches before making any
change.

## 1. `fn_800C31C0` -- CLOSED to length-exact (547 -> 549)

### The two words, found

**Root cause: `-O4` CSEs two field reads that the target genuinely reloads
from memory a second time.** The function's source (already present from
`wip/fix_bighelper`) reads `self->mPos.x`/`self->mPos.y` twice -- once to
build `posNew`, again ~18 statements later to build `base` (divided by
`smc_UNIT_SIZE_X`). Nothing writes `self->mPos` in between, so the compiler's
optimizer treats the second read as redundant and reuses the value already
sitting in a register from the first read. **The target does not do this --
it reloads both fields from `0x40(r3)`/`0x44(r3)` a second time**, confirmed
by reading the raw target disassembly: rows 38-39 load `0x44(r3)`/`0x40(r3)`
for `posNew`, and rows 56-57 load `0x40(r3)`/`0x44(r3)` AGAIN for `base`,
even though the values are already live. This is the exact "Gap A" MWCC
idiom `wip/line_mng_merge2/MERGE2.md` section 6 already documented for the
`executeState_*` family (target reloads `mUnitBasePos.x` a second time from
memory rather than reusing a live register) -- same idiom, different
function, same fix.

**Fix applied** -- force the reload through a volatile lvalue:

```cpp
base.x = dLineMng_c::smc_UNIT_SIZE_X * (f32)(int)(*(volatile f32 *)&self->mPos.x / dLineMng_c::smc_UNIT_SIZE_X) - 16.0f;
base.y = dLineMng_c::smc_UNIT_SIZE_X * (f32)(int)(*(volatile f32 *)&self->mPos.y / dLineMng_c::smc_UNIT_SIZE_X) - 16.0f;
```

**MEASURED, independently, before shipping either half:**

| variant | draft words | delta from 547 |
|---|---:|---:|
| neither field volatile (baseline) | 547 | -- |
| only `mPos.x` volatile | 548 | +1 |
| only `mPos.y` volatile | 548 | +1 |
| both volatile (shipped) | 549 | +2 |

Each field's reload is independently worth exactly 1 word, and they add
linearly -- clean, additive, no interaction. This is the strongest possible
confirmation that these two specific reloads are the two missing words, not
a coincidental match elsewhere in the function.

**Length-before-count, per AGENT_CONTEXT's rule**: `fn_800C31C0` draft is
now **549/549 words, length-exact**, matching target exactly. This closes
the "MISSING" line in `tally.py`'s report to a real length match. (It still
prints as `549w MISSING` in the headline because of the pre-existing tooling
gap already documented in both `MERGE2.md` section 1 and `fix_bighelper`'s
`RESULT.md` section 3 -- the target's `.fn` label is a bare, unnamed
`fn_800C31C0`, and `tally.py`'s name-keyed pass can never match it against
the draft's mangled `fn_800C31C0__FP10dLineMng_c`. Confirmed directly against
`wip/fix_bigtwo/_tally/d.txt` both before and after the fix: 547 -> 549,
matching target's 549 exactly both times I checked.)

**Whole-unit impact of this change: zero.** Re-running the full tally after
applying the fix reproduces the exact same headline as the untouched
baseline -- `101/182 functions, 2122/7631 words, 27.8%` -- confirming no
regression anywhere else in the unit.

### Is the function byte-exact now? No -- and this is the register-permutation wall, reported not chased

Length-exact does not mean byte-exact. Diffing the now-549-word draft
against target (raw bytes, position-for-position) still shows real
differences, but they are cleanly separable from the length question:

- **The prologue (`base`/`posOld`/`posNew` construction, up to and including
  the first `bl getLineUnitNo`)** differs entirely in *scheduling order* from
  target -- target computes each field pair with both `fctiwz`s issued back
  to back before either store; my draft computes one field fully (load,
  round, store) before starting the next. Both shapes are 20/18/28
  instructions for the three field-pair blocks respectively (verified by
  counting), so this reorder costs nothing in LENGTH, only in byte-for-byte
  content within that span.
- **The switch/jump-table dispatch body (`bl line0_cross_chk` onward)**:
  after realigning draft and target by the confirmed +2-word offset, the
  TAIL LENGTHS MATCH EXACTLY -- 462 target instructions vs. 462 draft
  instructions, zero length discrepancy anywhere past the prologue. The
  remaining differences here are pure register numbers (`r30` vs. `r31` for
  `this`, i.e. a swapped register role) and one flipped branch polarity
  (`beq`/`b` vs. `bne`/`b` around each case's result test) repeated
  identically at every one of the ~30 case sites. This is EXACTLY the
  pattern `wip/fix_bighelper/RESULT.md` section 6 already tried and
  reverted: uniformly flipping all 27 single-condition cases' branch
  polarity regressed the function from 547 to 564 words, with `this` moving
  from a stable `r30` cache to `r28`. I did **not** repeat that experiment --
  it is already a documented dead end for this exact function, and
  AGENT_CONTEXT's own "if something reaches the right instruction count and
  differs ONLY in register numbers, STOP and report it" applies squarely
  here (100+ variants project-wide, zero successes on this class of
  residual).

**Reported, not chased further, per the brief's own instruction.** The
length question (this round's actual assignment) is closed; the
register/polarity residual is a pre-existing wall, unaffected by this
round's fix in either direction (present in the 547-word version too, at the
identical relative positions).

## 2. `__sinit_\d_line_mng_cpp` -- re-measured; residual re-characterised; NOT source-fixable from this TU

### Re-measurement, as instructed ("re-measure it now")

`__sinit` remains **length-exact at 1193/1193 words**, confirmed on this
much-changed unit (25 states declared, layout corrected, `fn_800C31C0`
authored and now length-exact, four prior declaration fixes). Raw byte diff:
**175 differing words out of 1193**, zero length discrepancy.

### The equal-blocks-constant-delta signature still holds -- but the constant has changed

The 175 differing words decompose EXACTLY into **25 blocks of 7 words each**
(one block per declared state: 8 + 24*47 = the 25 group-start offsets, every
one showing exactly 7 differing rows at the identical within-block
positions `[0,1,2,3,7,8,12]`). 25 * 7 = 175 -- exact accounting, nothing left
over. This confirms the brief's framing that a residual decomposing into
equal blocks with a constant delta is POSITIONAL, not content -- but the
actual delta measured today is **NOT** the previously-recorded "+0x14,
+0x10" alternating pair. It is a **single flat +0x40 (64 bytes), uniform
across all 25 blocks with zero exceptions**, confirmed instruction-by-
instruction (every `addi rN, r28, 0xNNNN` / `lwz rN, 0xNNNN(r28)` in the
draft reads exactly 0x40 higher than the corresponding target instruction, in
all 25 groups, no variation). The earlier reading is stale, exactly as the
brief warned it might be -- re-measurement was necessary and the constant
had in fact changed.

### Root cause, evidenced: this is link-time weak-vtable deduplication, not a declaration-order problem

The brief's assumed lever is declaration order of the static objects. I
looked for it and found something else instead, with hard evidence, and I am
reporting the contradiction rather than reconciling it into the brief's
framing.

`r28` is anchored to the SAME anonymous `.data` pool object used by
`fn_800C31C0`'s own jump table (`"@55792_80316CA0"` in target,
`...data.0`/`@7469` in the draft). Reading my own compiled object's full
`.data` layout (`bin\dtk-windows-x86_64.exe elf info wip/fix_bigtwo/_tally/d.o`)
shows this pool, after the four jump tables (`fn_800C31C0`'s own plus
`mov_to_rightupper`/`mov_to_leftupper`/`mov_to_leftlower`'s -- all three of
which are independently confirmed BYTE-EXACT matches against target, so the
jump-table span itself is not in question), contains **12 consecutive
template vtables** for the `s_State.hpp` state-machine framework
`mStateMgr` pulls in:

```
sFStateStateMgr_c<dLineMng_c,...>   0x40   -- templated on dLineMng_c
sStateStateMgr_c<dLineMng_c,...>    0x40   -- templated on dLineMng_c
sFStateMgr_c<dLineMng_c,...>        0x30   -- templated on dLineMng_c
sStateMgr_c<dLineMng_c,...>         0x30   -- templated on dLineMng_c
sFStateFct_c<dLineMng_c>            0x14   -- templated on dLineMng_c
sFState_c<dLineMng_c>               0x18   -- templated on dLineMng_c
sStateStateMgrIf_c                  0x40   -- PLAIN class, not templated
sStateMgrIf_c                       0x30   -- PLAIN class, not templated
sStateIDChk_c                       0x10   -- PLAIN class, not templated
sStateIDChkIf_c                     0x10   -- PLAIN class, not templated
sStateFctIf_c                       0x14   -- PLAIN class, not templated
sStateIf_c                          0x18   -- PLAIN class, not templated
```

The first six are templated on `dLineMng_c` (their mangled names carry
`<10dLineMng_c,...>`) -- genuinely unique to this TU, nobody else can
provide them. **The last six are plain, non-template interface classes,
shared by every class in the game that uses this state-machine framework**
(`dCourseSelectGuide_c`, `dWarningBattery_c`, and many others reference the
same `sFStateFct_c`/`sStateIf_c` family per `bin/dtk/wiimj2d_symbols.txt`).
Grepping the retail binary's own symbol table directly:

```
__vt__13sStateMgrIf_c    = .data:0x802FEDC8; // scope:weak
__vt__13sStateIDChk_c    = .data:0x802FEDF8; // scope:weak
__vt__15sStateIDChkIf_c  = .data:0x802FEE08; // scope:weak
__vt__13sStateFctIf_c    = .data:0x802FEE18; // scope:weak
__vt__10sStateIf_c       = .data:0x802FEE30; // scope:weak
```

**All five are physically hosted at `0x802FEDC8`-`0x802FEE48` -- nowhere
near `d_line_mng.cpp`'s own `.data` slice (`0x80316xxx`-`0x80317xxx`).**
`.data` begins at `0x802fe6a0` (`slices/wiimj2d.json` `meta.sections`), so
this address is very near the START of the whole binary's `.data`, i.e. some
other, much-earlier-linked TU is the canonical host for these five weak
vtables. `__vt__18sStateStateMgrIf_c` does not appear anywhere in the symbol
table by that name at all (grepped directly, zero hits) -- either it is
never independently needed anywhere in the retail binary, or dtk could not
attribute a name to wherever it actually lives.

This is precisely the mechanism this session's own memory note already
names: *"unreferenced weak symbols aren't placed; don't read an object's
size as a link overflow."* `d_line_mng.cpp` compiled **in isolation** (as
`tally.py` necessarily does -- there is no whole-program link happening)
has no way to know that five of its twelve local vtable copies are
redundant against a TU that isn't even present in this compile. My own
`.o`'s `.data` therefore contains all twelve, computed at a LOCAL offset of
`+0x3C8` to the state-struct table that follows them. The retail binary's
LINKER discarded the redundant weak copies during the real, whole-program
link and correspondingly reflows/repatches the immediate displacement values
in `.text` that reference anything past the discarded bytes -- which is
exactly why the SAME instructions in target read a flat `-0x40` relative to
my draft, uniformly, everywhere past that point.

**I could not pin down the PRECISE byte-for-byte accounting of which subset
of the six non-template vtables totals exactly 64 bytes** (my own manual
reading of the raw retail `.data` bytes around this region was imprecise
about exactly where real vtable content ends and zero-fill begins, and I do
not trust that manual reading enough to assert a specific vtable-by-vtable
attribution). What I DO have solid, direct evidence for:

1. The delta is exactly and uniformly 64 bytes, confirmed instruction by
   instruction across all 25 state blocks -- not in question.
2. At least five of the six candidate non-template vtables are confirmed,
   by direct symbol-table lookup, to be hosted by a TU other than
   `d_line_mng.cpp` in the real retail binary.
3. This is therefore a whole-program, LINK-TIME phenomenon. It cannot be
   observed, reproduced, or fixed by any source change to `d_line_mng.cpp`
   compiled in isolation, because the compiler (correctly, for an isolated
   TU) always emits its own local copy of every weakly-linked template
   object it references, regardless of what the eventual link will keep.

### This contradicts the brief's assumed lever -- flagging it as instructed

The brief frames this residual as fixable by reordering the declaration
order of the 25 static state objects. Based on the evidence above, I do not
believe that is the actual lever for this specific 64-byte delta: reordering
the 25 `STATE_FUNC_DECLARE` state declarations would not change which of the
SIX FRAMEWORK vtables (declared entirely inside the already-landed, shared
`s_State.hpp`, not in this TU's own header) the compiler decides to
instantiate, nor would it change which of them the real linker keeps. The
states themselves are not the object under discussion -- the vtables that
sit BEFORE them in the shared pool are.

**I did not attempt a source change here.** `s_State.hpp` is out of scope to
edit per the hard rules, and I have no lever inside `d_line_mng.cpp` itself
that would suppress local emission of a weak template vtable the compiler
has already decided (correctly, per C++ semantics) that this TU's own
`mStateMgr` odr-uses. My expectation, stated as a hypothesis for Claude the
integrator to confirm at real link time (never run by me, per the hard
rules): **this residual should resolve itself automatically once
`d_line_mng.cpp` is linked into the whole program**, exactly as it already
does for the retail binary, as long as whichever TU is the real canonical
host for these five-or-six weak vtables is present in the link (which it
must be, since the retail binary itself proves it exists somewhere). If it
does NOT resolve automatically at real link time, that would be a genuinely
new and important finding -- but I have no way to test that without running
the linker, which I am instructed never to do.

## 3. Files

- `wip/fix_bigtwo/d_line_mng.cpp` -- the draft. One functional change from
  `wip/line_mng_merge2/d_line_mng.cpp`: the two `volatile` casts in
  `fn_800C31C0`'s `base.x`/`base.y` construction (full diff below).
- `wip/fix_bigtwo/shadow_include/` -- unchanged copy of
  `wip/line_mng_merge2/shadow_include/`. No header changes proposed this
  round.
- `wip/fix_bigtwo/_tally/d.txt`, `.o` -- last compiled disassembly/object.
- `wip/fix_bigtwo/elfinfo.txt` -- `dtk elf info` dump of the compiled
  object, used for the `.data` layout analysis in section 2.
- `wip/fix_bigtwo/tally_final.txt` -- the verbatim tally output from section 0/headline.
- `wip/fix_bigtwo/*.py` -- disposable analysis scripts used to produce the
  measurements above (raw-byte diffing, group-delta extraction); not part of
  the deliverable, kept for reproducibility.

## 4. Mergeable diff

```diff
--- wip/line_mng_merge2/d_line_mng.cpp
+++ wip/fix_bigtwo/d_line_mng.cpp
@@ -753,9 +753,20 @@
 
     // Grid-snap mPos down to its UNIT_SIZE cell, then step back one more
     // cell so the 3x3 scan below is centred on the unit mPos sits in.
+    // Both fields are re-read here through a volatile lvalue rather than the
+    // (semantically identical) plain `self->mPos.x`/`self->mPos.y`. MEASURED:
+    // without this, -O4 CSEs these two reads against the already-loaded
+    // register values from posNew's construction above, landing 2 words
+    // (1 per field, confirmed independently) short of the target, which
+    // genuinely reloads both from memory a second time. Same "Gap A" MWCC
+    // idiom already documented for the executeState_* family in this unit
+    // (MERGE2.md section 6) -- there it was diagnostic-only because it
+    // traded the gap for a different exposed residual; here it closes the
+    // length gap outright with the pre-existing register-scheduling
+    // residual in the switch body unaffected either way. See RESULT.md.
     mVec2_c base;
-    base.x = dLineMng_c::smc_UNIT_SIZE_X * (f32)(int)(self->mPos.x / dLineMng_c::smc_UNIT_SIZE_X) - 16.0f;
-    base.y = dLineMng_c::smc_UNIT_SIZE_X * (f32)(int)(self->mPos.y / dLineMng_c::smc_UNIT_SIZE_X) - 16.0f;
+    base.x = dLineMng_c::smc_UNIT_SIZE_X * (f32)(int)(*(volatile f32 *)&self->mPos.x / dLineMng_c::smc_UNIT_SIZE_X) - 16.0f;
+    base.y = dLineMng_c::smc_UNIT_SIZE_X * (f32)(int)(*(volatile f32 *)&self->mPos.y / dLineMng_c::smc_UNIT_SIZE_X) - 16.0f;
 
     mVec2_c pos;
     mVec2_c corner;
```

## 5. Headline (unchanged from baseline, confirmed after the fix)

```
(1 paired by CONTENT -- unnamed target vs mangled draft name)
matched 101/182 functions   2122/7631 words = 27.8% BY BYTES
```

`fn_800C31C0` does not appear in this count because of the pre-existing
`tally.py` bare-address keying gap (section 1), not because it is unmatched
-- it is 549/549, length-exact, confirmed directly against
`wip/fix_bigtwo/_tally/d.txt`.

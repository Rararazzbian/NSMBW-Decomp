# `wip/fix_bighelper` — RESULT.md

Assignment: author `fn_800C31C0` (549 words, largest function in the unit),
confirm/reproduce `fn_800C15B0`'s real caller, then whatever else fit in the
budget from the secondary list. Starting point: `wip/line_mng_merge/d_line_mng.cpp`
(100/182, 27.7%), copied into this directory unmodified before any edits.
Nothing under `source/`, `include/`, `syms.txt`, `slices/*.json`,
`wip/line_mng_shared/`, `wip/line_mng_merge/`, or any other agent's directory
was touched. `wip/fix_states/` (the `executeState_*` family another agent is
working this round) was also left alone, **with one necessary exception**
explained in section 2 — I did not touch any of the other agent's functions,
only the one specific triple that turned out to be `fn_800C31C0`'s real
caller and was still the untouched base-skeleton stub.

## 1. Headline: `tally.py` output, verbatim

```
matched 100/182 functions   2115/7631 words = 27.7% BY BYTES

 1193w              LEN OK  __sinit_\d_line_mng_cpp
  549w             MISSING  fn_800C31C0
  128w              LEN OK  CalcAdjustPosY__10dLineMng_cFff
  121w  118w vs 121w  STRUCTURAL  line_cross_chk1__10dLineMng_cFffRC7mVec2_c7mVec2_c7mVec2_cR7mVec2_
  104w  105w vs 104w  STRUCTURAL  executeState_Right60Down__10dLineMng_cFv
  104w  106w vs 104w  STRUCTURAL  executeState_Right30Right__10dLineMng_cFv
  103w  105w vs 103w  STRUCTURAL  executeState_Right30Left__10dLineMng_cFv
  102w  103w vs 102w  STRUCTURAL  executeState_Left60Down__10dLineMng_cFv
  101w              LEN OK  executeState_Right60Up__10dLineMng_cFv
  101w              LEN OK  executeState_Left60Up__10dLineMng_cFv
  100w  99w vs 100w  STRUCTURAL  line_cross_chk2__10dLineMng_cFfRC7mVec2_c7mVec2_c7mVec2_cRf
  100w  101w vs 100w  STRUCTURAL  executeState_Left30Right__10dLineMng_cFv
   99w              LEN OK  executeState_Left30Left__10dLineMng_cFv
   94w              LEN OK  start_line_move__10dLineMng_cFv
   79w              LEN OK  move_on_circle2__10dLineMng_cFff
   78w              LEN OK  lineRHUL_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   78w              LEN OK  circle_ul2_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   77w              LEN OK  move_on_circle4__10dLineMng_cFff
   76w              LEN OK  lineRHUR_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   76w              LEN OK  circle_ur2_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   75w              LEN OK  move_on_circle_speedset__10dLineMng_cFff
   75w              LEN OK  lineRHLL_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   75w              LEN OK  circle_dl2_cross_chk__10dLineMng_cFRC7mVec2_c7mVec2_c7mVec2_c
   74w              LEN OK  move_on_circle3__10dLineMng_cFff
   73w              LEN OK  check_term__10dLineMng_cFv
```

**Still reads 100/182, 27.7% by bytes.** This is a real, measured limitation
of `tally.py`, not a null result — see section 3. `fn_800C31C0` and
`executeState_FallDown` (the function I had to author alongside it, see
section 2) both compile and are **length-exact or 1 word off**, but neither
counts in the headline because of a keying gap in the tool. I did not touch
`tally.py` (it lives in `wip/line_mng_shared/`, off-limits). Compiles clean,
zero warnings, verified via `harness.compile_draft` directly right before
writing this file.

## 2. `fn_800C15B0`: the brief's hypothesis is WRONG — reporting a negative result

**`fn_800C31C0` does NOT call `fn_800C15B0`.** I read `fn_800C31C0`'s full
549-word target body (`wip/fix_bighelper/target_800C31C0.txt`) instruction by
instruction and extracted every unique `bl` target
(`grep -oP '(?<=\bbl )\S+' | sort -u`): 23 distinct callees, all
`getLineUnitNo`/`lineN_cross_chk`/`circle_*2_cross_chk`/`lineRH**_cross_chk`.
None of them is `fn_800C15B0`. I checked this is not a scope-of-extraction
artifact by also grepping the *entire* `wip/line_mng_shared/target.txt`
(all 182 functions) for any reference to address `0x800C15B0`:

```
grep -n "15B0\|15b0" wip/line_mng_shared/target.txt
  442: bl move__10dLineMng_cFv          <- byte-OFFSET column, not an address, false hit
  549: .fn fn_800C15B0, global           <- the function's own definition
  557: .endfn fn_800C15B0
```

**Zero callers anywhere in this 182-function unit.** I also individually
checked the other three still-unauthored unnamed helpers
(`circle_nextpos_set`, `fn_800C3BA0`, `fn_800C3BF0`) by disassembling each in
full — none of them calls it either (`circle_nextpos_set` calls
`CosFIdx`/`SinFIdx`; `fn_800C3BA0`/`fn_800C3BF0` are pure `s16`-arithmetic
angle-wrap helpers with no `bl` at all).

**So `fn_800C15B0` remains correctly absent from the merge.** I left
`setArrElem_800C15B0` exactly as the coordinator's merge already had it
(declared, unreferenced, dead-stripped by `-O4` — confirmed: no
`fn_800C15B0`/`setArrElem` symbol anywhere in my compiled object either). I
did **not** add a fake caller to force emission; that would misrepresent the
unit. Its real caller is external to this TU (most likely the still-
undecompiled owning class MAPPING.md flagged — a background-control actor
holding a `dLineMng_c` member), consistent with `fn_800C15B0`'s `global`
ELF-binding tag (which the earlier merge round already showed is not
diagnostic of static-vs-external C++ linkage on its own — `fn_800C3B20`/
`fn_800C3B60`, both confirmed `static`, carry the same tag).

**What I found instead, since the brief's premise didn't hold**: I grepped
the *whole* target file for `fn_800C31C0`'s own address as a *callee* (who
calls the big function itself, not who it calls) and found exactly one hit:

```
/* 800C562C 000057B0  48 00 00 00 */  b fn_800C31C0
```

— inside `executeState_FallDown`, as a **tail call** (`b`, not `bl`).
`executeState_FallDown` was still the un-authored base-skeleton empty stub
(`void dLineMng_c::executeState_FallDown() {}`) — MERGE.md's headline result
had listed `initializeState_FallDown`/`finalizeState_FallDown` as correctly
empty in target, but did **not** claim `executeState_FallDown` was empty, and
it isn't: it does real gravity-integration work (15 words) before tail-
calling `fn_800C31C0`. Since this is the one function needed to give
`fn_800C31C0` a real caller — otherwise `fn_800C31C0` itself would be
dead-stripped exactly like `fn_800C15B0` is — I authored it. This is the one
`executeState_*` I touched; everything else in that family is
`wip/fix_states/`'s and I left it alone.

**Confirmed `fn_800C31C0` is genuinely called and genuinely emitted** once
`executeState_FallDown` exists: grepping my compiled object shows
`fn_800C31C0__FP10dLineMng_c, local` present at 547 words (see section 4).
Removing `executeState_FallDown`'s body (reverting it to `{}`) makes
`fn_800C31C0` vanish from the compiled object again — I checked this both
ways, it is not incidentally kept alive by something else.

**`fn_800C15B0` reported byte-exact once genuinely called?** No — because it
is *not* genuinely called by anything in this unit, full stop. This
directly contradicts the brief's framing ("confirm that from the target
disassembly... verify it returns byte-exact once genuinely called"). I could
not verify something that does not happen. Flagging this loudly, as
instructed: the brief was wrong on this point, with the grep evidence above
to check it.

## 3. Tooling gap, reconfirmed (not new — MERGE.md section 7 already flagged this class of issue for 3 other functions)

`fn_800C31C0` compiles under the mangled name `fn_800C31C0__FP10dLineMng_c`
(anonymous-namespace-free but still C++-mangled, since I declared it a plain
`static` C++ function, not `extern "C"`). The target's `.fn` label for it is
the bare `fn_800C31C0` (dtk has no symbol name for it in the retail binary).
`tally.py`'s `parse()` keys strictly by literal `.fn` label text, so
`k in d` never matches regardless of byte content — this is the exact
mechanism MERGE.md's section 7 already documented for `fn_800C1EE0`/
`fn_800C3B20`/`fn_800C3B60`. I hand-verified both affected functions instead
of trusting the tool's "MISSING" label (see section 4). This does not change
the headline 100/182 either way, same as MERGE.md found — but it means the
tool under-reports by 2 more real, present, near-length-exact functions this
round.

## 4. Length-before-count, every function I touched

Per AGENT_CONTEXT.md's rule, length reported before any differing-instruction
count, since a length mismatch invalidates a raw diff count.

| function | draft words | target words | length | notes |
|---|---:|---:|---|---|
| `fn_800C31C0` | 547 | 549 | **2 words short** | full switch/loop/jump-table structure reproduced; residual is a register-caching (`frsp`) + 2 redundant reload elisions the compiler makes that I could not reproduce from source — see section 6 |
| `executeState_FallDown` | 17 | 16 | **1 word over** | real content authored (was `{}`); residual is a single `frsp` on the clamped value, register-scheduling only |
| `check_term` | 73 | 73 | **exact length** | closed from `72 vs 73` (real content gap) to exact; real byte residual is a register/instruction-order permutation only (see section 6) |
| `start_line_move` | 94 | 94 | **exact length** | closed from `95 vs 94` for free, as a side effect of the `getLineUnitNo` static fix (see section 5) |
| `line_cross_chk1` | 118 | 121 | **3 words short** | closed from `117 vs 121` by fixing a real sign bug (see section 5); remaining 3 is a tail-merge/branch-consolidation scheduling difference |
| `line_cross_chk2` | 99 | 100 | **1 word short** | untouched behaviourally; read but not modified — see section 7 for why I stopped |
| `init_term_ck_pos` | 39 | 37 | **2 words over** | untouched net (three variants tried, all regressed or were neutral — see section 6); fixed the static-storage-duration/symbol-naming issue, which is real but not word-count-visible |

None of the seven is counted in `tally.py`'s matched set. `check_term` and
`start_line_move` are **length-exact but not byte-identical** — confirmed
register-permutation-only, not content, by mnemonic-frequency comparison
(every opcode count matches 1:1 between draft and target; only physical
register numbers and label numbering differ). Full evidence for `check_term`
in section 6.

## 5. Real bugs found and fixed (not scheduling — content)

1. **`getLineUnitNo` must be `static`, not a plain member.** The shared
   header declared `u32 getLineUnitNo(f32, f32);` (implicit `this`). THREE
   independent call sites — `check_term`, `start_line_move` (both
   already-authored, both stuck at `+1`/`-1` length before this fix), and my
   new `fn_800C31C0` — show **no `mr r3,<this>` anywhere before the `bl`** in
   the target disassembly, with `this` (r3-equivalent) demonstrably clobbered
   by unrelated computation immediately beforehand in all three. A
   non-static declaration forces the compiler to pass `this` regardless of
   whether the callee uses it, which the target never does here. Declaring
   it `static u32 getLineUnitNo(f32, f32);` removed the spurious `mr` at all
   three sites and closed `start_line_move` to exact length immediately, with
   zero cost anywhere else in the unit (call syntax at every existing site is
   unqualified `getLineUnitNo(x, y)`, valid for both static and non-static
   member calls, so nothing else needed to change).
   **Offset-perturbing: NO** — declaration-only change, no layout impact.

2. **14 `lineN_cross_chk` functions were declared `void`, must be `bool`.**
   `line0_cross_chk`, `line1_cross_chk`, `line3h_cross_chk`, `line3v_cross_chk`,
   `line4_cross_chk` .. `lineE_cross_chk` were pure declarations with **no
   caller anywhere in the merged TU before `fn_800C31C0`** — the coordinator's
   merge correctly flagged them as never-tested. Every one of `fn_800C31C0`'s
   30 switch cases does `cmpwi r3,0x0` immediately after the `bl` and branches
   on it; a `void` function leaves no defined value in r3 to test. This is
   exactly the "return type proof nobody could get because no caller existed"
   situation the brief predicted for `CalcAdjustPosY` — except it landed on
   this family instead, and there are 14 of them, which is a $bigger$ find
   than the one the brief anticipated. **Offset-perturbing: NO.**

3. **`line_cross_chk1`: `out.x >= 0.1f` should be `out.x >= -0.1f`.** Read the
   raw bytes at `.sdata2` for the constant target loads at that comparison
   (`"@55106_8042CB4C"` = `0xBDCCCCCD` = **-0.1f**, not `+0.1f` — confirmed
   the *other* constant at that same check, `"@55107_8042CB50"` =
   `0x3DCCCCCD` = `+0.1f`, used correctly for the `<= p2 + 0.1f` half). This
   is a real semantic bug in the pre-existing author_geom body, not
   mine — closed the function from 117→118/121 immediately with no other
   change. **Offset-perturbing: NO.**

## 6. Variants tried that did NOT help (so nobody repeats them)

**`fn_800C31C0`, register/branch-polarity attempt (reverted):** Converting
all 27 single-condition switch cases from `if (cond) goto found; break;` to
`if (!cond) break; goto found;` DID flip the branch polarity to match target
exactly on the 2 cases I spot-checked (`case 1`/`case 2` individually) — but
applying it to all 27 at once regressed the function from 547 to **564**
words (+17), with `this` moving from a stable `r30` cache to `r28` and
totally different `bne`/`beq` counts than either the before- or after-state.
Reverted in full. **Lesson: this specific branch-polarity lever (documented
generally in AGENT_CONTEXT lever #5) does not transfer safely inside a large
switch with 30 cases sharing one exit label — it interacts with whatever
register-pressure/tail-merging heuristic MWCC applies across the whole
switch, not just the one case being edited.** I did not have budget to
bisect exactly how many cases can safely take the transform before it flips
sign; flagging as unsolved rather than guessing further.

**`fn_800C31C0`, `mVec2_c base` local (kept, +3 words, this is why the
function is 547 not 544):** Original draft declared `f32 baseX, baseY;`
separately; target has them living in a genuinely distinct `mVec2_c`-shaped
stack slot pair with its own address, confirmed by target's extra
`stfs f1,0x128(r1)` / `stfs f0,0x12c(r1)` pair that has no counterpart in a
flat two-scalar draft. Switching to a single `mVec2_c base;` object closed
544→547. Kept.

**`fn_800C31C0`, redundant switch guard (kept, -1 word, part of the 544 base):**
Original draft wrapped the switch in an explicit `if (id <= 0x20) { switch... }`,
which is logically redundant with the switch's own jump-table bounds check —
confirmed by literally two consecutive identical `bgt` instructions in the
compiled output. Removing the manual guard (bare `switch(id)`) let the
compiler's own bounds check do the job alone, exactly matching target's
single `bgt`. Kept.

**`init_term_ck_pos`, `mVec2_c *end = mDirVec + N;` named end-pointer local
(reverted, catastrophic):** Tried this hoping to match a `blt`-based
comparison I'd seen in target's raw bytes. Extracting the loop bound into a
named variable made MWCC's optimizer conclude the trip count was no longer
provably small and constant, and it **8×-unrolled the 3-iteration zero-fill
loop into ~68 words of unrolled/remainder-handling code** (39 → 107 words).
Confirmed this is specifically about the *named local*, not the `!=`-vs-`<`
comparison operator: reverting to the inline `p < mDirVec + 3` form (keeping
`<`, dropping only the named local) *still* regressed to 45 words — so both
changes needed reverting together, back to the original `do {...} while
(p != mDirVec + N)` idiom, which is the one that reproduces the
non-unrolled 39-word shape. **This is a sharper, function-specific version of
the general "do-while(!=) avoids mtctr" lever already in AGENT_CONTEXT — here
the trigger for a much worse pathology (full unrolling, not just mtctr) was
extracting the loop bound into its own named variable, not the comparison
operator itself.** Recording this because the two earlier variants looked
individually plausible and each one, alone, would have been a very expensive
mistake to leave in without checking the compiled output.

**`init_term_ck_pos`, function-local static → file-scope static (kept, evidence-only, no word-count change):**
The un-fixed draft's guard bool and direction table were function-local
`static`s, which MWCC mangles as `@LOCAL@init_term_ck_pos__10dLineMng_cFv...`.
Target's actual symbols for the equivalent objects are bare `lbl_80359740`/
`lbl_8042A270` — the shape dtk uses for a file-scope object with no
recovered name, never for a function-local static in this codebase (compare
`fn_800C3B20`'s already-confirmed-`static`-and-correct file-scope helpers,
which also come through as bare `lbl_`/`fn_`-style names, never `@LOCAL@`).
Converted `static mVec2_POD_c d_dir[4]; static u8 s_init;` from
function-local to file-scope `static mVec2_POD_c s_dDir[4]; static u8
s_dDirInit;` right above the function. This fixed the symbol-shape mismatch
(confirmed: `s_dDir@ha`/`s_dDirInit@sda21` in my object, matching target's
`lbl_*` pattern instead of the wrong `@LOCAL@*` pattern) but did **not**
close the remaining 2-word gap — that residual is a `mr r6,r3` vs.
direct-r3-use register-allocation choice for the loop pointer, unrelated to
storage duration. Kept the file-scope change (it is strictly more correct
evidence-wise, even though word-count-neutral) but did not chase the
remaining 2 words further given the unrolling trap already cost one round of
this function's budget.

## 7. `line_cross_chk2` — read, not modified, and why I stopped there

99w vs target 100w, 1 word short. Diffing draft vs. target shows the
**frame size itself differs** (my draft: `stwu -0x30`, no `f30` save; target:
`stwu -0x40`, saves/restores `f30` in addition to `f31`) — meaning the
target genuinely holds a value live across a `bl` (`line_cross_slope_check`)
that my current body does not need to. I read enough of the target
(`wip/fix_bighelper` scratch, not saved as a separate file) to see the extra
saved register corresponds to caching `intercept` (or a related temporary)
across the `line_cross_range_check` call rather than reloading it from the
stack afterward — a real, second-order control-flow difference from
`line_cross_chk1`'s (already-fixed) shape, not just a scheduling permutation.
This would need a genuine re-derivation of the function's branch structure,
not a one-line fix like the `-0.1f` sign bug was, and it is squarely
author_geom's family, not my primary assignment. Flagging as open with the
concrete lead (extra `f30` save = extra cross-call live value) rather than
guessing at a fix I could not verify in the time this round had left.

## 8. Bodies, as a mergeable block

Everything below is already in `wip/fix_bighelper/d_line_mng.cpp`, in target
address order, ready to lift into the next merge pass:

- `fn_800C31C0` (static file-scope helper) — inserted immediately after
  `lineRHLR_cross_chk`, before the `fn_800C3B20`/`fn_800C3B60` section
  (matches target address order: `lineRHLR_cross_chk` → `fn_800C31C0` →
  `circle_nextpos_set` (still unauthored) → `fn_800C3B20`).
- `executeState_FallDown` — replaces the `{}` stub in place, same position
  in the `initializeState_FallDown`/`finalizeState_FallDown`/
  `executeState_FallDown` triple.
- `check_term` — body content unchanged from the merge except the
  `mVec2_c testPos` local and Y-before-X statement order (section 4/6).
- `line_cross_chk1` — one-line sign fix (`-0.1f`), in place.
- `init_term_ck_pos` — `d_dir`/`s_init` promoted to file-scope `s_dDir`/
  `s_dDirInit` statics declared immediately above the function; function
  body otherwise unchanged.

Header (`wip/fix_bighelper/shadow_include/game/bases/d_line_mng.hpp`),
diffed against the coordinator's `wip/line_mng_merge/shadow_include/...`
copy — full diff is 64 lines, `diff -u` output available in this directory's
history; the four semantic changes are:

```cpp
// 1. static, not member (proven, see section 5.1)
static u32 getLineUnitNo(f32, f32);

// 2. bool, not void, x14 (proven, see section 5.2)
bool line0_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool line1_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool line3h_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool line3v_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool line4_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool line5_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool line7_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool line8_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool line9_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool lineA_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool lineB_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool lineC_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool lineD_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);
bool lineE_cross_chk(const mVec2_c &, const mVec2_c &, const mVec2_c &);

// 3. new friend, needed by fn_800C31C0's direct self->mPos/self->mOldPos
//    reads (same evidence shape as the three friends already in the merge)
friend void fn_800C31C0(dLineMng_c *self);
```

Everything else in the header diff is comments only (documenting the above).
**No offset/layout change anywhere** — every edit is a declaration-only
type/linkage correction or a new `friend`, none of which touches
`sizeof(dLineMng_c)` or any member offset.

## 9. Files

- `wip/fix_bighelper/d_line_mng.cpp` — the draft, self-contained.
- `wip/fix_bighelper/shadow_include/game/bases/d_line_mng.hpp` — header with
  the 4 semantic changes in section 8, diffable against
  `wip/line_mng_merge/shadow_include/game/bases/d_line_mng.hpp`.
- `wip/fix_bighelper/target_800C31C0.txt` — `fn_800C31C0`'s full 549-word
  target body, extracted from `wip/line_mng_shared/target.txt` for reference.
- `wip/fix_bighelper/_tally/d.txt`, `.o` — last compiled disassembly/object,
  regenerate with `python wip/line_mng_shared/tally.py
  wip/fix_bighelper/d_line_mng.cpp wip/fix_bighelper/shadow_include`.

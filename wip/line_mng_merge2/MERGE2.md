# `d_line_mng.cpp` merge2 report

Folded `wip/fix_bighelper` and `wip/fix_states` (both independently derived
from `wip/line_mng_merge`'s 100/182 baseline) into one TU:
`wip/line_mng_merge2/d_line_mng.cpp`, against a MERGE2-LOCAL header copy
`wip/line_mng_merge2/shadow_include/game/bases/d_line_mng.hpp` (the shared
header plus the `friend`/`bool`x14/`smc_UNIT_SIZE_X` additions, taken
wholesale from `wip/fix_bighelper`'s local header -- see section 2). Applied
the `getLineUnitNo` `static` fix (already present in
`wip/line_mng_shared/shadow_include/...` at the time this round started --
`wip/line_mng_merge`'s own header copy is byte-identical to the shared one,
confirmed by `diff`, so the baseline this round started from already had it).
Nothing under `source/`, `include/`, `syms.txt`, `slices/*.json`,
`wip/line_mng_shared/`, `wip/fix_bighelper/`, `wip/fix_states/`, or
`wip/line_mng_merge/` was written.

## 1. Headline: `tally.py` output, verbatim

```
(1 paired by CONTENT -- unnamed target vs mangled draft name)
matched 101/182 functions   2122/7631 words = 27.8% BY BYTES

 1193w              LEN OK  __sinit_\d_line_mng_cpp
  549w             MISSING  fn_800C31C0
  128w              LEN OK  CalcAdjustPosY__10dLineMng_cFff
  121w  118w vs 121w  STRUCTURAL  line_cross_chk1__10dLineMng_cFffRC7mVec2_c7mVec2_c7mVec2_cR7mVec2_
  104w  103w vs 104w  STRUCTURAL  executeState_Right60Down__10dLineMng_cFv
  104w  103w vs 104w  STRUCTURAL  executeState_Right30Right__10dLineMng_cFv
  103w  101w vs 103w  STRUCTURAL  executeState_Right30Left__10dLineMng_cFv
  102w  101w vs 102w  STRUCTURAL  executeState_Left60Down__10dLineMng_cFv
  101w   99w vs 101w  STRUCTURAL  executeState_Right60Up__10dLineMng_cFv
  101w   99w vs 101w  STRUCTURAL  executeState_Left60Up__10dLineMng_cFv
  100w   99w vs 100w  STRUCTURAL  line_cross_chk2__10dLineMng_cFfRC7mVec2_c7mVec2_c7mVec2_cRf
  100w   99w vs 100w  STRUCTURAL  executeState_Left30Right__10dLineMng_cFv
   99w   97w vs 99w  STRUCTURAL  executeState_Left30Left__10dLineMng_cFv
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

**101/182 functions matched, 2122/7631 words = 27.8% by bytes.** Up from the
prior merge's 100/182, 27.7%. Compiles clean, zero warnings (verified via
`harness.compile_draft` directly, not inferred from `tally.py`'s exit alone).
The one gain is `fn_800C15B0`, closed via the non-static-linkage experiment
in section 4, paired by `tally.py`'s content-based fallback (its target
label is a bare address, so name-keying can never match it -- see MERGE.md
section 7 for the same tooling gap on three other functions, unaffected by
this round).

## 2. Collisions

**None**, same finding as the first merge. `wip/fix_bighelper` and
`wip/fix_states` both derived independently from `wip/line_mng_merge`'s
100/182 baseline and touched **disjoint** function sets:

- `fix_bighelper`: `fn_800C31C0` (new), `executeState_FallDown` (new
  content, was `{}`), `check_term` (rewritten body), `start_line_move`
  (unchanged -- already matched baseline), `line_cross_chk1` (one-line sign
  fix), `init_term_ck_pos` (file-scope statics).
- `fix_states`: the 8 `executeState_*` functions
  (`Right30Right`/`Right60Down`/`Right30Left`/`Left60Down`/`Left30Right`/
  `Right60Up`/`Left60Up`/`Left30Left`).

No function appears in both sets. The two agents' header proposals also
agree completely where they overlap (both assume `getLineUnitNo` static,
both correctly inherited the header state from their shared starting point)
-- `diff`ing `wip/fix_states/shadow_include/.../d_line_mng.hpp` against
`wip/line_mng_merge/shadow_include/.../d_line_mng.hpp` (its own starting
point) shows **zero differences**; `fix_states` proposed no header changes
at all. `fix_bighelper`'s header additions (the four items in section 8 of
its RESULT.md: `friend fn_800C31C0`, `static u32 getLineUnitNo` [already
present], 14x `bool line*_cross_chk`, `friend`s carried over from the first
merge) are exactly what I applied wholesale as the merge2-local header (see
below) -- nothing to reconcile.

**One thing worth flagging as a near-collision, resolved by inspection, not
guesswork**: I initially assumed (wrongly) that `wip/line_mng_merge`'s
baseline bodies for `executeState_Right30Left`/`executeState_Right30Right`
already matched `fix_states`' improved shape, because both used a
`mVec2_c newBase(...)` local somewhere in the function. On closer
comparison they did **not** -- the baseline called `getLineUnitNo` with a
bare `mUnitBasePos.x + 16.0f` expression and only constructed `newBase`
*after* the call, inside each branch (recomputing the offset a second time);
`fix_states`' shape constructs `newBase` *once*, *before* the call, and
reuses it in every branch. I caught this by diffing the two bodies
side-by-side rather than trusting the visual similarity, and replaced both
functions with `fix_states`' actual shape (see the `d_line_mng.cpp` diff for
these two). This mattered: with the stale baseline shape, these two
functions measured 2 words **over** target instead of the expected 1-2
words *under* that every other one of the 8 shows -- i.e. it would have
silently reported the wrong story about which family-wide gap remains open
had I not caught it.

## 3. The header, applied wholesale from `wip/fix_bighelper`

Copied `wip/fix_bighelper/shadow_include/game/bases/d_line_mng.hpp` in as
`wip/line_mng_merge2/shadow_include/game/bases/d_line_mng.hpp` unmodified.
It already contains everything both contributing agents needed:

1. `static u32 getLineUnitNo(f32, f32);` -- matches
   `wip/line_mng_shared/shadow_include/...` exactly (confirmed by `diff`,
   zero output on this declaration).
2. 14x `bool line{0,1,3h,3v,4,5,7,8,9,A,B,C,D,E}_cross_chk(...)` (was
   `void`) -- see section 5 for the compiled proof this is load-bearing.
3. `friend void fn_800C3B20/fn_800C3B60(dLineMng_c *self);`,
   `friend bool fn_800C1EE0(...)` -- carried over from the first merge,
   unchanged.
4. `friend void fn_800C31C0(dLineMng_c *self);` -- new, needed because
   `fn_800C31C0` reads `self->mPos`/`self->mOldPos` directly.
5. `static const float smc_UNIT_SIZE_X;` -- unchanged from the first merge,
   deliberately left undefined in this TU (see MERGE.md section 3.1 for why
   -- giving it a value here lets `-O4` fold two `fdivs` away that target
   still emits).

No `sizeof(dLineMng_c)` or member-offset change anywhere in this header --
every addition is a `friend` declaration, a `static` member declared-only,
or a return-type correction. **Offset-perturbing: NO** for the whole header.

## 4. `fn_800C15B0`: linkage result, MEASURED

**The non-static hypothesis is correct.** Declaring the free function
without `static` --

```cpp
void setArrElem_800C15B0(mVec2_c *arr, const mVec2_c *src, int idx)
{
    arr[idx] = *src;
}
```

-- with **no forced caller anywhere**, no dummy wrapper, same 3-line body as
before -- is sufficient on its own. Measured: `tally.py` goes from
`549w MISSING fn_800C31C0` plus a silent absence of `fn_800C15B0` entirely,
to `(1 paired by CONTENT...)` and 101/182 matched, 2122/7631 words. Isolated
check before folding into the merge: compiling just this change against
`wip/line_mng_merge2`'s baseline moved the count from 100/182 (2115w) to
101/182 (2122w) -- exactly `+7` words, `fn_800C15B0`'s own target length,
and nothing else changed (confirmed by diffing the two `tally.py` outputs
line for line before merging the change into the working file).

This closes the open question from the brief. The mechanism, MEASURED not
merely asserted:

- `wip/line_mng_shared/target.txt` line 549 reads `.fn fn_800C15B0, global`
  -- the target's OWN disassembly listing tags this function `global`,
  sitting between `acm_angle`'s `global` entry (line 537) and
  `start_line_move`'s (line 559). This is direct, not inferred from ELF
  section layout the way `fix_bighelper`'s "same tag as `fn_800C3B20`" note
  was (that comparison turned out to be inconclusive, since `fn_800C3B20`
  is `static` yet still carried `global` binding).
- Confirmed independently: searching the entire DOL for `0x800C15B0` as a
  big-endian absolute word finds zero hits, and `fix_bighelper` enumerated
  all 23 of `fn_800C31C0`'s callees and none is this function. So there is
  genuinely no caller anywhere in this 182-function unit.
- Both facts are simultaneously true because branches in a DOL are
  PC-relative: an absolute-word search can never find a `bl` from another
  translation unit, and the function's real caller is external (a
  still-un-decompiled TU), consistent with "global linkage, no local
  caller."
- Declaring it non-`static` reproduces this exactly: MWCC emits an
  external-linkage function regardless of whether the local TU calls it
  (matches `-O4`'s standard "unreferenced weak/local symbols are stripped,
  but a referenced-or-external one is always placed" behaviour already
  documented in AGENT_CONTEXT.md's weak-symbol section, generalised here to
  plain global functions).

**General rule for the project, worth recording**: for an unreferenced
helper with no in-TU caller, the presence or absence of a `global` tag in
the target's own `.fn` listing settles static-vs-external linkage directly
and cheaply, without needing a caller to exist. Check that tag before
assuming "no caller found" means "must be `static`."

I did **not** need to change anything about the function's position in the
file -- it was already correctly placed (in target address order) between
`acm_angle` and `start_line_move` from the first merge.

## 5. The 14 `bool` changes: MEASURED to alter generated code, and load-bearing for compilation itself

Not just "changed the byte count" -- reverting all 14 back to `void` in a
throwaway header copy (nothing else touched) makes the **whole file fail to
compile**:

```
### mwcceppc.exe Compiler:
#    770:                   if (self->line0_cross_chk(pos, posOld, posNew)) goto found;
#   Error:                                                                 ^
#   (10376) illegal operand 'void'
### mwcceppc.exe Compiler:
#    773:                   if (self->line1_cross_chk(pos, posOld, posNew)) goto found;
#   Error:                                                                 ^
#   (10376) illegal operand 'void'
[... same error for every one of the 14, all inside fn_800C31C0's switch]
```

This is stronger evidence than `fix_bighelper`'s original finding (which
only had "no caller exists yet to test the return type against"): now that
`fn_800C31C0` is authored and genuinely calls all 14 through `if (...)`
conditions, `void` is not merely wrong, it is **uncompilable** given the
target's own dispatch shape (every one of `fn_800C31C0`'s 30 switch cases
tests the callee's result). Confirms the brief's framing that these 14 were
"a bigger find than `CalcAdjustPosY`'s situation" -- once a real caller
exists, the return type is not just provable, it becomes mandatory for the
source to compile as target's control flow demands.

## 6. The eight `executeState_*` one-word gaps: NOT closed, both components isolated and measured

**Starting point applied**: `fix_states`'s `mVec2_c`-typed-local shape
(construct `mVec2_c newBase(...)` once, before the call, reuse its `.x`/
`.y` in every branch), for all 8 functions -- see section 2 for the two
(`Right30Left`/`Right30Right`) that needed correcting from a stale baseline
shape to actually match this pattern.

**Result after applying it, with the now-already-static `getLineUnitNo`
(the merge2 baseline never had the "spurious `mr r3`" masking bug the brief
warned about -- see intro, the header this round started from already had
`getLineUnitNo` `static`)**: all 8 are **1-2 words SHORT** of target, a
clean, honest gap with no cancellation involved:

| function | target | draft (this round) | delta |
|---|---:|---:|---:|
| executeState_Right60Down  | 104 | 103 | -1 |
| executeState_Right30Right | 104 | 103 | -1 |
| executeState_Right30Left  | 103 | 101 | -2 |
| executeState_Left60Down   | 102 | 101 | -1 |
| executeState_Right60Up    | 101 |  99 | -2 |
| executeState_Left60Up     | 101 |  99 | -2 |
| executeState_Left30Right  | 100 |  99 | -1 |
| executeState_Left30Left   |  99 |  97 | -2 |

None matched. Instruction-level diff of `executeState_Left30Left` (99w
target, 97w draft after the shape, 59 differing rows before any further
work) isolates the shortfall to exactly what `fix_states`' RESULT.md called
"Gap A": target reloads `mUnitBasePos.x` a **second time** from `0x50(r30)`
and **recomputes** `mUnitBasePos.x + 16.0f` a second time when constructing
`newBase`, even though the identical value is already live in a register
from evaluating the `else if (mPos.x >= mUnitBasePos.x + 16.0f)` condition
one basic block earlier. My natural compile reuses the live register
(cheaper, and correct-looking source), which is exactly one `lfs`/`fadds`
pair (1 word for the register-swap variant, but for `newBase.y` too on some
functions, hence some are -2) short of target.

**Confirmed Gap B (the spurious `mr r3, r30` this-argument setup) is
already gone in this baseline** -- there is no `mr r3, <this>` anywhere
before any of the 8 functions' `bl getLineUnitNo` in the compiled draft,
matching all 19 target call sites project-wide. So unlike `fix_states`'
original round (which measured against a header where `getLineUnitNo` was
still non-`static`), there is **no cancellation left to strip away here** --
what's shown above is the raw, uncancelled Gap A, exactly as the brief
predicted it would look once Gap B was gone.

**Shape tried to close Gap A**: forcing a fresh reload with
`*(volatile f32 *)&mUnitBasePos.x + 16.0f` in place of the bare
`mUnitBasePos.x + 16.0f` inside `newBase`'s constructor call, tested on
`executeState_Left30Left` in an isolated scratch copy (not shipped).
**Result: closes Gap A completely** (draft goes to 99w, length-exact) but
**exposes a second, independent, pre-existing residual**: a register swap
(`f0`/`f1`) for `mBaseSpeed` and the `0.8910065f` constant at the very top
of the function (`mSpeed.x = mBaseSpeed * 0.8910065f;`), present **whether
or not** the `volatile` forcing is applied -- confirmed by diffing the raw
(non-`volatile`) 97-word draft against target: the same `f0`/`f1` swap
already appears there at the corresponding instructions (rows 2, 10, 33,
34 of that diff), so it predates and is independent of Gap A. **Tried
swapping the multiplication operand order** (`0.8910065f * mBaseSpeed`
instead of `mBaseSpeed * 0.8910065f`) hoping to influence register
allocation -- **zero effect**, byte-for-byte identical diff before and
after. This matches AGENT_CONTEXT.md's documented "pure register-permutation
residual... not source-addressable" pattern precisely, and I did not find a
source-level lever for it in the time this round had.

**Net result: `volatile` forcing does not actually flip any of the 8 into
the matched set** (it trades a definite, understood 1-word-short gap for an
equally-sized but *different*, apparently-unrelated residual), and it is a
forcing hack, not real source -- per AGENT_CONTEXT's "a guess must be
labelled a guess" and "report a negative result rather than manufacturing a
positive one," I did **not** ship it. **The natural `mVec2_c`-typed-local
shape (no `volatile`) is what's in the merge**, since it's strictly better
understood (single isolated named gap, not two overlapping ones) and no
worse in matched-function count either way (0 either way, this round).

**Every shape tried, for the record**:
1. `fix_states`'s `mVec2_c newBase(...)` shape, as-is -- shipped. -1/-2 words
   each, isolated Gap A only.
2. `*(volatile f32 *)&mUnitBasePos.x + 16.0f` forcing -- closes Gap A,
   exposes an unrelated register-swap residual of the same size. Not
   shipped (diagnostic only, matches `fix_states`' own labelling of this
   exact trick).
3. `0.8910065f * mBaseSpeed` operand-order swap, tried against variant 2 --
   zero effect on the exposed register-swap residual. Not shipped (no
   benefit).

**Open for whoever picks this up next**: the register-swap residual in
variant 2 needs its own investigation, independent of Gap A/`newBase`
entirely -- it is present in the *unmodified* baseline body too, just
currently invisible under the larger, well-understood Gap A gap. It
affects at minimum `mSpeed.x = mBaseSpeed * 0.8910065f`, and by
similarity of source shape probably several sibling `executeState_*`
functions using the same idiom (`0.8910065f`/`mBaseSpeed` products)
beyond just the 8 in this round's scope.

## 7. Regression check vs both contributing agents' reported results

Compared the merge2's matched set (101 functions) against the merge's
prior matched set (100 functions), by exact-match set difference:

```
old (wip/line_mng_merge) matched: 100
new (wip/line_mng_merge2) matched: 101
lost (regression): {}  <- empty set, ZERO regressions
gained: {'fn_800C15B0'}
```

**Zero regressions.** Every one of the first merge's 100 matched functions
is still matched in merge2.

Checked both contributing agents' own claimed-byte-exact functions
separately, since neither reported any NEW byte-exact function beyond the
first merge's 100 (both explicitly say so in their own RESULT.md's --
`fix_bighelper` section 4's table shows every touched function 1-3 words
off; `fix_states` section 2 explicitly states "None of the 8 are
byte-exact"). So there was nothing at risk of regressing from either
agent's own claims -- this matches what the set-difference check above
independently confirms.

Hand-verified the four bare-address (`fn_800C1EE0`/`fn_800C3B20`/
`fn_800C3B60`/`fn_800C31C0`) functions' lengths reproduce `fix_bighelper`'s
own numbers exactly (same tooling gap as MERGE.md section 7 -- bare-address
target labels never key-match a mangled draft name, so none of these ever
counted in any headline, old or new):

| target symbol | target words | draft words (this round) | fix_bighelper's own number | match? |
|---|---:|---:|---:|---|
| `fn_800C1EE0` | 36 | 40 | 40 | reproduced exactly |
| `fn_800C3B20` | 15 | 15 (len-exact, not byte) | 15 | reproduced exactly |
| `fn_800C3B60` | 15 | 16 | 16 | reproduced exactly |
| `fn_800C31C0` | 549 | 547 | 547 | reproduced exactly |

## 8. Function definition order

Verified: sorting the compiled draft's named functions by their position in
`wip/line_mng_shared/target.txt`'s `.fn` order finds **6** out-of-order
pairs, all among compiler-synthesised, weak-linkage destructors and
`sFState*_c<dLineMng_c>` template methods (`__dt__...sFStateStateMgr_c...`,
`build`/`dispose`/`initialize`/`execute`/`finalize` on
`sFState_c<dLineMng_c>`/`sFStateFct_c<dLineMng_c>`) -- none of these are
functions this TU defines; they are implicitly instantiated from the
already-landed template headers, and their relative emission order is
fixed by the C++ ABI's base-class construction/destruction ordering and
MWCC's own template-instantiation sequencing, not by anything in
`d_line_mng.cpp`'s text. **Confirmed pre-existing and unaffected by this
round**: running the identical check against `wip/line_mng_merge`'s own
compiled output (before any of this round's edits) finds the **exact same 6
violations**, byte-for-byte identical indices. No new order problem was
introduced, and none of my edits (new/moved hand-written function
definitions) touch these weak template members at all -- `fn_800C31C0` and
the 8 `executeState_*` edits are all placed at their correct target-address
position (verified directly against `target.txt`'s address-ordered `.fn`
sequence for every hand-written function touched this round).

## 9. Files

- `wip/line_mng_merge2/d_line_mng.cpp` -- the merged unit.
- `wip/line_mng_merge2/shadow_include/game/bases/d_line_mng.hpp` -- header,
  copied wholesale from `wip/fix_bighelper`'s local header (section 3);
  diffable against `wip/line_mng_shared/shadow_include/.../d_line_mng.hpp`.
- `wip/line_mng_merge2/_tally/` -- last compiled disassembly/object/tally
  output, regenerate with `python wip/line_mng_shared/tally.py
  wip/line_mng_merge2/d_line_mng.cpp wip/line_mng_merge2/shadow_include`.

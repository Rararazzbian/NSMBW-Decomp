# `d_line_mng.cpp` merge report

Merged `wip/author_mov`, `wip/author_geom`, `wip/author_states`, `wip/author_core`
into one TU: `wip/line_mng_merge/d_line_mng.cpp`, against a MERGE-LOCAL header
copy `wip/line_mng_merge/shadow_include/game/bases/d_line_mng.hpp` (the shared
header plus three additions not yet folded in -- see section 3). Nothing under
`source/`, `include/`, `syms.txt`, `slices/*.json`, `wip/line_mng_shared/`, or
any `wip/author_*/` directory was written.

## 1. Headline: `tally.py` output, verbatim

```
matched 100/182 functions   2115/7631 words = 27.7% BY BYTES

 1193w              LEN OK  __sinit_\d_line_mng_cpp
  549w             MISSING  fn_800C31C0
  128w              LEN OK  CalcAdjustPosY__10dLineMng_cFff
  121w  117w vs 121w  STRUCTURAL  line_cross_chk1__10dLineMng_cFffRC7mVec2_c7mVec2_c7mVec2_cR7mVec2_
  104w  106w vs 104w  STRUCTURAL  executeState_Right60Down__10dLineMng_cFv
  104w  107w vs 104w  STRUCTURAL  executeState_Right30Right__10dLineMng_cFv
  103w  106w vs 103w  STRUCTURAL  executeState_Right30Left__10dLineMng_cFv
  102w  104w vs 102w  STRUCTURAL  executeState_Left60Down__10dLineMng_cFv
  101w  102w vs 101w  STRUCTURAL  executeState_Right60Up__10dLineMng_cFv
  101w  102w vs 101w  STRUCTURAL  executeState_Left60Up__10dLineMng_cFv
  100w  99w vs 100w  STRUCTURAL  line_cross_chk2__10dLineMng_cFfRC7mVec2_c7mVec2_c7mVec2_cRf
  100w  102w vs 100w  STRUCTURAL  executeState_Left30Right__10dLineMng_cFv
   99w  100w vs 99w  STRUCTURAL  executeState_Left30Left__10dLineMng_cFv
   94w  95w vs 94w  STRUCTURAL  start_line_move__10dLineMng_cFv
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
   73w  72w vs 73w  STRUCTURAL  check_term__10dLineMng_cFv
```

**100/182 functions matched, 2115/7631 words = 27.7% by bytes.** Compiles
clean, no warnings. (`tally.py` truncates its per-function list to the 25
largest non-matches; the full 82-entry non-match list -- 26 not compiled, 56
present-but-differing -- was generated for this report with a throwaway
script built on `tally.parse`/`tally.matched`, same gate, no shortcuts.)

## 2. Collisions

**None.** Scope was cleanly disjoint (mov=8, geom=16, states=60, core=17+3
unnamed) and no two authors wrote conflicting bodies for the same target
function. Two things could be mistaken for collisions and are worth recording
as *complementary*, not conflicting:

- **`fn_800C3B20`/`fn_800C3B60`**: author_states declared them as unresolved
  `static void fn_800C3B20(dLineMng_c*);` externals (no body, correctly
  assumed signature) and called them from 9 `initializeState_*` bodies;
  author_core actually authored the bodies (as `clampPosX_800C3B20`/
  `clampPosY_800C3B60`). I renamed core's definitions to `fn_800C3B20`/
  `fn_800C3B60` (matching states' call sites) and made them `static` for the
  merge -- same bodies, same evidence, just reconciled naming. No signature
  disagreement existed; states' assumed signature was exactly right.
- **`fn_800C1EE0`**: author_core listed it as "left open, depends on
  `line_cross_chk1`'s real signature, owned by another agent's family" and
  did not author it. author_geom *did* author it (needed it to compile/test
  `width_cross_chk`). I used geom's body as-is; no conflict, core correctly
  deferred to the family owner.

Every trivial `void {}` state stub that appears in more than one author's
file (all four started from the same base skeleton, so all four contain all
25 states' declarations) is byte-identical dead duplication of the base file,
not a content collision.

## 3. Local-header overrides not already in the shared header

I diffed all four `local_shadow*`/`shadow_include` copies against
`wip/line_mng_shared/shadow_include/game/bases/d_line_mng.hpp` (the version
you supplied, already carrying the `mov_to_*`/`is_unit_circle*`/
`getLineUnitNo`/`check_term`/`CalcAdjustPosY`/`mUnk6a`/the 16 cross-check
`static`+`bool` fixes). Every override in `author_mov`, `author_geom`, and
`author_states`'s local copies is **already present** in your shared header
-- those three diffs are stale snapshots from before your update, and in a
couple of spots (author_mov's/author_states'/author_geom's still-`void`
`CalcAdjustPosY`, author_geom's still-"padding" comment for `0x6a`) they are
now *weaker* than the shared header, not proposing anything new. **Only
`author_core`'s local header has genuinely new content**, all three items
below still needed for my merge to compile and now duplicated into my own
`wip/line_mng_merge/shadow_include/.../d_line_mng.hpp` (MERGE-LOCAL, not
applied to your shared copy):

1. **`static const float smc_UNIT_SIZE_X;`** (public class member, declared
   only, no in-TU definition). Needed by `init()`, `check_term()`,
   `start_line_move()`. Evidence (author_core): value `16.0f`, read directly
   from `original/wiimj2d.dol` at `.sdata2:0x8042CB18` (raw bytes
   `41 80 00 00`). Must stay **undefined in this TU** -- giving it an
   initializer here lets `-O4` constant-fold `init()`'s `pos.x /
   smc_UNIT_SIZE_X` into a multiply-by-reciprocal (measured: zero `fdivs`
   emitted vs the target's two), proving its real defining TU is elsewhere,
   not yet decompiled.

2. **`friend void fn_800C3B20(dLineMng_c *self); friend void
   fn_800C3B60(dLineMng_c *self);`** Needed because these two free functions
   write `mPos`/`mUnitBasePos` directly through the passed pointer (no
   accessor calls in their disassembly), and both are private. Evidence
   (author_core): `stb r30, 0x6a(r29)`-style direct member writes through the
   argument register, not `r1` (not a stack spill).

3. **`friend bool fn_800C1EE0(dLineMng_c *, f32, f32, const mVec2_c &, const
   mVec2_c &, const mVec2_c &, const mVec2_c &);`** Same reasoning, from
   author_geom: `width_cross_chk` passes its own incoming `this` straight
   through unmodified, and the target's disassembly shows **no mangled name
   at all** for the callee -- unlike every other `static` member in this
   unit, which does carry one -- so it cannot be a class member as currently
   declared, only a free function with `friend` access.

**Open question for you to rule on (author_geom flagged this, not
resolved)**: should `fn_800C1EE0` (and by the same logic `fn_800C3B20`/
`fn_800C3B60`) be a `friend` free function as tested here, or a private
`static` member instead? Both compile; I picked `friend` free functions
because that is what both authors actually tested and measured against.

## 4. Helper-signature disagreements

Checked every one AGENT_CONTEXT.md flagged as a likely break point, plus
everything each RESULT.md listed under "assumed helper signatures":

- **`fn_800C3B20`/`fn_800C3B60`**: both authors assumed `static void
  (dLineMng_c*)`, taking only `this`. Consistent -- no disagreement, and
  core's actual bodies match that shape exactly.
- **`mov_to_*` return values**: proven `bool`, consumed by `mov_frm_*`
  callers (`cmpwi r3,0x0`/`bne` after the `bl`) and by author_core's
  `move_on_circle1..4` (`mov_frm_rightlower(dst, true)` etc. results are
  *not* read there, but the four `mov_to_*` themselves are not called from
  `move_on_circle*` -- only `mov_frm_*` are). Already folded into the shared
  header; no disagreement.
- **`mov_frm_*` return values**: author_mov's RESULT.md flagged "I never
  consume a return value... but the coordinator's mov_to_* comment mentioned
  mov_frm_* callers ALSO read r3 somewhere, which is NOT true anywhere in my
  scope" -- I checked every call site of `mov_frm_rightupper/leftlower/
  rightlower/leftupper` across all four merged files (in `mov_to_*` itself:
  none call `mov_frm_*`; in `move_on_circle1..4`: results discarded; in the
  `executeState_*` family: results discarded, every call is immediately
  followed by an unconditional fall-through to the epilogue). **No call site
  anywhere in the merged unit reads `mov_frm_*`'s return value.** The shared
  header's `void` declaration is unchallenged by anything in this merge; the
  earlier note about a reader was a false alarm (or refers to a caller
  outside this TU that I cannot see).
- **`getLineUnitNo`**: `u32`, confirmed independently by author_mov, author_
  states, and author_core, all three by the same "no `clrlwi` mask before a
  `cmplwi`/`cmpwi` on the raw return register" test. Consistent.
- **`is_unit_circle2x2`/`is_unit_circle4x4`**: `static bool`, confirmed by
  author_mov via call-site register count, unchallenged elsewhere. Consistent.
- **`check_term`**: `bool`, found independently by author_states and author_
  core from the same "`li r3,0x1`/`li r3,0x0` before both epilogues" pattern.
  Consistent, already in the shared header.
- **`CalcAdjustPosY`**: `f32`, author_core's finding (no in-TU caller exists,
  so this is the weakest-evidence type in the unit, exactly as the shared
  header's doc comment already flags). No other author touches it.
- **`line_cross_*`/`*_cross_chk` family (16 fns)**: `static bool` for the
  five primitives, `bool` for the rest -- author_geom's only author, already
  in the shared header, nothing to cross-check against another author.

**No disagreement changed any generated code in the merge.** The one thing
that *did* change generated code was not a signature disagreement but
definition placement -- see next section.

## 5. `__sinit` finding

Measured (via `tally.parse`) the `__sinit_\d_line_mng_cpp` word count in each
of the four authors' **own isolated compiles**, plus the merge and target:

| source | words | first 5 instructions |
|---|---:|---|
| target | 1193 | `stwu -0x3b0` / `mflr` / `stw r0,0x3b4` / `addi r11,r1,0x3b0` / **`bl _savegpr_27`** |
| author_mov (isolated) | 1193 | identical to target, byte-for-byte on this prefix |
| author_states (isolated) | **1220** | `stwu -0x3a0` / `mflr` / `lis r4,"@..."@ha` / ... (inline `stw`s, no `bl`) |
| author_core (isolated) | **1220** | `stwu -0x3a0` / `mflr` / `lis r4,"@..."@ha` / ... (inline `stw`s, no `bl`) |
| **merge (all four)** | **1193** | identical to target |

**The merge already reproduces the target's 1193 words.** The actual
difference is not a word-count-vs-structural distinction and not about
*content* being added or removed from `__sinit` itself (nothing in
`__sinit`'s own body differs across drafts -- it always registers the same
25 `STATE_DEFINE` state objects, common to all four files from the shared
base). It is a **prologue/epilogue convention MWCC chose differently**: the
target and the correct drafts call the shared runtime helper `bl
_savegpr_27` (and, symmetrically, `_restgpr_27` at the tail) to save/restore
nonvolatile GPRs `r27`-`r31` in one instruction, in a `0x3b0`-byte frame;
author_states's and author_core's *isolated* compiles instead inline every
register store individually, in a `0x3a0`-byte frame -- 27 words shorter per
register-save site because the loads/stores are literally different
instructions, not a reordering of the same ones.

**What changed between "isolated" and "merged" is the *whole compiled TU*,
not `__sinit`'s own source.** `author_mov`'s isolated file (which already
gets 1193, matching target) is the *smallest* of the four -- only the 8
`mov_to_*`/`mov_frm_*` bodies plus the shared 25-state skeleton. `author_
states` and `author_core`, isolated, each add many more register-heavy
functions of their own (states: 60 state bodies; core: the circular-motion
family, `CalcAdjustPosY`'s two saved-register loops, etc.) yet get the
*wrong* prologue convention alone. The merge -- which is *larger* than any
single author's file -- gets it *right* again. This is consistent with `-ipa
file`'s whole-translation-unit view: MWCC's choice of whether `_savegprN`/
`_restgprN` calls are worth it (they only pay off when the shared code is
reused enough times across the file to amortize the extra `bl`) is a
**file-level heuristic**, not a per-function one, and it can flip based on
what *other* functions are present in the same compile, not on anything in
`__sinit`'s own body. I did not bisect exactly which other author's presence
tips the heuristic (would need N compiles of partial combinations, more
research than this merge pass warrants) -- **the measured fact is that the
full four-way union already produces the target's exact 1193, so nothing
further is needed here**; I'm flagging the mechanism as my best-supported
inference, not a fully traced cause.

This directly contradicts the "structural, not scheduling" framing in the
brief only in one respect worth being precise about: it's real content in
the sense that different bytes are genuinely emitted (not a display
artifact), but the *source text* of `__sinit` never changes -- the difference
is a compiler-side, whole-file codegen decision triggered by which other
functions exist in the same object, not by declaration order within
`__sinit` or by anything corrigible in the C++ text of the affected function.

## 6. Regression check: byte-exact-in-isolation functions that are NOT
   byte-exact in the merge

Checked every function each RESULT.md claimed byte-exact (49 total: mov 8,
states 32, core 8 named + `fn_800C15B0` unnamed, geom 0) against the merge's
matched set.

**48 of 49 reproduce cleanly. One genuine regression:**

- **`fn_800C15B0`** (target 7 words) -- author_core reported this byte-exact,
  but only by adding a throwaway `DUMMY_FORCE_EMIT_800C15B0()` global wrapper
  purely to force MWCC to emit an otherwise-dead `static` function for
  isolated diffing, with an explicit note in their own draft: *"DELETE at
  merge once a real caller exists."* I did not carry the dummy into the
  merge (it is not a target function and would corrupt the section with an
  extra symbol). `fn_800C15B0`'s real caller is presumably the un-authored
  `fn_800C31C0` (549 words, explicitly deferred by author_core). Without a
  caller, `setArrElem_800C15B0` is an unreferenced `static` and `-O4` dead-
  strips it entirely -- confirmed absent from the merge's compiled object
  (grepped the disassembly directly, not just the tally: no `15B0`/
  `setArrElem` symbol at all). **This is the correct, expected outcome of
  removing test scaffolding that was never meant to be merged**, not a
  defect in the merge -- but it is a real regression against the isolated
  number, so I'm reporting it plainly as instructed.

**All 8 `mov_to_*`/`mov_frm_*`, all 8 named `author_core` functions
(`init`, `move`, `SetPos`, `SetBaseSpeed`, `change_dir`, `getLineUnitNo`,
`is_unit_circle2x2`, `is_unit_circle4x4`), and all 32 `author_states`
functions are present and byte-exact in the merge with no regression.**

**Bonus, not a regression -- a merge-only improvement**: 8 of author_states's
`initializeState_*` functions (`CornerSideLine`, `Left30Left`, `Left30Right`,
`Left45`, `Right30Left`, `Right30Right`, `Right60Up`, `Side`) were reported as
"diff 1, naming artifact only" in isolation, because their only difference
from target was `bl fn_800C3B20` (target, no symbol) vs `bl
fn_800C3B20__FP10dLineMng_c` (an *unresolved external* in their isolated
compile). Author_states predicted in their own RESULT.md that *"once the
geom/mov agent's actual static definitions for these two functions land in
the same TU, this artifact disappears on its own."* That prediction is
confirmed: with `fn_800C3B20`'s real body (from author_core) now defined in
the same merged TU, all 8 of these functions are genuinely, fully
byte-exact matches in the merge -- not just length-exact. (The remaining
`fn_800C3B60`-calling `initializeState_*` functions -- `Height`,
`CornerHeightLine`, `Left60Up`, `Left60Down`, `Right60Down` -- do **not**
fully resolve the same way: they still show `LEN OK` but differing, because
author_states's own report already flagged a *second*, real residual in
those five beyond the naming artifact -- consistent, not a new problem.)

## 7. Tooling caveat found while measuring

`fn_800C1EE0`, `fn_800C3B20`, and `fn_800C3B60` show as **"MISSING"** in
`tally.py`'s own output above, but they are **not actually absent** --
grepping the compiled disassembly directly shows all three present under
their real (mangled) C++ symbol names
(`fn_800C1EE0__FP10dLineMng_cffRC7mVec2_cRC7mVec2_cRC7mVec2_cRC7mVec2_c`,
`fn_800C3B20__FP10dLineMng_c`, `fn_800C3B60__FP10dLineMng_c`). `tally.py`'s
`parse()` keys functions by their literal `.fn` label, and the target's
labels for these three are bare addresses (`fn_800C1EE0` etc., since dtk has
no symbol for an internal/stripped function in the retail binary) -- so the
lookup `k in d` never matches, regardless of byte content. I hand-checked all
three against target bytes directly (see below): **none of them would have
counted as a match anyway** (author_geom never claimed `fn_800C1EE0`
byte-exact; author_core's own isolated numbers already showed `fn_800C3B20`/
`fn_800C3B60` as length-complete-but-differing, not exact), so this tooling
gap does not change the headline 100/182 or 27.7% -- but it means the
"MISSING" list above over-reports by 3 real entries that are actually
"present, differing" (`fn_800C15B0` is the only one of the four `fn_`-named
merge items that is *genuinely* absent, per section 6). Hand-measured
comparison:

| target symbol | target words | draft words | first differing instruction |
|---|---:|---:|---|
| `fn_800C1EE0` | 36 | 40 | target `stwu -0x30`, draft `stwu -0x40` (frame size differs) |
| `fn_800C3B20` | 15 | 15 (length-exact) | target `fcmpo cr0,f0,f1`, draft `fcmpo cr0,f1,f0` (operand order) |
| `fn_800C3B60` | 15 | 16 | target `lfs f2,0x54(r3)`, draft `lfs f0,0x44(r3)` |

## 8. Other un-authored functions (expected gaps, not regressions)

26 target functions have no body anywhere across the four drafts and are
correctly absent from the merge (declared in the header only, where
declared at all): `GetPos`, `acm_angle`, `calc_rotate_to_circle_rev`,
`calc_rotate_to_circle_prev`, `circle_nextpos_set`, `fn_800C31C0`,
`fn_800C3BA0`, `fn_800C3BF0`, and the 11 `line0_cross_chk`...`lineE_cross_chk`
functions (declared `void` in the header, never touched by any author). The 9
`initializeState_Circle*` functions remain the base skeleton's trivial `{}`
(author_states explicitly left them out of scope, depending on the
un-authored `circle_nextpos_set`) and show up correctly as "present but
16w/13w/14w-differing" against a non-trivial target, not as missing.

## Files

- `wip/line_mng_merge/d_line_mng.cpp` -- the merged unit, defined in target
  address order throughout (verified against every `.fn` in
  `wip/line_mng_shared/target.txt`).
- `wip/line_mng_merge/shadow_include/game/bases/d_line_mng.hpp` -- the shared
  header plus the three MERGE-LOCAL additions in section 3, each marked and
  diffable against `wip/line_mng_shared/shadow_include/game/bases/d_line_mng.hpp`.

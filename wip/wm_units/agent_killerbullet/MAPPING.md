# WM_KILLERBULLET (daWmKillerBullet_c) -- function inventory

Reconstructed from disk by a clean rebuild + diff of every function against its target. Total
inventory: **37 functions**, spanning `.text` `0x1686e0`-`0x16a150`, split across three target
dumps:
- `target_auto_00_001686B4_text.txt` -- `0x1686b4`-`0x169fa0` (bulk of the unit)
- `target_auto_fn_2_169FA0_text.txt` -- one function, `0x169fa0`-`0x16a124`
- `target_auto_00_0016A124_text.txt` -- only the FIRST function, `fn_2_16A130` at `0x16a130`,
  is ours; `fn_2_16A150` onward belongs to the next unit

**Tally: 20/37 matched**, verified by direct diff of every pair (not taken on faith). "Matched"
means `harness.extract()`-canonicalised bodies are byte-identical OR differ ONLY in symbol naming
(a `bl`/reference to a still-unnamed sibling function, or a `lbl_2_*`/pooled-float address the
target can't name yet but we can) -- see HANDOFF's "residual lines that are purely symbol naming"
precedent. History: 17/37 at pickup -> 19/37 after the first round (2 new small functions
matched, one critical link-order fix) -> **20/37** this round (1 more new function matched: the
WM_KILLER cross-unit dependency).

## Function definition order -- STILL GREEN

The linker places `.text` in DEFINITION order; every function body in the `.cpp` is kept in
ascending target-address order end to end, verified after every change with
`wip/wm_units/check_fn_order.py wip/wm_units/agent_killerbullet/d_a_wm_killerbullet.cpp` (reports
`OK ... N addressed definitions, ascending`, currently N=19). **Every new function this round was
inserted at its correct address slot, never appended at the end.**

## MATCHED (20)

| draft name | target | notes |
|---|---|---|
| `daWmKillerBullet_c_classInit__Fv` | `fn_2_1686E0` | naming-only (1) |
| `__ct__18daWmKillerBullet_cFv` (ctor) | `fn_2_168710` | naming-only (2) |
| `__dt__18daWmKillerBullet_cFv` (dtor) | `fn_2_168770` | naming-only (2) |
| `draw` | `fn_2_168C00` | EXACT (0 diff) |
| `doDelete` | `fn_2_168C70` | EXACT (0 diff, trivial) |
| `endEffectAndResetState` | `fn_2_168E60` | EXACT (0 diff) |
| `state0` | `fn_2_168EB0` | naming-only (2) |
| `unk_168F00` | `fn_2_168F00` | naming-only (1: tail `b` to `unk_169E10` by real name) |
| `state4` | `fn_2_168F10` | naming-only (2) |
| `endStateOrTransition` | `fn_2_168F50` | naming-only (4) |
| `state1` | `fn_2_168FF0` | naming-only (4) |
| `state3` | `fn_2_1690F0` | naming-only (3) |
| `state2` | `fn_2_169280` | naming-only (12) |
| `unk_169430` | `fn_2_169430` | naming-only (2) |
| `unk_1694A0` | `fn_2_1694A0` | naming-only (3) |
| `checkParentFlag` | `fn_2_169500` | naming-only (1) |
| `unk_169510` | `fn_2_169510` | naming-only (2) |
| `unk_169530` | `fn_2_169530` | naming-only (1) |
| `unk_169B80` | `fn_2_169B80` | EXACT (0 diff) |
| `unk_169550` | `fn_2_169550` | **NEW this round.** naming-only (4: all `bl` to daWmKiller_c members by real mangled name) |

## REAL MISMATCHES -- pre-existing, untouched this round (per "no fourth variant" rule)

| draft name | target | size (target/draft) | diff | status |
|---|---|---|---|---|
| `create` | `fn_2_168860` | 74/74 | 18 | Real register-allocation difference. Not attempted this round. |
| `unk_168C80` | `fn_2_168C80` | 49/49 | 7 | POOL-DEPENDENT (HANDOFF-documented, and independently re-confirmed by the coordinator's own citation). **Re-diffed opportunistically after every function authored this round -- STILL 7, unchanged.** Everything authored this round added `.bss` statics and `.text` functions, not `.data` string-pool contributors, so this null result is exactly what the rule predicts, not a contradiction of it. Will move once whichever function contributes the missing `0x144` bytes of `.data` pool is authored (still `unk_168990`, `unk_168D50`, `unk_1691A0`, or `unk_1698E0` -- all 4 remaining fake stubs). |
| `execute` | `fn_2_168AB0` | 84/90 | 89 | Documented wall (5 variants on record). Checked: no `dPyMdlBase_c` usage in this unit, so the `getBodyMdl` header-fix news does not apply here. Not re-attempted. |
| `unk_169F00` | `fn_2_169F00` | 39/39 | 26 | Documented wall (3 variants on record, size-exact, branch polarity unreachable). Not re-attempted. |

## AUTHORED THIS ROUND (or last round), PARKED -- real content, real residual

All have size-exact or near-size-exact drafts; every residual is register allocation, instruction
scheduling, or (for two of them) a resistant `cror`-fusion/float-evaluation-order class already
seen elsewhere in this unit -- not wrong calls, wrong fields, or wrong constants.

- **`unk_1693C0`** (target 27 lines) -- 14/25 differing. `dBase_c::searchBaseByProfName` loop.
- **`unk_169080`** (target 28 lines, SAME size) -- 13/28 differing. State-3 transition + effect.
- **`unk_169DA0`** (target 26 lines, SAME size) -- 13/26 differing. Distance-vs-threshold check.
- **`unk_169E10`** (target 58 lines, SAME size) -- 19/58 differing. Jump-init + direction cache.
- **`unk_1695E0`** (target 116 lines, SAME size, was a FAKE STUB) -- 67/116 differing.
  **NEW this round.** Real content: early-out on `m_1d4`, a two-part attack-detection test
  (`unk_1697B0` + `unk_169DA0`), then either a star-mode short path (`attackMapEnemy` +
  `unk_168F00`) or a longer non-star path (midpoint effect, `.bss` caching, a delta vector into
  `s_bssDir10`, and a `dCsSeqMng_c::FUN_801017c0(SMC_DEMO_KILLER, ...)` cutscene trigger). Two
  structural bugs found and fixed (`.bss` static declaration order so a new `s_bssBox30` lands at
  the right offset; if/else branch polarity), dropping 83->67. Residual: symbol naming (5 lines),
  a float-argument-evaluation-order class already parked on `unk_169E10` (not re-attempted), and
  minor register renumbering in a delta computation whose CONTENT and ORDER are both confirmed
  correct.
- **`unk_1697B0`** (target 76 lines, draft 78) -- 73/76 differing. **NEW this round.**
  `bool (const float *box)`: a "did this bullet move" quadrant check (X/Z vs `mLastPos`, computed
  but only the `==6`/"didn't move" case is actually read -- matches the target's own apparent
  dead code, not simplified away) plus an expanded-AABB overlap test against `box`'s two corner
  points, using a genuinely-confirmed rodata constant (`sc_half = 0.5f`, read directly from
  `original/d_basesNP.rel` at file offset `0x1cf03c` -- not guessed). Two residual axes, three
  attempts each: the target's `==`-then-`<=` compare on the SAME operand pair does NOT fuse into
  a `cror`, but every phrasing of the Z-branch here DOES fuse (nested if/else with explicit `==6`
  leaf; a `!=`-guarded ternary; nested if/else with the default hoisted) -- trigger is nesting
  depth or block emptiness, not comparison shape, and no rewrite reached it; separately, the AABB
  half hits the same float-eval-order class as `unk_169E10`.
- **`processCutsceneCommand`** (`fn_2_169BC0`, vtable slot 24, target 117 lines, draft 111) --
  49/117 differing. **NEW this round.** Two structural fixes: (1) rewrote BOTH 3-way dispatches
  on `cutsceneCmd` from if/else-if chains to genuine `switch` statements -- the documented
  switch-vs-if-chain tell (all compares up front, bodies after, vs interleaved) -- which dropped
  102->49 differing on its own; (2) a proven **HEADER DEFECT**, see below. Remaining residual:
  four `this+0x60`-then-`+0x64`/`+0x68` raw function-pointer-table walks (modelled via raw casts,
  not an invented type, matching the destructor's own `m_1fc` precedent) consistently allocate
  r4-then-r12 where the target reuses r12 for both loads; tried both a named-intermediate and a
  fully-inlined phrasing with an IDENTICAL result both times (register-allocation wall, not
  source-shape-dependent) -- plus a few naming-only lines from float-constant pooling.

## HEADER DEFECT FOUND AND FIXED THIS ROUND (shadow copy only)

**`dWmRotater_c::calcRotate()` was declared `void`; it genuinely returns `bool`.**
`processCutsceneCommand`'s own call site, `if (!m_1fc->calcRotate()) return;`, CONSUMES the
result -- a call site consuming a `void` method's result is a header defect by definition (the
project's own established rule, and the news item from the previous round about
`dPyMdlBase_c::getBodyMdl` is the identical pattern). Fixed in the shadow header. `execute()`'s
own already-MATCHED call (`m_1fc->calcRotate();`, result discarded) is confirmed UNAFFECTED --
re-diffed after the fix, still naming-only. This defect is in `dWmRotater_c`, a class with no
landed header anywhere in the project yet (shadow-only, same as the rest of this unit), so there
is nothing to apply to the real tree -- recorded here for whichever agent eventually lands this
class for real.

## STUBS -- STILL NOT YET AUTHORED (fake bodies, `m_1c0 = <literal>`)

Down from 5 to 4 this round (`unk_1695E0` was replaced with real content, see above).

| draft name | target | target lines | calls (from scouting) |
|---|---|---|---|
| `unk_168990` | `fn_2_168990` | 71 | `endEffectAndResetState`, `unk_1693C0`, `unk_1694A0`, `unk_169080`; 3-way branch on `(u16)(mParam>>16)`; reads/writes a partially-mapped `.bss` cache at `lbl_2_bss_FE10` |
| `unk_168D50` | `fn_2_168D50` | 67 | unscouted |
| `unk_1691A0` | `fn_2_1691A0` | 55 | unscouted |
| `unk_1698E0` | `fn_2_1698E0` | 167 | largest remaining function; calls `fn_2_169B80` (now MATCHED) five times |

## STILL UNSTARTED -- 2 remain, BOTH deliberately left (coordinator's own call)

| target | size (lines) | notes |
|---|---|---|
| `fn_2_16A130` | 7 | Compiler-generated `__destroy_arr` boilerplate. Deprioritised per the coordinator's explicit instruction. |
| `fn_2_169FA0` | 97 | **INVESTIGATED this round, found to be compiler-generated, not hand-written.** Registered in `.ctors` (confirmed: the target dump's own trailing `.section .ctors` / `.4byte fn_2_169FA0`) -- it is the target's OWN static-initialization routine for THIS translation unit, generated automatically from `#include <game/bases/d_wm_lib.hpp>`'s file-scope `static dWmLib::ForceInCourseList_t sc_ForceList[] = {...}` (a REAL, already-landed array with real initializer values, confirmed by reading the header directly -- one entry is literally `{WORLD_7, "F7C0", WORLD_7, dCsvData_c::c_CASTLE_ID, 4, "W7C0", mVec3_c(2160.0f, -30.0f, -478.0f)}`, matching `2160.0` at rodata `0x8a40`, the constant immediately following our own claimed rodata bound). **Our draft ALREADY auto-generates the matching piece** -- `"__sinit_\d_a_wm_killerbullet_cpp"` in the current draft.txt already contains `sc_ForceList`/`c_CASTLE_ID`/`c_START_ID`/`__register_global_object` in the SAME shape, just currently only 33 lines vs the target's 97. The missing 64 lines are a SEPARATE guarded static-local object (a `bne`-gated init flag at `.bss+0x48`, further `mVec3_c::Ex`/`mVec3_c::Zero`-derived writes into `R_2_5_45428`'s own sub-entries) that almost certainly lives inside one of the 4 still-unauthored functions above -- NOT inside `fn_2_16A130`'s own sibling `.ctors` registration, since `fn_2_169FA0` and `fn_2_16A130` are positioned in the target immediately AFTER every hand-written function in address order, exactly where a TU's `__sinit`/array-dtor pair always lands. **Prediction for the next round: once the 4 remaining stubs are authored for real, re-diff the EXISTING `__sinit`/`__arraydtor$12783` (no new code needed) -- they should close toward `fn_2_169FA0`/`fn_2_16A130` on their own**, the same "pool can't be right until every contributor exists" rule already established for `unk_168C80`, just for compiler-generated code instead of hand-written.

## Shadow header changes this round

`shadow_include/game/bases/d_a_wm_killerbullet.hpp`:
- Added declarations for `unk_169550`, `unk_1697B0`, `unk_169E10` (from the prior round's
  handoff), `unk_168F00`, and `virtual void processCutsceneCommand(...)`.
- **`dWmRotater_c::calcRotate()` changed from `void` to `bool`** -- proven header defect, see
  above.

New shadow copies added this round (both are copies of the REAL landed header plus additions,
same convention as the unit's own header):
- `shadow_include/game/bases/d_a_wm_player.hpp` -- added `static bool isWalkToAttackPoint();` and
  `void attackMapEnemy(bool);`, neither in the real landed header yet.
- `shadow_include/game/bases/d_cs_seq_manager.hpp` -- added `int GetCutArg0();`, not in the real
  landed header yet.

New file-scope statics in the `.cpp` (all `.bss`, matching the confirmed-empty-ctor-lands-in-bss
rule already established for `s_bssDir10`): `s_bssVec1c`, `s_bssParam28`, `s_bssParam2c`,
`s_bssBox30` -- **declaration ORDER matters** (the linker packs `.bss` in declaration order like
`.text`); the correct order, confirmed by rebuild+diff, is `s_bssDir10` (already existed) then
`s_bssVec1c`, `s_bssParam28`, `s_bssParam2c`, `s_bssBox30` in that exact sequence.

## No OTHER proven header defects this round

Checked specifically for the `dPyMdlBase_c::getBodyMdl()` vtable-0x28 pattern per the original
coordinator news -- this unit has no `dPyMdlBase_c` usage at all (grepped, confirmed last round).
The `dWmRotater_c::calcRotate()` fix above is a DIFFERENT instance of the same general rule
(void-declared method whose result is consumed at a call site), found independently this round.

## How to reproduce this tally

```
python wip/wm_units/agent_killerbullet/build.py
python wip/wm_units/check_fn_order.py wip/wm_units/agent_killerbullet/d_a_wm_killerbullet.cpp
python wip/wm_units/agent_killerbullet/difftool.py \
    wip/wm_units/agent_killerbullet/target_auto_00_001686B4_text.txt \
    wip/wm_units/agent_killerbullet/draft.txt \
    fn_2_<addr> <draft_symbol>
```
A "MATCH" in this MAPPING counts a residual as closed only when every differing line is a
`bl`/`lis`/`addi` reference to a symbol name the target can't carry (address-only) but the draft
can (real mangled name or `R_`/`lbl_`-prefixed constant) -- confirmed line-by-line per function
above, not assumed from a low count.

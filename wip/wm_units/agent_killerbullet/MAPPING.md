# WM_KILLERBULLET (daWmKillerBullet_c) -- function inventory

Reconstructed from disk (predecessor left no MAPPING.md) by a clean rebuild + diff of every
function against its target, then continued this round. Total inventory: **37 functions**,
spanning `.text` `0x1686e0`-`0x16a150` (per HANDOFF's own pinned bounds), split across three
target dumps:
- `target_auto_00_001686B4_text.txt` -- `0x1686b4`-`0x169fa0` (bulk of the unit)
- `target_auto_fn_2_169FA0_text.txt` -- one function, `0x169fa0`-`0x16a124`
- `target_auto_00_0016A124_text.txt` -- `0x16a124`-onward (only the FIRST function,
  `fn_2_16A130` at `0x16a130`, is ours; `fn_2_16A150` onward belongs to the next unit)

**Tally this round: 19/37 matched** (up from a measured 17/37 at pickup). "Matched" means
`harness.extract()`-canonicalised bodies are byte-identical OR differ ONLY in symbol naming
(a `bl`/reference to a still-unnamed sibling function or a `lbl_2_*` address the target can't
name yet but we can) -- see HANDOFF's "residual lines that are purely symbol naming" precedent.
Verified by direct diff of every pair, not taken on faith.

## CRITICAL FIX THIS ROUND: function definition order

The linker places `.text` in DEFINITION order, and every function name here carries its real
target address. The draft as left by the predecessor had **6 ordering inversions** (flagged by
the coordinator's `wip/wm_units/check_fn_order.py`, run against this file). **Fixed by pure
reordering, zero logic changes** -- rebuilt and re-diffed every function afterward; the tally
was IDENTICAL before and after (confirms the reorder was a true no-op on content). The file is
now `check_fn_order.py`-clean end to end. **Any new function must be inserted at its correct
address position, not appended at the end** -- this file follows that discipline throughout.

## MATCHED (19)

| draft name | target | notes |
|---|---|---|
| `daWmKillerBullet_c_classInit__Fv` | `fn_2_1686E0` | naming-only (1 residual: `bl` to ctor by real name) |
| `__ct__18daWmKillerBullet_cFv` (ctor) | `fn_2_168710` | naming-only (2: `__vt__` name vs `lbl_2_data_455C0`) |
| `__dt__18daWmKillerBullet_cFv` (dtor) | `fn_2_168770` | naming-only (2, same vtable-name residual) |
| `draw` | `fn_2_168C00` | EXACT (0 diff) |
| `doDelete` | `fn_2_168C70` | EXACT (0 diff, trivial) |
| `endEffectAndResetState` | `fn_2_168E60` | EXACT (0 diff) |
| `state0` | `fn_2_168EB0` | naming-only (2) |
| `unk_168F00` | `fn_2_168F00` | **NEW this round.** naming-only (1: tail `b` to `unk_169E10` by real name) |
| `state4` | `fn_2_168F10` | naming-only (2) |
| `endStateOrTransition` | `fn_2_168F50` | naming-only (4) |
| `state1` | `fn_2_168FF0` | naming-only (4) |
| `state3` | `fn_2_1690F0` | naming-only (3) |
| `state2` | `fn_2_169280` | naming-only (12, all `bl`/`lis` symbol names) |
| `unk_169430` | `fn_2_169430` | naming-only (2) |
| `unk_1694A0` | `fn_2_1694A0` | naming-only (3) |
| `checkParentFlag` | `fn_2_169500` | naming-only (1) |
| `unk_169510` | `fn_2_169510` | naming-only (2) |
| `unk_169530` | `fn_2_169530` | naming-only (1) |
| `unk_169B80` | `fn_2_169B80` | **NEW this round.** EXACT (0 diff) |

## REAL MISMATCHES -- genuine content/codegen gaps (not naming)

| draft name | target | size (target/draft lines) | diff | status |
|---|---|---|---|---|
| `create` | `fn_2_168860` | 74/74 | 18 | Pre-existing. Real register-allocation difference around the `m_04`/`m_08` bgmSync field reads (lines 25-28 of the raw diff) -- not touched this round; not attempted (budget went to new functions per the "go wide" directive). |
| `unk_168C80` | `fn_2_168C80` | 49/49 | 7 | Pre-existing, POOL-DEPENDENT (HANDOFF-documented): content is correct, but string-pool offsets are short by exactly `0x144` bytes because sibling pool-contributing functions (the 5 stubs below) aren't real yet. Expected to close on its own once those are authored -- do not grind variants on this one directly. |
| `execute` | `fn_2_168AB0` | 84/90 | 89 | Pre-existing DOCUMENTED WALL (HANDOFF: 5 variants already tried, register over-preservation, `-0x20`/5-reg draft vs target's `-0x10`/2-reg). Not re-attempted this round -- the header-defect fix news (`dPyMdlBase_c::getBodyMdl`) does NOT apply to this unit (grepped; no `dPyMdlBase_c`/vtable-0x28-consuming code exists here), so there was no new information to re-test against. |
| `unk_169F00` | `fn_2_169F00` | 39/39 | 26 | Pre-existing DOCUMENTED WALL (HANDOFF: size-exact, branch polarity unreachable under 3 phrasings). Not re-attempted. |

## NEWLY AUTHORED THIS ROUND, PARKED (measured, not yet closed)

All four have size-exact or near-size-exact drafts and fully confirmed content/structure; every
residual is register allocation or instruction scheduling, not a wrong call, wrong field, or
wrong constant. Three genuinely different attempts were made on each before parking, per the
project's own three-strikes rule.

- **`unk_1693C0`** (`fn_2_1693C0`, target 27 lines) -- 14/25 differing. A
  `dBase_c::searchBaseByProfName(fProfile::WM_KILLER, ...)` loop matching the caller's own
  `(mParam>>8)&0xff` against each found actor's own `mParam&0xff`. Attempts: (1) `(u8)==(u8)`
  compare emitted `cmplw` vs target's `cmpw` (signedness lever, fixed by widening through `int`);
  (2, kept) int-masked compare, `while`+`break` -- 14 differing, residual is the loop-exit path
  (target re-materialises `li r3,0` AND merges both exits into one epilogue; this compiles two
  separate return sites); (3) folding the break into the `while` condition itself regressed to 19.
- **`unk_169080`** (`fn_2_169080`, target 28 lines) -- 13/28 differing, SAME size. Transitions to
  state 3, reloads `m_1b8` from the shared table (+0x4e), fires the same "skl_root" effect
  `state2` already fires. Three statement orderings tried (`m_1c0`/`m_1b8`/`m_1b0`;
  `m_1c0`/`m_1b0`/`m_1b8`; `m_1b8`/`m_1c0`/`m_1b0`, kept as best at 13) -- the scheduler does not
  appear steerable by source order alone here; every residual is which GPR holds which small
  constant and whether the `0x4e` short-load is hoisted early or held back.
- **`unk_169DA0`** (`fn_2_169DA0`, target 26 lines) -- 13/26 differing, SAME size. Null-checks
  `mParentKiller`, then `mPos.distTo(mParentKiller->mPos) < R_2_4_89B8[5]` (a second, distinct
  shared rodata table, OUTSIDE this unit's own `0x89f0-0x8a3c` bounds -- declared `extern "C"`
  via the `.rodata`-is-section-4 convention, `R_2_4_89B8`). Attempts: (1) threshold loaded after
  the two calls dropped the `f31` save/restore entirely (frame `-0x10` vs target's `-0x20`) --
  confirmed wrong; (2, kept) naming the threshold as a local BEFORE the calls restored the exact
  frame shape and got size-exact, residual now pure branch polarity + return-path merging;
  (3) single-exit `bool result` style regressed to 17.
- **`unk_169E10`** (`fn_2_169E10`, target 58 lines) -- 19/58 differing, SAME size. Builds an
  offset spawn position from `mPos` plus two shared-table deltas, calls the real base-class
  `_initDemoJumpBase(pos, 0, frames, jumpSpeed, startScale, targetScale, mVec3_c::Ey)` with
  scales derived from `mScale.x` times two more shared-table entries, then saves/restores
  `mAngle3D` around a `setDirection` call whose dir arg is a NEW unit-owned `.bss` static
  (`s_bssDir10` / `lbl_2_bss_FE20`, the `+0x10` slot inside the already-pinned `0xfe10-0xfe3c`
  bounds), then clears any active effect. Attempts: (1) 3-arg constructor `mVec3_c(x,y,z)` --
  19 differing, all float-load ordering; (2) explicit `pos.y=...; pos.z=...; pos.x=...;`
  field-by-field in the target's own STORE order -- regressed to 21, reverted to (1).

## STUBS -- NOT YET AUTHORED (fake bodies, `m_1c0 = <literal>`, left by predecessor)

Not touched this round (correctly deprioritized: each is large and each blocks on functions
that were themselves unstarted at pickup). Two of the five (`unk_168990`, `unk_1698E0`) now call
functions this round DID author (`unk_1693C0`, `unk_169080`, `unk_169B80`), so a future round
authoring these stubs has real building blocks to call into.

| draft name | target | target lines | calls (from scouting) |
|---|---|---|---|
| `unk_168990` | `fn_2_168990` | 71 | `endEffectAndResetState`, `unk_1693C0`, `unk_1694A0`, `unk_169080`; 3-way branch on `(u16)(mParam>>16)`; reads a NOT-YET-FULLY-MAPPED `.bss` cache at `lbl_2_bss_FE10` (only the `+0x10` slot, `s_bssDir10`, is claimed so far -- `+0x00`, `+0x1c`, `+0x28` remain open) |
| `unk_168D50` | `fn_2_168D50` | 67 | unscouted this round |
| `unk_1691A0` | `fn_2_1691A0` | 55 | unscouted this round |
| `unk_1695E0` | `fn_2_1695E0` | 116 | calls `fn_2_169DA0` (now authored as `unk_169DA0`, parked at 13 diff) |
| `unk_1698E0` | `fn_2_1698E0` | 167 | largest remaining function; calls `fn_2_169B80` (now MATCHED as `unk_169B80`) five times |

## STILL FULLY UNSTARTED (no draft symbol at all) -- 5 remain, down from 11

| target | size (lines) | notes |
|---|---|---|
| `fn_2_16A130` | 7 | Looked tiny but is compiler-generated `__destroy_arr` boilerplate for a SECOND static `dWmLib::ForceInCourseList_t` array (distinct from the one already backing `__arraydtor$12781` in the draft) at `lbl_2_data_453F8` -- the static array itself hasn't been located/declared yet. Deprioritized: fragile compiler-generated shape, not real "wide" work until the owning static is found. |
| `fn_2_169550` | 34 | The WM_KILLER cross-unit dependency (HANDOFF: `R_2_1_169550`) -- calls two `daWmKiller_c` member functions (`fn_2_1682D0`, `fn_2_1682B0`) on `mParentKiller` in a loop, needs two new `extern "C"` wrappers matching WM_KILLER's own mangled names (same pattern as `unk_1684A0__12daWmKiller_cFb`/`unk_168260__12daWmKiller_cFi` already in this file). High coordination value if authored -- resolves the pending landing-order note. |
| `fn_2_1697B0` | 76 | Calls `unk_168F00` (now authored/matched) among its own body; not otherwise scouted. |
| `fn_2_169BC0` | 117 | `processCutsceneCommand`, vtable slot 24 -- declared in the shadow header but never implemented. |
| `fn_2_169FA0` | 97 | Not scouted this round. |

## Header changes (shadow copy only, `shadow_include/game/bases/d_a_wm_killerbullet.hpp`)

- Added `int m_1c8;` at offset `0x1c8`, splitting what had been one 16-byte `mPad_1c4[0x10]`
  placeholder into `mPad_1c4[0x4]` + `m_1c8` (int) + `mPad_1cc[0x8]`. Proof: `unk_169B80`'s own
  `stw`/`lwz` at that exact offset, word-width, a wrapping counter -- not inferred, measured.
- Added declarations for the 6 newly authored functions (`unk_1693C0`, `unk_169080`,
  `unk_169B80`, `unk_169DA0`, `unk_169E10`, `unk_168F00`).

## No proven header defects this round

Checked specifically for the `dPyMdlBase_c::getBodyMdl()` pattern (vtable offset 0x28 consuming
a `void`-typed result) per the coordinator's news -- this unit has no `dPyMdlBase_c` usage at
all (grepped), so that fix does not apply here and nothing was re-tested against it.

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

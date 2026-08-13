# Sibling map: `d_a_player_demo_manager.cpp` / `daPyDemoMng_c`

`.text` 0x8005B3A0 - 0x8005D7E0 - span 9,280 B, bodies 8,976 B (304 B in
gaps/padding not counted as functions), **51 functions**.

Generated with `tools/sibmap.py` (FAMILY extended with the four player-side
TUs listed below) against a corpus of 5,050 functions built from every banked
matching slice plus our own compiled objects.

## Verification performed

- **`tools/sibmap.py` FAMILY list edited** (around line 265): added
  `dol_bases_d_a_player_base`, `dol_bases_d_a_player`, `dol_bases_d_ac_py_key`,
  `dol_bases_d_a_player_hio_ADJ` — this unit is player code, and the prior
  enemy-actor FAMILY entries contribute almost nothing to it. All four resolve
  to real corpus objects (verified via `check_family`'s stderr warning, which
  after the edit reports only one **pre-existing, unrelated** dead entry:
  bare `d_a_en_dpakkun` at the end of the list — not touched, out of scope for
  this unit, flagged here so it isn't lost).
- **Target disassembly comes from three dtk split objects that tile the range
  exactly**: `bin/dtkspl/obj/auto_03_8005B3A0_text.o` (0x8005B3A0-0x8005D750),
  `auto_sinit__d_a_player_de_text.o` (0x8005D750-0x8005D7C0),
  `auto_03_8005D7C0_text.o` (0x8005D7C0-...). The symbol immediately after the
  range, `__ct__17dAcPy_HIO_Speed_cFv` at 0x8005D7E0, confirms the upper bound
  is exact (start of `d_a_player_hio_ADJ.cpp`).
- **Every twin claim below was checked with BOTH raw instruction words and dtk
  disassembly TEXT** (which resolves relocated callees/symbols that raw words
  zero out). Where the two disagree it is stated explicitly — and it did
  happen once, badly (see Finding 2).
- 51/51 target functions confirmed to be the full symbol-table population of
  the range (two unnamed `fn_0x…` functions and one `__arraydtor$…` included,
  matching `bin/dtk/wiimj2d_symbols.txt`).

## Headline findings (read these before assigning anything)

1. **The brief's "~7% name-level precedent" is confirmed accurate at the name
   level and is an OVERESTIMATE of body-level value.** Exactly 8/51 functions
   (720/8976 B = **8.0% by bytes**) share a basename with something already
   banked anywhere in the corpus. But body-checking all 8: only **2 are real**
   (constructor e=0.938, destructor e=0.842 — see Finding 3). The other 6 —
   `initStage`, `init`, `update`, both `isDemoMode` overloads,
   `setEnemyStageClearDemo` — score **e=0.019 to e=0.200** against their
   namesakes in `daPlBase_c`/`dAcPyKey_c`: noise, same as the last unit's
   `setCcLine`/`model_set` trap. **Genuine same-name/same-body precedent is
   136/8976 B = 1.5% of the unit.** Do not brief an agent that a namesake is a
   template without the score attached.

2. **The tool's own "BIT-IDENTICAL IN-FILE TWINS" flag produced a false
   positive, caught by the raw-words-vs-disasm-text check the brief
   mandates.** `endControlDemoAll` (0x8005CA50) and `fn_8005CE50` (0x8005CE50)
   have **identical raw instruction words** — the tool flags them as
   bit-identical. Disassembly text shows why that's misleading: dtk zeroes
   the `bl` target field before encoding, and these two functions call
   **different functions** at that exact word position:
   `endControlDemoAll` calls `endControlDemo__10daPlBase_cFi`;
   `fn_8005CE50` calls `setControlDemoCutscene__10daPlBase_cFQ210daPlBase_c14AnimePlayArg_e`.
   Everything else — the 4-player bitmask loop, the two-`beq` guard structure,
   the frame layout — is genuinely identical. **Verdict: real near-twins, not
   duplicates.** Use `fn_8005CE50`'s skeleton for `endControlDemoAll` (or vice
   versa) but do not literally copy the call instruction.

3. **Two verified TRUE cross-TU twins, byte- and symbol-identical, not just
   shape-similar**: `__sinit_\d_a_player_demo_manager_cpp` (0x8005D750, 112 B)
   is **word-for-word identical, including every relocation target**, to
   `__sinit_\d_md_actor_cpp` and four other TUs' `__sinit` (`d_wm_actor`,
   `d_wm_demo_actor`, `d_wm_enemy`, `d_wm_obj_actor`) — confirmed by reading
   both disassembly texts side by side: same `sc_ForceList__6dWmLib` object,
   same `c_CASTLE_ID`/`c_START_ID`/`c_StartPointKinokoHouseID` symbols. This
   is compiler-emitted boilerplate common to any TU pulling in the same
   header state, **not specific to this unit's own logic** — copy verbatim.
   Its paired `__arraydtor$72504` (0x8005D7C0, 28 B) is the same story,
   verified against `CMP_dol_bases_d_md_actor::__arraydtor$14253`: identical
   bytes AND identical symbol names (`sc_ForceList__6dWmLib`,
   `__dt__Q26dWmLib19ForceInCourseList_tFv`). **These two functions (140 B,
   1.6% of the unit) can be authored by direct copy with zero risk.**

4. **The constructor is a verified real precedent; a nearby destructor is a
   verified near-twin.** `__ct__13daPyDemoMng_cFv` (e=0.938) vs
   `dCyuukan_c::__ct__10dCyuukan_cFv`: identical except this unit's ctor has
   one extra instruction, `stw r3, mspInstance__13daPyDemoMng_c@sda21(r0)`
   (registering the manager singleton) — `dCyuukan_c` has no such singleton.
   Confirmed by reading both disassembly texts. `__dt__13daPyDemoMng_cFv`
   (e=0.842) vs `CMP_dol_bases_d_a_player_hio_ADJ::__dt__17dAcPy_HIO_Speed_cFv`:
   same "if (this) { if (arg>0) delete-helper }" idiom, differing only in
   what the middle does with a static (this unit clears a singleton pointer
   with `li 0`/`stw`; the HIO class decrements a live-instance counter with
   `lwz`/`subi`/`stw`). Both are genuine, verified templates for singleton-
   manager ctor/dtor pairs — copy the skeleton, not the middle.

5. **The dominant real lever in this unit is INTRA-file, not cross-TU** — a
   repeating "for i in 0..4: test bit i of `mActPlayerInfo`, call
   `getCtrlPlayer(i)`, act on the result" idiom. **16 of 51 functions**
   contain the full bitmask-guarded loop (verified by literal instruction-text
   grep across the target disassembly, not scoring):
   `executeGoalDemo_Pole`, `executeGoalDemo_JumpCheck`, `executeGoalDemo_Land`,
   `executeGoalDemo_KimeWait`, `isAllPlayerGoalIn`, `calcGoalCenterPos`,
   `startControlDemoAll`, `isAllPlayerControlDemo`, `endControlDemoAll`,
   `getControlDemoPlayerNum`, `onLandStopReq`, `startControlDemoLandPlayer`,
   `isLandAll`, `fn_8005CE50`, `executeStartToride`,
   `setEnemyStageClearDemo`. A further 5 call `getCtrlPlayer` without the full
   bitmask-guard wrapper (`calcNotGoalPlayer`, `executeGoalDemo_PoleDown`,
   `executeGoalDemo_Jump`, `setGoalDemoKimeAll`, `setGoalDemoRunCastle`).
   Together these 21 functions total **4,436/8,976 B (49.4%)** of the unit —
   but only the loop skeleton itself (~10-12 words / ~40-48 B per function) is
   shared; the per-iteration body differs. **Do not report this as 49% body
   precedent** — report it as: 21 functions get a verified, reusable *loop
   skeleton*, cutting real authoring work on those functions by roughly a
   third to a half each, not eliminating it.
   The cleanest exemplar pair for the minimal form is
   `endControlDemoAll` / `fn_8005CE50` (30 insns each, one conditional call);
   for the boolean-return variant, `isAllPlayerControlDemo` / `isLandAll`.

6. **Two cross-TU candidates that scored respectably were checked by text and
   are genuine noise — rejected explicitly** (see Rejected section). Both
   involve `isYossyColor` / a HIO constructor matching purely on instruction
   *count*, not logic.

7. **`setHanabiEffect__13daPyDemoMng_cFv`** (0x8005C2B0, 348 B) has **nine
   function-scope static tables**, `@LOCAL@setHanabiEffect__13daPyDemoMng_cFv@scHanabiOffset_1`
   through `_9` (`.rodata`, sizes 0xC..0x6C in +0xC steps — an obvious
   1..9-element `mVec3_c[]` progression), plus `scHanabiOffsetDt` and
   `scHanabiEffectID` in `.data`. **This is exactly the `@LOCAL@` function-
   scope-static pattern the brief warned about** — these carry their own
   numbering rules (`@0` through `@9` suffixes already present in the symbol
   table) and must be declared in the same order inside the function body,
   not reordered. No cross-TU precedent exists for this function (best score
   e=0.253, noise) — full-effort authoring target.

## Verified twin pairs

| Target | Precedent | exact | shape | Verified how | Take |
|---|---|---|---|---|---|
| `__ct__13daPyDemoMng_cFv` (0x8005B3A0, 64B) | `dol_bases_d_cyuukan::__ct__10dCyuukan_cFv` | 0.938 | 0.938 | text-diffed, byte-for-byte except one extra singleton-register store | vtable-store + singleton-register + `bl init` skeleton |
| `__dt__13daPyDemoMng_cFv` (0x8005B3E0, 72B) | `CMP_dol_bases_d_a_player_hio_ADJ::__dt__17dAcPy_HIO_Speed_cFv` | 0.842 | 0.895 | text-diffed | null-check + delete-flag-arg idiom; middle (pointer-clear vs refcount) differs |
| `initStage__13daPyDemoMng_cFv` (0x8005B430, 52B) | **in-file**: `releaseDemoMode__13daPyDemoMng_cFi` | n/a (same-shape) | n/a | text-diffed both bodies directly | prologue + single `bl` + store-immediate-to-member + epilogue; only the callee/imm/offset differ |
| `releaseDemoMode__13daPyDemoMng_cFi` (0x8005B5D0, 52B) | **in-file**: `initStage__13daPyDemoMng_cFv` | n/a | n/a | (same pair as above) | (same as above) |
| `endControlDemoAll__13daPyDemoMng_cFi` (0x8005CA50, 120B) | **in-file**: `fn_8005CE50` (0x8005CE50, 120B) | words-identical (see Finding 2) | — | text-diffed — **the tool's "bit-identical" claim is corrected here** | 4-player bitmask loop skeleton identical; the single conditional `bl` target differs (`endControlDemo__10daPlBase_cFi` vs `setControlDemoCutscene__10daPlBase_cFQ210daPlBase_c14AnimePlayArg_e`) — copy skeleton, not the call |
| `fn_8005CE50` (0x8005CE50, 120B) | **in-file**: `endControlDemoAll` | (same pair) | — | (same pair) | (same as above) |
| `__sinit_\d_a_player_demo_manager_cpp` (0x8005D750, 112B) | `dol_bases_d_md_actor::__sinit_\d_md_actor_cpp` (also identical: `d_wm_actor`, `d_wm_demo_actor`, `d_wm_enemy`, `d_wm_obj_actor`) | 1.000 | 1.000 | text-diffed — every symbol reference matches, not just word shape | copy verbatim, zero risk |
| `__arraydtor$72504` (0x8005D7C0, 28B) | `CMP_dol_bases_d_md_actor::__arraydtor$14253` | 1.000 | 1.000 | text-diffed — symbol names match exactly | copy verbatim, zero risk |

## Rejected candidates (checked and discarded — do not chase these)

| Target | Rejected candidate | exact | Why rejected |
|---|---|---|---|
| `isGoalAllEntryPlayer__13daPyDemoMng_cFv` (60B) | `CMP_dol_bases_d_a_player_hio_ADJ::__ct__14dPyModel_HIO_cFv` | 0.733 | Text-diffed: target is a `subf`/`cntlzw`/`srwi` integer-compare idiom (`getEntryNum() == field`); candidate is an unrelated HIO model constructor. Same instruction *count* only — pure coincidence. |
| `isAllPlayerControlDemo__13daPyDemoMng_cFv` (128B) | `CMP_dol_bases_d_a_en_blockmain::isYossyColor__15daEnBlockMain_cFUs` | 0.656 | Text-diffed: both are "loop 4, call a per-index helper, compare, return bool" but the helper calls differ (`getCtrlPlayer`+`isStatus` vs `getYoshiDirectP`+`getModel`) and the loop guard differs (bitmask test vs none). Structurally-adjacent, not a real body precedent. |
| `isLandAll__13daPyDemoMng_cFv` (124B) | `CMP_dol_bases_d_a_en_blockmain::isYossyColor__15daEnBlockMain_cFUs` | 0.645 | Same rejection as above — this is the *same* corpus function surfacing as top hit for a second, unrelated target, which is itself a sign the match is a generic idiom collision, not precedent. |
| 6 of the 8 same-basename hits (`initStage`, `init`, `update`, `isDemoMode` x2, `setEnemyStageClearDemo`) | see Finding 1 table | 0.019–0.200 | Name-sharing only; bodies are unrelated (different classes, different logic despite the shared method name). |

## Per-function table

Legend: **IDIOM-L** = full 4-player bitmask-guarded loop (Finding 5, 16 fns);
**IDIOM-P** = calls `getCtrlPlayer` without the full loop wrapper (5 fns);
**TWIN** = verified pair (Finding 2-4); Verdict `NONE` = best cross-TU score
is noise (<~0.5 on a non-trivial function, or a tied/generic collision) —
these are where authoring effort should concentrate.

| Addr | Function | Bytes | Family | Best cross-TU candidate (exact) | Verdict |
|---|---|---:|---|---|---|
| 8005B3A0 | `__ct__13daPyDemoMng_cFv` | 64 | TWIN | `dol_bases_d_cyuukan::__ct__10dCyuukan_cFv` (0.938) | **VERIFIED** |
| 8005B3E0 | `__dt__13daPyDemoMng_cFv` | 72 | TWIN | `CMP_..._hio_ADJ::__dt__17dAcPy_HIO_Speed_cFv` (0.842) | **VERIFIED near-twin** |
| 8005B430 | `initStage__13daPyDemoMng_cFv` | 52 | TWIN (in-file) | — | **VERIFIED** (see releaseDemoMode) |
| 8005B470 | `initCourseIn__13daPyDemoMng_cFv` | 36 | — | `dol_bases_d_res_info::__ct__Q26dRes_c6info_cFv` (0.444) | NONE |
| 8005B4A0 | `init__13daPyDemoMng_cFv` | 176 | — | (0.023) | **NONE — author fresh** |
| 8005B550 | `update__13daPyDemoMng_cFv` | 100 | — | (0.160) | **NONE — author fresh** |
| 8005B5C0 | `setDemoMode__13daPyDemoMng_cFQ2...ei` | 12 | — | generic 1-store setter (0.333, tied) | trivial, no precedent needed |
| 8005B5D0 | `releaseDemoMode__13daPyDemoMng_cFi` | 52 | TWIN (in-file) | — | **VERIFIED** (see initStage) |
| 8005B610 | `isDemoMode__13daPyDemoMng_cCFQ2...e` | 20 | — | tied generic predicates (0.600) | trivial, noise |
| 8005B630 | `isDemoMode__13daPyDemoMng_cCFQ2...ei` | 88 | — | `dol_bases_d_a_player::isRideJrCrownOwn` (0.750) | weak/unverified — call+cmp+bool idiom, plausible but not text-confirmed as intentional precedent |
| 8005B690 | `deleteNotGoalPlayer__13daPyDemoMng_cFv` | 12 | — | tied generic 1-call stub (0.333) | trivial, noise |
| 8005B6A0 | `calcNotGoalPlayer__13daPyDemoMng_cFv` | 216 | IDIOM-P | (0.407) | **NONE beyond partial idiom — author fresh** |
| 8005B780 | `setGoalDemoList__13daPyDemoMng_cFi` | 68 | — | (0.176) | **NONE — author fresh** |
| 8005B7D0 | `isGoalAllEntryPlayer__13daPyDemoMng_cFv` | 60 | — | REJECTED (see table above) | **NONE — author fresh** |
| 8005B810 | `stopBgmGoalDemo__13daPyDemoMng_cFv` | 36 | — | tied generic short-fn hits (0.444) | NONE |
| 8005B840 | `getPoleBelowPlayer__13daPyDemoMng_cFi` | 84 | — | (0.267) | **NONE — author fresh** |
| 8005B8A0 | `executeGoalDemo_Pole__13daPyDemoMng_cFv` | 1100 | IDIOM-L | (0.095) | **NONE — largest function after fn_8005D280, author fresh; only the loop skeleton is reusable** |
| 8005BCF0 | `executeGoalDemo_PoleDown__13daPyDemoMng_cFv` | 108 | IDIOM-P | tied `checkEntry`/`move` (0.630) | weak, generic short if-loop shape — treat as NONE |
| 8005BD60 | `executeGoalDemo_JumpCheck__13daPyDemoMng_cFv` | 208 | IDIOM-L | (0.442) | **NONE beyond loop skeleton — author fresh** |
| 8005BE30 | `executeGoalDemo_Jump__13daPyDemoMng_cFv` | 216 | IDIOM-P | (0.298) | **NONE — author fresh** |
| 8005BF10 | `executeGoalDemo_Land__13daPyDemoMng_cFv` | 180 | IDIOM-L | (0.511) | **NONE beyond loop skeleton — author fresh** |
| 8005BFD0 | `executeGoalDemo_KimeWait__13daPyDemoMng_cFv` | 192 | IDIOM-L | (0.346) | **NONE beyond loop skeleton — author fresh** |
| 8005C090 | `executeGoalDemo__13daPyDemoMng_cFv` | 156 | — | `dol_bases_d_a_player_base::executeState_DemoGoal` (0.487) | weak, unverified |
| 8005C130 | `setGoalDemoKimeAll__13daPyDemoMng_cFv` | 100 | IDIOM-P | `dol_bases_d_a_en_bigpile::move` (0.680) | weak, unverified — worth a quick text check before relying on it |
| 8005C1A0 | `setGoalDemoRunCastle__13daPyDemoMng_cFv` | 112 | IDIOM-P | tied `PlayerCarryCheck`/`isPlayerDemo` (0.607) | weak, unverified |
| 8005C210 | `isAllPlayerGoalIn__13daPyDemoMng_cFv` | 160 | IDIOM-L | (0.465) | **NONE beyond loop skeleton — author fresh** |
| 8005C2B0 | `setHanabiEffect__13daPyDemoMng_cFv` | 348 | — | (0.253) | **NONE — has 9 `@LOCAL@` static tables, see Finding 7, full-effort target** |
| 8005C410 | `executeGoalCastle__13daPyDemoMng_cFv` | 692 | — | (0.191) | **NONE — 3rd-largest function, author fresh** |
| 8005C6D0 | `calcGoalCenterPos__13daPyDemoMng_cFv` | 344 | IDIOM-L | (0.279) | **NONE beyond loop skeleton — author fresh** |
| 8005C830 | `setZoromeGoal__13daPyDemoMng_cFv` | 256 | — | (0.266) | **NONE — author fresh** |
| 8005C930 | `startControlDemoAll__13daPyDemoMng_cFv` | 160 | IDIOM-L | (0.450) | **NONE beyond loop skeleton — author fresh** |
| 8005C9D0 | `isAllPlayerControlDemo__13daPyDemoMng_cFv` | 128 | IDIOM-L | REJECTED (see table above) | **NONE beyond loop skeleton — author fresh** |
| 8005CA50 | `endControlDemoAll__13daPyDemoMng_cFi` | 120 | TWIN (in-file) + IDIOM-L | — | **VERIFIED near-twin, see Finding 2** |
| 8005CAD0 | `getControlDemoPlayerNum__13daPyDemoMng_cCFv` | 136 | IDIOM-L | `dol_bases_d_res::getResSilently` (0.559) | weak, unverified beyond loop skeleton |
| 8005CB60 | `setBossDownPlayerNo__13daPyDemoMng_cFi` | 12 | — | tied generic setter (0.333) | trivial |
| 8005CB70 | `onLandStopReq__13daPyDemoMng_cFv` | 140 | IDIOM-L | `dol_bases_d_a_en_bigpile::wait` (0.514) | weak, unverified beyond loop skeleton |
| 8005CC00 | `startControlDemoLandPlayer__13daPyDemoMng_cFv` | 208 | IDIOM-L | (0.442, same `break_balloon` hit recurring elsewhere — sign of noise) | **NONE beyond loop skeleton — author fresh** |
| 8005CCD0 | `fn_8005CCD0` | 252 | IDIOM-P | `dol_bases_d_a_player_base::checkSlipEndKey` (0.365) | **NONE — author fresh, unnamed function** |
| 8005CDD0 | `isLandAll__13daPyDemoMng_cFv` | 124 | IDIOM-L | REJECTED (see table above) | **NONE beyond loop skeleton — author fresh** |
| 8005CE50 | `fn_8005CE50` | 120 | TWIN (in-file) + IDIOM-L | — | **VERIFIED near-twin, see Finding 2. Unnamed function.** |
| 8005CED0 | `executeStartToride__13daPyDemoMng_cFv` | 216 | IDIOM-L | (0.426) | **NONE beyond loop skeleton — author fresh** |
| 8005CFB0 | `executeEndToride__13daPyDemoMng_cFv` | 160 | — | `dol_bases_d_a_player_base::executeState_DemoGoal` (0.450, same hit as executeGoalDemo — recurring, likely noise) | NONE |
| 8005D050 | `setCourseOutList__13daPyDemoMng_cFSc` | 64 | — | (0.188) | **NONE — author fresh** |
| 8005D090 | `checkDemoNo__13daPyDemoMng_cFSc` | 40 | — | (0.455) | weak/noise |
| 8005D0C0 | `getNextDemoNo__13daPyDemoMng_cFv` | 8 | — | tied trivial getters (0.500) | trivial, no precedent needed |
| 8005D0D0 | `turnNextDemoNo__13daPyDemoMng_cFv` | 36 | — | (0.111) | **NONE — author fresh** |
| 8005D100 | `clearDemoNo__13daPyDemoMng_cFSc` | 380 | — | (0.092) | **NONE — author fresh, large function** |
| 8005D280 | `fn_8005D280` | 1064 | — | (0.038) | **NONE — LARGEST function in the unit (11.9% of it), unnamed, full-effort authoring target** |
| 8005D6B0 | `setEnemyStageClearDemo__13daPyDemoMng_cFi` | 148 | IDIOM-L | name-match rejected (see Finding 1); mechanical top `dol_bases_d_res::getResInfo` (0.595) | weak beyond loop skeleton |
| 8005D750 | `__sinit_\d_a_player_demo_manager_cpp` | 112 | TWIN | 5-way tie, all 1.000 | **VERIFIED, copy verbatim** |
| 8005D7C0 | `__arraydtor$72504` | 28 | TWIN | `CMP_dol_bases_d_md_actor::__arraydtor$14253` (1.000) | **VERIFIED, copy verbatim** |

## Functions with NO usable precedent anywhere (concentrate authoring effort here)

By bytes, these are the biggest-ticket fresh-authoring items — the four
largest alone are 3,548 B, 39.5% of the whole unit:

| Addr | Function | Bytes | Note |
|---|---|---|---|
| 8005D280 | `fn_8005D280` | 1064 | Largest function in the unit; unnamed |
| 8005B8A0 | `executeGoalDemo_Pole__13daPyDemoMng_cFv` | 1100 | Largest overall; has partial loop-skeleton help only |
| 8005C410 | `executeGoalCastle__13daPyDemoMng_cFv` | 692 | |
| 8005D100 | `clearDemoNo__13daPyDemoMng_cFSc` | 380 | |
| 8005C2B0 | `setHanabiEffect__13daPyDemoMng_cFv` | 348 | 9 `@LOCAL@` function-scope static tables — see Finding 7 |
| 8005C6D0 | `calcGoalCenterPos__13daPyDemoMng_cFv` | 344 | Loop-skeleton help only |
| 8005C830 | `setZoromeGoal__13daPyDemoMng_cFv` | 256 | |
| 8005CCD0 | `fn_8005CCD0` | 252 | Unnamed |
| 8005B6A0 | `calcNotGoalPlayer__13daPyDemoMng_cFv` | 216 | Loop-skeleton help only |
| 8005BE30 | `executeGoalDemo_Jump__13daPyDemoMng_cFv` | 216 | |
| 8005CED0 | `executeStartToride__13daPyDemoMng_cFv` | 216 | Loop-skeleton help only |
| 8005BD60 | `executeGoalDemo_JumpCheck__13daPyDemoMng_cFv` | 208 | Loop-skeleton help only |
| 8005CC00 | `startControlDemoLandPlayer__13daPyDemoMng_cFv` | 208 | Loop-skeleton help only |
| 8005B4A0 | `init__13daPyDemoMng_cFv` | 176 | |
| 8005BF10 | `executeGoalDemo_Land__13daPyDemoMng_cFv` | 180 | Loop-skeleton help only |
| 8005BFD0 | `executeGoalDemo_KimeWait__13daPyDemoMng_cFv` | 192 | Loop-skeleton help only |
| 8005C210 | `isAllPlayerGoalIn__13daPyDemoMng_cFv` | 160 | Loop-skeleton help only |
| 8005CFB0 | `executeEndToride__13daPyDemoMng_cFv` | 160 | |
| 8005C930 | `startControlDemoAll__13daPyDemoMng_cFv` | 160 | Loop-skeleton help only |
| 8005C090 | `executeGoalDemo__13daPyDemoMng_cFv` | 156 | |
| 8005D6B0 | `setEnemyStageClearDemo__13daPyDemoMng_cFi` | 148 | Loop-skeleton help only |
| 8005CB70 | `onLandStopReq__13daPyDemoMng_cFv` | 140 | Loop-skeleton help only |
| 8005CAD0 | `getControlDemoPlayerNum__13daPyDemoMng_cCFv` | 136 | Loop-skeleton help only |
| 8005C1A0 | `setGoalDemoRunCastle__13daPyDemoMng_cFv` | 112 | |
| 8005CDD0 | `isLandAll__13daPyDemoMng_cFv` | 124 | REJECTED candidate; loop-skeleton help only |
| 8005C9D0 | `isAllPlayerControlDemo__13daPyDemoMng_cFv` | 128 | REJECTED candidate; loop-skeleton help only |
| (+ 19 smaller functions, mostly trivial getters/setters under 100B) | | | |

## Honest overall precedent percentage, BY BYTES

Total unit: **8,976 B** across 51 functions.

- **Verified true/near-twin precedent covering the great majority of the
  function body**: `__ct__` (64) + `__dt__` (72) + `initStage` (52) +
  `releaseDemoMode` (52) + `endControlDemoAll` (120) + `fn_8005CE50` (120) +
  `__sinit` (112) + `__arraydtor$72504` (28) = **620 B = 6.9%**.
- **Genuine same-name AND same-body precedent** (subset of the above):
  `__ct__` + `__dt__` = **136 B = 1.5%** — this is the number to quote if
  asked "does sharing a name mean sharing a body" (answer: almost never,
  here).
- **Partial (loop-skeleton-only) intra-file precedent**: the 21
  `IDIOM-L`/`IDIOM-P` functions total 4,436 B = 49.4% of the unit, but only
  the shared ~40-48 B loop skeleton per function is actually reusable —
  treat this as "meaningfully eases 21 functions," not "49% precedented."
- **No usable precedent at all** (cross-TU noise and no intra-file idiom):
  roughly **3,548-4,000 B (≈ 40-45%)** of the unit is genuine fresh-authoring
  territory, concentrated in the four largest functions plus the trivial
  getters/setters that don't need precedent anyway.

**Bottom line: real, verified, body-level cross-TU precedent is ~1.5-6.9% of
this unit by bytes — the brief's "~7% name-level" figure is directionally
right at the name level (8.0% measured) and, as with the previous unit, an
overestimate of usable body-level leverage.** The one real lever worth
planning around is intra-file: the 4-player bitmask loop idiom, verified by
direct text inspection across 16-21 functions.

## FAMILY edits made to `tools/sibmap.py`

Added four entries (with comment explaining why): `dol_bases_d_a_player_base`,
`dol_bases_d_a_player`, `dol_bases_d_ac_py_key`, `dol_bases_d_a_player_hio_ADJ`.
All four resolve to real corpus files (confirmed via the post-edit
`check_family` stderr, which flags zero new dead entries). Did not touch or
remove the pre-existing dead `d_a_en_dpakkun` entry (unrelated to this unit;
flagged in the Verification section above rather than silently fixed, since
this map's job is the player-demo unit, not general tool hygiene).

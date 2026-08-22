# Next targets — scouting round, `wiimj2d.dol` game code

Read-only round. Nothing under `source/`, `include/`, `wip/`, `slices/` or
`syms.txt` was touched. All tooling written for this round lives in
`scratch/next_targets/`.

**Headline: three units whose bounds are not derived at all — they are BRACKETED,
exactly, in every section, by already-landed neighbours on both sides.** For
`d_a_en_obj_coinblock.cpp` all five sections are `EXACTLY ADJACENT` at both ends;
for `d_a_ice.cpp` seven of eight are; for `d_a_farBG.cpp` six of eight are. That
is the strongest bounds evidence this project has available, and it is the
condition HANDOFF's "Bounds may not need an agent at all" describes.

Combined size of the five candidates below: **70,320 bytes of `.text`**, i.e.
**2.30% of `wiimj2d.dol`** / **1.08% of the whole project** — roughly three times
what the two landed reference units (`d_a_en_bros_base` + `d_a_en_blockmain`,
24,716 B) delivered between them.

---

## 0. Method, and the one assumption everything rests on

Sources used, in the order AGENT_CONTEXT ranks them:

1. `bin/dtk/dtk_splits_wiimj2d.txt` — read first, as instructed. **It does not
   help for any of these candidates and I want that stated plainly**: it is
   generated from landed slices only, so it contains an entry for every
   *neighbour* and none for any *candidate*. Its value here is exactly the
   bracketing — the neighbours' section ends/starts — which is the same
   information `slices/wiimj2d.json` carries, cross-checked against it.
2. `bin/dtk/wiimj2d_symbols.txt` — every symbol, parsed into
   `scratch/next_targets/symmap.py`.
3. `slices/wiimj2d.json` — 144 slices; unclaimed-range inventory in
   `scratch/next_targets/gaps.py`.
4. `bin/dtkspl/obj/auto_*` — **the previous DOL scout
   (`wip/dol_scout/DOL_TARGETS.md`) states these exist only for the four `.rel`
   modules. That is wrong.** `bin/dtkspl/obj/` holds 694 objects, 235 of them
   `auto_03_<VA>_text.o` for the DOL, plus 92 `auto_07_*_data.o`, 38
   `auto_08_*_bss.o`, 28 `auto_06_*_rodata.o`, and 148 `auto_sinit__*_text.o`.
   They disassemble and they resolve cross-object references **by symbol name**,
   which gave this round a DOL ownership test (below) the earlier scout did not
   have.
5. The `.ctors` array read raw out of `original/wiimj2d.dol`
   (`symmap.read_ctors()`, 180 entries) — used as an independent ordering check.

**The assumption, stated so it can be attacked:** the linker lays translation
units down in the *same relative order in every section*. Every bounds kit in
section B depends on it. Four independent corroborations, all measured this
round:

- For `d_a_en_obj_coinblock.cpp`, the *same pair* of landed neighbours
  (`d_a_en_net_nokonoko_base.cpp` / `d_a_en_shell.cpp`) brackets it in `.text`,
  `.ctors`, `.data`, `.bss` **and** `.sdata2`, with zero slack at all ten edges.
  A coincidence of that shape is not available if the ordering assumption is
  false.
- dtk's own split objects begin **exactly** at every derived boundary:
  `auto_03_80036930_text.o`, `auto_07_803059D8_data.o`,
  `auto_08_80353AE0_bss.o` (coinblock); `auto_03_80115BD0_text.o`,
  `auto_07_80323DF8_data.o`, `auto_08_80375730_bss.o` (farBG);
  `auto_03_8011B640_text.o`, `auto_07_80324288_data.o`,
  `auto_08_803758A0_bss.o`, `auto_06_802F4F80_rodata.o` (ice).
- `d_a_farBG.cpp` and `d_a_ice.cpp` share one `.sdata` hole
  (`0x1C78-0x1CD8`). Ownership-testing each separately partitions it
  **exactly and exhaustively** at `0x1CA0`, with no bytes left over. Two
  independent measurements meeting at one address.
- The `.ctors` word each candidate's claim covers, read raw out of
  `original/wiimj2d.dol`, points at that candidate's own `__sinit`:
  `0x802EDD3C -> 0x80037750` (coinblock), `0x802EDEB8 -> 0x8011A4C0` (farBG),
  `0x802EDEC0 -> 0x80122330` (ice), `0x802EDEA4 -> 0x8010E940` (WarningManager).
  Each sits with zero slack against the neighbouring landed `.ctors` claims.

**DOL ownership test (new this round).** The REL playbook proves ownership from
the module's relocation stream; the DOL is fully linked and has none, which is
why the earlier scout fell back on naming plausibility. Instead:
`scratch/next_targets/ownership.py` disassembles the candidate's own `.text`
split objects (including its `auto_sinit__*` object, which is a separate file and
is otherwise silently missing) and marks every symbol in a bracketed data hole
`REF` / not-`REF`. It resolved three real questions below.

---

## A. Ranked candidate table

| # | unit | `.text` | size | fns | words | vtable | slots | base class | dependency risk | sibling support |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `dol/bases/d_a_en_obj_coinblock.cpp` | `0x80036930-0x80037EA0` | `0x1570` (5,488 B) | 39 | 1,372 | **none** | n/a | none — non-polymorphic | **LOW.** 8 outbound game-code calls. `dBg_c`, `dBg_ctr_c`, `daPyMng_c`, `daEnBlockMain_c`, `fManager_c`, `dActor_c` all have headers; `dPSwManager_c` and `dObjBlockMng_c` have none (one method each); `fn_80087F40` needs a `syms.txt` line. | **HIGH.** Sits between two landed, byte-exact enemy units; calls into landed `d_a_en_blockmain.cpp`. 18 landed `d_a_en_*` siblings. |
| 2 | `dol/bases/d_a_ice.cpp` | `0x8011B640-0x801233F0` | `0x7DB0` (32,176 B) | 151 | 8,044 | `0xE4` | **55** | `dActorState_c` (landed) | **LOW-MED.** No base-class blocker. ~45 outbound game-code calls, but `dBc_c`, `dBg_ctr_c`, `dIceMng_c`, `dEnCombo_c`, `dEnemyMng_c`, `dScoreMng_c`, `dQuake_c`, `dActorMng_c` **all already have headers**. Only `dIceEfMaker_c` is undeclared. | **VERY HIGH.** Both immediate neighbours are landed and share its base class: `daEnemyIce_c` `0xE0`/54 slots, `daIceBall_c` `0xE0`/54 slots — ice is those +1 virtual. 19-state `sFStateMgr_c` machine, the exact shape the state-framework lever pays on. |
| 3 | `dol/bases/d_a_farBG.cpp` | `0x80115BD0-0x8011A5B0` | `0x49E0` (18,912 B) | 55 | 4,728 | `0xD4` | **51** | `dActor_c` (landed) | **LOW-MED.** Vtable is `dActor_c`'s exact size — **zero added virtuals**. ~12 outbound game-code calls + 3 unnamed (`fn_80081BE0`, `fn_80081C40`, `fn_80089030`) needing `syms.txt`. | **MEDIUM.** Same `dActor_c` shape as every landed actor; `d_a_mask.cpp` (candidate 5) has the identical `0xD4` vtable. Not an enemy, so `sibmap.py`'s FAMILY corpus is thinner. |
| 4 | `dol/bases/d_WarningManager.cpp` | `0x8010D270-0x8010F080` | `0x1E10` (7,696 B) | 67 | 1,924 | `0x50` | **18** | `dBase_c` (landed) | **MEDIUM.** Vtable is `dBase_c`'s exact size — zero added virtuals, and `include/game/bases/d_WarningManager.hpp` **already exists**. But it calls out to **five sibling classes that are declared nowhere** (`dWarningBattery_c`, `dWarningErrorInfo_c`, `dWarningNunchuk_c`, `dWarningOther_c`, `dWarningYoKo_c`), ~8 methods each. That is 5 new shared headers, and shared-header changes have failed verification three times on this project. | **HIGH for the framework** — `sFStateMgr_c<dWarningManager_c, sStateMethodUsr_FI_c>` with 8 states, the same instantiation as the landed `d_pausewindow.cpp`. ~23 of its 67 functions are template boilerplate that a correct member declaration emits for free. |
| 5 | `dol/bases/d_a_mask.cpp` *(filename inferred — no `__sinit`)* | `0x80124EB0-0x80126650` | `0x17A0` (6,048 B) | 29 | 1,512 | `0xD4` | **51** | `dActor_c` (landed) | **LOW.** Zero added virtuals over `dActor_c`. No `.ctors` entry at all. | **MEDIUM.** Same shape as candidate 3. |

Slot counts are `(vtable size - 8) / 4`, read from `bin/dtk/wiimj2d_symbols.txt`
and confirmed against the dumped `.data` objects. Reference sizes measured the
same way: `__vt__8dActor_c = 0xD4` (51), `__vt__13dActorState_c = 0xE0` (54),
`__vt__7dBase_c = 0x50` (18), `__vt__7fBase_c = 0x4C` (17).

**The vtable-sanity check passes on all five and is informative on three:**
farBG and mask are *exactly* `dActor_c`'s slot count (no override beyond the
five they define), WarningManager is *exactly* `dBase_c`'s, and ice is
`dActorState_c` + 1. Two slots in ice's vtable point at unnamed functions
(`fn_8011C0F0` slot 27, `fn_8011C160` slot 57) — **both are inside ice's own
`.text` claim**, so neither is the `d_a_wm_kinoko_1up` failure mode.

---

## B. Bounds kits for the top 3

Section bases, read from `slices/wiimj2d.json` `meta.sections` (not recalled),
and subtracted per section. These are the same for all three kits:

| section | base subtracted |
|---|---|
| `.text` | `0x80006780` |
| `.ctors` | `0x802EDCE0` |
| `.rodata` | `0x802EDFE0` |
| `.data` | `0x802FE6A0` |
| `.bss` | `0x80351980` |
| `.sdata` | `0x80427980` |
| `.sbss` | `0x80429EA0` |
| `.sdata2` | `0x8042B360` |

`0x80004000` is `.init`'s base and is **not** used anywhere in this report.

### B1 — `dol/bases/d_a_en_obj_coinblock.cpp`

```json
{
  "source": "dol/bases/d_a_en_obj_coinblock.cpp",
  "memoryRanges": {
    ".text":   "0x301b0-0x31720",
    ".ctors":  "0x5c-0x60",
    ".data":   "0x7338-0x75a0",
    ".bss":    "0x2160-0x2358",
    ".sdata2": "0x508-0x520"
  }
}
```

No `.rodata`, `.sdata`, `.sbss`, `.sbss2` — those holes are **size 0** between
the same two neighbours, so the unit demonstrably has none.

`check_bounds.py` (see note on the shim at the end):

```
.text  0x301b0-0x31720  (0x1570 bytes)
  first: 0x0301b0 nice_search__18daEnObjCoinBlock_cFv (0xa0)
  last : 0x0316f0 finalizeState__33sFStateID_c<18daEnObjCoinBlock_c>CFR18daEnObjCoinBlock_c (0x30)
.ctors 0x5c-0x60  (0x4 bytes)   [no symbols -- dtk does not label .ctors]
.data  0x7338-0x75a0  (0x268 bytes)
  first: 0x007338 @78228 (0xc)
  last : 0x00756c __vt__33sFStateID_c<18daEnObjCoinBlock_c> (0x34)
.bss   0x2160-0x2358  (0x1f8 bytes)
  first: 0x002160 @76455 (0xc)
  last : 0x00233c l_block_bgc_info (0x1c)
.sdata2 0x508-0x520  (0x18 bytes)
  first: 0x000508 @78046 (0x4)
  last : 0x00051c @78061 (0x4)

BOUNDS PLAUSIBLE
```

Every range starts on a symbol and ends where the last symbol ends. No gaps
reported to any neighbour symbol.

Overlap-and-adjacency (`scratch/next_targets/adjacency.py`, offset space,
against `slices/wiimj2d.json`):

```
.text   below d_a_en_net_nokonoko_base.cpp ends 0x301B0 EXACTLY ADJACENT | above d_a_en_shell.cpp starts 0x31720 EXACTLY ADJACENT
.ctors  below                             ends 0x5C    EXACTLY ADJACENT | above                     starts 0x60    EXACTLY ADJACENT
.data   below                             ends 0x7338  EXACTLY ADJACENT | above                     starts 0x75A0  EXACTLY ADJACENT
.bss    below                             ends 0x2160  EXACTLY ADJACENT | above                     starts 0x2358  EXACTLY ADJACENT
.sdata2 below                             ends 0x508   EXACTLY ADJACENT | above                     starts 0x520   EXACTLY ADJACENT
OVERLAP CLEAN
```

**Ten edges, ten exact adjacencies, one pair of landed neighbours.** This is the
cleanest bracket in the DOL that I found.

Supporting facts, all measured:

- The `.data` block is 21 consecutive pool IDs `@78228…@78248` followed by
  `@78282…@78288` and terminated by `__vt__33sFStateID_c<18daEnObjCoinBlock_c>`
  — the terminal-vtable rule holds exactly at the claim end.
- Each `@78228`-family object decodes as `{0x00000000, 0xFFFFFFFF, <fn>}` — the
  **non-virtual PMF encoding**, so the states are `STATE_DEFINE`, not
  `STATE_VIRTUAL_DEFINE`. Seven states: `SerchRailID`, `SerchObjPolID`,
  `SerchWaterMoveID`, `SerchSpinLiftID`, `SerchSpinLiftChildID`, `CoinWait`,
  `BlockWait`.
- `.bss` is the seven `StateID_*__18daEnObjCoinBlock_c` (`0x30` each) with the
  `0xC` `__register_global_object` node before each, then `l_coin_bgc_info` and
  `l_block_bgc_info` (`0x1C` each). Ends exactly on the second one.
- **No vtable, no constructor, no destructor, no `create`/`execute`/`draw`
  anywhere in the symbol map for this class.** So `daEnObjCoinBlock_c` is a
  non-polymorphic helper — the same profile as `dLineMng_c` and
  `dIggyWanKusari_c`. There is **no base class to be blocked on.**
- The class name is exact from the CFront length prefix: `18` →
  `daEnObjCoinBlock_c`, 18 characters. It appears nowhere in `include/` or
  `source/`, so it must be authored from scratch.

**Confidence: HIGH.** What would raise it further: nothing about the bounds.
The one open item is `fn_80087F40` (`0xE4`, adjacent to `dBgTexMng_c` in the
un-landed `d_bg_unit.cpp` region) — it needs a `syms.txt` entry before the unit
links. `dPSwManager_c` and `dObjBlockMng_c` have no headers either; both are
called with a single method each and the mangled names give the signatures.

### B2 — `dol/bases/d_a_ice.cpp`

```json
{
  "source": "dol/bases/d_a_ice.cpp",
  "memoryRanges": {
    ".text":   "0x114ec0-0x11cc70",
    ".ctors":  "0x1e0-0x1e4",
    ".rodata": "0x6fa0-0x7020",
    ".data":   "0x25be8-0x26408",
    ".bss":    "0x23f20-0x24428",
    ".sdata":  "0x1ca0-0x1cd8",
    ".sdata2": "0x2498-0x25f0"
  }
}
```

No `.sbss`, no `.sbss2` (ownership test: zero referenced symbols in either hole).

`check_bounds.py`:

```
.text   0x114ec0-0x11cc70 (0x7db0)  first daIce_c_classInit__Fv (0x270)
                                    last  finalizeState__21sFStateID_c<7daIce_c>CFR7daIce_c (0x30)
.ctors  0x1e0-0x1e4       (0x4)     [no symbols -- dtk does not label .ctors]
.rodata 0x6fa0-0x7020     (0x80)    first l_float_speed (0x10)
                                    last  @LOCAL@initializeState_PushSink__7daIce_cFv@cs_float_speed (0xc)
.data   0x25be8-0x26408   (0x820)   first g_profile_ICE_ACTOR (0xc)
                                    last  @STRING@create__14dIceFreezeEf_cFRC7mVec3_c (0x10)
  >>> .data begins at a PROFILE symbol ... very likely 0x34 too high
.bss    0x23f20-0x24428   (0x508)   first l_mdl_defsize (0x48)
                                    last  StateID_Melt_Normal__7daIce_c (0x30)
.sdata  0x1ca0-0x1cd8     (0x38)    first @75701 (0x7)   last @81933 (0x4)
.sdata2 0x2498-0x25f0     (0x158)   first @81717 (0x4)   last @83628 (0x4)

1 problem(s) -- do not build this
```

**That one "problem" is a FALSE POSITIVE of the `wm`-family rule, and I can
disprove it two ways.** The rule fires because `.data` opens on a `g_profile_*`
symbol; it exists because a `d_basesNP` `wm` actor opens on its two anonymous
`sc_ForceList` strings, `0x34` earlier.

1. **Six landed, byte-exact DOL actors open `.data` exactly at their profile:**
   `d_a_cursor.cpp` → `g_profile_CURSOR`, `d_a_en_eatcoin.cpp` →
   `g_profile_EN_EATCOIN`, `d_a_en_hatena_balloon.cpp` →
   `g_profile_EN_HATENA_BALLOON`, `d_a_enemy_ice.cpp` → `g_profile_ENEMY_ICE`,
   `d_a_fireball_player.cpp` → `g_profile_PL_FIREBALL`, `d_a_iceball.cpp` →
   `g_profile_ICEBALL`. The DOL convention is the opposite of the `wm` one.
2. **There is no room to open `0x34` earlier.** `d_a_fireball_player.cpp`'s
   landed `.data` claim *ends* at `0x25BE8`, which is `g_profile_ICE_ACTOR`'s
   address. Opening earlier overlaps a landed slice.

Recommendation to the lead: `check_bounds.py`'s two `wm`-family rules should be
gated on the module, or at least on `d_basesNP`. As written they will fire on
every DOL actor and train people to ignore the output.

Overlap-and-adjacency:

```
.text   below d_a_fireball_player.cpp ends 0x114EC0 EXACT | above d_a_iceball.cpp starts 0x11CC70 EXACT
.ctors  below                        ends 0x1E0    EXACT | above                  starts 0x1E4    EXACT
.rodata below                        ends 0x6FA0   EXACT | above                  starts 0x7020   EXACT
.data   below                        ends 0x25BE8  EXACT | above                  starts 0x26408  EXACT
.bss    below                        ends 0x23F20  EXACT | above                  starts 0x24428  EXACT
.sdata  below d_a_enemy_ice.cpp      ends 0x1C78   gap 0x28 | above d_last_actor.cpp starts 0x1CD8 EXACT
.sdata2 below d_a_fireball_player.cpp ends 0x2498  EXACT | above d_a_iceball.cpp starts 0x25F0   EXACT
OVERLAP CLEAN
```

Twelve of fourteen edges exact. **The one gap is explained and is not slack:**
the `0x28` below `.sdata` is precisely `d_a_farBG.cpp`'s `.sdata` claim
(`0x1C78-0x1CA0`, candidate 3). The two units partition that hole exactly.

Supporting facts:

- `.rodata`: all six symbols are referenced from ice's own `.text`
  (`l_float_speed`, `l_toge_float_speed`, `l_attach_float_speed`, `l_ice_cc`,
  and two `@LOCAL@…daIce_c…` function statics). Zero foreign symbols.
- `.data` opens on `g_profile_ICE_ACTOR` and ends on a `@STRING@` object for
  `dIceFreezeEf_c::create` — the last of eight small `dIce*Ef_c` effect helper
  classes that live in this TU (`dIceWaterBreakEf_c`, `dIcePoisonEf_c`,
  `dIceYoganEf_c`, `dIceThawEf_c`, `dIceReleaseEf_c`, `dIceBreakEf_c`,
  `dIceSmokeEf_c`, `dIceFreezeEf_c`), each a `create`/`follow` pair at the tail
  of `.text`. Budget for those: they are part of the unit.
- `.bss` is 19 `StateID_*__7daIce_c` objects at `0x30` with `0xC` register nodes
  interleaved, led by `l_mdl_defsize` (`0x48`).
- `.sdata`'s six `@75701…@75706` (7 bytes each) are **not** referenced from
  ice's `.text`. They are attributed to ice on **consecutive pool IDs** — ice's
  `.bss` register-nodes run `@75598…@75670`, so `@75701…@75706` is the same
  numeric band, and farBG's band is `@74779`/`@78778…@79790`. Combined with the
  exact partition of the hole at `0x1CA0`, I am confident, but **this specific
  sub-range is the weakest link in the kit** and it is 0x30 bytes.

**Confidence: HIGH on `.text`/`.ctors`/`.rodata`/`.data`/`.bss`/`.sdata2`
(all exact-adjacent between the same landed pair); MEDIUM on `.sdata`.**
What would raise the `.sdata` claim to HIGH: decoding the six 7-byte strings out
of the DOL image and matching them to a `daIce_c` construct (they are almost
certainly short animation or effect names), or finding the `lis`/`addi` pair that
reaches them by base+displacement, the way farBG's format strings turned out to
work (see B3).

### B3 — `dol/bases/d_a_farBG.cpp`

```json
{
  "source": "dol/bases/d_a_farBG.cpp",
  "memoryRanges": {
    ".text":   "0x10f450-0x113e30",
    ".ctors":  "0x1d8-0x1dc",
    ".rodata": "0x6f18-0x6f28",
    ".data":   "0x25758-0x259c0",
    ".bss":    "0x23db0-0x23ec0",
    ".sdata":  "0x1c78-0x1ca0",
    ".sbss":   "0x738-0x748",
    ".sdata2": "0x23e0-0x2460"
  }
}
```

No `.sbss2` (the one symbol in that hole is
`@LOCAL@createLayout__14dGameDisplay_cFv@…`, i.e. `d_gamedisplay.cpp`'s).

`check_bounds.py`:

```
.text   0x10f450-0x113e30 (0x49e0)  first __ct__13daFarBG_HIO_cFv (0x188)
                                    last  __arraydtor$74778 (0x1c)
.ctors  0x1d8-0x1dc       (0x4)     [no symbols -- dtk does not label .ctors]
.rodata 0x6f18-0x6f28     (0x10)    first/last @LOCAL@GetZoomMagnif__9daFarBG_cFv@l_zoom_magnif_table (0x10)
.data   0x25758-0x259c0   (0x268)   first g_profile_FAR_BG (0xc)
                                    last  __vt__Q29daFarBG_c14nodeCallback_c (0x18)
  >>> .data begins at a PROFILE symbol ... very likely 0x34 too high
.bss    0x23db0-0x23ec0   (0x110)   first @74779 (0xc)  last m_HIO__9daFarBG_c (0x100)
.sdata  0x1c78-0x1ca0     (0x28)    first l_TestScale (0x4)  last @79662 (0x7)
.sbss   0x738-0x748       (0x10)    first c_PIC_WIDTH__9daFarBG_c  last c_PIC_HEIGHT_HALF__9daFarBG_c
.sdata2 0x23e0-0x2460     (0x80)    first @78525 (0x4)  last @80247 (0x4)

1 problem(s) -- do not build this
```

Same false positive, same two disproofs as B2 — `d_a_enemy_ice.cpp`'s landed
`.data` ends at `0x25758`, which *is* `g_profile_FAR_BG`.

Note the `.text` end. `__sinit_\d_a_farBG_cpp` is at VA `0x8011A4C0` (`0x88`) and
ends at `0x8011A548`; **two more functions follow it inside the unit** —
`__dt__13daFarBG_HIO_cFv` at `0x8011A550` (`0x40`) and `__arraydtor$74778` at
`0x8011A590` (`0x1C`) — before the claim closes at `0x8011A5B0` (four bytes of
alignment padding). This is exactly the recorded rule: a unit's `.text` does not
end at its `__sinit`, it ends after its own array destructor. A claim stopped at
`__sinit`'s end would be `0x68` short. (`check_bounds.py`'s `wm` rule for this
looks at the *next* symbol being `0x1c`; here the `0x1c` symbol is correctly
*inside* the claim, so it does not fire, correctly.)

Overlap-and-adjacency:

```
.text   below d_a_enemy_ice.cpp        ends 0x10F450 EXACT | above d_a_fireball_player.cpp starts 0x113E30 EXACT
.ctors  below                          ends 0x1D8    EXACT | above                          starts 0x1DC   EXACT
.rodata below d_a_en_hatena_balloon.cpp ends 0x6F18  EXACT | above d_a_fireball_player.cpp starts 0x6F28  EXACT
.data   below d_a_enemy_ice.cpp        ends 0x25758  EXACT | above                          starts 0x259C0 EXACT
.bss    below                          ends 0x23DB0  EXACT | above                          starts 0x23EC0 EXACT
.sdata  below                          ends 0x1C78   EXACT | above d_last_actor.cpp starts 0x1CD8 gap 0x38
.sbss   below d_a_cursor.cpp           ends 0x738    EXACT | above d_a_player.cpp   starts 0x750  gap 0x8
.sdata2 below d_a_enemy_ice.cpp        ends 0x23E0   EXACT | above d_a_fireball_player.cpp starts 0x2460  EXACT
OVERLAP CLEAN
```

Fourteen of sixteen edges exact. **Both gaps are explained and neither is
slack:**

- the `0x38` above `.sdata` is exactly `d_a_ice.cpp`'s `.sdata` claim (B2);
- the `0x8` above `.sbss` is two `@GUARD@…calcDarkHandLight__8daMask_cFv…` bytes
  plus alignment. I checked whether that breaks the ordering assumption: it does
  not — `calcDarkHandLight__8daMask_cFv` is at `.text:0x80125410`, i.e. inside
  candidate 5's range, which is *after* farBG. Consistent.

**Two findings on this unit that change what has to be authored:**

1. **`.data` ends with `0xE8` of zeros** (`gap_07_80323F78_data`, `0x80323F78`
   → `0x80324060`) after the last vtable. That is a genuine part of the claim —
   the next landed slice starts at `0x80324060` and no other TU sits in this
   `.text` gap. It is the shape HANDOFF records as *"runtime-constructed statics
   have a zero static image"*: `__ct__Q29daFarBG_c8bgData_tFv` exists in `.text`,
   `__arraydtor$74778` closes `.text`, and `@74779` (`0xC`) in `.bss` is the
   `__register_global_object` bookkeeping node. So the source has a **file-scope
   array of `daFarBG_c::bgData_t` with a non-trivial constructor**, `0xE8` bytes
   of it. This will not fall out of writing the methods; it has to be
   deliberately reconstructed, and it is the main authoring risk on this unit.
2. **Four `.data` strings that the ownership test first reported as
   unreferenced are in fact referenced by base+displacement off the profile.**
   `@78778 = "_Agb%04X"`, `@78779 = "g3d/_Agb%04X.brr"`, `@78780 = "_Bgb%04X"`,
   `@78781 = "g3d/_Bgb%04X.brr"` sit at `+0xC`, `+0x18`, `+0x2C`, `+0x38` from
   `g_profile_FAR_BG`, and `GetRes__9daFarBG_cFUsPcPcPc` reaches them with
   `addi r31, r31, g_profile_FAR_BG@l` then `addi r4, r31, 0xc / 0x18 / 0x2c /
   0x38` before each `bl sprintf`. **Ownership is proven, not inferred** — and
   the layout is load-bearing: those four strings must be emitted immediately
   after the profile, in that order, at those exact offsets.

**Confidence: HIGH on all eight ranges.** What would raise the *authoring*
confidence (not the bounds): identifying what fills the `0xE8` zero tail before
anyone starts, by comparing constructor offsets the way the sandpillar
`sizeof`-gap technique does.

---

## C. Ruled out, and why

Labelled `[elimination]` where that is all the evidence is.

**Already in flight — do not collide:**

- `d_line_mng.cpp` (`0x7BE0`) — lead's unit, 181/182, blocked on the
  `smc_UNIT_SIZE_X` static-const-float trap.
- `d_enemy_toride_kokoopa.cpp` (`0x8310`) — Gemini, rounds 20-22.
- `d_bg_actor_mng.cpp` / `d_bg_ctr.cpp` — Qwen, rounds 23-24.
- `d_a_player_manager.cpp` (`0x2A10`), `m_pad.cpp` (`0x1790`) — parked drafts in
  `wip/`.

**Hard dependency blockers, measured:**

- `d_enemy_jr_clown_base.cpp` (`0x800A2870-0x800A7DA0`). `__vt__16dEnJrClownBase_c`
  is `0x3B4` (235 slots) against `__vt__9dEnBoss_c` `0x390` (226). `dEnBoss_c` is
  declared **nowhere** in `include/` or `source/` and lives in a different
  unclaimed gap (`0x91BD0-0x98370`). Identical failure mode to
  `d_a_wm_kinoko_1up.cpp`. Also: that gap holds **at least three** TUs
  (`dEnFumiCheck_c`/`dEnFumiProc_c` lead it; `dEnemyMng_c`, `0x950`, closes it),
  so the bounds are not free either.
- `d_hana_body.cpp` — five polymorphic Hana classes with `0x4C` vtables and an
  unverified base; the earlier scout flagged this and I did not clear it.

**Bounds not free — the gap holds more than one TU:**

- `0x80007510-0x8000FBF0` (`0x86E0`, `d_3d.cpp`). Largest single-`__sinit` gap in
  DOL game code, but the tail (`0x8000F270-0x8000FBF0`) is
  `dProcShareProc_c` / `dScnGroupShareProc_c` / `dRailScnGroup_c` with no sinit
  to anchor the split. `[elimination]` would be the only tool for that boundary.
- `0x800F9380-0x800FD8F0` (`0x4570`, `d_wm_lib.cpp`). The `dWmLib` namespace runs
  cleanly to `__sinit` at `0x800FD610`, but `dWmMaterial_c` (4 functions,
  `0x250`) follows it and is *not* an array destructor, so it is a second TU with
  no anchor. `d_wm_lib.cpp` would be `0x800F9380-0x800FD6A0` `[elimination]`.
- `0x80069020-0x8006C420` (`0x3400`, `d_audio.cpp`). At least four TUs —
  `dHeapAllocator_c`, `dAttention_c`, `dAudio`+`NM*SndObject` templates, then
  `dBalloonMng_c` and `dAcPy_c` after the sinit.
- `0x8006CF40-0x8008C200` (`0x1F2C0`) and `0x800A8710-0x800C89A0` (`0x20290`) —
  the two largest game-code gaps, 5 and 11 TUs respectively, and both already
  have work in them.
- `0x800D03C0-0x800D91B0` (`0x8DF0`, 4 sinits), `0x800E8410-0x800F2820`
  (`0xA410`, 4 sinits), `0x800FDC40-0x8010C130` (`0xE4F0`, **15** sinits),
  `0x8014A830-0x8015A480` (`0xFC50`, 3 sinits) — all multi-TU, all would need a
  per-boundary derivation round before anything could be authored.

**Deprioritised for a reason already recorded:**

- Every `lib/` gap, including the three largest in the binary
  (`0x2229F0-0x2B3C90` = `0x912A0`, `0x1CE600-0x222790` = `0x54190`,
  `0x2B4F40-0x2D62F0` = `0x213B0`). SDK code, the `GXSetTevColor` /
  `GXGetViewportv` register-allocation wall, and the measured 19:1 throughput
  argument for DOL game code.
- `RIVER`, `CASTLE_BG`, and the five order-blocked `d_basesNP` units.

**Looked at and passed over, but worth keeping on the list** (single-TU gaps,
bounds probably as free as the top three, just smaller):
`d_actor_groupid_mng.cpp` (`0xEF0`), `d_a_en_coin_main.cpp` (`0xED0`),
`d_tencoin_mng.cpp` (`0xC70`), `d_wm_connect.cpp` (`0x970`),
`d_message.cpp` (`0x450`), and the four unnamed single-TU gaps at
`0x800CED00-0x800CFCE0` (`0xFE0`), `0x800DF950-0x800E1AA0` (`0x2150`),
`0x800451F0-0x800460D0` (`0xEE0`), `0x80014330-0x80014F10` (`0xBE0`).

---

## D. Tooling written this round (all in `scratch/next_targets/`)

| file | what it does |
|---|---|
| `symmap.py` | parses `wiimj2d_symbols.txt`, `slices/wiimj2d.json`, and reads the raw `.ctors` array out of `original/wiimj2d.dol` |
| `gaps.py` | every unclaimed range per section, with the `__sinit` anchors inside it |
| `carve.py` | carves a gap into class-prefix runs + `__sinit` anchors |
| `bracket.py` | the bracketing landed slice in **every** section for a candidate `.text` range |
| `ownership.py` | the DOL ownership test — disassembles the candidate's split objects (incl. its `auto_sinit__*`) and marks hole symbols `REF` / not |
| `adjacency.py` | AGENT_CONTEXT §6 check 1, offset space, prints the base subtracted per section |
| `mkshim.py` | builds `scratch/next_targets/{bin/dtk,slices}` with DOL symbol addresses converted to **section-relative offsets**, and copies `check_bounds.py` to `scratch/next_targets/shim/wm_units/` so it resolves `ROOT` to the scratch tree |

**Why the shim.** `wip/wm_units/check_bounds.py` compares a claim (offsets)
against `bin/dtk/<module>_symbols.txt`. For a REL those symbols *are* offsets;
for the DOL they are VAs, so run against `wiimj2d` directly it reports
`no symbols in range` on every section and then prints `BOUNDS PLAUSIBLE` — a
**silent pass on an unvalidated claim**, which is exactly the failure class the
tool exists to prevent. The shim converts the addresses and leaves the tool
itself byte-identical. Command actually run:

```
python scratch/next_targets/mkshim.py
python scratch/next_targets/shim/wm_units/check_bounds.py wiimj2ddol '<slice JSON>'
```

Two things the lead should consider landing:

1. Make `check_bounds.py` DOL-aware (subtract `meta.sections[...].addr` when the
   module is `wiimj2d`), so nobody gets the silent pass.
2. Gate the two `wm`-family `.data` rules on the module. They fire on every DOL
   actor, and six landed byte-exact DOL units prove the opposite convention.

Note also that `check_bounds.py`'s **ownership check never ran** on any of these:
it reads `original/<module>.rel`, which does not exist for the DOL. That half of
the tool is silently inert here, and `ownership.py` above is the replacement.

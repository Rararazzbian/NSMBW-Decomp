# DOL scouting: the d_enemy_state.cpp -> d_lytbase.cpp gap

Gap: `.text` `0x800A8710`-`0x800C89A0` (offsets `0xA1F90`-`0xC2220` from `.text`
base `0x80006780`), size `0x20290` (131728 bytes). Confirmed unclaimed: no
`slices/wiimj2d.json` entry overlaps it, and `bin/dtk/dtk_splits_wiimj2d.txt`
independently brackets it exactly -- `d_enemy_state.cpp` `.text end:0x800A8710`,
`d_lytbase.cpp` `.text start:0x800C89A0`. Both bounds are therefore hard,
non-inferred facts.

Method note: this gap has **no** entry in `bin/dtkspl/obj/` (that directory only
holds split objects for the four `.rel` modules, not the DOL), so per-TU object
disassembly the way `HANDOFF.md`/`wm_units` tooling does it for RELs is not
available here. Everything below is derived from three sources instead:
`bin/dtk/wiimj2d_symbols.txt` (every symbol, section, address, size),
`__sinit_\<file>_cpp` symbols (which name the real source file directly), and
the raw `.ctors` function-pointer array read straight out of `original/wiimj2d.dol`
(file offset `0x2E9DE0`, VA base `0x802EDCE0` -- the DOL is a fully-linked image,
so `.ctors` entries are already-resolved absolute addresses, not relocations).
`wip/wm_units/scout_unit.py` was read but not run: it walks a REL's relocation
stream, and the DOL has no relocation stream to walk (it is position-fixed), so
its method does not transfer as-is.

## Finding that contradicts the brief

The brief's premise -- "eighteen `d_a_en_*` units are already landed... this
region is very likely more of the same family" -- **does not hold for this
specific gap**, and I want to flag that plainly rather than force-fit it.
Of the ~23 translation units the gap carves into, exactly **one** is a
`d_a_en_*`-shaped enemy actor (`d_enemy_toride_kokoopa.cpp`), and it is
**blocked** (see below). Everything else is non-enemy support/utility
material: a fader, a flag controller, a font manager, a speech-bubble
("fukidashi") widget, a grab-bag "game common" file, game-pad input cores, a
decorative multi-part flower prop, ice-effect/ice-parameter helpers, Iggy
Koopa's swinging chain, a world/stage info singleton, several "lift" (moving
platform) draw helpers, a light class, and a large stand-alone line/rail
movement-math manager. The `d_a_en_*` shelf of landed siblings is a much
weaker guide here than the brief assumed; the better guide turned out to be
the **state-machine framework** siblings (`d_pausewindow.cpp`,
`d_a_spin_child_base.cpp`), which is a different resemblance axis than the one
suggested.

## The carve

All addresses below are VAs; the offset-from-base column subtracts `.text`
base `0x80006780` (matching the convention in `slices/wiimj2d.json`). Ordered
by address, contiguous and non-overlapping -- the 23 ranges sum to exactly
`0x20290`, the full gap, with no leftover bytes.

| # | source (my inferred name) | `.text` VA range | `.text` offset range | size | has own `.ctors`/sinit? |
|---|---|---|---|---|---|
| 1 | `d_enemy_toride_kokoopa.cpp` | `0x800A8710`-`0x800B0A20` | `0xA1F90`-`0xAA2A0` | `0x8310` (33552) | YES |
| 2 | `d_fader.cpp` @unofficial name | `0x800B0A20`-`0x800B0EA0` | `0xAA2A0`-`0xAA720` | `0x480` (1152) | no |
| 3 | `d_flag_ctrl.cpp` @unofficial name | `0x800B0EA0`-`0x800B11D0` | `0xAA720`-`0xAAA50` | `0x330` (816) | no |
| 4 | `d_font_manager.cpp` | `0x800B11D0`-`0x800B14E0` | `0xAAA50`-`0xAAD60` | `0x310` (784) | YES |
| 5 | `d_fukidashiInfo.cpp` | `0x800B14E0`-`0x800B2D20` | `0xAAD60`-`0xAC5A0` | `0x1840` (6208) | YES |
| 6 | `d_game_common.cpp` (dGameCom) | `0x800B2D20`-`0x800B5930` | `0xAC5A0`-`0xAF1B0` | `0x2C10` (11280) | YES |
| 7 | `d_game_key.cpp` @unofficial name | `0x800B5930`-`0x800B64A0` | `0xAF1B0`-`0xAFD20` | `0xB70` (2928) | no |
| 8 | `d_graph.cpp` @unofficial name | `0x800B64A0`-`0x800B6570` | `0xAFD20`-`0xAFDF0` | `0xD0` (208) | no |
| 9 | `d_hana_body.cpp` | `0x800B6570`-`0x800B8130` | `0xAFDF0`-`0xB19B0` | `0x1BC0` (7104) | YES |
| 10 | `d_ice_effect_maker.cpp` | `0x800B8130`-`0x800B8490` | `0xB19B0`-`0xB1D10` | `0x360` (864) | YES |
| 11 | `d_ice_param.cpp` (class is `dIceMng_c`) | `0x800B8490`-`0x800B90A0` | `0xB1D10`-`0xB2920` | `0xC10` (3088) | YES |
| 12 | `d_iggy_wan_kusari.cpp` | `0x800B90A0`-`0x800BB0E0` | `0xB2920`-`0xB4960` | `0x2040` (8256) | YES |
| 13 | `d_info.cpp` | `0x800BB0E0`-`0x800BBD80` | `0xB4960`-`0xB5600` | `0xCA0` (3232) | YES |
| 14 | `d_kadr.cpp` @unofficial name | `0x800BBD80`-`0x800BC4B0` | `0xB5600`-`0xB5D30` | `0x730` (1840) | no |
| 15 | `d_kinoko_kasa_draw.cpp` @unofficial name | `0x800BC4B0`-`0x800BC9A0` | `0xB5D30`-`0xB6220` | `0x4F0` (1264) | no |
| 16 | `d_kinoko_lift_dr.cpp` @unofficial name | `0x800BC9A0`-`0x800BCBD0` | `0xB6220`-`0xB6450` | `0x230` (560) | no |
| 17 | `d_kinopio_mdl.cpp` @unofficial name | `0x800BCBD0`-`0x800BD7F0` | `0xB6450`-`0xB7070` | `0xC20` (3104) | no |
| 18 | `d_lift_allhit_draw.cpp` @unofficial name | `0x800BD7F0`-`0x800BFDD0` | `0xB7070`-`0xB9650` | `0x25E0` (9696) | no |
| 19 | `d_lift_allhit_draw2.cpp` | `0x800BFDD0`-`0x800C0060` | `0xB9650`-`0xB98E0` | `0x290` (656) | YES |
| 20 | `d_lift_down_on_normal_draw.cpp` @unofficial name | `0x800C0060`-`0x800C0360` | `0xB98E0`-`0xB9BE0` | `0x300` (768) | no |
| 21 | `d_lift_normal_model_draw.cpp` @unofficial name | `0x800C0360`-`0x800C0B30` | `0xB9BE0`-`0xBA3B0` | `0x7D0` (2000) | no |
| 22 | `d_light.cpp` @unofficial name | `0x800C0B30`-`0x800C0DC0` | `0xBA3B0`-`0xBA640` | `0x290` (656) | no |
| 23 | `d_line_mng.cpp` | `0x800C0DC0`-`0x800C89A0` | `0xBA640`-`0xC2220` | `0x7BE0` (31712) | YES |

**Confidence on the boundaries**, per item:
- **HIGH** (named `__sinit`/`.ctors` anchor sits exactly at the transition, or
  the class-prefix change is unambiguous): #1 start/end, #4, #5, #6 start, #9,
  #10, #11, #12, #13, #19, #23 (both ends, cross-checked against
  `d_lytbase.cpp`'s `.ctors` claim -- see below).
- **MEDIUM** (clean class-name transition, no framework noise, but no sinit to
  anchor it): #2/#3 boundary, #7/#8 boundary, #14-#18, #20-#22.
- **LOW / explicitly uncertain**: the #1/#2 boundary (see "FumiCheck cluster"
  below) and whether `dFunsuiAct_c::posMove` at the front of #6 is really part
  of `d_game_common.cpp` or its own one-function TU (see open questions).

### The `.ctors` cross-check (strong corroborating evidence)

I read the raw `.ctors` array directly out of `original/wiimj2d.dol`
(`.ctors` VA base `0x802EDCE0`, file offset `0x2E9DE0`, 182 four-byte
entries) rather than trusting the symbol table's naming alone. Indices 60-70
are, **in address order, with zero gaps**:

```
60  0x802EDDD0 -> 0x800AED40  d_enemy_toride_kokoopa_cpp
61  0x802EDDD4 -> 0x800B1490  d_font_manager_cpp
62  0x802EDDD8 -> 0x800B28F0  d_fukidashiInfo_cpp
63  0x802EDDDC -> 0x800B5890  d_game_common_cpp
64  0x802EDDE0 -> 0x800B8110  d_hana_body_cpp
65  0x802EDDE4 -> 0x800B8390  d_ice_effect_maker_cpp
66  0x802EDDE8 -> 0x800B8FF0  d_ice_param_cpp
67  0x802EDDEC -> 0x800BA630  d_iggy_wan_kusari_cpp
68  0x802EDDF0 -> 0x800BBCE0  d_info_cpp
69  0x802EDDF4 -> 0x800BFEA0  d_lift_allhit_draw2_cpp
70  0x802EDDF8 -> 0x800C7600  d_line_mng_cpp
```

This is offset-relative `.ctors` `0xF0`-`0x11C` (i.e. `0xF0,0xF4,...,0x118`
each 4 bytes, base `0x802EDCE0`). Slot 59 (just before) belongs to
`d_enemy_state.cpp`, whose slice entry already claims `.ctors` `0xec-0xf0` --
**exactly adjacent**, no gap. Slot 71 would be next; `d_lytbase.cpp`'s slice
entry claims `.ctors` `0x11c-0x120` -- **exactly adjacent** to slot 70's
`0x118-0x11c`. Both ends of this eleven-entry run bracket perfectly against
already-landed neighbours. This confirms (a) the 11 sinit-bearing TUs are in
this exact relative order and none are missing between them, and (b) my overall
gap bound is right, independent of the `.text`-only evidence.

## Item 1: the blocked giant -- `d_enemy_toride_kokoopa.cpp`

`dEnTorideKokoopa_c` (a boss-shell Koopa riding a fortress cart -- "Toride" =
fortress; this is a mid-world tower-boss enemy). 33552 bytes, ~26 states
(Jump, BigJump, LandOn, AttackReady/Begin/Search/Attack/End, FumiHit, FireHit,
StarHit, SlideHit, QuakeHit, ShellHit, ShellAtk_St/ShellAtk/ShellOut,
DieFumi_St, DemoAwake(_Wait), DemoIkaku(_Wait), DemoEscape_St). Its own
`__sinit` alone is `0x1698` (5784) bytes -- consistent with ~26
`sFStateID_c`-style static objects needing construction.

**Blocker, measured not inferred**: `__vt__18dEnTorideKokoopa_c` is `0x5E4`
bytes = `(0x5E4-8)/4` = **375 virtual slots**. Compare `__vt__9dEnBoss_c` =
`0x390` bytes = **226 slots**. `dEnBoss_c` is a real, substantial class (its
own ctor/dtor/create/many virtuals live at `.text` `0x800983C0`+) but it is
**not declared anywhere in `include/` or `source/`** -- `grep -rl "dEnBoss_c"
include/ source/` returns nothing. Its address (`0x800983C0`, offset `0x91C40`)
falls in a **second, separate unclaimed gap**: `d_enemy.cpp` ends `0x91BD0`,
`d_enemy_carry.cpp` starts `0x98370` -- a `0x66C0` (26304-byte) hole that is
not part of the gap this task scoped me to, and that the task's framing did not
mention. Corroborating evidence for the inheritance claim: inside
`d_enemy_toride_kokoopa.cpp`'s own address range there are three weak
`baseID_*<9dEnBoss_c>` template instantiations
(`baseID_DemoWait<9dEnBoss_c>`, `baseID_DieShell<9dEnBoss_c>`,
`baseID_DieFire<9dEnBoss_c>`, each 12 bytes, at `0x800B04A0`-`0x800B04CC`) --
these are `dEnBoss_c`'s OWN state-ID accessors, placed here only because
TorideKokoopa's TU is the first (and, in this gap, only) place that
instantiates them, exactly the "weak dedup, first user wins placement" pattern
`AGENT_CONTEXT.md` documents. 375 > 226 by 149 slots, consistent with
TorideKokoopa adding its own ~26-state virtual table on top of a `dEnBoss_c`
base.

This is the same shape as the `d_a_wm_kinoko_1up.cpp` blocker already
documented in `AGENT_CONTEXT.md`: an out-of-line base class in an un-decompiled
TU. **I am not recommending this unit first** despite its size, because
authoring it correctly requires either decompiling `dEnBoss_c` first (a whole
separate ~26KB unit in a different gap) or accepting an unverifiable vtable.

`.ctors`: owns slot 60 (`0xF0-0xF4`). `.data`: terminal object is its own
vtable at `0x80314360` (`0x5E4`), preceded by `__vt__33sFStateID_c<...>` and
`__vt__40sFStateVirtualID_c<...>` (both `0x34`) at `0x80315230`/`0x80315264`
-- wait, note these two per-TU template vtables are at a LOWER `.data` address
than the main vtable, meaning `.data` for this TU is not one contiguous run in
address order the way `.text` is; I did not fully map its `.data`/`.bss` span,
see open questions. `.bss`: 26 `StateID_*__18dEnTorideKokoopa_c` objects,
`0x34` bytes each, `0x80358444`-`0x80358B38`.

**Tail attribution, low confidence**: immediately after TorideKokoopa's
trailing weak template symbols (ending `0x800B07B0`), there is a small
cluster -- `FumiCcInfo_c::getFumiRev` (`0x50` bytes), `MugenComboFumiCheck_c::
operate`/`~MugenComboFumiCheck_c` (both global, not weak), and
`KokoopaSpFumiCheck_c::operate`/`~KokoopaSpFumiCheck_c` (also global) -- ending
at `0x800B0A20` where `dFader_c` begins. `KokoopaSpFumiCheck_c` is not declared
in any header and is Kokoopa-specific by name; `MugenComboFumiCheck_c` **is**
declared in `include/game/bases/d_en_fumi_check.hpp` (ctor/dtor/vtable inline
= weak there) but its `operate()` body is only declared, not defined, in that
header -- meaning this cluster is the ODR-unique real definition, and
`d_a_en_shell.cpp:1049` (`new MugenComboFumiCheck_c()`) is a *user* of it, not
its owner. I attribute this 0x270-byte cluster to `d_enemy_toride_kokoopa.cpp`'s
tail (no sinit boundary interrupts it, and `KokoopaSpFumiCheck_c`'s naming
makes a locally-scoped class inside that file plausible), but I did not
independently confirm this via `.data` vtable pool adjacency the way
`AGENT_CONTEXT.md`'s "consecutive `@NNNNN` pool IDs" technique would -- **this
is an inference, not a measurement.** Also worth flagging: `FumiCcInfo_c::
getFumiRev()` is not declared in the current `d_en_fumi_check.hpp` at all --
the header will need that method added before this unit can compile, which
is a shared-header change and out of scope for this read-only round.

## Item 12 (recommended #2): `d_iggy_wan_kusari.cpp`

Iggy Koopa's swinging wrecking-ball chain. Two classes: `dIggyWanKusari_c`
(the chain/manager, `create(int)`, `allocate`, `execute`, `draw`, `remove`,
`make_kusari`, states Ready/Normal/Tight/Release/Collapse/Dead, all six states
present with non-trivial bodies) and `dIggyWanKusariPiece_c` (each visible
link: `createMdl`, `calcMdl`, `draw`, `calcPosAngle`, `collapseMove`,
`setCollapseSpeed`). Combined `0x2040` (8256) bytes.

**Structurally clean, measured**: `grep "__vt__.*dIggyWanKusari"` finds
**no** `__vt__16dIggyWanKusari_c` and **no** `__vt__21dIggyWanKusariPiece_c` --
neither class has its own vtable at all (only the framework's
`__vt__31sFStateID_c<16dIggyWanKusari_c>` template vtable exists, at
`0x80315DD0`, `0x34` bytes). Zero virtual functions means **zero risk of a
missing-base-class vtable blocker** -- there is no base class to depend on in
the first place. This is the opposite risk profile from item 1.

`.ctors`: owns slot 67 (`0x10C-0x110`). `.bss`: six `StateID_*
__16dIggyWanKusari_c` objects (`0x30` each), `0x80358ED8`-`0x80358FF8` --
`Tight`/`Release`/`Collapse` entries show no `data:4byte` annotation the
others have, unexplained, worth checking when authored but likely just a
symbol-table annotation quirk, not a size difference (all are `0x30`).
`.sdata`: one 8-byte model-name pointer local to `createMdl`.
`.sbss`: one 2-byte `smc_ANGLE_DIST_RATE`. `.sdata2`: two locals
(`cs_init_angle`, `4` bytes; `smc_LENGTH`, `4` bytes) plus an 8-byte
`cs_dir_prm` local to `setCollapseSpeed`. All small, all named, nothing
anonymous/pool-shaped to chase.

**Sibling**: `source/dol/bases/d_a_spin_child_base.cpp` (landed, 131 lines) is
the closest match I found for the *idiom* -- it drives its state machine via
`STATE_DEFINE(daSpinChildBase_c, ...)` and an `sFStateMgr_c`-style
`mStateMgr.changeState(...)`/`.executeState()` member, same as
`dIggyWanKusari_c`'s `sFStateID_c<16dIggyWanKusari_c>` usage. It is not a
byte-for-byte structural twin (spin_child_base is a real `dEn_c` actor with a
handful of states; IggyWanKusari is a non-actor manager with six), but it
demonstrates the same state-macro idiom already working in this codebase.

## Item 23 (recommended #1): `d_line_mng.cpp`

`dLineMng_c` -- a line/rail movement-math manager: 27 direction/shape states
(Idle, FallDown, Left45/Right45, Side, Height, CornerHeightLine,
CornerSideLine, Left30Left/Right, Right30Left/Right, Left60Up/Down,
Right60Up/Down, Circle, Circle2x2×4, Circle4x4×4), plus a large family of
free geometry-check functions (`line_cross_chk1..lineF_cross_chk`,
`circle_ul2_cross_chk` etc., `mov_to_right/leftupper/lower`,
`move_on_circle1..4`). `0x7BE0` (31712) bytes -- **the single largest
candidate in the gap**, bigger than the two landed reference units
(`d_a_en_bros_base.cpp` + `d_a_en_blockmain.cpp`, 24716 bytes) combined.

**Structurally clean, measured**: same check as above -- no
`__vt__10dLineMng_c` anywhere. Zero virtuals, zero base-class dependency risk.
It is the **last** TU in the gap, and its right edge is the strongest-anchored
boundary in this whole carve: `.ctors` slot 70 (`0x118-0x11C`) sits directly
against `d_lytbase.cpp`'s already-landed `.ctors` claim (`0x11c-0x120`) with
zero slack, and its `.text` end (`0x800C89A0`) is `d_lytbase.cpp`'s official
`dtk_splits_wiimj2d.txt` start address verbatim.

`.ctors`: owns slot 70. `.data`: seven framework template vtables
(`sFStateStateMgr_c`, `sStateStateMgr_c`, `sFStateMgr_c`, `sStateMgr_c`,
`sFStateFct_c`, `sFState_c`, `sFStateID_c`, all `<dLineMng_c,...>`),
`0x80316E98`-`0x80317738`, total `0x1A0` (416) bytes -- no separate vtable for
`dLineMng_c` itself, consistent with it not being polymorphic. `.bss`: 27
`StateID_*__10dLineMng_c` objects, `0x30` each = `0x510` (1296) bytes,
`0x80359110`-`0x80359740`. `.rodata`: two small local lookup tables,
`is_unit_circle2x2`'s `d_unit` (`0x10`) and `is_unit_circle4x4`'s `d_unit`
(`0x20`), both `@LOCAL@`-scoped (function-local statics, not shared pool
entries) at `0x802F12E8`-`0x802F1318`. `.sdata2`: one float,
`smc_UNIT_SIZE_X`.

The template-instantiation family it pulls in (`sStateMgr_c`,
`sFStateMgr_c`, `sFState_c`, `sFStateFct_c`, `sStateIDChk_c`,
`sStateMethodUsr_FI_c`) is defined in `dol/sLib/s_lib.cpp`,
`s_StateMethod.cpp`, `s_StateMethodUsr_FI.cpp`, `s_StateID.cpp`,
`s_Phase.cpp` -- **all already landed** per `dtk_splits_wiimj2d.txt`. Per the
`d_a_wm_sandpillar.cpp` precedent recorded in `AGENT_CONTEXT.md` ("declaring a
templated member correctly instantiates a whole family of methods that may
already match the target with zero code written... 0/66 to 40/66 in one
round"), declaring `dLineMng_c`'s member correctly should immediately match a
meaningful fraction of its ~174 total symbols for free -- most of the tiny
`sStateStateMgr_c<...>`/`sFStateMgr_c<...>` boilerplate (`initializeState`,
`executeState`, `getStateID`, `isSubState`, etc., dozens of them, many `0x10`-
`0x20` bytes) are exactly the shape that matched immediately on sandpillar.

**Sibling**: `include/game/bases/d_pausewindow.hpp` declares
`sFStateMgr_c<Pausewindow_c, sStateMethodUsr_FI_c> mStateMgr;` on
`Pausewindow_c : public dBase_c` -- **the same template instantiation shape**
`dLineMng_c` uses (`sFStateMgr_c<10dLineMng_c,20sStateMethodUsr_FI_c>` appears
verbatim in the vtable list). `source/dol/bases/d_pausewindow.cpp` (391 lines,
landed) is therefore the best available worked example of *this exact
framework member, declared on a plain non-actor class* -- closer in shape to
`dLineMng_c` than any `d_a_en_*` file, since `dLineMng_c` is also not a
`dEn_c`/`fBase_c` actor. `dLineMng_c` additionally uses the two-level
`sStateStateMgr_c<dLineMng_c, sFStateMgr_c<...>, ...>` nesting (a
"state-within-a-state" system for its 27 states), which `Pausewindow_c` does
not need -- so the sibling covers the framework idiom but not the specific
nested-state complexity; that part is genuinely new ground.

**Risk I could not rule out**: the bulk of the actual new code here is
geometry/trig math (`line_cross_chk1` through `lineF_cross_chk`,
`circle_*_cross_chk`, `mov_to_*upper/lower`, `move_on_circle1..4`) -- dozens of
mid-size (`0x60`-`0x3DC` byte) floating-point functions with many branches.
`AGENT_CONTEXT.md`'s framing that the DOL wins had "no register-allocation
wall" was observed on `d_a_en_bros_base.cpp`/`d_a_en_blockmain.cpp`, which are
actor dispatch/state-transition code, not dense geometry math -- I have no
measurement either way for whether that observation transfers to this much
trig-heavy a unit. Flagging this as an inference-free unknown, not a
prediction.

## Recommendation

**Author `d_line_mng.cpp` first.** Reasoning, weighted:
1. Both `.text` bounds are the highest-confidence bounds in the whole gap
   (`.ctors`-adjacency-confirmed on the end, `.ctors`-adjacency-confirmed on
   the neighbour's start).
2. No vtable => no base-class blocker => cannot repeat the item-1 failure
   mode.
3. Largest candidate in the gap (31712 bytes) -- best size-to-round ratio if
   it goes cleanly.
4. The state-machine framework it needs is already fully landed elsewhere,
   with a structurally-matching worked example (`d_pausewindow.cpp`) to read
   before writing a single body, per the "read a function that already
   matches" technique.
5. Exactly one static-initializer entry (`.ctors` slot 70) -- not the
   sprawling ~26-object static-init table item 1 has.

**If it turns out too large for one round, fall back to `d_iggy_wan_kusari.cpp`**
(item 12): same "no vtable, no blocker" safety profile, a third the size
(8256 bytes), also single-`.ctors`-entry, also has a locatable idiom sibling
(`d_a_spin_child_base.cpp`), and is a genuine enemy/hazard object rather than
infrastructure, which better matches the project's usual authoring flow.

**Do not author `d_enemy_toride_kokoopa.cpp` next**, despite being the
largest single class in the gap, until `dEnBoss_c` (in the separate,
unrelated `0x91BD0`-`0x98370` gap) is decompiled or its vtable layout is
otherwise confirmed.

## Open questions (could not establish)

1. **`.data`/`.bss` full span for `d_enemy_toride_kokoopa.cpp`.** I found its
   vtable and `.bss` StateID objects but did not walk the complete pool the
   way `check_bounds.py` would for a landed unit -- not needed for the
   recommendation (it's not being authored first) but will be needed before
   anyone attempts it.
2. **The FumiCheck cluster's true owner** (`FumiCcInfo_c::getFumiRev`,
   `MugenComboFumiCheck_c`/`KokoopaSpFumiCheck_c` bodies, `0x800B07B0`-
   `0x800B0A20`). I attributed it to `d_enemy_toride_kokoopa.cpp`'s tail on
   naming plausibility and position, not on pool-adjacency proof. If it is
   instead its own tiny TU, the #1/#2 boundary in the carve table shifts by
   `0x270` bytes.
3. **`dFunsuiAct_c::posMove`** (single function, `0x800B2D20`-`0x800B2E70`,
   `0x14C` bytes) -- sits between `d_fukidashiInfo.cpp` and `d_game_common.cpp`
   with no sinit on either side to anchor it. I folded it into item 6's range
   above; it may be its own one-function TU instead.
4. **Cross-file member definitions inside item 6/17.** `dWarningManager_c::
   isWarning()` and `dActor_c::screenCullCheck()` (both global, not weak) sit
   physically inside `d_game_common.cpp`'s address range; `dPlayerMdl_c::
   getBodyMdl()`/`getLegLengthP()` and `dPyMdlBase_c::updateBonusCap()`
   (also global) sit inside `d_kinopio_mdl.cpp`'s range. Since these are
   non-weak (ODR-unique) definitions, and MWCC does not interleave two TUs'
   `.text`, this most likely means the original source really did implement
   these classes' methods in these files rather than in a file named after the
   class -- but I have not independently confirmed this is not instead a
   boundary-attribution error on my part (e.g. `dInfo_c`/`dWarningManager_c`/
   `dActor_c` methods actually starting a same-address different TU that I
   merged into the wrong neighbour). Worth a second look before authoring
   item 6 or item 17.
5. **The `d_ice_param_cpp` / `dIceMng_c` name mismatch** (item 11): the
   `__sinit` names the file `d_ice_param_cpp` but the class living there is
   `dIceMng_c`, not `dIceParam_c`/similar. Possibly the file holds a
   `dIceParam_c` (or similarly-named) static/namespace I did not spot amid
   `dIceMng_c`'s 17 functions, or the sinit name reflects an older/renamed
   file. Not investigated further.
6. **`d_hana_body.cpp`'s base class.** All five Hana classes
   (`dHanaBodyBase_c`/`dHanaBody_c`/`dHanaHead_c`/`dHanaBigBody_c`/
   `dHanaBigHead_c`) have their own `0x4C`-byte (17-slot) vtables, so unlike
   items 12/23 this family **is** polymorphic and I did not check whether its
   base class is already landed or sits in the same undeclared-`dEnBoss_c`
   category. Not recommended without that check first.
7. **Whether item 1's dense geometry-math risk (see "Risk I could not rule
   out" above) actually manifests** -- genuinely untested; would need an
   actual draft compile of one `line_cross_chk*`/`mov_to_*` function to know.

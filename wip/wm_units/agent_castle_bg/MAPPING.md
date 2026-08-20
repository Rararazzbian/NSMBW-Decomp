# MIDDLE_BG_FOR_CASTLE_LUDWIG + BOTTOM_BG_FOR_CASTLE_LUDWIG -- function inventory

Coordinator-scoped unit, one translation unit, two profiles. `.text 0xf5130-0xf6150`, 0x1020
bytes, 33 functions total (confirmed against `bin/dtk/d_basesNP_symbols.txt`). Object list in
`build.py` covers all three dtk-split objects overlapping the range (`auto_00_000F4FB0_text.o`,
`auto_fn_2_F5C80_text.o`, `auto_00_000F5DA4_text.o`) -- `check_target_objs.py` reports this unit
clean. **Tally: 12/33 matched.** Not reachable to N/N this round -- a real `dEn_c`-derived
class with a heap allocator, a model, two `dBg_ctr_c` zones, nine of its own new virtuals, AND
(discovered this round) a state machine with at least two states.

## REAL CLASS NAMES -- corrected this round, read from `.data`, not guessed

`daMiddleBGForCastleLudwig_c` (capital `BG`) -- found as a literal ASCII string in `.data`,
`"daMiddleBGForCastleLudwig_c::StateID_DemoWait"`, sitting immediately after this class's own
vtable object in `lbl_2_data_30C3C` (dtk merges adjacent no-gap `.data` objects under one
label -- that string is NOT part of the vtable itself, see below). `daBottomBGForCastleLudwig_c`
by the same capitalization convention (not independently string-confirmed this round, but the
sibling relationship and naming pattern are certain).

## MAJOR CORRECTION THIS ROUND: vtable-slot/class attribution was backwards

An earlier draft this round diffed BOTTOM_BG's vtable (`lbl_2_data_30998`) against
MIDDLE_BG's (`lbl_2_data_30C3C`) only across the first ~70 of 169 words by eye, saw the first
four base-method slots (create/doDelete/execute/draw) match, and assumed the remaining nine
"new virtual" slots were ALSO all shared -- with only the destructor actually differing. A full
PROGRAMMATIC slot-by-slot diff of the complete 169-word vtables (not an eyeballed prefix) found
this was wrong: **THREE of the nine new virtuals also differ, not just the destructor**:

| slot | offset | BOTTOM_BG (`lbl_2_data_30998`) | MIDDLE_BG (`lbl_2_data_30C3C`) |
|---|---|---|---|
| 18 | 0x48 | `fn_2_F5C20` (dtor) | `fn_2_F5240` (dtor) |
| 160 | 0x280 | `fn_2_F5C10` (empty, 4 bytes) | `fn_2_F5380` (0x98 bytes, real) |
| 166 | 0x298 | `fn_2_F59A0` (`createModel`, 0x124 bytes) | `fn_2_F5550` (`createModel`, ALSO 0x124 bytes) |
| 167 | 0x29c | `fn_2_F5AD0` (0x128 bytes) | `fn_2_F5680` (0x18C bytes -- the LARGEST function in the unit) |

Every other one of the 169 words (all four base-method slots, and six of the nine new virtuals)
is confirmed byte-identical between the two vtables. The earlier draft had `fn_2_F5C10`/
`fn_2_F59A0`/`fn_2_F5AD0` attached to `daMiddleBGForCastleLudwig_c` (as if they were the base's
own shared implementation, inherited unmodified by BOTTOM_BG) -- exactly backwards. **Fixed
before being reported done**, per this project's own "individually diff anything you believe is
boilerplate" caution (the coordinator's own citation: another unit's 23 "template-matched"
functions turned out to include a destructor credited to the wrong class and two functions with
swapped roles -- this is the same class of bug, caught the same way, by not trusting a
partial/eyeballed comparison).

One useful consequence: `fn_2_F5550` (MIDDLE_BG's own `createModel` override) is
BYTE-FOR-BYTE THE SAME SHAPE as `fn_2_F59A0` (BOTTOM_BG's own), confirmed by direct disasm
comparison -- same instruction sequence, only the string-pool references differ. Both are now
authored with REAL content (see below).

## Real strings read directly from `.data` (not guessed)

- `lbl_2_data_30910`/`lbl_2_data_3091C`: `"window_left"` / `"window_right"` -- the 2-entry node
  name table (`lbl_2_data_30930`) `vf284` (fn_2_F5430, still a stub) passes to
  `GetResNode`/`setNodeVisibility`.
- `lbl_2_data_30938`/`lbl_2_data_30954`: `"g3d/W7_shiroboss_bg_M.brres"` / `"W7_shiroboss_bg_M"`
  -- MIDDLE_BG's own model/arc strings, used in `daMiddleBGForCastleLudwig_c::createModel()`.
- `lbl_2_data_30968`/`lbl_2_data_30984`: `"g3d/W7_shiroboss_bg_D.brres"` / `"W7_shiroboss_bg_D"`
  -- BOTTOM_BG's own model/arc strings (`D` likely "Down", matching BOTTOM's own role), used in
  `daBottomBGForCastleLudwig_c::createModel()`.

Both `createModel()` overrides now use these REAL strings (previously a clearly-marked
`"CASTLE_BG"` placeholder). Not yet re-measured against the exact target byte content beyond
the tally above (both still show as "differing", expected -- likely register-allocation
residual now that the pool references are correct in kind, not necessarily in exact scheduling).

## NEW THIS ROUND: this unit has a state machine, at least two states -- NOT YET AUTHORED

The coordinator's own relocation lookup found the trampolines `fn_2_F60C0`/`fn_2_F60F0`/
`fn_2_F6120` (the unit's last three functions) referenced from TWO separate 3-word groups in
`.data` (`0x30F5C-0x30F64` and `0x30F90-0x30F98`), both listing the same three addresses in the
same order. Investigated further this round:

- The trampolines are `__ptmf_scall` call-through-pointer-to-member-function adaptors at stride
  `0xc` (offsets `0xc`/`0x18`/`0x24` on some small ~0x30-byte type, NOT
  `daMiddleBGForCastleLudwig_c` itself -- real fields there start at `0x524`). This matches
  `sFStateID_c<T>::initializeState()`/`executeState()`/`finalizeState()` -- the TEMPLATE class
  this project's `STATE_FUNC_DECLARE`/`STATE_DEFINE` macros generate (see
  `include/game/sLib/s_State.hpp`), instantiated once for our class and shared by every
  `STATE_FUNC_DECLARE`'d (non-virtual) state.
- `lbl_2_data_30F34` (0x6C bytes) decodes as TWO 13-word vtables (real inherited `sStateID_c`
  method names -- `isNull`/`isEqual`/`operator==`/`operator!=`/`name`/`number`, plus the three
  trampolines at the tail) -- these ARE `sFStateID_c<daMiddleBGForCastleLudwig_c>`'s own class
  vtable (one copy per translation-unit-local instantiation site, not per state instance).
- Confirmed at least one state name directly: `lbl_2_data_30C3C`'s own trailing bytes (right
  after its class vtable, no gap, merged under the same dtk label) decode to the ASCII string
  `"daMiddleBGForCastleLudwig_c::StateID_DemoWait"` -- ONE state is named `DemoWait`. The
  matching PMF fields immediately before that string (raw ints `0/0x294/0x60`, `0/0x290/0x60`,
  `0/0x28c/0x60`) are vtable-relative (not fixed-function) pointer-to-member values, pointing at
  `vf294`/`vf290`/`vf28c` -- meaning `DemoWait`'s own initialize/execute/finalize are declared
  `virtual` and happen to reuse those three already-authored "shared" slots. **This means
  `vf294`/`vf290`/`vf28c` are very likely actually
  `initializeState_DemoWait`/`executeState_DemoWait`/`finalizeState_DemoWait` (or some
  permutation), not generic new virtuals unrelated to the state machine** -- their CONTENT is
  still correct as authored (confirmed against real target bytes), but their ROLE/NAME may need
  correcting once the state declarations are added and re-diffed, per the coordinator's own
  warning that "a template family emitting more members changes batching around it."
- The second state's own name was NOT found this round (ran out of time before locating a
  second `"...::StateID_<name>"` string) -- next round should search the FULL `.data` dump
  (`target_auto_04_000132B0_data.txt`) for another `"::StateID_"` ASCII occurrence, and also
  check whether `vf280`/`vf284`/`vf288` (the other three of the nine) are that second state's
  own init/exec/finalize instead of ordinary virtuals -- would explain why `vf280` differs
  per-class (a per-class override of a STATE method is completely ordinary) while `vf284`/
  `vf288` are shared (default behavior only one side overrides).
- **NOT attempted yet**: adding the actual `STATE_FUNC_DECLARE`/`STATE_DEFINE` (or
  `STATE_VIRTUAL_FUNC_DECLARE`/`STATE_VIRTUAL_DEFINE`, depending on which of the two mechanisms
  is actually in play here -- both are now plausible candidates given the virtual-PMF finding)
  and re-diffing the WHOLE unit afterward, per the coordinator's own explicit instruction. This
  is the clear next step and was not reached this round due to the class-attribution correction
  taking priority once found.
- Ten landed precedents exist (`grep -rl STATE_DEFINE source/`); none needs an explicit-
  instantiation trick per the coordinator, so if the trampolines don't fall out for free from a
  plain `STATE_FUNC_DECLARE`/`STATE_DEFINE` pair, the likely answer is that `STATE_VIRTUAL_*`
  (matching `dEn_c`'s own existing `DieFumi`/`EatIn`/etc. convention, since `dEn_c` itself is
  already in this class's ancestry) is the mechanism actually used here, not the plain variant
  `d_a_enemy_ice.hpp`/`d_a_remo_door.hpp` (both `dActorState_c`-based, not `dEn_c`-based) use.

## MATCHED (12/33)

| draft name | target | size | notes |
|---|---|---|---|
| `daMiddleBGForCastleLudwig_c_classInit__Fv` | `fn_2_F5130` | 12/12 | naming-only |
| `daBottomBGForCastleLudwig_c_classInit__Fv` | `fn_2_F5160` | 19/19 | naming-only (own vtable symbol) |
| `getNullState__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F51B0` | 3/3 | EXACT. `return &sStateID::null;` (real landed extern) |
| `__ct__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F51C0` | 29/29 | EXACT. Shared base ctor |
| `__dt__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5240` | 35/35 | EXACT. Base dtor |
| `entryOrRelease__27daMiddleBGForCastleLudwig_cFb` | `fn_2_F52D0` | 6/6 | EXACT |
| `vf2a0__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5890` | 21/21 | EXACT. Matrix update |
| `vf28c__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5970` | 1/1 | EXACT. Empty body -- possibly `finalizeState_DemoWait`, see above |
| `vf294__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5980` | 1/1 | EXACT. Empty body -- possibly `initializeState_DemoWait`, see above |
| `vf288__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5C00` | 4/4 | EXACT. Forwards to `vf2a0()` |
| `vf280__27daBottomBGForCastleLudwig_cFv` | `fn_2_F5C10` | 1/1 | EXACT. Empty body -- BOTTOM_BG's OWN override (corrected this round) |
| `__dt__27daBottomBGForCastleLudwig_cFv` | `fn_2_F5C20` | 22/22 | EXACT. BOTTOM_BG's trivial derived dtor |

## PARKED, real content, close (1)

- **`vf290__27daMiddleBGForCastleLudwig_cFv`** (`fn_2_F5990`, 4/4 lines) -- 2/4 differing.
  Forwarding thunk into `mModel`'s own vtable at offset 0x1c, target uses r12 exclusively via
  load-with-update addressing; two variants tried, both land on r4. Register-allocation
  residual, not content -- park. Possibly `executeState_DemoWait`, see above.

## SCOUTED, FAKE STUBS this round -- 8

All placed at their correct address slot.

| draft name (class) | target | size | role |
|---|---|---|---|
| `create` (base, shared) | `fn_2_F54D0` | 32 | vtable slot 2. Confirmed identical in both vtables (full diff). Unscouted content. |
| `doDelete` (base, shared) | `fn_2_F58F0` | 20 | vtable slot 5. Same caveat. |
| `execute` (base, shared) | `fn_2_F5810` | 31 | vtable slot 8. Same caveat. |
| `draw` (base, shared) | `fn_2_F5940` | 12 | vtable slot 11. Same caveat. |
| `vf284` (base, shared) | `fn_2_F5430` | 38 | offset 0x284. PARTIALLY scouted: copies a 2-entry visibility byte array at `+0x764` from another instance of this class, then `GetResNode`/`setNodeVisibility` against the REAL, confirmed node-name table (`"window_left"`/`"window_right"`). Body still a stub. |
| `vf280` (MIDDLE_BG's OWN) | `fn_2_F5380` | 42 | offset 0x280. Unscouted (0x98 bytes) -- **corrected this round to the right class**. |
| `vf29c` (MIDDLE_BG's OWN) | `fn_2_F5680` | 99 | offset 0x29c. Unscouted (0x18C bytes -- LARGEST in unit) -- **corrected this round to the right class**. |
| `vf29c` (BOTTOM_BG's OWN) | `fn_2_F5AD0` | 74 | offset 0x29c. Unscouted (0x128 bytes). |

`createModel` (both overrides, `fn_2_F5550`/`fn_2_F59A0`) now has REAL structural content and
REAL strings (see above) but still shows as differing on the tally -- not re-verified
byte-for-byte this round; likely just scheduling residual now that the pool references are the
right kind.

## NOT YET TOUCHED THIS ROUND -- 13 functions, absent (not stubbed in the wrong slot)

| target | size | notes |
|---|---|---|
| `fn_2_F52F0` | 35 | Unscouted. |
| `fn_2_F5C80` | 73 | `__sinit`, the `.ctors`-registered static initialiser. Now visible (object-list fixed this round) but not yet read. |
| `fn_2_F5DB0` | 22 | Unscouted -- likely one of the two `sFStateID_c` state-object DESTRUCTORS (matches its own appearance as `fn_2_F5DB0` inside `lbl_2_data_30F34`'s second 13-word block, at the dtor slot). |
| `fn_2_F5E10` | 23 | The coordinator's own cited "one internal `.text` target" -- likely the OTHER state object's own dtor override (appears in `lbl_2_data_30F34`'s first block at the dtor slot). |
| `fn_2_F5E70` | 55 | Unscouted -- likely a state-object override (appeared in the `lbl_2_data_30F34` vtable region at the "isSameName"-equivalent or "number"-equivalent slot). |
| `fn_2_F5F50` | 56 | Unscouted. |
| `fn_2_F6030` | 34 | Unscouted -- likely the SHARED "isSameName" override for both state objects (appeared identically in both 13-word blocks). |
| `fn_2_F60C0`/`fn_2_F60F0`/`fn_2_F6120` | 12 each | The three `sFStateID_c<T>::initializeState/executeState/finalizeState` template trampolines -- SHOULD be emitted automatically once the real `STATE_FUNC_DECLARE`/`STATE_VIRTUAL_FUNC_DECLARE` state declarations are added, not hand-authored. |

## Gates

- **Function order**: GREEN. `order_sweep.py` reports `ok agent_castle_bg 12/33`.
- **`.ctors`**: GREEN. `0x288 -> __sinit at .text 0xf5c80` for BOTTOM_BG_FOR_CASTLE_LUDWIG,
  matching the coordinator's own citation exactly.
- **Target-object coverage**: GREEN. `check_target_objs.py` does not flag this unit (fixed this
  round -- `auto_fn_2_F5C80_text.o` was missing from `build.py`'s object list).

## Real defect found and fixed: struct size (`sizeof`)

First draft computed `0x750` against target's `0x768` (both classInits' own alloc constant) --
0x18 bytes short, fixed with a trailing `u8 mPad74c[0x1c]` (content still unconfirmed, size
confirmed exact).

## Shadow header

`shadow_include/game/bases/d_a_castle_bg.hpp`. Every real header it depends on (`dEn_c`,
`dHeapAllocator_c`, `dBg_ctr_c`, `m3d::mdl_c`, `sStateID_c`) is ALREADY LANDED; no shadow copy of
any of them was needed. BINDING rule applied throughout (all new virtuals defined out-of-line,
not in-class, matching the DUMMY_DOOR lesson).

## How to reproduce this tally

```
python wip/wm_units/agent_castle_bg/build.py
python wip/wm_units/agent_castle_bg/difftool.py \
    wip/wm_units/agent_castle_bg/target_auto_00_000F4FB0_text.txt \
    wip/wm_units/agent_castle_bg/draft.txt \
    fn_2_<addr> <draft_symbol>
```
(Functions at or past `0xf5db0` are in `target_auto_00_000F5DA4_text.txt`; `fn_2_F5C80` is in
its own `target auto_fn_2_F5C80` object, not yet dumped to a standalone `.txt` -- use
`build.py`'s own object list / `verify_anon.py` output for it.)

## Plan for the next round

1. Find the SECOND state's name (search `.data` for a second `"::StateID_"` string).
2. Determine whether the mechanism is `STATE_FUNC_DECLARE`/`STATE_DEFINE` (plain, matching the
   `sFStateID_c<T>` trampolines) or `STATE_VIRTUAL_FUNC_DECLARE`/`STATE_VIRTUAL_DEFINE` (matching
   the virtual-PMF finding on `DemoWait`) -- possibly BOTH, one per state.
3. Add the state declarations, re-diff the WHOLE unit (not just the trampolines), per the
   coordinator's own instruction -- expect `vf280`/`vf284`/`vf288`/`vf28c`/`vf290`/`vf294` to
   possibly need renaming to their real `initializeState_X`/`executeState_X`/`finalizeState_X`
   identities once the state classes exist, and expect `fn_2_F60C0`/`F60F0`/`F6120`/`F5DB0`/
   `F5E10`/`F6030` to fall out largely for free.
4. Then the remaining unscouted functions smallest-first: `fn_2_F52F0`(35), `fn_2_F5F50`(56),
   `create`/`execute`/`doDelete`/`draw` (shared, high-value), `vf280`(MIDDLE_BG, 42),
   `vf29c`(MIDDLE_BG, 99 -- largest), `vf29c`(BOTTOM_BG, 74), `fn_2_F5C80`(__sinit, 73).

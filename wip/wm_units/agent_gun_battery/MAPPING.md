# MINI_GAME_GUN_BATTERY_MGR / MINI_GAME_GUN_BATTERY_MGR_OBJ -- function inventory

Brand-new unit, `d_basesNP` `.text 0xF8980-0xF9B40` (0x11C0 bytes), ONE translation unit
covering both profiles (single `.ctors` entry `0x294 -> __sinit fn_2_F97D0`, confirmed with
`python wip/wm_units/ctors_map.py d_basesNP GUN_BATTERY`).

**Tally: 21/49 matched byte-for-byte modulo naming-only residuals** (12 are true 0-diff EXACT
matches; the other 9 differ only in symbol names the target dump structurally cannot show).
Both gates green: `check_fn_order.py` reports 0 inversions, `ctors_map.py` reports exactly one
`.ctors` entry. ~16 more functions are template boilerplate whose content was read and matched
against known template source (should require no hand-authoring). 1 more (`perPlayerRestCheck`,
`fn_2_F8DF0`) is structurally and size-exact but has a register-allocation-only residual, parked
after 3 variants. See history below for how this unit got here across 4 rounds.

## Round 1: base classes corrected

`MINI_GAME_GUN_BATTERY_MGR_OBJ` derives `dBase_c` (not `dActor_c` as the original brief said --
confirmed off `fn_2_F8AA0`'s `__ct__7dBase_cFv` call, and `sizeof(dActor_c)==0x398` is larger
than the whole `0xF4` object, which settles it independently). `BSS_SINGLETONS.md` corrected by
the coordinator. `MINI_GAME_GUN_BATTERY_MGR` derives `dActor_c` with ZERO extra fields.

## Round 2: read the class's own vtable from the `.data` SPLIT OBJECT

Disassembling the `.data` split object containing the vtable
(`bin/dtkspl/d_basesNP/obj/auto_04_000132B0_data.o` -> `target_data_132B0.txt`) prints every
slot with its real mangled name. MGR_OBJ overrides exactly 4 of `fBase_c`/`dBase_c`'s 18
primary-vtable slots: `create`, `execute`, `preExecute`, and its own destructor. Authoring these
four collapsed the constructor from 58 differing to a handful of residuals, confirming "a stuck
constructor is often a symptom of undeclared virtuals."

Pooled string literals in the same dump (`lbl_2_data_31AD0`) name the class and every state in
plain ASCII: `"daMiniGameGunBatteryMgrObj_c::StateID_ShowRule"`, `"...::StateID_Play"`,
`"...::StateID_ShowResult"` -- confirming the guessed class name exactly.

## Round 3: study a landed `STATE_DEFINE` unit (`d_pausewindow.cpp`), author from its structure

Reproduced the exact file layout (`STATE_FUNC_DECLARE` in-class, `STATE_DEFINE` calls right
after `BASE_PROFILE`, `mStateMgr(*this, StateID_ShowRule)` in the ctor init-list). Read the 9
`sFStateID_c<T>` PMF triples out of `lbl_2_data_31AD0`, identifying every state body's real
address without disassembling a single one first. Authored the 3 empty `initialize` bodies and
`finalizeState_ShowRule` (all EXACT). Found and authored a missing function the inventory had
omitted entirely: `daMiniGameGunBatteryMgr_c`'s own destructor (`fn_2_F9520`). Read (not yet
authored) 16 template-boilerplate functions' content and confirmed them against known template
source (`sStateMgr_c`/`sFStateFct_c`/`sFStateID_c` destructors, thunks, `build()`/`isSameName()`
matching their headers verbatim, `__ptmf_scall` adjustor thunks). Added a shadow header for
`PauseManager_c` (a new field at `0x1d`, justified by the header's own comment that nothing
embeds the class by value).

## Round 4 (this round): structural ctor fix, dBg_c precedent, four helpers wired in

1. **Coordinator applied the `PauseManager_c` header proposal to the real header.** Deleted the
   shadow copy, rebuilt against the real header -- all 14 previously-matched functions still
   hold.
2. **`mGunSlot[4]` restructure, exactly as the coordinator directed.** `fn_2_F8DC0` (read in
   round 3) showed `m_70`/`m_74`/`m_78` were really "gun slot index 0" of a 4-element sequence.
   Replacing "3 separate named fields + `daGunBatteryGunSlot_t mGunSlot[3]`" with a single
   `daGunBatteryGunSlot_t mGunSlot[4]` and re-testing the constructor: 58 differing (round 2) ->
   14, all naming-only, exact line-for-line size match (59/59). **The constructor is now
   MATCHED.** This confirms the coordinator's reasoning precisely -- a real 4-element array
   construction produces the target's genuine 3-iteration loop from element 0, with element 0's
   own construction folded into the same codegen path as elements 1-3.
3. **Found the `dBg_c` precedent before proposing any header change**, per instruction:
   `source/d_basesNP/bases/d_a_wm_note.cpp` (around line 164-169) reaches past `dWCamera_c`'s
   documented layout (a DIFFERENT under-documented class, not `dBg_c`, but the exact same
   situation) with a local raw byte-pointer cast confined to its own `.cpp`:
   `u8 *cam = (u8 *) camera; *(u32 *) (cam + 0x604) = 1;`. Applied the identical technique to
   `dBg_c::m_bg_p` for `finalizeState_Play` (`fn_2_F9160`) and `executeState_Play`
   (`fn_2_F9200`) -- **both matched, exact size (37/37 and 44/44), all residuals naming-only.**
   No `dBg_c` header change was needed or made.
4. **Wired in the four already-read helper functions.** `addToSlot` (`fn_2_F8DC0`), `setM_f0`
   (`fn_2_F8ED0`), and `markSlotUsed` (`fn_2_F8EE0`) are all EXACT matches, written directly as
   ordinary array-member access (`mGunSlot[gunIndex].m_04 += amount;` etc.) -- the multiply-index
   addressing the disassembly showed is just the compiler's own codegen for that array access,
   confirmed by testing rather than assumed. `finalizeState_ShowResult` (`fn_2_F92C0`), which
   calls two of them, MATCHED (naming-only). `perPlayerRestCheck` (`fn_2_F8DF0`) is
   structurally and size-exact (35/35 lines) after 3 variants, but has a 10-line
   register-allocation-only residual (loop index in r29 vs r30; a vtable-pointer dereference
   chained through r12 directly vs staged through r4) -- parked, not a content or structure
   problem.

## Function inventory (49 real functions, 0x11C0 bytes total; sizes sum to exactly 0x11C0)

| addr | size | status |
|---|---|---|
| F8980 | 0x30 | MATCHED (naming-only) -- MGR classInit |
| F89B0 | 0x30 | MATCHED (naming-only) -- MGR_OBJ classInit |
| F89E0 | 0x60 | MATCHED (naming-only) -- MGR ctor |
| F8A40 | 0x10 | MATCHED, EXACT -- daMiniGameGunBatteryMgr_c::create() |
| F8A50 | 0x44 | MATCHED (naming-only) -- daMiniGameGunBatteryMgr_c::doDelete() |
| F8AA0 | 0xF0 | MATCHED (naming-only, 59/59 lines) -- MGR_OBJ ctor |
| F8B90 | 0x64 | boilerplate, confirmed by content (sStateMgr_c dtor, intermediate level) |
| F8C00 | 0x60 | boilerplate, confirmed (sStateMgr_c dtor, final level) |
| F8C60 | 0x40 | boilerplate, confirmed (sFStateFct_c<T> dtor) |
| F8CA0 | 0x40 | boilerplate, confirmed (sFStateID_c<T> dtor) |
| F8CE0 | 0x60 | MATCHED, EXACT -- daMiniGameGunBatteryMgrObj_c::create() |
| F8D40 | 0x40 | MATCHED (naming-only) -- daMiniGameGunBatteryMgrObj_c::preExecute() |
| F8D80 | 0x30 | MATCHED, EXACT -- daMiniGameGunBatteryMgrObj_c::execute() |
| F8DB0 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F8DC0 | 0x24 | MATCHED, EXACT -- addToSlot(int, int) |
| F8DF0 | 0x90 | PARKED -- perPlayerRestCheck(), 10/35 differing, register allocation only |
| F8E80 | 0x48 | identified, not authored -- timer/key-check gate, not wired in |
| F8ED0 | 0x8 | MATCHED, EXACT -- setM_f0(int) |
| F8EE0 | 0x34 | MATCHED, EXACT -- markSlotUsed(int) |
| F8F20 | 0x48 | MATCHED, EXACT -- finalizeState_ShowRule |
| F8F70 | 0x1D0 | PARKED -- executeState_ShowRule, real game logic, not attempted |
| F9140 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F9150 | 0x4 | MATCHED, EXACT -- initializeState_ShowRule (empty) |
| F9160 | 0x94 | MATCHED (naming-only) -- finalizeState_Play |
| F9200 | 0xB0 | MATCHED (naming-only) -- executeState_Play |
| F92B0 | 0x4 | MATCHED, EXACT -- initializeState_Play (empty) |
| F92C0 | 0x54 | MATCHED (naming-only) -- finalizeState_ShowResult |
| F9320 | 0x1F0 | PARKED -- executeState_ShowResult, real game logic, not attempted |
| F9510 | 0x4 | MATCHED, EXACT -- initializeState_ShowResult (empty) |
| F9520 | 0x22 | MATCHED, EXACT -- daMiniGameGunBatteryMgr_c::~daMiniGameGunBatteryMgr_c() |
| F9580 | 0x1E | MATCHED, EXACT -- daMiniGameGunBatteryMgrObj_c::~daMiniGameGunBatteryMgrObj_c() |
| F9600 | 0x60 | boilerplate, confirmed (sFStateFct_c<T>::build(), matches header verbatim) |
| F9660 | 0x10 | boilerplate, confirmed (sFStateFct_c<T>::dispose()) |
| F9670 | 0x20 | boilerplate, confirmed (sStateID_c comparison thunk) |
| F9690 | 0x20 | boilerplate, confirmed (sStateID_c comparison thunk) |
| F96B0 | 0x20 | boilerplate, confirmed (sStateID_c comparison thunk) |
| F96D0 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F96E0 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F96F0 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F9700 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F9710 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F9720 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F9730 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F9740 | 0x30 | boilerplate, confirmed (__ptmf_scall adjustor thunk) |
| F9770 | 0x30 | boilerplate, confirmed (__ptmf_scall adjustor thunk) |
| F97A0 | 0x30 | boilerplate, confirmed (__ptmf_scall adjustor thunk) |
| F97D0 | 0x280 | __sinit -- correctly untouched, always last |
| F9A50 | 0x58 | boilerplate, confirmed (sStateID_c base dtor thunk) |
| F9AB0 | 0x88 | boilerplate, confirmed (sFStateID_c<T>::isSameName(), matches header verbatim) |

None of the 16 "boilerplate, confirmed" functions have been individually diffed against target
(time) -- their CONTENT was read and matched against known landed template source, which is
strong evidence, not a guess, but still worth an explicit diff pass before calling the unit done.

## PARKED: `perPlayerRestCheck` (`fn_2_F8DF0`, 35/35 lines, 10 differing)

Register-allocation-only after 3 source variants (see Round 4 above): loop index register
(r29 vs r30) and whether the vtable-pointer dereference chains directly through r12 or stages
through r4 first. Content, control flow and every store/load target are identical to the
target -- not a content or structural problem.

## Not fully worked out

- `fn_2_F8E80` (0x48): read but not fully understood -- decrements `m_f0`, checks
  `dGameKey_c::m_instance` controller-input flags, looks like a "countdown expired OR button
  pressed" gate. Not wired into the class (nothing currently calls it), so its absence causes no
  link error; needs its own investigation before it can be tested.
- `executeState_ShowRule` (`fn_2_F8F70`, 0x1D0) and `executeState_ShowResult` (`fn_2_F9320`,
  0x1F0): the two largest functions in the unit, real minigame logic, not attempted this round.
  Best next targets.

## How to reproduce this tally

```
python wip/wm_units/agent_gun_battery/build.py
python wip/wm_units/check_fn_order.py wip/wm_units/agent_gun_battery/d_a_mini_game_gun_battery.cpp
python wip/wm_units/ctors_map.py d_basesNP GUN_BATTERY
python wip/wm_units/agent_gun_battery/difftool.py \
    wip/wm_units/agent_gun_battery/target_auto_00_F85C4_text.txt \
    wip/wm_units/agent_gun_battery/draft.txt \
    fn_2_<addr> <draft_symbol>
```

(`fn_2_F9A50`/`fn_2_F9AB0` are in `target_auto_00_F9A4C_text.txt` instead.)

Target dumps in this directory (freshly regenerated, not reused): `target_auto_00_F85C4_text.txt`
(0xF85C4-0xF97A0), `target_auto_fn2_F97D0_text.txt` (`__sinit`), `target_auto_00_F9A4C_text.txt`
(0xF9A4C onward), `target_data_132B0.txt` (the `.data` object holding every vtable/state-ID
object this unit owns, with real symbol names for every inherited/default vtable slot -- use
this first for any vtable question, not the `.text`-relocation walk).

Shadow headers: `shadow_include/game/snd/snd_scene_manager.hpp` adds `fn_8019C010(int)`
following the header's own existing naming convention for un-decoded DOL member functions --
pure addition, no layout change, cannot disturb any landed TU.

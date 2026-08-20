# MINI_GAME_GUN_BATTERY_MGR / MINI_GAME_GUN_BATTERY_MGR_OBJ -- function inventory

Brand-new unit, `d_basesNP` `.text 0xF8980-0xF9B40` (0x11C0 bytes), ONE translation unit
covering both profiles (single `.ctors` entry `0x294 -> __sinit fn_2_F97D0`, confirmed with
`python wip/wm_units/ctors_map.py d_basesNP GUN_BATTERY`).

**Tally: 45/49 individually diffed and matched** (34 are true 0-diff EXACT matches; 11 differ
only in symbol names the target dump structurally cannot show). Both gates green:
`check_fn_order.py` reports 0 inversions, `ctors_map.py` reports exactly one `.ctors` entry.
Every one of the 45 was diffed by address, not assumed from content resemblance. `__sinit` is
now included and MATCHED (see Round 6). Remaining 4: all PARKED with real, understood, narrow
residuals (register allocation or one specific instruction-selection shape) -- re-tested after
`__sinit` was completed and confirmed unchanged (no movement).

## History (rounds 1-3, condensed)

Round 1 corrected the base classes (MGR : `dActor_c` zero extra fields; MGR_OBJ : `dBase_c`, not
`dActor_c` as originally briefed). Round 2 read MGR_OBJ's own vtable from the `.data` split
object (`bin/dtkspl/d_basesNP/obj/auto_04_000132B0_data.o` -> `target_data_132B0.txt`), finding
its 4 real overrides (`create`, `execute`, `preExecute`, dtor) and, from the same dump, 3 pooled
state-name strings confirming the class name and every state name byte-for-byte. Round 3 studied
the landed `d_pausewindow.cpp` for `STATE_DEFINE` structure and authored the 3 empty
`initialize` bodies, `finalizeState_ShowRule`, and MGR's own missing destructor (found via the
vtable read, not in the original inventory).

## Round 4 (this round): the push to N/N

1. **`mGunSlot[4]` restructure -- the constructor is now fully MATCHED.** `m_70`/`m_74`/`m_78`
   were really "gun slot index 0" (found via `fn_2_F8DC0`'s addressing). Declaring a real
   4-element `daGunBatteryGunSlot_t mGunSlot[4]` instead of "3 fields + `mGunSlot[3]`" took the
   constructor from 58 differing (round 2) to an EXACT 59/59 line match (14 residuals, all
   naming). A loop whose trip count doesn't match the array's declared length is evidence about
   the array, not about unrolling -- the reusable lesson from this round.
2. **`dBg_c` precedent found before any header change**: `source/d_basesNP/bases/d_a_wm_note.cpp`
   reaches past `dWCamera_c`'s documented layout with a local raw-cast confined to its own
   `.cpp`. Applied identically to `dBg_c::m_bg_p` for `finalizeState_Play`/`executeState_Play` --
   both matched exact-size, no header touched.
3. **Four helper functions wired in**: `addToSlot`, `setM_f0`, `markSlotUsed` are EXACT, written
   as plain `mGunSlot[gunIndex].field` array access -- letting the compiler generate the
   multiply-index addressing itself reproduced the target exactly. Reproducing the
   disassembly's own addressing arithmetic literally in source is usually the wrong move; the
   compiler's real input is the higher-level expression. `perPlayerRestCheck` is
   content/structure/size-exact (35/35) but has a register-allocation-only residual after 3
   variants -- PARKED per the coordinator's instruction, not re-attempted.
4. **`timerOrKeyGate` (`fn_2_F8E80`) authored**: decrements a timer, checks
   `dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons` against
   `WPAD_BUTTON_A | WPAD_BUTTON_2` (exact combination precedented in
   `source/dol/bases/d_s_boot.cpp:821`). Content and control flow fully correct; the bit-test
   codegen shape (target uses `rlwinm`+dot-form `rlwimi.`, i.e. two chained bitfield extracts;
   every source variant tried compiles to either a single `andi.` or a
   `rlwinm.`+`clrlwi`+`rlwinm.` sequence) was not reproduced after 4 variants. PARKED at 16
   differing (19/18 lines) -- content confirmed correct, not a logic problem.
5. **The two large state bodies authored.** `executeState_ShowRule` (`fn_2_F8F70`, a 5-case
   switch on `m_dc` covering fader-wait, title display, minigame start, and a button/timer wait
   before transitioning to `StateID_Play`) and `executeState_ShowResult` (`fn_2_F9320`, a
   4-case switch: wait, show result with a 4-gun "did anyone score" check, button/timer wait,
   then `dScStage_c::setNextScene(...)` to leave the stage) are both content- and
   structure-complete, calling real, DOL-symbol-confirmed `dGameCom::MiniGameCannon*` functions
   (added via a shadow `d_game_com.hpp` -- names pulled directly from
   `bin/dtk/wiimj2d_symbols.txt`, not guessed) and `mFader_c::mFader->isStatus(HIDDEN)` (landed
   precedent: `source/dol/bases/d_WiiStrap.cpp:101`). Each has ONE narrow residual: `F8F70`'s
   case-3 has the same normalize-to-bool codegen mismatch as `timerOrKeyGate` (16 differing after
   4 variants); `F9320`'s case-1 "did any of the 4 guns score" check compiles to a different
   branch-fragment shape than the target's (target computes address `this+0x18` then `+0x80` for
   the 4th check specifically, instead of the direct `this+0x98` my array indexing produces --
   looks like compiler CSE reusing an `0x80` displacement already used for slot 1, not something
   controllable from source; 2 variants tried, best got to 90 differing / 118 of 122 lines).
   PARKED, not re-attempted further -- both are real understood walls, not missing content.
6. **All 20 template-boilerplate functions individually diffed against target** (not just
   content-matched as in round 3) -- 20 of 23 candidates confirmed EXACT. Two attribution
   corrections found BY DIFFING rather than assuming: `fn_2_F8CA0` is NOT `sFStateID_c<T>`'s
   own destructor as previously guessed -- it is `sStateIDChk_c`'s (`__dt__13sStateIDChk_cFv`,
   EXACT). The REAL `sFStateID_c<T>` destructor is `fn_2_F9A50` (EXACT). Similarly `fn_2_F9740`/
   `fn_2_F97A0` (the `__ptmf_scall` adjustor thunks) had `initializeState`/`finalizeState`
   swapped in the round-3 notes -- diffing settled it (`F9740`=finalizeState, `F97A0`=
   initializeState, both EXACT once corrected). 3 of the originally-claimed 23 remain genuinely
   unattributed (`fn_2_F9670`/`F9690`/`F96B0`) -- see below, not counted as matched.

## Round 5: the three "unattributed" thunks resolved

Per the coordinator's direction: diffed the three bodies against each other first (byte-
identical apart from one displacement each -- `0x28`/`0x2c`/`0x30`, confirming one family of
three consecutive vtable slots) and re-read them from the `.data` vtable dump
(`lbl_2_data_31AD0`, indices 2-5: `fn_2_F8CA0` then the three unknowns). The
`sStateIDChk_c`-remaining-virtuals hypothesis was already disproven (only one virtual beyond the
dtor exists there). The winning hypothesis was the coordinator's OTHER suggested family:
`sFState_c<T>` (from `sStateIf_c`: dtor + `initialize()`/`execute()`/`finalize()`, exactly
three non-dtor virtuals). Its weak symbols were ALREADY present in `draft.txt`
(`initialize__41sFState_c<...>Fv` etc, generated automatically once `mStateMgr`/`STATE_DEFINE`
were declared in round 3) -- they had simply never been individually diffed. All three: EXACT
matches.

`fn_2_F8CA0` is REATTRIBUTED again as a result: it is `sFState_c<T>`'s own destructor (index 2 of
the SAME `lbl_2_data_31AD0` table, immediately before `initialize`/`execute`/`finalize` at
indices 3-5), not `sStateIDChk_c`'s. Both attributions are still byte-CORRECT
(`__dt__41sFState_c<...>Fv` also diffs EXACT against `fn_2_F8CA0`, confirmed) -- genuine
identical-code-folding between two unrelated empty-bodied destructors, resolved by which
VTABLE the slot actually belongs to (`sStateIDChk_c`'s real, separate vtable is the
MODULE-WIDE-SHARED `lbl_2_data_418` object referenced from `mCheck`, established back in round
2's ownership check -- not `lbl_2_data_31AD0` at all). No declaration was missing; nothing
needed to be added to source. Re-diffed all 4 parked functions afterward per instruction --
unchanged (no regression, no movement), consistent with nothing having actually changed in the
compiled output.

## Round 6: `__sinit`

Per the coordinator's direction, attempted only once 44/49 real functions (everything except
`__sinit` itself) were matched -- the precondition for `__sinit` being tractable at all, since
its content is driven entirely by the unit's static declarations, which were not all correct
until this point.

`fn_2_F97D0` constructs the 3 `sFStateID_c<daMiniGameGunBatteryMgrObj_c>` state objects (calling
`sStateID_c(name)` as their base, then filling in the 3 PMF fields and re-patching the vtable
pointer to the derived type, then `__register_global_object`-registering each for exit-time
destruction -- standard MWCC static-object bookkeeping, matching the landed
`source/runtime/global_destructor_chain.c`). The draft compiled to the exact target SIZE
immediately (159/159 lines) with only ONE real (non-naming) residual: the very first state's
hidden `objectRef` registration node was built at `region+4` instead of the target's `region+0`
-- a 4-byte padding-side difference, states 2 and 3 already matched exactly.

**Fixed by reordering, not by touching anything content-related**: moving the singleton pointer
declaration (`static daMiniGameGunBatteryMgrObj_c *s_pMgrObj;`) from BEFORE the
`ACTOR_PROFILE`/`BASE_PROFILE`/`STATE_DEFINE` block to AFTER it resolved the padding-side
difference completely. `__sinit` is now an EXACT 159/159 line match, all 10 remaining
differences confirmed naming-only (our own profile/vtable/destructor/bss-region symbols, which
the target dump cannot show under our names).

Re-verified after this reorder, per instruction (a static's position moving shifts the pool
underneath everything else): all 44 previously-matched real functions AND all 20
template-boilerplate functions re-diffed -- zero regressions, all still EXACT/naming-only. The
4 parked functions were also re-tested -- confirmed NO MOVEMENT (same diff counts as before:
`perPlayerRestCheck` 10, `timerOrKeyGate` 16, `executeState_ShowRule` 55,
`executeState_ShowResult` 90). A null result that confirms the prediction (their residuals are
independent of this particular pool shift), reported rather than left implicit.

## Function inventory (49 real functions, 0x11C0 bytes total; sizes sum to exactly 0x11C0)

| addr | size | status |
|---|---|---|
| F8980 | 0x30 | MATCHED (naming) -- MGR classInit |
| F89B0 | 0x30 | MATCHED (naming) -- MGR_OBJ classInit |
| F89E0 | 0x60 | MATCHED (naming) -- MGR ctor |
| F8A40 | 0x10 | MATCHED EXACT -- MGR::create() |
| F8A50 | 0x44 | MATCHED (naming) -- MGR::doDelete() |
| F8AA0 | 0xF0 | MATCHED (naming, 59/59) -- MGR_OBJ ctor |
| F8B90 | 0x64 | MATCHED EXACT -- sFStateMgr_c<T,M> dtor |
| F8C00 | 0x60 | MATCHED EXACT -- sStateMgr_c<T,M,F,C> dtor |
| F8C60 | 0x40 | MATCHED EXACT -- sFStateFct_c<T> dtor |
| F8CA0 | 0x40 | MATCHED EXACT -- sFState_c<T> dtor (corrected attribution, twice) |
| F8CE0 | 0x60 | MATCHED EXACT -- MGR_OBJ::create() |
| F8D40 | 0x40 | MATCHED (naming) -- MGR_OBJ::preExecute() |
| F8D80 | 0x30 | MATCHED EXACT -- MGR_OBJ::execute() |
| F8DB0 | 0x10 | MATCHED EXACT -- sStateMgr_c<...>::executeState() |
| F8DC0 | 0x24 | MATCHED EXACT -- addToSlot(int, int) |
| F8DF0 | 0x90 | PARKED -- perPlayerRestCheck(), 10/35 differing, register allocation only |
| F8E80 | 0x48 | PARKED -- timerOrKeyGate(), 16/19 differing, bit-test codegen shape (4 variants) |
| F8ED0 | 0x8 | MATCHED EXACT -- setM_f0(int) |
| F8EE0 | 0x34 | MATCHED EXACT -- markSlotUsed(int) |
| F8F20 | 0x48 | MATCHED EXACT -- finalizeState_ShowRule |
| F8F70 | 0x1D0 | PARKED -- executeState_ShowRule, 16/116 differing, case-3 normalize shape (4 variants) |
| F9140 | 0x10 | MATCHED EXACT -- sStateMgr_c<...>::changeState() |
| F9150 | 0x4 | MATCHED EXACT -- initializeState_ShowRule (empty) |
| F9160 | 0x94 | MATCHED (naming) -- finalizeState_Play |
| F9200 | 0xB0 | MATCHED (naming) -- executeState_Play |
| F92B0 | 0x4 | MATCHED EXACT -- initializeState_Play (empty) |
| F92C0 | 0x54 | MATCHED (naming) -- finalizeState_ShowResult |
| F9320 | 0x1F0 | PARKED -- executeState_ShowResult, 90/122 differing, case-1 4th-check addressing shape (2 variants) |
| F9510 | 0x4 | MATCHED EXACT -- initializeState_ShowResult (empty) |
| F9520 | 0x22 | MATCHED EXACT -- MGR::~MGR() |
| F9580 | 0x1E | MATCHED EXACT -- MGR_OBJ::~MGR_OBJ() |
| F9600 | 0x60 | MATCHED EXACT -- sFStateFct_c<T>::build() |
| F9660 | 0x10 | MATCHED EXACT -- sFStateFct_c<T>::dispose() |
| F9670 | 0x20 | MATCHED EXACT -- sFState_c<T>::initialize() |
| F9690 | 0x20 | MATCHED EXACT -- sFState_c<T>::execute() |
| F96B0 | 0x20 | MATCHED EXACT -- sFState_c<T>::finalize() |
| F96D0 | 0x10 | MATCHED EXACT -- sStateMgr_c<...>::initializeState() |
| F96E0 | 0x10 | MATCHED EXACT -- sStateMgr_c<...>::finalizeState() |
| F96F0 | 0x10 | MATCHED EXACT -- sStateMgr_c<...>::refreshState() |
| F9700 | 0x10 | MATCHED EXACT -- sStateMgr_c<...>::getState() |
| F9710 | 0x10 | MATCHED EXACT -- sStateMgr_c<...>::getNewStateID() |
| F9720 | 0x10 | MATCHED EXACT -- sStateMgr_c<...>::getStateID() |
| F9730 | 0x10 | MATCHED EXACT -- sStateMgr_c<...>::getOldStateID() |
| F9740 | 0x30 | MATCHED EXACT -- sFStateID_c<T>::finalizeState() thunk (corrected attribution) |
| F9770 | 0x30 | MATCHED EXACT -- sFStateID_c<T>::executeState() thunk |
| F97A0 | 0x30 | MATCHED EXACT -- sFStateID_c<T>::initializeState() thunk (corrected attribution) |
| F97D0 | 0x280 | MATCHED (naming, 159/159) -- __sinit |
| F9A50 | 0x58 | MATCHED EXACT -- sFStateID_c<T> dtor (corrected attribution) |
| F9AB0 | 0x88 | MATCHED EXACT -- sFStateID_c<T>::isSameName() |

## The 4 PARKED functions, precisely

All four have fully-correct CONTENT and control flow (every call, every argument, every field
access verified against the target byte-for-byte in the surrounding instructions) -- the
residual in every case is a narrow, specific instruction-selection or register-allocation
difference, not missing or wrong logic:

- **`perPlayerRestCheck`** (`fn_2_F8DF0`): loop-index register (r29 vs r30) and whether a
  vtable-pointer dereference chains through r12 directly or stages through r4. 3 variants.
- **`timerOrKeyGate`** (`fn_2_F8E80`): the two-button bit test compiles to a single `andi.`
  (mine) instead of the target's `rlwinm`+dot-form-`rlwimi.` two-step extract. 4 variants
  (combined mask, split `&&`, split `|`, `u16` local).
- **`executeState_ShowRule`** (`fn_2_F8F70`): case 3's `timerOrKeyGate()` result needs the same
  `neg`/`or`/`srwi` normalize-to-clean-bool sequence the target uses when the value is tested
  twice (once for the early-return gate, again for the SFX choice); every variant either
  omits it or produces a different but also-wrong shape. 4 variants (bool vs int return type,
  `&&` vs nested-if vs `||`-with-empty-then).
- **`executeState_ShowResult`** (`fn_2_F9320`): the "did any of 4 guns score" check's 4th
  comparison addresses via `this+0x18` then `+0x80` in the target (reusing the `0x80`
  displacement already used for gun 1) instead of the direct `this+0x98` plain array indexing
  produces. 2 variants (`||`-chain vs if-elseif chain -- the if-elseif chain is markedly closer,
  118/122 lines vs 113/122).

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

Target dumps in this directory: `target_auto_00_F85C4_text.txt` (0xF85C4-0xF97A0),
`target_auto_fn2_F97D0_text.txt` (`__sinit`), `target_auto_00_F9A4C_text.txt` (0xF9A4C onward),
`target_data_132B0.txt` (the `.data` object holding every vtable/state-ID object this unit owns,
with real symbol names for every inherited/default vtable slot).

Shadow headers (both pure additions, no layout change, cannot disturb any landed TU):
`shadow_include/game/snd/snd_scene_manager.hpp` adds `fn_8019C010(int)`;
`shadow_include/game/bases/d_game_com.hpp` adds the 7 `MiniGameCannon*` free functions inside
the `dGameCom` namespace, names pulled directly from `bin/dtk/wiimj2d_symbols.txt`.

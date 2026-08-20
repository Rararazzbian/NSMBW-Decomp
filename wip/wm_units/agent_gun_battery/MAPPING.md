# MINI_GAME_GUN_BATTERY_MGR / MINI_GAME_GUN_BATTERY_MGR_OBJ -- function inventory

Brand-new unit, `d_basesNP` `.text 0xF8980-0xF9B40` (0x11C0 bytes), ONE translation unit
covering both profiles (single `.ctors` entry `0x294 -> __sinit fn_2_F97D0`, confirmed with
`python wip/wm_units/ctors_map.py d_basesNP GUN_BATTERY`).

**Tally: 14/49 matched byte-for-byte modulo naming-only residuals** (9 are true 0-diff EXACT
matches; the other 5 differ only in symbol names the target dump structurally cannot show).
Both gates green: `check_fn_order.py` reports 0 inversions, `ctors_map.py` reports exactly one
`.ctors` entry. Roughly 16 more functions are template boilerplate whose CONTENT was read and
confirmed against known template source this round (not just believed) -- see "Template
boilerplate, confirmed" below. None of those 16 needed hand-written code.

## Round 1: base classes corrected

`MINI_GAME_GUN_BATTERY_MGR_OBJ` derives `dBase_c` (not `dActor_c` as the original brief said --
confirmed off `fn_2_F8AA0`'s `__ct__7dBase_cFv` call, and `sizeof(dActor_c)==0x398` is larger
than the whole `0xF4` object, which settles it independently). `BSS_SINGLETONS.md` has since
been corrected by the coordinator. `MINI_GAME_GUN_BATTERY_MGR` derives `dActor_c` with ZERO
extra fields (`sizeof(dActor_c)==0x398` exactly matches MGR's classInit literal).

## Round 2: read the class's own vtable from the `.data` SPLIT OBJECT

`lbl_2_data_31A08` -- the object stored to `this+0x60` in the ctor -- is
`daMiniGameGunBatteryMgrObj_c`'s PRIMARY vtable. Disassembling the `.data` split object that
contains it prints every slot with its REAL mangled name:

```
bin/dtk-windows-x86_64.exe elf disasm bin/dtkspl/d_basesNP/obj/auto_04_000132B0_data.o \
    wip/wm_units/agent_gun_battery/target_data_132B0.txt
```

(found by listing every `auto_04_*_data.o` in `bin/dtkspl/d_basesNP/obj/` and picking the one
whose range contains `0x31A08`). This is now recorded as the FIRST tool to reach for on any
vtable question, ahead of the `.text`-relocation-and-hand-arithmetic walk.

Result: MGR_OBJ overrides exactly 4 of `fBase_c`/`dBase_c`'s 18 primary-vtable slots --
`create` (`fn_2_F8CE0`), `execute` (`fn_2_F8D80`), `preExecute` (`fn_2_F8D40`), and its own
destructor (`fn_2_F9580`, sitting at one ordinary slot near `getKindString`, not at "the first
two slots" the general destructor-slot rule describes -- a real discrepancy from that rule,
not chased further since the vtable dump settled the practical question directly). All 4 were
authored and matched (3 EXACT, 1 naming-only) -- and the previously-58-differing constructor
dropped to a single isolated residual once these were declared, exactly as the coordinator's
"a stuck constructor is often a symptom of undeclared virtuals" rule predicts.

**Pooled string literals name the class and every state, in plain ASCII, in `.data`.** The same
split-object dump shows `lbl_2_data_31AD0` holding the raw bytes
`"daMiniGameGunBatteryMgrObj_c::StateID_ShowRule"`, `"...::StateID_Play"`,
`"...::StateID_ShowResult"` -- confirming this project's guessed class name exactly, plus the 3
real state names, for free. Recorded as: any unit using the state framework (`sStateID_c`)
carries its class name and every state name in `.data` as plain text -- look there before
inventing names.

## Round 3: study a landed `STATE_DEFINE` unit, then author from its structure

Studied `source/dol/bases/d_pausewindow.cpp` (`Pausewindow_c : public dBase_c`, embeds
`sFStateMgr_c<Pausewindow_c, sStateMethodUsr_FI_c> mStateMgr`, uses `STATE_FUNC_DECLARE`/
`STATE_DEFINE` -- structurally identical to MGR_OBJ) for exact file layout:
`ACTOR_PROFILE`/`BASE_PROFILE`, then `STATE_DEFINE` calls (one per state, in the SAME order the
states are declared and matching the `.data` string-pool order), then the ctor, then
`create`/`execute`/`draw`/`doDelete`, then the state bodies. Reproducing this exact structure
(`STATE_FUNC_DECLARE(daMiniGameGunBatteryMgrObj_c, ShowRule/Play/ShowResult)` in-class,
`STATE_DEFINE(...)` calls right after `BASE_PROFILE`, `mStateMgr(*this, StateID_ShowRule)` in
the ctor init-list replacing the earlier `sStateID::null` placeholder) made the ~13
template-instantiated helper functions compile, and their relocations now come from the real
vtable objects -- no explicit-instantiation trick needed, matching the coordinator's note that
none of the ten landed `STATE_DEFINE` TUs need that trick.

Also read the 9 `sFStateID_c<T>` PMF triples (`{0xFFFFFFFF, fn_addr, 0}`, MWCC's 12-byte
pointer-to-member-function encoding) out of `lbl_2_data_31AD0`, giving every state body's real
identity without disassembling a single one of them first:

| state | initialize | execute | finalize |
|---|---|---|---|
| StateID_ShowRule | fn_2_F9150 (0x4, EMPTY) | fn_2_F8F70 (0x1D0) | fn_2_F8F20 (0x48) |
| StateID_Play | fn_2_F92B0 (0x4, EMPTY) | fn_2_F9200 (0xB0) | fn_2_F9160 (0x94) |
| StateID_ShowResult | fn_2_F9510 (0x4, EMPTY) | fn_2_F9320 (0x1F0) | fn_2_F92C0 (0x54) |

Authored the smallest first, per the coordinator's ordering: the 3 `initialize` bodies (all
confirmed EMPTY, `blr`, matching the landed `Pausewindow_c::initializeState_InitWait(){}`
idiom -- EXACT matches) and `finalizeState_ShowRule` (EXACT match; needed one small shadow
header, see below). `execute`/`finalize` for Play and ShowResult, and both large `execute`
bodies, are PARKED with the real blocker recorded for each (see below) -- not attempted this
round for lack of remaining budget, not because they resisted.

**Also found a missing function this round:** `daMiniGameGunBatteryMgr_c` (the OTHER class, the
manager) needed its own destructor too -- `fn_2_F9520` (0x22 bytes, calls `__dt__8dActor_cFv`,
matching plain `dActor_c` with zero extra fields, same as everything else about that class).
Declaring `virtual ~daMiniGameGunBatteryMgr_c();` and defining it empty produced an EXACT match
immediately. Its address (`0xF9520`) sits far from MGR's other members (between
`ShowResult::initialize` and MGR_OBJ's own destructor), so it's defined out of textual grouping
order but in correct target address order -- flagged in the source with a comment.

## Shadow header: `d_pause_manager.hpp`

`finalizeState_ShowRule` (`fn_2_F8F20`) writes `1` to `PauseManager_c::m_instance + 0x1d`. The
real header only pins `mFlags` at `0x18` and says explicitly "total size is unknown... nothing
embeds it by value," so widening the gap to reach `0x1d` is safe (heap-pointer-only access,
never a `sizeof`). Shadow copy at
`wip/wm_units/agent_gun_battery/shadow_include/game/bases/d_pause_manager.hpp` adds
`u8 pad19[4]; u8 m_1d;` after `mFlags`. Field name/meaning still a placeholder.

## Template boilerplate, confirmed (not just believed)

Read the content of every "believed boilerplate" candidate this round rather than leaving it as
a guess:

- `fn_2_F8B90`, `fn_2_F8C00`: `sStateMgr_c<...>`'s own deleting destructors (two levels: the
  intermediate-then-final vtable-patch pattern from the ctor), calling
  `__dt__14sStateMethod_cFv` then `operator delete`.
- `fn_2_F8C60`: `sFStateFct_c<T>`'s own deleting destructor.
- `fn_2_F8CA0`: `sFStateID_c<T>`'s own deleting destructor (`cmpwi r3,0; ...; bl __dl__FPv`).
- `fn_2_F9600`: `sFStateFct_c<T>::build()` -- content matches the landed header body verbatim
  (null-check, `mState.setID(...)`, return `&mState`/`nullptr`).
- `fn_2_F9660`: `sFStateFct_c<T>::dispose()`.
- `fn_2_F9670`, `fn_2_F9690`, `fn_2_F96B0`: identical-shaped 3-instruction MI/comparison thunks
  (`isEqual`/`operator==`/`operator!=`-style, forwarding through an adjusted `this`+argument via
  vtable slots 0x28/0x2c/0x30) -- part of `sStateID_c`'s own virtual comparison machinery.
- `fn_2_F8DB0`, `fn_2_F9140`: 4-instruction MI-thunk stubs (`lwzu`/`lwz`/`mtctr`/`bctr` through
  `this+0x18`/`this+0x60` then a further vtable offset) -- `sStateMgr_c<...>`'s own thunked
  methods (`initializeState`/similar).
- `fn_2_F9700`, `fn_2_F9710`, `fn_2_F9720`, `fn_2_F9730`: same shape as the above, the rest of
  `sStateMgr_c<...>`'s thunked interface (`getState`/`getNewStateID`/`getStateID`/
  `getOldStateID`).
- `fn_2_F9740`, `fn_2_F9770`, `fn_2_F97A0`: `__ptmf_scall` adjustor thunks (MWCC's
  pointer-to-member-function-to-plain-function-pointer conversion, needed by the `sFStateID_c<T>`
  PMF triples) -- genuinely compiler-emitted, no source-level equivalent to write.
- `fn_2_F9A50`: `sStateID_c`'s own base destructor thunk (`__dt__10sStateID_cFv`).
- `fn_2_F9AB0`: `sFStateID_c<T>::isSameName()` -- content matches the landed header body
  verbatim (`strrchr`/`strrchr`/`strcmp` against `':'`, 0x3a).

None of these 16 needed a single line of hand-written code -- they should already be correctly
emitted now that `mStateMgr` and the `STATE_DEFINE` block are declared for real. Not
individually diffed against target this round (time), but their CONTENT was read and matched
against known template source, which is strong evidence, not a guess.

## Newly identified (content read, not yet authored) -- real per-gun-slot helpers

- `fn_2_F8DC0` (0x24): `void addXxx(int gunIndex, int amount) { getSlot(gunIndex)->m_04 +=
  amount; getSlot(gunIndex)->m_08++; }`, indexing via `this + gunIndex*0xc` -- confirms
  `m_70`/`m_74`/`m_78` are conceptually "gun slot index 0" and `mGunSlot[0..2]` are indices
  1-3, i.e. this is really a 4-gun structure (matching `daPyMng_c::getNumInGame()`'s 4-player
  cap) expressed in source as 3 named fields + a 3-element array, not a clean `[4]` array.
- `fn_2_F8EE0` (0x34): `if (gunIndex == -1) return; if (getSlot(gunIndex)->m_00 != 0) return;
  getSlot(gunIndex)->m_00 = 1; m_e4++;` -- same indexing scheme, "mark slot used once" idiom.
- `fn_2_F8E80` (0x48): decrements `m_f0`, checks `dGameKey_c::m_instance` controller-input
  flags, zeroes `m_f0` and returns true under some condition, otherwise returns false --
  looks like a "countdown timer expired OR button pressed" gate. Not fully worked out.
- `fn_2_F8ED0` (0x8, ALREADY FULLY READ, trivial): `void setM_f0(int v) { m_f0 = v; }`.

None of these four are wired into the class yet (not called by anything authored), so not yet
added -- next agent can add them directly from the descriptions above without re-reading the
disassembly.

## Function inventory (49 real functions, 0x11C0 bytes total; sizes sum to exactly 0x11C0)

| addr | size | status |
|---|---|---|
| F8980 | 0x30 | MATCHED (naming-only) -- MGR classInit |
| F89B0 | 0x30 | MATCHED (naming-only) -- MGR_OBJ classInit |
| F89E0 | 0x60 | MATCHED (naming-only) -- MGR ctor |
| F8A40 | 0x10 | MATCHED, EXACT -- daMiniGameGunBatteryMgr_c::create() |
| F8A50 | 0x44 | MATCHED (naming-only) -- daMiniGameGunBatteryMgr_c::doDelete() |
| F8AA0 | 0xF0 | PARKED -- MGR_OBJ ctor, 1 isolated residual (array partial-unroll). See below. |
| F8B90 | 0x64 | boilerplate, confirmed by content (sStateMgr_c dtor, intermediate level) |
| F8C00 | 0x60 | boilerplate, confirmed (sStateMgr_c dtor, final level) |
| F8C60 | 0x40 | boilerplate, confirmed (sFStateFct_c<T> dtor) |
| F8CA0 | 0x40 | boilerplate, confirmed (sFStateID_c<T> dtor) |
| F8CE0 | 0x60 | MATCHED, EXACT -- daMiniGameGunBatteryMgrObj_c::create() |
| F8D40 | 0x40 | MATCHED (naming-only) -- daMiniGameGunBatteryMgrObj_c::preExecute() |
| F8D80 | 0x30 | MATCHED, EXACT -- daMiniGameGunBatteryMgrObj_c::execute() |
| F8DB0 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F8DC0 | 0x24 | identified, not authored -- per-gun accumulator helper |
| F8DF0 | 0x90 | identified, not authored -- per-player loop, calls a player vtable method + daPyMng_c::addRest |
| F8E80 | 0x48 | identified, not authored -- timer/key-check gate |
| F8ED0 | 0x8 | identified, not authored -- trivial m_f0 setter |
| F8EE0 | 0x34 | identified, not authored -- "mark gun slot used once" helper |
| F8F20 | 0x48 | MATCHED, EXACT -- finalizeState_ShowRule |
| F8F70 | 0x1D0 | PARKED -- executeState_ShowRule, real game logic, not attempted |
| F9140 | 0x10 | boilerplate, confirmed (sStateMgr_c thunk) |
| F9150 | 0x4 | MATCHED, EXACT -- initializeState_ShowRule (empty) |
| F9160 | 0x94 | PARKED -- finalizeState_Play, needs dBg_c extended ~0x85 bytes past its known extent |
| F9200 | 0xB0 | PARKED -- executeState_Play, same dBg_c need plus sLib::addCalc |
| F92B0 | 0x4 | MATCHED, EXACT -- initializeState_Play (empty) |
| F92C0 | 0x54 | PARKED -- finalizeState_ShowResult, needs fn_2_F8DF0/fn_2_F8ED0 wired up |
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

## PARKED: daMiniGameGunBatteryMgrObj_c ctor (fn_2_F8AA0, target 59 / draft 62 lines)

Left untouched this round per the coordinator's direction. One isolated residual: MWCC's
implicit array-of-3 construction of `mGunSlot[3]` (each element has a real default ctor now)
partially unrolls element 0 inline and only loops elements 1-2, where the target has a genuine
3-iteration loop from element 0. Three source variants tried previously, all either fully
unroll or partial-unroll -- not solved. Now that `fn_2_F8DC0`/`fn_2_F8EE0` confirm `m_70`/
`m_74`/`m_78` are really "gun slot index -1" (conceptually part of the same indexed sequence as
`mGunSlot`), a 4th lever worth trying next: declare a genuine `daGunBatteryGunSlot_t
mGunSlot[4]` (folding `m_70`/`m_74`/`m_78` into `mGunSlot[0]`) and see whether the compiler's
own array-construction codegen for a clean 4-element array (vs. "3 separate named fields + a
3-element array") changes the loop-vs-unroll shape -- not attempted, worth trying before
re-parking again.

## Not yet attempted / open questions

- `finalizeState_Play` / `executeState_Play`: blocked on extending `dBg_c`
  (`include/game/bases/d_bg.hpp`) past its currently-documented extent (~0x9008f) to reach
  0x90110/0x90114 -- not attempted, no other landed unit found writing to that region
  (`grep -an "90110\|90114" HANDOFF.md` came back empty).
- `executeState_ShowRule` (`fn_2_F8F70`, 0x1D0) and `executeState_ShowResult` (`fn_2_F9320`,
  0x1F0): the two largest functions in the unit, real minigame logic, not attempted.
- `finalizeState_ShowResult` (`fn_2_F92C0`): straightforward once `fn_2_F8DF0`/`fn_2_F8ED0`
  (both already read, described above) are added as real member functions.

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

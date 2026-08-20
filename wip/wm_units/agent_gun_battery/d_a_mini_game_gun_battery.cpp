#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_actor.hpp>
#include <game/bases/d_base.hpp>
#include <game/sLib/s_FStateMgr.hpp>
#include <game/sLib/s_StateMethodUsr_FI.hpp>
#include <game/sLib/s_State.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_pause_manager.hpp>

/// @unofficial `fn_2_420`, called from `preExecute()` -- outside our claim, not yet
/// landed. Signature guessed as no-arg bool predicate (its call site never sets an
/// argument register explicitly; r3 just happens to hold `preExecute`'s own return
/// value at that point, unused by the callee). Linked via the project's
/// `R_2_1_<offset>` convention for un-landed same-REL callees.
extern "C" int R_2_1_420();

/// @unofficial Provisional reconstruction of the mini-game "gun battery" manager
/// pair.
///
/// **CORRECTION to the coordinator's brief**: `MINI_GAME_GUN_BATTERY_MGR_OBJ`
/// does NOT derive from `dActor_c`. Its constructor (`fn_2_F8AA0`) calls
/// `__ct__7dBase_cFv` directly (`dBase_c::dBase_c()`), not any `dActor_c`
/// ctor -- confirmed straight off the mangled name, unambiguous. The
/// `dActor_c` finding in the brief ("Base class dActor_c... creates a child
/// via createChild... at 0xF8A14") describes `fn_2_F89E0`, which is the
/// **MGR**'s own ctor, not MGR_OBJ's -- both cited offsets (0xF89F4, 0xF8A14)
/// disassemble inside fn_2_F89E0 (0xF89E0-0xF8A34), confirmed directly.
/// `sizeof(dActor_c)` was probed via the template-error trick and is 0x398 --
/// which is exactly `MINI_GAME_GUN_BATTERY_MGR`'s own classInit literal, i.e.
/// **MGR** is `dActor_c` with ZERO extra fields (it derives dActor_c, adds no
/// data, only overrides `create()`/`doDelete()`). `sizeof(dBase_c)` probed at
/// 0x70, matching MGR_OBJ's own fields starting immediately at `this+0x70`.
///
/// MGR_OBJ also embeds a real state-machine member,
/// `sFStateMgr_c<daMiniGameGunBatteryMgrObj_c, sStateMethodUsr_FI_c> mStateMgr`
/// (same pattern as the landed `Pausewindow_c`, source/dol/bases/d_pausewindow.cpp)
/// -- confirmed by the ctor calling
/// `__ct__20sStateMethodUsr_FI_cFR15sStateIDChkIf_cR13sStateFctIf_cRC12sStateIDIf_c`
/// with `this+0xb8` as the Method's own address, `this+0xa4`/`this+0xa8` as the
/// Check/Factory references -- exactly `sStateMgr_c`'s declared member order
/// (`mCheck`, `mFactory`, `mMethod`) and `sFStateFct_c<T>`'s own layout
/// (own vtable + embedded `sFState_c<T> mState` = vtable+owner-ref+id-ptr).
/// Computed layout, all confirmed against the ctor's own stores:
///   0x70 dBase_c ends
///   0x70 u8 (0), 0x74 s32 (0), 0x78 s32 (-1)        -- @unofficial, PLACEHOLDER names/types
///   0x7c mGunSlot[3]  -- 12 bytes each: u8(0), pad3, s32(0), s32(-1)  -- PLACEHOLDER
///   0xa0 mStateMgr (sFStateMgr_c<...>, 0x3c bytes, ends 0xdc)
///   0xdc s32 (=7 in create()), 0xe0 u8 (=0), 0xe4 s32 (=0),
///   0xe8 s32 (=daPyMng_c::getNumInGame() in create()), 0xec s32 (=0), 0xf0 s32 (=0)
///   -- ends exactly at 0xf4. All 6 fields read directly off `create()` (`fn_2_F8CE0`).
///
/// **Round 2: read the class's own vtable directly.** `lbl_2_data_31A08` (the object
/// stored to `this+0x60` in the ctor) is `daMiniGameGunBatteryMgrObj_c`'s PRIMARY
/// vtable, dumped in full with real symbol names via
/// `bin/dtk-windows-x86_64.exe elf disasm bin/dtkspl/d_basesNP/obj/auto_04_000132B0_data.o`
/// (see `wip/wm_units/agent_gun_battery/target_data_132B0.txt:37987`). Its 18 real
/// entries (2 leading null header words, then 18 function pointers) pair EXACTLY with
/// `fBase_c`'s 16 declared virtuals + `dBase_c`'s own 2 (in declaration order,
/// confirmed word-for-word against `bin/dtk/wiimj2d_symbols.txt`):
///   create=fn_2_F8CE0(OURS) preCreate=default postCreate=default doDelete=default
///   preDelete=default postDelete=default execute=fn_2_F8D80(OURS)
///   preExecute=fn_2_F8D40(OURS) postExecute=default draw=default preDraw=default
///   postDraw=default deleteReady=default entryFrmHeap=default
///   entryFrmHeapNonAdjust=default createHeap=default THEN fn_2_F9580(OURS) THEN
///   getKindString=default. So MGR_OBJ overrides exactly FOUR of fBase_c/dBase_c's
///   own virtuals (create/execute/preExecute), plus ONE more slot immediately
///   before `getKindString` in declaration order -- `fn_2_F9580`'s own content
///   (calls `sStateMethod_c` dtor conditionally, then `__dt__7dBase_cFv`, then
///   `__dl__7fBase_cFPv`) is unmistakably `~daMiniGameGunBatteryMgrObj_c()` itself.
///
/// The SAME data dump also gave, for free, byte-verified confirmation of the exact
/// class name (a pooled string literal reads
/// `"daMiniGameGunBatteryMgrObj_c::StateID_ShowRule"`,
/// `"...::StateID_Play"`, `"...::StateID_ShowResult"` at `lbl_2_data_31AD0` --
/// this project's guessed name was exactly right, verified not inferred) and the
/// state handler triples (`sFStateID_c<T>`'s init/execute/finalize member-function
/// pointers, MWCC's 12-byte PMF encoding, `{0xFFFFFFFF, fn_addr, 0}`):
///   StateID_ShowRule:  init=fn_2_F9150 execute=fn_2_F8F70 finalize=fn_2_F8F20
///   StateID_Play:      init=fn_2_F92B0 execute=fn_2_F9200 finalize=fn_2_F9160
///   StateID_ShowResult:init=fn_2_F9510 execute=fn_2_F9320 finalize=fn_2_F92C0
/// **This corrects the assumption that the two large functions (`fn_2_F8F70`
/// 0x1D0, `fn_2_F9320` 0x1F0) are MGR_OBJ's `execute()` override** -- `execute()`
/// itself is `fn_2_F8D80` (tiny, forwards to `mStateMgr.executeState()`). The two
/// large functions are each ONE STATE's execute-phase body instead (ShowRule's and
/// Play's respectively), reached through the state machine's own dispatch, not the
/// C++ vtable. Not yet authored -- the `sFStateID_c<T>` array/`STATE_DEFINE`-style
/// declarations that would produce `lbl_2_data_31AD0` are the next concrete step.
///
/// `lbl_2_data_31A58`/`lbl_2_data_31A88` are `mStateMgr`'s own two vtables
/// (intermediate `sStateMgr_c<T,...>` level, then final `sFStateMgr_c<T,...>`
/// level -- both auto-generated by the compiler from the already-landed template
/// headers, matching this project's own already-generated weak symbols by name:
/// `__dt__97sStateMgr_c<...>Fv` etc.). `lbl_2_data_31AB8` is `sFStateFct_c<T>`'s
/// own vtable (`~dtor`=fn_2_F8C60, `build`=fn_2_F9600, `dispose`=fn_2_F9660 --
/// `fn_2_F9600`'s content matches `sFStateFct_c<T>::build()`'s header body
/// verbatim: null-check, `mState.setID(...)`, return `&mState`/`nullptr`). None of
/// these four need hand-written code -- they should already emit correctly once
/// `mStateMgr` is a real member, which it already is.
class daMiniGameGunBatteryMgr_c : public dActor_c {
public:
    daMiniGameGunBatteryMgr_c();
    virtual ~daMiniGameGunBatteryMgr_c(); ///< `fn_2_F9520`. Calls `__dt__8dActor_cFv` --
    ///< matches plain `dActor_c` with zero extra fields, same as everything
    ///< else about this class.

    virtual int create();
    virtual int doDelete();
};

class daMiniGameGunBatteryMgrObj_c;

struct daGunBatteryGunSlot_t { ///< @unofficial PLACEHOLDER name/types, 0xc bytes, confirmed by the ctor's init loop.
    daGunBatteryGunSlot_t() : m_00(0), m_04(0), m_08(-1) {} ///< @unofficial matches sizeof/offset exactly (probed), but its
    ///< IMPLICIT array-of-3 construction partially unrolls (element 0 inline,
    ///< elements 1-2 as a real loop) where the target has a genuine 3-iteration
    ///< loop from element 0 -- PARKED, see class comment.
    u8 m_00;
    s32 m_04;
    s32 m_08;
};

class daMiniGameGunBatteryMgrObj_c : public dBase_c {
public:
    daMiniGameGunBatteryMgrObj_c();
    virtual ~daMiniGameGunBatteryMgrObj_c(); ///< `fn_2_F9580`. See class comment.

    virtual int create(); ///< `fn_2_F8CE0`.
    virtual int preExecute(); ///< `fn_2_F8D40`.
    virtual int execute(); ///< `fn_2_F8D80`.

    /// @unofficial 3 states, names verified byte-for-byte from a pooled string
    /// literal at `lbl_2_data_31AD0` ("daMiniGameGunBatteryMgrObj_c::StateID_
    /// ShowRule" etc, `target_data_132B0.txt`). Declaration order matches the
    /// string pool order (ShowRule, Play, ShowResult).
    STATE_FUNC_DECLARE(daMiniGameGunBatteryMgrObj_c, ShowRule);
    STATE_FUNC_DECLARE(daMiniGameGunBatteryMgrObj_c, Play);
    STATE_FUNC_DECLARE(daMiniGameGunBatteryMgrObj_c, ShowResult);

    u8 m_70; ///< @unofficial PLACEHOLDER
    s32 m_74; ///< @unofficial PLACEHOLDER
    s32 m_78; ///< @unofficial PLACEHOLDER
    daGunBatteryGunSlot_t mGunSlot[3]; ///< @unofficial PLACEHOLDER, 0x7c-0xa0

    sFStateMgr_c<daMiniGameGunBatteryMgrObj_c, sStateMethodUsr_FI_c> mStateMgr; ///< 0xa0-0xdc

    s32 m_dc; ///< @unofficial PLACEHOLDER, init 7 in create().
    u8 m_e0; ///< @unofficial PLACEHOLDER, init 0 in create().
    s32 m_e4; ///< @unofficial PLACEHOLDER, init 0 in create().
    s32 m_e8; ///< @unofficial PLACEHOLDER, init daPyMng_c::getNumInGame() in create().
    s32 m_ec; ///< @unofficial PLACEHOLDER, init 0 in create().
    s32 m_f0; ///< @unofficial PLACEHOLDER, init 0 in create().
};

/// @unofficial Singleton pointer to the single spawned MGR_OBJ child.
/// `.bss lbl_2_bss_C460`. Create site: MGR's ctor (`fn_2_F8A1C`). Destroy site:
/// MGR's `doDelete` (`fn_2_F8A78`).
static daMiniGameGunBatteryMgrObj_c *s_pMgrObj;

// classInit functions are emitted here (matching target address order:
// fn_2_F8980 == MGR classInit, fn_2_F89B0 == MGR_OBJ classInit, BOTH before
// the ctor/create/doDelete bodies below -- the linker places .text in
// DEFINITION order, so these macros must appear textually first).
ACTOR_PROFILE(MINI_GAME_GUN_BATTERY_MGR, daMiniGameGunBatteryMgr_c, 0);
BASE_PROFILE(MINI_GAME_GUN_BATTERY_MGR_OBJ, daMiniGameGunBatteryMgrObj_c);

STATE_DEFINE(daMiniGameGunBatteryMgrObj_c, ShowRule);
STATE_DEFINE(daMiniGameGunBatteryMgrObj_c, Play);
STATE_DEFINE(daMiniGameGunBatteryMgrObj_c, ShowResult);

daMiniGameGunBatteryMgr_c::daMiniGameGunBatteryMgr_c() {
    s_pMgrObj = (daMiniGameGunBatteryMgrObj_c *)fBase_c::createChild(
        fProfile::MINI_GAME_GUN_BATTERY_MGR_OBJ, this, 0, 0);
}

int daMiniGameGunBatteryMgr_c::create() {
    return SUCCEEDED;
}

int daMiniGameGunBatteryMgr_c::doDelete() {
    if (s_pMgrObj) {
        s_pMgrObj->deleteRequest();
        s_pMgrObj = nullptr;
    }
    return SUCCEEDED;
}

daMiniGameGunBatteryMgrObj_c::daMiniGameGunBatteryMgrObj_c() :
    m_70(0), m_74(0), m_78(-1),
    mStateMgr(*this, StateID_ShowRule)
{
}

int daMiniGameGunBatteryMgrObj_c::create() {
    m_e0 = 0;
    m_e4 = 0;
    m_e8 = daPyMng_c::getNumInGame();
    m_ec = 0;
    m_f0 = 0;
    m_dc = 7;
    return SUCCEEDED;
}

int daMiniGameGunBatteryMgrObj_c::preExecute() {
    return !dBase_c::preExecute() ? 0 : !R_2_1_420();
}

int daMiniGameGunBatteryMgrObj_c::execute() {
    mStateMgr.executeState();
    return SUCCEEDED;
}

// State bodies, in TARGET ADDRESS order (finalize, execute, initialize per
// state -- NOT the STATE_FUNC_DECLARE-declared init/exec/final order; this
// project's own definition order is what the linker uses for .text
// placement, and this unit's own author happened to write finalize first).
// ShowRule: fn_2_F8F20(final) fn_2_F8F70(exec) fn_2_F9150(init)
// Play:     fn_2_F9160(final) fn_2_F9200(exec) fn_2_F92B0(init)
// ShowResult: fn_2_F92C0(final) fn_2_F9320(exec) fn_2_F9510(init)

void daMiniGameGunBatteryMgrObj_c::finalizeState_ShowRule() {
    m_ec = 0;
    m_dc = 0;
    m_e0 = 0;
    daPyDemoMng_c::mspInstance->startControlDemoAll();
    PauseManager_c::m_instance->m_1d = 1;
}

/// @unofficial PARKED -- not yet authored (0x1D0 bytes, real game logic).
/// `fn_2_F8F70`.
void daMiniGameGunBatteryMgrObj_c::executeState_ShowRule() {
}

/// @unofficial `fn_2_F9150` -- confirmed EMPTY (`blr`, 4 bytes), matches the
/// landed `Pausewindow_c::initializeState_InitWait(){}` idiom exactly.
void daMiniGameGunBatteryMgrObj_c::initializeState_ShowRule() {
}

/// @unofficial PARKED -- not yet authored. Needs `dBg_c` extended past its
/// current known extent (`include/game/bases/d_bg.hpp` ends around raw
/// offset 0x9008f; this function touches 0x90110/0x90114, ~0x85 bytes
/// further, unexplored and not grepped against any other landed user of
/// that region this round). `fn_2_F9160`.
void daMiniGameGunBatteryMgrObj_c::finalizeState_Play() {
}

/// @unofficial PARKED -- not yet authored. Same `dBg_c` extension need as
/// `finalizeState_Play`, plus `sLib::addCalc(float*, float, float, float,
/// float)`. `fn_2_F9200`.
void daMiniGameGunBatteryMgrObj_c::executeState_Play() {
}

/// @unofficial `fn_2_F92B0` -- confirmed EMPTY (`blr`, 4 bytes).
void daMiniGameGunBatteryMgrObj_c::initializeState_Play() {
}

/// @unofficial PARKED -- not yet authored. Needs two more member helpers
/// (`fn_2_F8DF0`, a per-player loop over `daPyMng_c::getPlayer()` that reads
/// `m_74`/`mGunSlot[i].m_04` via raw pointer arithmetic across the field
/// boundary -- confirmed by disassembly, not yet written; and `fn_2_F8ED0`,
/// a trivial one-line setter for `m_f0`, disassembly already read and
/// trivial to write, just not wired up yet). `fn_2_F92C0`.
void daMiniGameGunBatteryMgrObj_c::finalizeState_ShowResult() {
}

/// @unofficial PARKED -- not yet authored (0x1F0 bytes, real game logic).
/// `fn_2_F9320`.
void daMiniGameGunBatteryMgrObj_c::executeState_ShowResult() {
}

/// @unofficial `fn_2_F9510` -- confirmed EMPTY (`blr`, 4 bytes).
void daMiniGameGunBatteryMgrObj_c::initializeState_ShowResult() {
}

// fn_2_F9520 -- daMiniGameGunBatteryMgr_c's OWN destructor. Its target
// address (0xF9520) sits between ShowResult::initialize (0xF9510) and
// MGR_OBJ's own destructor (0xF9580), i.e. textually far from MGR's other
// members above -- kept here, in true address order, rather than grouped
// with the rest of daMiniGameGunBatteryMgr_c's definitions.
daMiniGameGunBatteryMgr_c::~daMiniGameGunBatteryMgr_c() {
}

// fn_2_F9580 -- defined last: its target address (0xF9580) comes AFTER many
// other functions this unit has not authored yet (0xF8DB0..0xF9510). Kept
// here (out of true address order) only until those are written; a real
// function-order violation exists between this destructor and whatever gets
// authored next in that gap -- WATCH THIS when resuming.
daMiniGameGunBatteryMgrObj_c::~daMiniGameGunBatteryMgrObj_c() {
}

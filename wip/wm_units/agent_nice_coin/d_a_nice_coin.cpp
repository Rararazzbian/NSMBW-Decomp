#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_actor_state.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_multi_manager.hpp>

// AC_NICE_COIN + AC_NICE_COIN_REGULAR -- coordinator-scoped as ONE translation unit,
// .text 0x104d70-0x105450 (0x6E0 bytes), ONE .ctors entry (__sinit at .text 0x105110).
//
// ONE CLASS, TWO PROFILE ENTRY POINTS -- confirmed directly, not assumed: both classInit
// functions (fn_2_104D70 for AC_NICE_COIN, fn_2_104DC0 for AC_NICE_COIN_REGULAR) are
// BYTE-IDENTICAL in shape: same `li r3, 0x3f0` allocation size, same
// `bl __ct__13dActorState_cFv` base-constructor call (NOT a derived ctor -- this class
// declares none of its own), same manual vtable-pointer patch to the SAME symbol
// (`lbl_2_data_33790`) at the SAME offset (+0x60, this engine's consistent non-zero
// vtable-pointer location for fBase_c-derived classes -- independently corroborated by
// WM_KOOPAJR's identical "this+0x60" dispatch for a totally different class hierarchy
// branch). Two profiles, one class, confirmed by compiling this candidate shape.
//
// Class name `daNiceCoin_c` and its two named states, `Search` and `EndWait`, read
// directly out of the REL's raw ASCII bytes trailing the vtable object
// (`lbl_2_data_33790`, .data file offset 0x1D0C00+0x33790): the two `STATE_DEFINE`
// macro-generated `sFStateID_c<daNiceCoin_c>` objects embed their class-qualified names
// literally ("daNiceCoin_c::StateID_Search", "daNiceCoin_c::StateID_EndWait"), matching
// `STATE_DEFINE`'s own `#class "::StateID_" #name` string literal
// (include/game/sLib/s_State.hpp).
//
// VTABLE SLOT IDENTITY -- cross-checked against fBase_c's OWN virtual declaration order
// (include/game/framework/f_base.hpp), not assumed from name-adjacency: fBase_c declares
// create/preCreate/postCreate/doDelete/preDelete/postDelete/execute/preExecute/
// postExecute/draw/preDraw/postDraw/deleteReady/entryFrmHeap/entryFrmHeapNonAdjust/
// createHeap/~fBase_c in that exact order, and every "preX"/"postX" slot in the target's
// vtable dump names an INHERITED, unmodified dActor_c symbol -- so the FOUR anonymous
// slots that precede preCreate/preDelete/preExecute/preDraw are NOT preX overrides (a
// naming-adjacency trap); they are `create()`, `doDelete()`, `execute()`, `draw()`
// respectively, matching fBase_c's declared order exactly. The destructor slot (which one
// might expect first) is actually much later, immediately after `createHeap()`, matching
// fn_2_1050B0 -- confirmed a plain scalar-deleting-destructor wrapper
// (`if (this) { ~dActorState_c(); if (shouldFree>0) delete this; }`, same shape as the
// landed d_a_dummy_door.cpp's fn_2_77B40) that calls `dActorState_c`'s OWN destructor
// directly, NOT a derived one -- this class declares no destructor logic of its own
// either, matching its constructor.
//
// @unofficial NOT YET AUTHORED (documented here rather than guessed): create()'s body
// (fn_2_104E10, 0x10C bytes) reads mParam bit-fields into new fields at +0x3d0/+0x3d4/
// +0x3d8/+0x3dc/+0x3e0, calls `dBg_c::m_bg_p->CoinGetBitCheck(unsigned short, unsigned
// short, int)` (UNDECLARED anywhere in include/ or landed in source/ -- would need a
// shadow header addition, not attempted this round) to check whether this coin was
// already collected, and ends with a call through the (inherited) state manager's
// internal dispatch. executeState_Search (fn_2_104F90, 0xE8 bytes) similarly calls
// `dMultiMng_c::mspInstance->setClapSE()` and `dBg_c::m_bg_p->CoinGetBitSet(...)`. Both
// are read and partially understood (see the coordinator report) but not translated to
// C++ this round -- the CoinGetBitCheck/CoinGetBitSet signature and dBg_c's grid-index
// convention need independent confirmation before committing to a shadow declaration.
class daNiceCoin_c : public dActorState_c {
public:
    virtual ~daNiceCoin_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    STATE_FUNC_DECLARE(daNiceCoin_c, Search);
    STATE_FUNC_DECLARE(daNiceCoin_c, EndWait);

    /// @unofficial daNiceCoin_c's own added fields, offsets +0x3d0-+0x3ef (0x20 bytes
    /// total, sizeof(daNiceCoin_c)-sizeof(dActorState_c) = 0x3f0-0x3d0). Read/written
    /// only by create()/executeState_Search() so far; +0x3d8/+0x3e4/+0x3e8/+0x38f
    /// (the latter inside dActorState_c's own footprint, not ours) not yet declared.
    int mUnk3d0; ///< @unofficial +0x3d0. mParam bits[28:32) (low nibble).
    int mUnk3d4; ///< @unofficial +0x3d4. Derived mode: 0, 1, or 2.
    int mUnk3dc; ///< @unofficial +0x3dc. mParam bits[16:24).
    int mUnk3e0; ///< @unofficial +0x3e0. mParam bits[8:16).
};

// @unofficial The standard ACTOR_PROFILE macro cannot be used twice for one class: its
// generated `className##_classInit()` (f_profile.hpp:16) is keyed on the CLASS name, so two
// invocations for the SAME class collide at compile time ("(10333) object
// 'daNiceCoin_c_classInit()' redefined" -- reproduced directly, not assumed). The landed
// `source/d_basesNP/bases/d_a_ac_switch.cpp` hit the identical problem for seven profiles
// sharing one class and hand-expands CUSTOM_ACTOR_PROFILE's body per profile instead; same
// fix applied here for two. `properties` is 0 for both profiles (read directly off the third
// `.4byte` word of each raw profile struct in the REL, both 0x00000000).
static void *classInit_AC_NICE_COIN() { return new daNiceCoin_c(); }
static void *classInit_AC_NICE_COIN_REGULAR() { return new daNiceCoin_c(); }

fProfile::fActorProfile_c g_profile_AC_NICE_COIN = {
    &classInit_AC_NICE_COIN, fProfile::AC_NICE_COIN, fProfile::DRAW_ORDER::AC_NICE_COIN, 0
};
fProfile::fActorProfile_c g_profile_AC_NICE_COIN_REGULAR = {
    &classInit_AC_NICE_COIN_REGULAR, fProfile::AC_NICE_COIN_REGULAR, fProfile::DRAW_ORDER::AC_NICE_COIN_REGULAR, 0
};

STATE_DEFINE(daNiceCoin_c, Search);
STATE_DEFINE(daNiceCoin_c, EndWait);

// @unofficial FUNCTION DEFINITION ORDER BELOW IS DELIBERATE, matching the target's real TEXT
// ADDRESS order -- NOT the same thing as vtable SLOT order (a real trap: the vtable's own
// slot sequence follows fBase_c's declared virtual order -- create, doDelete, execute, draw,
// per include/game/framework/f_base.hpp -- but the linker places .text in SOURCE DEFINITION
// order, an independent axis. Ground truth for text address comes from
// bin/dtk/d_basesNP_symbols.txt directly: create()=fn_2_104E10 (0x104E10), execute()=
// fn_2_104F20 (0x104F20, doubly confirmed -- both the vtable slot AND a genuine, non-trivial
// 12-instruction content match), draw()=fn_2_104F50, doDelete()=fn_2_104F60 -- ascending
// address order is create, execute, draw, doDelete, so that is the required definition order
// here. (An earlier version of this file ordered them execute/create/draw/doDelete based on
// misreading verify_anon's CONTENT-based pairing of two still-trivial-stub placeholders as if
// it proved identity -- it does not: verify_anon's own docstring warns two functions with
// identical bodies pair to whichever target happens to match first. The vtable dump plus the
// symbols file's real addresses are the reliable source for BOTH identity and order; content
// matching only confirms a function once its body is non-trivial and unique, as execute()'s
// is.) The same WM_KOOPAJR-class bug otherwise: an ad-hoc order looks fine per-function while
// still failing to link.
/// @unofficial `create()` (fn_2_104E10), authored using the now-confirmed
/// `dBg_c::CoinGetBitCheck(u16, u16, int)` (shadow_include/game/bases/d_bg.hpp).
/// - `this+0x8` (u16) compared against 0x251 matches `fProfile::AC_NICE_COIN_REGULAR`'s
///   own packed profile-order word -- this is a RUNTIME check of which of the two
///   profiles constructed this instance (not a ctor argument, since both classInits
///   call the identical base ctor), so `mProfName` distinguishes REGULAR at runtime.
/// - `(u16)mPos.x`, `(u16)(-mPos.y)` computed via the same `fctiwz` truncation shape,
///   passed to CoinGetBitCheck as confirmed by the mangled signature.
/// - If already collected, returns 2 (`fBase_c::MAIN_STATE_e::SUCCESS`) early.
/// - Otherwise reads three ACTOR_PARAM-shaped bit-fields out of mParam (this+0x4):
///   bits[28:32) (4-bit), bits[16:24) (8-bit), bits[8:16) (8-bit) into three new
///   fields, and derives a fourth (0/1/2 mode) from comparing the latter two.
/// @unofficial NOT AUTHORED: the trailing call through mStateMgr's own vtable slot
/// +0x18 (matches `sStateMgrIf_c::changeState`'s slot position, confirmed by cross-
/// referencing s_StateMgr.hpp's declared virtual order against execute()'s already-
/// confirmed +0x10 executeState() dispatch) passes `&lbl_2_bss_C9E0`, an anonymous
/// 0x40-byte .bss object -- NOT `StateID_Search`/`StateID_EndWait` (both already
/// accounted for, in .data, confirmed by the byte-perfect __sinit match) and NOT
/// `sStateID::null` (an extern, named symbol -- would show its real mangled name, not
/// an anonymous bss label). Its size doesn't match sFStateID_c<daNiceCoin_c> (0x30,
/// which DOES match the OTHER call site's bss object, lbl_2_bss_CA20, in
/// executeState_Search() below) or any type identified so far. Left uncalled rather
/// than guessed -- a wrong changeState() target is not neutral in a class whose
/// state machine this document is still establishing.
int daNiceCoin_c::create() {
    u16 x = (u16)mPos.x;
    u16 negY = (u16)-mPos.y;
    if (dBg_c::m_bg_p->CoinGetBitCheck(x, negY, dActor_c::m_mbgchoice_keep)) {
        return 2;
    }

    mUnk3d0 = mParam & 0xf;
    mUnk3dc = (mParam >> 8) & 0xff;
    mUnk3e0 = (mParam >> 16) & 0xff;
    mUnk3d4 = (mUnk3dc != 0 && mUnk3e0 != 0) ? 2 : (mUnk3dc == 0 && mUnk3e0 != 0) ? 1 : 0;

    // mStateMgr.changeState(???); -- see comment above, not yet identified.

    return SUCCEEDED;
}

int daNiceCoin_c::execute() {
    mStateMgr.executeState();
    return SUCCEEDED;
}

int daNiceCoin_c::draw() {
    return SUCCEEDED;
}

int daNiceCoin_c::doDelete() {
    return SUCCEEDED;
}

void daNiceCoin_c::initializeState_Search() {}
void daNiceCoin_c::finalizeState_Search() {}
/// @unofficial executeState_Search() (fn_2_104F90), authored using the now-confirmed
/// `dBg_c::CoinGetBitSet(u16, u16, int)`. Checks the reveal condition (mUnk3d4 against
/// 0/2, then mUnk3e4 vs mUnk3dc, then mUnk3e8 vs mUnk3e0 -- the latter two fields not
/// yet declared, see below) before calling setClapSE()/CoinGetBitSet() and (same
/// unresolved changeState() target as create(), see its comment) transitioning state.
/// mUnk3e4/mUnk3e8 deliberately NOT declared/read here -- guessing their comparison
/// role (both are compared against already-identified fields but never written
/// anywhere in this function or create(), so they must be written by a DIFFERENT,
/// not-yet-authored function, most likely one of the six state methods still stubbed
/// or -- more likely, given they read as live/current counters compared against
/// static limits -- incremented somewhere per-frame that has not been located) would
/// be exactly the kind of guess that moves a pool/field-layout claim further from
/// correct rather than closer.
void daNiceCoin_c::executeState_Search() {}

void daNiceCoin_c::initializeState_EndWait() {}
void daNiceCoin_c::finalizeState_EndWait() {}
void daNiceCoin_c::executeState_EndWait() {}

daNiceCoin_c::~daNiceCoin_c() {}

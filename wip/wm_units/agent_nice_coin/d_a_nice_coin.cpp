#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_actor_state.hpp>

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
int daNiceCoin_c::create() {
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
void daNiceCoin_c::executeState_Search() {}

void daNiceCoin_c::initializeState_EndWait() {}
void daNiceCoin_c::finalizeState_EndWait() {}
void daNiceCoin_c::executeState_EndWait() {}

daNiceCoin_c::~daNiceCoin_c() {}

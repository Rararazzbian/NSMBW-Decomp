#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>
#include <game/sLib/s_State.hpp>

// @unofficial LEMMY_FOOTHOLD_MAIN's own class layout, read directly off its
// constructor (fn_2_C5CC0) and cross-checked against the already-landed
// d_a_wm_antlion.cpp's own m3d-model construction idiom (same module,
// d_basesNP), which uses the identical "base ctor call, then vtable-patch
// to the derived animation type" shape for its own mChrAnim/mAnimTexSrt
// members.
//
// sizeof == 0x6a8 (both LEMMY_FOOTHOLD and LEMMY_FOOTHOLD_MAIN allocate
// this exact size -- `li r3, 0x6a8` in both classInits).
//
// Base: dEn_c (sizeof 0x528, confirmed via STATIC_ASSERT), NOT
// dActorState_c/dWmEnemy_c -- confirmed via the 5-character mangled ctor
// call `__ct__5dEn_cFv`.
//
// Members, offsets read directly from the ctor:
//   dHeapAllocator_c mAllocator  @ +0x524  (sizeof 0x1c, probed) --
//     reuses 4 bytes of dEn_c's own tail alignment padding (dEn_c's
//     reported sizeof 0x528 rounds up from a true content end at 0x524).
//   u32 m_540                    @ +0x540  (unidentified, explicitly
//     zeroed -- fills the 4-byte gap between mAllocator's end (0x540)
//     and mModel's start (0x544))
//   m3d::mdl_c mModel            @ +0x544  (sizeof 0x40, probed)
//   u32 m_584                    @ +0x584  (unidentified, explicitly
//     zeroed -- fills the 4-byte gap between mModel's end (0x584) and
//     mAnimTexSrt's start (0x588))
//   m3d::anmTexSrt_c mAnimTexSrt @ +0x588  (sizeof 0x2c, probed;
//     matches d_a_wm_antlion.cpp's own anmTexSrt_c probe exactly) --
//     its own embedded banm_c-level fields (mpObj/mpHeap/mAllocator)
//     construct first (vtable briefly `__vt__Q23m3d6banm_c`), then its
//     own vtable overwrites to `__vt__Q23m3d11anmTexSrt_c` as the last
//     step -- ordinary C++ construction order, not hand-rolled.
//   dBg_ctr_c mBgCtr             @ +0x5b4 in LEMMY_FOOTHOLD_MAIN
//                                 (sizeof 0xe4, probed), but @ +0x5c0 in
//     plain LEMMY_FOOTHOLD (fn_2_C61E0) -- a real, confirmed 0xc-byte
//     difference between the two classes, not yet resolved (see
//     MAPPING.md).
//
// UNRESOLVED, not modelled below: the two 4-byte gaps (m_540, m_584) are
// real fields whose names/types are not yet identified -- kept as raw
// u32 placeholders rather than guessed at. The tail after mBgCtr (0x10
// bytes in MAIN, 0x4 bytes in plain FOOTHOLD, ending at the shared
// sizeof 0x6a8) is also not yet modelled. The 0xc-byte layout difference
// between the two classes' own dBg_ctr_c offset is confirmed real (not a
// misread) but its cause (an extra member specific to plain
// LEMMY_FOOTHOLD) is not yet identified.

class daLemmyFoothold_c : public dEn_c {
public:
    daLemmyFoothold_c();

    // @unofficial Three states, read directly from .data (the
    // coordinator's own scan of the state-name strings, not guessed):
    // StateID_DemoWait, StateID_DemoDown, StateID_DemoUp. StateID_DemoWait
    // COLLIDES in name with daLemmyFootholdMain_c's own StateID_DemoWait
    // (see the hand-expanded definition below the class bodies) --
    // STATE_VIRTUAL_DEFINE's baseID_##name helper is file-scope, so two
    // classes sharing a state NAME in one TU collide on the helper name
    // alone, independent of any inheritance relationship.
    STATE_VIRTUAL_FUNC_DECLARE(daLemmyFoothold_c, DemoWait);
    STATE_VIRTUAL_FUNC_DECLARE(daLemmyFoothold_c, DemoDown);
    STATE_VIRTUAL_FUNC_DECLARE(daLemmyFoothold_c, DemoUp);

    dHeapAllocator_c mAllocator;
    u32 m_540;
    m3d::mdl_c mModel;
    u32 m_584;
    m3d::anmTexSrt_c mAnimTexSrt;
    u8 mUnk5B4[0xc];
    dBg_ctr_c mBgCtr;
    u8 mTail[0x4];
};

daLemmyFoothold_c::daLemmyFoothold_c() : m_540(0), m_584(0) {}
ACTOR_PROFILE(LEMMY_FOOTHOLD, daLemmyFoothold_c, 0);

// @unofficial State bodies -- STUBS for now (empty), not yet read against
// target bytes. Declaring them (even empty) is what is needed to get the
// framework-generated machinery (state object dtor/number()/superID()/
// isSameName()/three trampolines -- none hand-authored) to compile and
// be positioned correctly; the REAL per-state behaviour is separate,
// unauthored work for a later pass.
// @unofficial Attributed by reading the PMF function-pointer values
// directly out of the target's own vtable (per the coordinator's
// technique), NOT from verify_anon's size-based candidate pairing.
// daLemmyFoothold_c's states use the VIRTUAL (vtable-offset) PMF
// encoding in target -- unlike daLemmyFootholdMain_c's own (direct
// address) encoding -- so the state object's own PMF fields in .data
// hold a vtable BYTE OFFSET, not a relocatable address; reading target's
// own vtable (lbl_2_data_27E10) at those exact offsets gave the real
// function addresses. WHY the two classes differ in PMF encoding is not
// resolved (both use the identical STATE_VIRTUAL_FUNC_DECLARE/_DEFINE
// shape) -- flagged as a genuine open question, not chased further this
// round since it only affects __sinit's own bytes (explicitly
// deprioritised, see MAPPING.md).
void daLemmyFoothold_c::initializeState_DemoWait() {}
void daLemmyFoothold_c::executeState_DemoWait() { mAnimTexSrt.play(); }
void daLemmyFoothold_c::finalizeState_DemoWait() {}
void daLemmyFoothold_c::initializeState_DemoDown() {
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    mAccelY = -0.1850000023841858f;
}
void daLemmyFoothold_c::executeState_DemoDown() {}
void daLemmyFoothold_c::finalizeState_DemoDown() {}
void daLemmyFoothold_c::initializeState_DemoUp() {}
void daLemmyFoothold_c::executeState_DemoUp() {}
void daLemmyFoothold_c::finalizeState_DemoUp() {}

class daLemmyFootholdMain_c : public dEn_c {
public:
    daLemmyFootholdMain_c() : m_540(0), m_584(0) {}

    // @unofficial Two states, read directly from .data: StateID_DemoWait
    // (registered FIRST in __sinit -- this class's own STATE_VIRTUAL_DEFINE
    // is therefore the one that legitimately owns the shared baseID_DemoWait
    // helper; daLemmyFoothold_c's own DemoWait reuses it, hand-expanded,
    // below) and StateID_Wait.
    STATE_VIRTUAL_FUNC_DECLARE(daLemmyFootholdMain_c, DemoWait);
    STATE_VIRTUAL_FUNC_DECLARE(daLemmyFootholdMain_c, Wait);

    dHeapAllocator_c mAllocator;
    u32 m_540;
    m3d::mdl_c mModel;
    u32 m_584;
    m3d::anmTexSrt_c mAnimTexSrt;
    dBg_ctr_c mBgCtr;
    u8 mTail[0x10];
};

ACTOR_PROFILE(LEMMY_FOOTHOLD_MAIN, daLemmyFootholdMain_c, 0);

// @unofficial State bodies -- STUBS for now, see the note on
// daLemmyFoothold_c's own state bodies above.
// @unofficial Attributed by reading the PMF function-pointer relocations
// directly out of the state objects' own .data fields (per the
// coordinator's technique) -- daLemmyFootholdMain_c's states use DIRECT
// (relocatable-address) PMF encoding, unlike daLemmyFoothold_c's own
// (vtable-offset) encoding above. Confirmed: initializeState_DemoWait/
// finalizeState_DemoWait/initializeState_Wait/finalizeState_Wait are all
// genuinely trivial (target bodies are bare `blr`); executeState_DemoWait/
// executeState_Wait both resolve to the mAnimTexSrt vtable dispatch thunk
// (`lwzu r12,0x588(r3); lwz r12,0x14(r12); mtctr; bctr`) at
// 0xC6170/0xC61A0 -- confirmed via a probe compile of m3d::anmTexSrt_c
// showing play() lands at vtable byte offset 0x14 exactly.
void daLemmyFootholdMain_c::initializeState_DemoWait() {}
void daLemmyFootholdMain_c::executeState_DemoWait() { mAnimTexSrt.play(); }
void daLemmyFootholdMain_c::finalizeState_DemoWait() {}
void daLemmyFootholdMain_c::initializeState_Wait() {}
void daLemmyFootholdMain_c::executeState_Wait() { mAnimTexSrt.play(); }
void daLemmyFootholdMain_c::finalizeState_Wait() {}

// @unofficial __sinit registers these five state objects in this exact
// order (read directly off fn_2_C6920's own sequence of
// __ct__10sStateID_cFPCc/__register_global_object call pairs, matching
// the coordinator's own .data-address scan): MAIN::DemoWait, MAIN::Wait,
// FOOTHOLD::DemoWait, FOOTHOLD::DemoDown, FOOTHOLD::DemoUp. Static
// initializers run in TEXTUAL declaration order within a TU, so this
// block's own ordering is load-bearing, not cosmetic.
//
// MAIN::DemoWait uses the ordinary macro -- it is the one that legitimately
// defines the shared file-scope baseID_DemoWait<T> template (and its
// sStateID_c specialization) that FOOTHOLD's own DemoWait, below, reuses.
STATE_VIRTUAL_DEFINE(daLemmyFootholdMain_c, DemoWait);
STATE_VIRTUAL_DEFINE(daLemmyFootholdMain_c, Wait);

// @unofficial daLemmyFoothold_c::StateID_DemoWait -- HAND-EXPANDED per the
// coordinator's direction (precedent: source/d_basesNP/bases/d_a_ac_switch.cpp
// hand-expands ACTOR_PROFILE for the same reason, a macro that cannot be
// invoked twice for the same name in one TU). Deliberately reuses the
// EXISTING baseID_DemoWait<T> template (defined above by MAIN's own
// STATE_VIRTUAL_DEFINE) rather than re-emitting it -- confirmed against
// the actual __sinit bytes: the superState argument is NOT a plain
// `sStateID::null` load, it is a real `bl` to a small helper function
// (matching fn_2_C61B0's own `lis/addi null__8sStateID; blr` shape) --
// i.e. the target's own compiled code goes through the SAME templated
// baseID_ mechanism for both classes' DemoWait, not an inlined constant.
// daLemmyFoothold_c and daLemmyFootholdMain_c are confirmed SIBLINGS (both
// call `__ct__5dEn_cFv` directly, neither derives from the other), so
// `StateIDBase_DemoWait` resolves to `sStateID_c` here exactly as it does
// for MAIN, and the call lands on the SAME already-defined
// `baseID_DemoWait<sStateID_c>` specialization -- read from the bytes,
// not assumed from the inheritance shape alone.
sFStateVirtualID_c<daLemmyFoothold_c> daLemmyFoothold_c::StateID_DemoWait(
    baseID_DemoWait<daLemmyFoothold_c::StateIDBase_DemoWait>(),
    "daLemmyFoothold_c::StateID_DemoWait",
    &daLemmyFoothold_c::initializeState_DemoWait,
    &daLemmyFoothold_c::executeState_DemoWait,
    &daLemmyFoothold_c::finalizeState_DemoWait);

STATE_VIRTUAL_DEFINE(daLemmyFoothold_c, DemoDown);
STATE_VIRTUAL_DEFINE(daLemmyFoothold_c, DemoUp);

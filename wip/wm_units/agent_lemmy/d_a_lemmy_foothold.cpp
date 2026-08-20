#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>

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

class daLemmyFootholdMain_c : public dEn_c {
public:
    daLemmyFootholdMain_c() : m_540(0), m_584(0) {}
    dHeapAllocator_c mAllocator;
    u32 m_540;
    m3d::mdl_c mModel;
    u32 m_584;
    m3d::anmTexSrt_c mAnimTexSrt;
    dBg_ctr_c mBgCtr;
    u8 mTail[0x10];
};

ACTOR_PROFILE(LEMMY_FOOTHOLD_MAIN, daLemmyFootholdMain_c, 0);

#include <types.h>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/mdl.hpp>

class daWmKinokoBase_c : public dWmObjActor_c {
public:
    enum ANIM_e { ANIM_0, ANIM_1, ANIM_COUNT };
    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    m3d::anmChr_c mChrAnim[ANIM_COUNT];
    m3d::anmChrBlend_c mChrBlend;
    u32 f1; u32 f2; u32 f3;
};

template <bool B> struct STATIC_CHECK;
template <> struct STATIC_CHECK<true> {};
STATIC_CHECK<sizeof(daWmKinokoBase_c) == 0x290> check_kinokobase;

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
    u32 mUnk280;
};

class daWmKinoko1up_c : public daWmKinokoBase_c {
public:
    u32 mUnk284;
    const char *const *mAnimResNames;
    const char *mModelResName;
    u32 mFlag;
};

template<bool B> struct STATIC_CHECK;
template<> struct STATIC_CHECK<true> {};
STATIC_CHECK<sizeof(daWmKinokoBase_c) == 0x284> checkBase;
STATIC_CHECK<sizeof(daWmKinoko1up_c) == 0x294> check1up;

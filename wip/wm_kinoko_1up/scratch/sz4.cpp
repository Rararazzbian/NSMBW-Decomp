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
};

template<int N> struct Sz;
Sz<sizeof(daWmKinokoBase_c)> dummy;
Sz<sizeof(dWmObjActor_c)> dummy2;
Sz<sizeof(dHeapAllocator_c)> dummy3;
Sz<sizeof(m3d::mdl_c)> dummy4;
Sz<sizeof(m3d::anmChr_c)> dummy5;
Sz<sizeof(m3d::anmChrBlend_c)> dummy6;
Sz<sizeof(nw4r::g3d::ResFile)> dummy7;

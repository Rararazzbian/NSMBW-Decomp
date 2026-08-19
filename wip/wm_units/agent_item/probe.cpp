#include <types.h>
#include <game/bases/d_base_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_3d/banm.hpp>
#include <game/mLib/m_allocator.hpp>
#include <lib/egg/core/eggAllocator.h>
#include <stddef.h>

STATIC_ASSERT(sizeof(dBaseActor_c) <= 0x128);

class daWmItem_layout_c : public dBaseActor_c {
public:
    dHeapAllocator_c mAllocator;
    m3d::mdl_c mModel;
    m3d::anmChr_c mAnmChr;
    float m_1bc[3];
    m3d::anmTexPat_c mAnmTexPat;
    float m_1f4, m_1f8, m_1fc;
    u32 m_200, m_204;
    u8 m_208, m_209, m_20a, m_20b;
    u32 mItemType;
};

template<int N> struct Probe;

Probe<offsetof(daWmItem_layout_c, mAllocator)> p1;
Probe<offsetof(daWmItem_layout_c, mModel)> p2;
Probe<offsetof(daWmItem_layout_c, mAnmChr)> p3;
Probe<offsetof(daWmItem_layout_c, m_1bc)> p3b;
Probe<offsetof(daWmItem_layout_c, mAnmTexPat)> p4;
Probe<sizeof(m3d::anmChr_c)> p4b;
Probe<sizeof(m3d::anmTexPat_c)> p4c;
Probe<offsetof(daWmItem_layout_c, m_1f4)> p5;
Probe<offsetof(daWmItem_layout_c, m_200)> p6;
Probe<offsetof(daWmItem_layout_c, m_208)> p7;
Probe<offsetof(daWmItem_layout_c, mItemType)> p8;
Probe<sizeof(daWmItem_layout_c)> p9;
Probe<sizeof(dBaseActor_c)> p10;
Probe<sizeof(m3d::banm_c)> p11;
Probe<sizeof(mAllocator_c)> p12;
Probe<sizeof(EGG::Allocator)> p13;
Probe<sizeof(m3d::fanm_c)> p14;

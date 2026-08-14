#pragma once
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_actor.hpp>

class daWmGrid_c : public dWmActor_c {
public:
    daWmGrid_c();
    virtual ~daWmGrid_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual int GetActorType();

    dHeapAllocator_c mAllocator;
    m3d::smdl_c mModel;
    u32 mUnk160;
};

#pragma once
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>

class daWmTower_c : public dWmObjActor_c {
public:
    daWmTower_c();
    virtual ~daWmTower_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    void createModel();
    void calcModel();
    void setClipSphere() {
        mClipSphere.set(mPos, 120.0f);
    }

    dHeapAllocator_c mAllocator;
    m3d::smdl_c mModel;
};

#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>

class daWmGhost_c : public dWmObjActor_c {
public:
    enum ANIM_e {
        cobGhostOpen,
        cobGhostClose,
        cobGhostOut,
        cobGhostCloseAll,
        cobGhostCloseWindow,
        cobGhostCloseDoor,
        ANIM_COUNT
    };

    daWmGhost_c();
    ~daWmGhost_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    void createModel();
    void initState();
    void calcModel();

    u32 mUnk188; ///< @unofficial
    dHeapAllocator_c mAllocator;
    m3d::smdl_c mModel;
    m3d::anmChr_c mChrAnim[ANIM_COUNT];
    m3d::smdl_c mUnusedModels[3]; ///< @unofficial
    m3d::anmChr_c mUnusedAnims[3]; ///< @unofficial
    bool mIsGhostOut; ///< @unofficial
    u8 mPad3D1[3]; ///< @unofficial
    int mCutscene; ///< @unofficial
};

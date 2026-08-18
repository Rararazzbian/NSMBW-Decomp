#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_mat_clr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>

// SPECULATIVE / IN PROGRESS -- see wip/wm_units/agent_course/NOTES.md.
// Member offsets 0x188 (mAllocator) through 0x244 (mOpenState) are confirmed
// against the target's createModel/execute/setMatClrAnim disassembly. Fields
// beyond 0x248 are placeholders pending analysis of the 3 largest functions.
class daWmCourse_c : public dWmObjActor_c {
public:
    enum ANIM_e {
        ANM_CLEAR,
        ANM_HELP,
        ANM_OPEN,
        ANIM_COUNT
    };

    daWmCourse_c();
    ~daWmCourse_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);
    virtual bool vf78();

    void createModel();
    void calcModel();
    void setMatClrAnm(int index, float rate, float frame);
    void updateState();
    void updateOpenAnim();
    void updateClearAnim(bool unused);
    void updateHelpFade();
    daWmCourse_c *searchOpenNeighbor();
    static void openNeighbors(bool fastRate);
    float getMatClrFrame();
    void updateSpecialWorld();
    bool isWorld2SpecialType();

    dHeapAllocator_c mAllocator; ///< @0x188
    nw4r::g3d::ResFile mResFile; ///< @0x1a4
    m3d::smdl_c mModel; ///< @0x1a8
    m3d::anmMatClr_c mMatClrAnim[ANIM_COUNT]; ///< @0x1b4
    int mCurrentIndex; ///< @0x238
    bool mUnk23c; ///< @0x23c @unofficial
    int mState; ///< @0x240
    int mOpenState; ///< @0x244
    u32 mUnk248; ///< @unofficial
    u32 mUnk24c; ///< @unofficial
    bool mUnk250; ///< @0x250 @unofficial
};

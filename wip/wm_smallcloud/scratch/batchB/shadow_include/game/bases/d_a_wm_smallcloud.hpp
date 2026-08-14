#pragma once

/// @unofficial SCRATCH SHADOW HEADER -- batchB working hypothesis only. NOT for landing.
/// Field layout up to mCurrProc (0x1f4) is proven byte-identical to the landed daWmCloud_c
/// (see wip/wm_smallcloud/BATCHB.md). Everything after mCurrProc is unconfirmed guesswork
/// carried over from the twin purely so the class compiles; batch A owns the real shape.

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_wm_bgm_sync.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_sphere.hpp>

class daWmSmallCloud_c : public dWmObjActor_c {
public:
    static const int NODE_COUNT = 20; ///< @unofficial guess, copied from twin; unused by batchB's functions

    enum ANIM_e {
        CS_Anim,
        ANIM_COUNT
    };

    typedef void (daWmSmallCloud_c::*ProcFunc)();

    daWmSmallCloud_c();
    ~daWmSmallCloud_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    // createModel(), initGroupNodeIds(), calcCulling() etc. are batch A's territory --
    // not declared here since batchB's functions never call them.

    void calcModel();

    void init_exec();
    void mode_exec();

    /// @unofficial batchB fn_2_179F10 -- looks up this actor's course-node position by name,
    /// indexed off ACTOR_PARAM(CourseNo), and writes it into mPos.
    void setPosFromCourseNode();

    /// @unofficial batchB fn_2_179E00 working name -- plays the same role as daWmCloud_c's
    /// initState() (sets up mChrAnim, calls init_exec()) but does NOT reset mPos to Zero and
    /// has extra CourseNo==3 / IsCourseClear handling daWmCloud_c's initState() does not have.
    /// Real name is very likely "initState" but is left renamed here to avoid colliding with
    /// batch A's own (currently absent) declaration. See BATCHB.md.
    void initStateLike();

    u32 mUnk188; ///< @unused, copied from twin
    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::smdl_c mModel; ///< proven @ 0x1ac (see BATCHB.md)
    m3d::anmChr_c mChrAnim[ANIM_COUNT]; ///< proven @ 0x1b8
    u32 mUnk1F0; ///< @unofficial proven-position pad @ 0x1f0; twin names the equivalent field
                 ///< mUnk250, which looks stale against the offset this batch measured
    PROC_TYPE_e mCurrProc; ///< proven @ 0x1f4

    int mGroupNodeIds[NODE_COUNT]; ///< @unofficial guess, not exercised by batchB
    mSphere_c mCurrNodeClipSphere; ///< @unofficial guess, not exercised by batchB
    dWmBgmSync_c *mpBgmSync; ///< @unofficial guess, not exercised by batchB
};

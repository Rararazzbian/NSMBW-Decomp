#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>

// SPECULATIVE / IN PROGRESS -- see report. Member offsets 0x18c (mAllocator)
// through 0x269 (mCompanionPlaced) are confirmed against the target's
// ctor/dtor/createModel/calcModel/initState disassembly. mUnk26c..mUnk280 are
// placeholders pending fn_2_1912B0 (execute)'s still-unmatched tail, and
// mReady is placeholder pending fn_2_1917E0 (processCutsceneCommand, 226
// instructions, deliberately last).
class daWmKoopaCastle_c : public dWmObjActor_c {
public:
    enum ANIM_e {
        ANM_OPEN,
        ANM_CLOSE,
        ANM_OUT,
        ANIM_COUNT
    };

    enum PROC_TYPE_e {
        PROC_TYPE_EXEC,
        PROC_COUNT
    };

    typedef void (daWmKoopaCastle_c::*ProcFunc)();

    daWmKoopaCastle_c();
    ~daWmKoopaCastle_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    void createModel();
    void calcModel();
    void initState();
    void constructCompanion();
    void init_exec();
    void mode_exec();
    static bool isReady();

    u32 mUnk188; ///< @0x188 @unofficial
    dHeapAllocator_c mAllocator; ///< @0x18c
    nw4r::g3d::ResFile mResFile; ///< @0x1a8
    m3d::smdl_c mModel; ///< @0x1ac
    m3d::anmChr_c mChrAnim[ANIM_COUNT]; ///< @0x1b8
    int mCutscene; ///< @0x260 @unofficial
    PROC_TYPE_e mProcIdx; ///< @0x264
    bool mIsOut; ///< @0x268 @unofficial
    bool mCompanionPlaced; ///< @0x269 @unofficial -- guards constructCompanion()
    float mUnk26c; ///< @0x26c @unofficial
    float mUnk270; ///< @0x270 @unofficial
    float mUnk274; ///< @0x274 @unofficial
    float mUnk278; ///< @0x278 @unofficial
    float mUnk27c; ///< @0x27c @unofficial
    float mUnk280; ///< @0x280 @unofficial
    bool mReady; ///< @0x284 @unofficial -- read by fn_2_191BF0 in daWmCourse_c
};

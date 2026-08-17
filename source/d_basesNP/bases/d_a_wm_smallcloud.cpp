#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_a_wm_smallcloud.hpp>
#include <game/sLib/s_GlobalData.hpp>

// __sinit (fn_2_179F40) reaches the sc_ForceList vec3 floats as an offset from
// mData's own address (lfs f2, 0x24(r5) with r5 = &mData) -- proving mData is
// defined in THIS TU, not a sibling one; a compiler cannot know the distance
// to an external symbol at compile time. See this task's report.
template <>
const daWmSmallCloud_c::GlobalData_t sGlobalData_c<daWmSmallCloud_c>::mData = {
    { 8, 0 },
    { 8, 0 }
};

ACTOR_PROFILE(WM_SMALLCLOUD, daWmSmallCloud_c, 0);

daWmSmallCloud_c::daWmSmallCloud_c() {}

daWmSmallCloud_c::~daWmSmallCloud_c() {
    if (mpBgmSync != nullptr) {
        delete mpBgmSync;
    }
}

int daWmSmallCloud_c::create() {
    dWmBgmSync_c *bgmSync = new dWmBgmSync_c();
    mpBgmSync = bgmSync;

    if (dScWMap_c::m_WorldNo == 5) {
        bgmSync->m_18 = GLOBAL_DATA.mBgmValueW5;
        // The volatile qualifier is a compiler barrier, not real hardware volatility:
        // mData is a plain compile-time constant, but MWCC's -ipa file folds a direct
        // GLOBAL_DATA.mBgmValueW5[n] read to an `li` immediate since the initializer is
        // visible in this same TU, and the target does NOT fold (it does a real `lha`
        // through mData's own address) -- see this task's report for every placement
        // and access-form tried that did NOT work before this one was found.
        bgmSync->m_04 = *(const volatile s16 *) &GLOBAL_DATA.mBgmValueW5[0] - 1;
        bgmSync->m_08 = *(const volatile s16 *) &GLOBAL_DATA.mBgmValueW5[1];
    } else {
        bgmSync->m_18 = GLOBAL_DATA.mBgmValue;
        bgmSync->m_04 = 7;
        bgmSync->m_08 = 0;
    }

    createModel();
    mClipSphere.set(mPos, 400.0f);

    calcModel();
    initState();
    return SUCCEEDED;
}

int daWmSmallCloud_c::execute() {
    mpBgmSync->execute();
    if (mpBgmSync->m_0c) {
        float rate = mpBgmSync->getAnmRate(mChrAnim->mFrameMax);
        mChrAnim->setRate(rate);
    }

    static const ProcFunc Proc_tbl[PROC_COUNT] = {
        &daWmSmallCloud_c::mode_exec
    };

    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    if (csSeqMng->FUN_80915600()) {
        processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    } else {
        (this->*Proc_tbl[mCurrProc])();
    }

    setPosFromCourseNode();
    mModel.play();
    calcModel();

    return SUCCEEDED;
}

int daWmSmallCloud_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmSmallCloud_c::doDelete() {
    return SUCCEEDED;
}

// Byte-exact, 101/101 -- see wip/wm_smallcloud/CREATEMODEL.md. The target's
// addi rX,rBASE,{0x88,0x98,0xa0} immediates are NOT this function's own aggregate:
// they are offsets from dWmLib::sc_ForceList (include/game/bases/d_wm_lib.hpp),
// already odr-used by this TU (initState()'s IsCourseClear call), which places its
// .data immediately before this function's own resMdlNames table/sprintf format
// string/archive path string, sharing the same base register.
void daWmSmallCloud_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    char arcName[6];
    sprintf(arcName, "CS_W%d", dScWMap_c::m_WorldNo + 1);
    mResFile = dResMng_c::m_instance->getRes(arcName, "g3d/model.brres");

    static const char *resMdlNames[4] = {
        "CS_W7_MoveCloud01",
        "CS_W7_MoveCloud02",
        "CS_W7_MoveCloud03",
        "CS_W6aCloud"
    };

    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl(resMdlNames[ACTOR_PARAM(CourseNo)]);
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    // Declaration order here sets .bss layout: the target has resAnmNames at
    // 0x102b0 and sInit after it, so resAnmNames must come first.
    static const char *resAnmNames[4];
    static char sInit = 0;
    static const m3d::playMode_e playModes[ANIM_COUNT] = {
        m3d::FORWARD_LOOP
    };

    for (int i = 0; i < ANIM_COUNT; i++) {
        if (!sInit) {
            resAnmNames[0] = resMdlNames[0];
            resAnmNames[1] = resMdlNames[1];
            resAnmNames[2] = resMdlNames[2];
            resAnmNames[3] = resMdlNames[3];
            sInit = true;
        }
        nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(resAnmNames[ACTOR_PARAM(CourseNo)]);
        mChrAnim[i].create(resMdl, resAnmChr, &mAllocator, nullptr);
        mChrAnim[i].mPlayMode = playModes[i];
        mChrAnim[i].setRate(0.0f);
        mChrAnim[i].setFrame(0.0f);
        mModel.setAnm(mChrAnim[CS_W7_SmallCloud]);
    }

    dWmActor_c::setSoftLight_Map(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmSmallCloud_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmSmallCloud_c::initState() {
    mChrAnim[CS_W7_SmallCloud].setRate(1.0f);
    mChrAnim[CS_W7_SmallCloud].setFrame(0.0f);
    setPosFromCourseNode();
    init_exec();
    int courseNo = ACTOR_PARAM(CourseNo);
    if (courseNo == 3) {
        mModel.setPriorityDraw(0, 0);
        if (dWmLib::IsCourseClear(5, 0x17)) {
            mVisible = true;
        } else {
            mVisible = false;
        }
    }
}

void daWmSmallCloud_c::init_exec() {
    mCurrProc = PROC_TYPE_EXEC;
}

void daWmSmallCloud_c::mode_exec() {}

void daWmSmallCloud_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId != dCsSeqMng_c::CUTSCENE_CMD_NONE && !isStaff()) {
        mIsCutEnd = true;
    }
}

void daWmSmallCloud_c::setPosFromCourseNode() {
    // Confirmed against the target's .data: the four entries are at 0x47308,
    // 0x47314, 0x47320 and 0x4732C, sizes 0xC/0xC/0xC/0xB. The placeholders that
    // were here were 5 bytes each, which is exactly why this unit's .data came
    // out 0x10 short.
    static const char *nodeNames[4] = {
        "MoveCloud01", "MoveCloud02", "MoveCloud03", "CloudLarge"
    };
    daWmMap_c::m_instance->GetNodePos(nodeNames[ACTOR_PARAM(CourseNo)], mPos);
}

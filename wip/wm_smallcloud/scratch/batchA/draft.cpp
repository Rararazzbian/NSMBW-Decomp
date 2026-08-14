#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_a_wm_smallcloud.hpp>
#include <game/sLib/s_GlobalData.hpp>

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
        bgmSync->m_04 = GLOBAL_DATA.mBgmValueW5[0] - 1;
        bgmSync->m_08 = GLOBAL_DATA.mBgmValueW5[1];
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

    updatePos();
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

void daWmSmallCloud_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId != dCsSeqMng_c::CUTSCENE_CMD_NONE && !isStaff()) {
        mIsCutEnd = true;
    }
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

// NOTE: initState/init_exec/mode_exec/updatePos (0x179E00, 0x179EA0, 0x179EB0,
// 0x179F10) belong to the OTHER batch. These bodies are placeholders only, so
// that THIS batch's functions (which call them) compile; they are not offered
// as a reconstruction of those functions' real bytes.
void daWmSmallCloud_c::initState() {
    mCurrProc = PROC_TYPE_EXEC;
}

void daWmSmallCloud_c::init_exec() {
    mCurrProc = PROC_TYPE_EXEC;
}

void daWmSmallCloud_c::mode_exec() {}

void daWmSmallCloud_c::updatePos() {}

void daWmSmallCloud_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    char arcName[8];
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

    static bool sInit = false;
    static const char *resAnmNames[4];
    if (!sInit) {
        resAnmNames[0] = resMdlNames[0];
        resAnmNames[1] = resMdlNames[1];
        resAnmNames[2] = resMdlNames[2];
        resAnmNames[3] = resMdlNames[3];
        sInit = true;
    }

    static const m3d::playMode_e playModes[ANIM_COUNT] = {
        m3d::FORWARD_LOOP
    };

    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(resAnmNames[ACTOR_PARAM(CourseNo)]);
    mChrAnim[CS_W7_SmallCloud].create(resMdl, resAnmChr, &mAllocator, nullptr);
    mChrAnim[CS_W7_SmallCloud].mPlayMode = playModes[CS_W7_SmallCloud];
    mChrAnim[CS_W7_SmallCloud].setRate(0.0f);
    mChrAnim[CS_W7_SmallCloud].setFrame(0.0f);
    mModel.setAnm(mChrAnim[CS_W7_SmallCloud]);

    dWmActor_c::setSoftLight_Map(mModel);
    mAllocator.adjustFrmHeap();
}

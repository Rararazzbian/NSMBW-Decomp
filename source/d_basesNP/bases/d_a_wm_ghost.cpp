#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_ghost.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_wm_se_manager.hpp>

ACTOR_PROFILE(WM_GHOST, daWmGhost_c, 0);

const char *sTeresaNodeNames[] = {
    "teresaC",
    "teresaR",
    "teresaL"
};

/// @unofficial The first is unreferenced by any placed function and exists to
/// hold pool slot 0; the second is create()'s clip-sphere radius. Splitting
/// them was forced by the LINKED binary: an earlier draft folded both into one
/// dummy array and passed 0.0f to mClipSphere.set(), which verify_anon cannot
/// distinguish -- it normalises the relocation symbol, so loading the wrong
/// pool entry compares byte-identical. The REL's relocation table showed it:
/// two entries pointing at .rodata+0x20 where the original points at +0x4.
const float sGhostUnusedFloat[] __attribute__((used)) = { 1.3f };
const float sGhostClipRadius[] = { 180.0f };

const m3d::playMode_e sGhostPlayModes[daWmGhost_c::ANIM_COUNT] = {
    m3d::FORWARD_ONCE,
    m3d::FORWARD_ONCE,
    m3d::FORWARD_ONCE,
    m3d::FORWARD_ONCE,
    m3d::FORWARD_ONCE,
    m3d::FORWARD_ONCE
};

daWmGhost_c::daWmGhost_c() {}
daWmGhost_c::~daWmGhost_c() {}

int daWmGhost_c::create() {
    createModel();
    initState();
    calcModel();
    mClipSphere.set(mPos, sGhostClipRadius[0]);
    return SUCCEEDED;
}

int daWmGhost_c::execute() {
    processCutsceneCommand(dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);

    daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    mModel.play();
    calcModel();

    return SUCCEEDED;
}

int daWmGhost_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmGhost_c::doDelete() {
    return SUCCEEDED;
}

void daWmGhost_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("cobGhost", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("cobGhost");

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    static const char *resAnmNames[ANIM_COUNT] = {
        "cobGhostOpen",
        "cobGhostClose",
        "cobGhostOut",
        "cobGhostCloseAll",
        "cobGhostCloseWindow",
        "cobGhostCloseDoor"
    };

    for (int i = 0; i < ANIM_COUNT; i++) {
        nw4r::g3d::ResAnmChr resAnmChr = resFile.GetResAnmChr(resAnmNames[i]);
        mChrAnim[i].create(resMdl, resAnmChr, &mAllocator, nullptr);
        mChrAnim[i].mPlayMode = sGhostPlayModes[i];
        mChrAnim[i].setRate(0.0f);
        mChrAnim[i].setFrame(0.0f);
    }

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmGhost_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmGhost_c::initState() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    daWmPlayer_c *player = daWmPlayer_c::ms_instance;
    mCutscene = -1;
    mIsGhostOut = false;
    int state = 0;

    if (IsCourseClear()) {
        state = 1;
        int result = GetCurrentPlayResultStatus();
        switch (result) {
            case 2:
            case 3:
            case 4:
            case 5:
                goto openGhost;

            case 6:
                if (IsCourseOtasukeClear()) {
                    mCutscene = 14;
                } else {
                    mCutscene = 15;
                }
                mModel.setAnm(mChrAnim[cobGhostOut]);
                mChrAnim[cobGhostOut].setRate(0.0f);
                mChrAnim[cobGhostOut].setFrame(0.0f);
                break;

            case 7:
openGhost:
                mModel.setAnm(mChrAnim[cobGhostOpen]);
                mChrAnim[cobGhostOpen].setFrame(0.0f);
                mCutscene = 13;
                break;

            case 0:
            case 8:
                if (!IsCourseOtasukeClear()) {
                    mIsGhostOut = true;
                    mModel.setAnm(mChrAnim[cobGhostOpen]);
                    mChrAnim[cobGhostOpen].setFrame(mChrAnim[cobGhostOpen].mFrameMax - 1.0f);
                }
                break;

            default:
                break;
        }

        if (!IsCourseOmoteClearSimple() && !IsCourseUraClearSimple()) {
            state = 0;
        }
    } else if (GetCurrentPlayResultStatus() == 1) {
        mCutscene = 14;
        mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
        mModel.setAnm(mChrAnim[cobGhostOut]);
        mChrAnim[cobGhostOut].setFrame(0.0f);
        mChrAnim[cobGhostOut].setRate(0.0f);
        state = 2;
    }

    dWmActor_c::construct(fProfile::WM_TERESA, this, state, &mPos, nullptr);
    if (mCutscene >= 0) {
        csSeqMng->FUN_801017c0((dCsSeqMng_c::CUTSCENE_e)mCutscene, this, player, 200);
    }
}

void daWmGhost_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == dCsSeqMng_c::CUTSCENE_CMD_NONE) {
        return;
    }

    if (!isStaff()) {
        mIsCutEnd = true;
        return;
    }

    if (isFirstFrame) {
        switch (cutsceneCommandId) {
            case 0x5f:
                if (GetClearStatus() == 4) {
                    if (GetCurrentPlayResultStatus() == 4 || GetCurrentPlayResultStatus() == 7) {
                        mIsGhostOut = false;
                        mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                        mModel.setAnm(mChrAnim[cobGhostCloseDoor]);
                        mChrAnim[cobGhostCloseDoor].setFrame(0.0f);
                        mChrAnim[cobGhostCloseDoor].setRate(1.0f);
                        dWmSeManager_c::m_pInstance->playSound(0x29, mPos, 1);
                    } else {
                        setCutEnd();
                    }
                } else {
                    setCutEnd();
                }
                break;

            case 0x15:
                if (!mIsGhostOut) {
                    mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                    mModel.setAnm(mChrAnim[cobGhostOpen]);
                    mChrAnim[cobGhostOpen].setRate(2.4f);
                    mChrAnim[cobGhostOpen].setFrame(0.0f);
                    dWmSeManager_c::m_pInstance->playSound(0x27, mPos, 1);
                }
                break;

            case 0x17:
                mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                mModel.setAnm(mChrAnim[cobGhostClose]);
                mChrAnim[cobGhostClose].setFrame(0.0f);
                mChrAnim[cobGhostClose].setRate(1.0f);
                dWmSeManager_c::m_pInstance->playSound(0x29, mPos, 1);
                break;

            case 0x16:
                mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                mModel.setAnm(mChrAnim[cobGhostOut]);
                mChrAnim[cobGhostOut].setFrame(0.0f);
                mChrAnim[cobGhostOut].setRate(1.0f);
                dWmSeManager_c::m_pInstance->playSound(0x28, mPos, 1);
                break;

            case 0x18:
                mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                mModel.setAnm(mChrAnim[cobGhostCloseAll]);
                mChrAnim[cobGhostCloseAll].setFrame(0.0f);
                mChrAnim[cobGhostCloseAll].setRate(1.0f);
                dWmSeManager_c::m_pInstance->playSound(0x29, mPos, 1);
                break;

            case 0x19:
                OSReport("testtest\n");
                mModel.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_CHR);
                mModel.setAnm(mChrAnim[cobGhostCloseWindow]);
                mChrAnim[cobGhostCloseWindow].setFrame(0.0f);
                mChrAnim[cobGhostCloseWindow].setRate(1.0f);
                break;

            default:
                break;
        }
    }

    switch (cutsceneCommandId) {
        case 0x5f:
            if (mChrAnim[cobGhostCloseDoor].isStop()) {
                mIsCutEnd = true;
            }
            break;

        case 0x15:
            if (mChrAnim[cobGhostOpen].isStop() || mIsGhostOut) {
                mIsGhostOut = true;
                mIsCutEnd = true;
            }
            break;

        case 0x17:
            if (mChrAnim[cobGhostClose].isStop()) {
                mIsGhostOut = false;
                mIsCutEnd = true;
            }
            break;

        case 0x16:
            mIsGhostOut = true;
            mIsCutEnd = true;
            break;

        case 0x18:
            if (mChrAnim[cobGhostCloseAll].isStop()) {
                mIsGhostOut = false;
                mIsCutEnd = true;
            }
            break;

        case 0x19:
            if (mChrAnim[cobGhostCloseWindow].isStop()) {
                mIsGhostOut = true;
                mIsCutEnd = true;
            }
            break;

        default:
            mIsCutEnd = true;
            break;
    }
}

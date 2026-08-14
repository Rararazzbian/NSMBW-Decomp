#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_a_wm_smallcloud.hpp>
#include <game/bases/d_a_wm_map.hpp>

void daWmSmallCloud_c::init_exec() {
    mCurrProc = PROC_TYPE_EXEC;
}

void daWmSmallCloud_c::mode_exec() {}

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

void daWmSmallCloud_c::setPosFromCourseNode() {
    // batch A's createModel() (scratch/batchA/draft.cpp) independently found a 4-entry
    // table indexed the same way (resMdlNames[ACTOR_PARAM(CourseNo)]); mirroring that
    // table size here since it is likely the real number of small-cloud variants, but
    // the actual node-name strings are unconfirmed -- placeholders only.
    static const char *nodeNames[4] = {
        "F0C0", "F0C1", "F0C2", "F0C3"
    };
    daWmMap_c::m_instance->GetNodePos(nodeNames[ACTOR_PARAM(CourseNo)], mPos);
}

void daWmSmallCloud_c::initStateLike() {
    mChrAnim[CS_Anim].setRate(1.0f);
    mChrAnim[CS_Anim].setFrame(0.0f);
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

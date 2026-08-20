#include <game/bases/d_a_peach_castle_seq.hpp>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_scene.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_game_com.hpp>

ACTOR_PROFILE(PEACH_CASTLE_SEQUENCE_MGR, daPeachCastleSequenceMgr_c, 0);
ACTOR_PROFILE(PEACH_CASTLE_SEQUENCE_MGR_OBJ, daPeachCastleSequenceMgrObj_c, 0);

STATE_DEFINE(daPeachCastleSequenceMgrObj_c, Wait);

daPeachCastleSequenceMgr_c::daPeachCastleSequenceMgr_c() {
    daPeachCastleSequenceMgrObj_c *obj = (daPeachCastleSequenceMgrObj_c *)
        fBase_c::createChild(fProfile::PEACH_CASTLE_SEQUENCE_MGR_OBJ, this, 0, 0);
    daPeachCastleSequenceMgrObj_c::m_instance = obj;
}

int daPeachCastleSequenceMgr_c::create() {
    return SUCCEEDED;
}

int daPeachCastleSequenceMgr_c::doDelete() {
    if (daPeachCastleSequenceMgrObj_c::m_instance) {
        daPeachCastleSequenceMgrObj_c::m_instance->deleteRequest();
        daPeachCastleSequenceMgrObj_c::m_instance = nullptr;
    }
    return SUCCEEDED;
}

int daPeachCastleSequenceMgr_c::execute() {
    ActorScrOutCheck(SKIP_ACTOR_DELETE);
    return SUCCEEDED;
}

daPeachCastleSequenceMgrObj_c::daPeachCastleSequenceMgrObj_c() :
    mStateMgr(*this, StateID_Wait) {}

int daPeachCastleSequenceMgrObj_c::create() {
    mTriggered = false;
    return SUCCEEDED;
}

int daPeachCastleSequenceMgrObj_c::execute() {
    mStateMgr.executeState();
    return SUCCEEDED;
}

int daPeachCastleSequenceMgrObj_c::draw() {
    return SUCCEEDED;
}

int daPeachCastleSequenceMgrObj_c::doDelete() {
    return SUCCEEDED;
}

void daPeachCastleSequenceMgrObj_c::controlDemo(bool stop) {
    if (stop) {
        daPyDemoMng_c::mspInstance->endControlDemoAll(0);
    } else {
        daPyDemoMng_c::mspInstance->startControlDemoAll();
    }
}

void daPeachCastleSequenceMgrObj_c::demoStart() {
    if (dScene_c::m_nextScene == fProfile::INVALID) {
        mTriggered = true;
        controlDemo(false);
        daPyDemoMng_c::mspInstance->m_58 = 1;
        daPyDemoMng_c::mspInstance->m_94 = 1;
    }
}

void daPeachCastleSequenceMgrObj_c::demoEnd() {
    mTriggered = false;
    controlDemo(true);
    daPyDemoMng_c::mspInstance->m_58 = 0;
    daPyDemoMng_c::mspInstance->m_94 = 0;
}

void daPeachCastleSequenceMgrObj_c::initializeState_Wait() {
    mCountdown = 0;
    mPhase = 0;
    if (dScStage_c::m_isOtehonReturn) {
        demoStart();
    }
}

void daPeachCastleSequenceMgrObj_c::finalizeState_Wait() {}

void daPeachCastleSequenceMgrObj_c::executeState_Wait() {
    switch (mPhase) {
    case 0:
        if (mTriggered) {
            mCountdown = 20;
            mPhase = 1;
        }
        break;
    case 1:
        mCountdown--;
        if (mCountdown <= 0) {
            dGameCom::ModelPlayMenuStart();
            mTriggered = false;
            mCountdown = 0;
            mPhase = 0;
        }
        break;
    }
}

daPeachCastleSequenceMgrObj_c *daPeachCastleSequenceMgrObj_c::m_instance = nullptr;

#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_game_display.hpp>
#include <game/bases/d_next.hpp>
#include <game/bases/d_quake.hpp>
#include <game/bases/d_stage_timer.hpp>
#include <game/bases/d_pause_manager.hpp>
#include <game/framework/f_manager.hpp>

void daPyMng_c::update() {
    char *base = (char *) m_playerID;

    checkLastAlivePlayer();

    dGameDisplay_c *disp = dScStage_c::getGameDisplay();
    if (disp != nullptr) {
        int *rest = (int *) (base + 0x80);
        int buf[4];
        for (int j = 0; j < 4; j++) {
            buf[j] = rest[j];
        }
        disp->setPlayNum(buf);
        disp->setCoinNum(getCoinAll());
        disp->setScore(mScore);
        disp->setCollect();
    }

    int *quakeTimer = (int *) (base + 0xa0);
    int *quakeEffectFlag = (int *) (base + 0xb0);
    for (int i = 0; i < 4; i++) {
        if (quakeTimer[i] != 0) {
            quakeTimer[i]--;
            if (quakeTimer[i] == 0) {
                quakeEffectFlag[i] = 0;
            }
        }
    }

    if (dNext_c::m_instance->mNextDataSet) {
        bool found = false;
        for (int i = 0; i < 4; i++) {
            if (checkPlayer(i)) {
                dAcPy_c *p = getCtrlPlayer(i);
                if (p != nullptr) {
                    if (!p->isStatus(daPlBase_c::STATUS_64) && !p->isWaitFrameCountMax()) {
                        found = true;
                    }
                }
            }
        }
        if (!found) {
            dNext_c::m_instance->mMultiplayerDelay = 0;
        }
    }

    if (dQuake_c::m_instance->mFlags & 0x38) {
        if (mQuakeTrigger == 0) {
            int *qef = (int *) (base + 0xb0);
            for (int i = 0; i < 4; i++) {
                dAcPy_c *p = getCtrlPlayer(i);
                if (p != nullptr) {
                    if (dQuake_c::m_instance->mFlags & 0x20) {
                        p->onStatus(daPlBase_c::STATUS_QUAKE_BIG);
                    } else if (dQuake_c::m_instance->mFlags & 0x08) {
                        p->onStatus(daPlBase_c::STATUS_QUAKE_SMALL);
                    } else if (*qef == 0) {
                        p->onStatus(daPlBase_c::STATUS_QUAKE_SMALL);
                    }
                }
                qef++;
            }
        }
        mQuakeTrigger = 1;
    } else {
        mQuakeTrigger = 0;
    }

    if (mPauseDisable == 0) {
        PauseManager_c::m_instance->setPauseEnable(true);
    } else {
        PauseManager_c::m_instance->setPauseEnable(false);
    }

    if (mStopTimerInfo != mStopTimerInfoOld) {
        if (mStopTimerInfo != 0) {
            dStageTimer_c::m_instance->mStopped = true;
        } else {
            dStageTimer_c::m_instance->mStopped = false;
        }
        mStopTimerInfoOld = mStopTimerInfo;
    }

    daPyDemoMng_c::mspInstance->update();
    dPyEffectMng_c::mspInstance->update();
}

bool daPyMng_c::isPlayerPauseEnable(s8 plrNo) {
    if (checkPlayer(plrNo) && (mPauseEnableInfo & (1 << plrNo))) {
        return true;
    }
    return false;
}

void daPyMng_c::setPlayer(int idx, dAcPy_c *player) {
    if (player == nullptr) {
        m_playerID[idx] = 0;
    } else {
        m_playerID[idx] = player->mUniqueID;
    }
}

dAcPy_c *daPyMng_c::getPlayer(int idx) {
    return (dAcPy_c *) fManager_c::searchBaseByID((fBaseID_e) m_playerID[idx]);
}

void daPyMng_c::decideCtrlPlrNo() {
    for (int i = 0; i < 4; i++) {
        if (mActPlayerInfo & (1 << i)) {
            mCtrlPlrNo = i;
            return;
        }
    }
}

bool daPyMng_c::setYoshi(daPlBase_c *player) {
    if (player == nullptr) {
        return false;
    }
    for (int i = 0; i < 4; i++) {
        if (m_yoshiID[i] == 0) {
            m_yoshiID[i] = player->mUniqueID;
            return true;
        }
    }
    return false;
}

void daPyMng_c::releaseYoshi(daPlBase_c *player) {
    if (player == nullptr) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        int id = m_yoshiID[i];
        if (id == player->mUniqueID) {
            m_yoshiID[i] = 0;
            return;
        }
    }
}

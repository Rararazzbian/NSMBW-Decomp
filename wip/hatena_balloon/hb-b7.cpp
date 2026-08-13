#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_balloon_manager.hpp>
#include <game/bases/d_enemy_manager.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_quake.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/mLib/m_effect.hpp>

// ---------------------------------------------------------------- 0x80111F90
u8 daEnHatenaBalloon_c::pause_check() {
    if (!isState(StateID_HipAttack)) {
        if (m_81d != 0) {
            m_81d--;
        }

        if (m_81d != 0) {
            m_7c4 = -m_7c4;
        } else {
            m_7c4 = 0.0f;
        }

        return m_81d;
    }
    return 0;
}

// ---------------------------------------------------------------- 0x801120D0
void daEnHatenaBalloon_c::shake_disp_check() {
    if (m_7e4 != 0) {
        m_7e4--;
    }

    m_7c8 = 0.0f;
    if (m_7e4 == 1) {
        anm_set(0);
    }
}

// ---------------------------------------------------------------- 0x80112110
void daEnHatenaBalloon_c::createItem() {
    mVec3_c pos(mPos);
    pos.y += 8.0f;
    pos.z = 600.0f;

    dActor_c *item = dActor_c::construct(fProfile::EN_ITEM, 0x07000008, &pos, nullptr, 0);
    if (item != nullptr) {
        dBalloonMng_c::m_instance->setItemId(item->mUniqueID);
    }
    dBalloonMng_c::m_instance->m_18 = 0;

    dAudio::SndObjctCmnMap_c *snd = dAudio::g_pSndObjMap;
    snd->SndObjctCmnMap::startSound(0x286, dAudio::cvtSndObjctPos(mPos), 0);
}

// ---------------------------------------------------------------- 0x801121C0
void daEnHatenaBalloon_c::break_balloon(s16 mode) {
    if (mBalloonType == 0) {
        dAcPy_c *breaker = daPyMng_c::getPlayer(m_81f);
        if (breaker != nullptr) {
            dAcPy_c *owner = daPyMng_c::getPlayer(mPlayerNo);
            if (owner != nullptr) {
                owner->setBreakBalloonJump(m_81f, mode);
                owner->mLayer = 0;
                owner->mAmiLayer = breaker->mAmiLayer;
            }
        }
    } else {
        createItem();
    }
}

// ---------------------------------------------------------------- 0x80112260
bool daEnHatenaBalloon_c::player_set() {
    u8 bgHit = 0;
    int hit = all_bgcheck(bgHit);
    if (bgHit != 0 || hit != 0) {
        return true;
    }

    if (daPyMng_c::mAllBalloon == 0) {
        break_balloon(0);
        break_effect();
        mCc.release();
        m_822 = 0;
    }
    return false;
}

// ---------------------------------------------------------------- 0x80113090
// @note The four coin-battle constants below are written as `base * 1.4f`
// rather than as folded literals.  Three of them fold to the same word either
// way, but `0.3f * 1.4f` folds to 0x3ED70A3E while the literal `0.42f` folds to
// 0x3ED70A3D -- and 0x3ED70A3E is what the DOL holds.  The .text comparator
// cannot see this: it canonicalises pool references positionally.
void daEnHatenaBalloon_c::remocon_speed_set() {
    mVec2_c delta;
    dAcPy_c *player = searchNearPlayer(delta);
    if (player == nullptr) {
        return;
    }

    if (dInfo_c::mGameFlag & dInfo_c::GAME_FLAG_IS_COIN_BATTLE) {
        if (player->mPos.x > mPos.x) {
            if (mSpeed.x > 0.0f) {
                mSpeed.x += (0.9f * 1.4f);
                mSpeedF += (0.3f * 1.4f);
            } else {
                mSpeed.x += (1.8f * 1.4f);
                mSpeedF += (0.45f * 1.4f);
            }
        } else {
            if (mSpeed.x < 0.0f) {
                mSpeed.x -= (0.9f * 1.4f);
                mSpeedF -= (0.3f * 1.4f);
            } else {
                mSpeed.x -= (1.8f * 1.4f);
                mSpeedF -= (0.45f * 1.4f);
            }
        }

        if (player->mPos.y > 16.0f + mPos.y) {
            if (mSpeed.y > 0.0f) {
                mSpeed.y += (0.9f * 1.4f);
                m_7cc += (0.3f * 1.4f);
            } else {
                mSpeed.y += (1.8f * 1.4f);
                m_7cc += (0.45f * 1.4f);
            }
        } else {
            if (mSpeed.y < 0.0f) {
                mSpeed.y -= (0.9f * 1.4f);
                m_7cc -= (0.3f * 1.4f);
            } else {
                mSpeed.y -= (1.8f * 1.4f);
                m_7cc -= (0.45f * 1.4f);
            }
        }
    } else {
        if (player->mPos.x > mPos.x) {
            if (mSpeed.x > 0.0f) {
                mSpeed.x += 0.9f;
                mSpeedF += 0.3f;
            } else {
                mSpeed.x += 1.8f;
                mSpeedF += 0.45f;
            }
        } else {
            if (mSpeed.x < 0.0f) {
                mSpeed.x -= 0.9f;
                mSpeedF -= 0.3f;
            } else {
                mSpeed.x -= 1.8f;
                mSpeedF -= 0.45f;
            }
        }

        if (player->mPos.y > 16.0f + mPos.y) {
            if (mSpeed.y > 0.0f) {
                mSpeed.y += 0.9f;
                m_7cc += 0.3f;
            } else {
                mSpeed.y += 1.8f;
                m_7cc += 0.45f;
            }
        } else {
            if (mSpeed.y < 0.0f) {
                mSpeed.y -= 0.9f;
                m_7cc -= 0.3f;
            } else {
                mSpeed.y -= 1.8f;
                m_7cc -= 0.45f;
            }
        }
    }
}

// ---------------------------------------------------------------- 0x801133A0
bool daEnHatenaBalloon_c::break_speed_set() {
    mVec3_c target(m_7b0);
    target.z = mPos.z;
    return cLib::chasePos(&mPos, target, 1.5f);
}

// ---------------------------------------------------------------- 0x80113400
void daEnHatenaBalloon_c::remocon_times_check() {
    m_7f8++;
    if (m_7f8 >= 3) {
        dGameCom::hideFukidashiForLevel(mPlayerNo, 0x14, 0);
        m_7f8 = 3;
    }
}

// ---------------------------------------------------------------- 0x80113460
bool daEnHatenaBalloon_c::player_out_check() {
    if (mBalloonType == 1) {
        return false;
    }

    dAcPy_c *player = daPyMng_c::getPlayer(mPlayerNo);
    if (player != nullptr && !player->isStatus(0x53)) {
        break_balloon(0);
        break_effect();
        mCc.release();
        m_822 = 0;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- 0x801134F0
void daEnHatenaBalloon_c::remocon_shake_check() {
    if (mBalloonType == 1) {
        return;
    }

    if (m_7e0 != 0) {
        m_7e0--;
    } else {
        dAcPy_c *player = daPyMng_c::getPlayer(mPlayerNo);
        if (player->mKey.triggerShakeJump()) {
            m_7e0 = 30;
            m_7e4 = 31;
            remocon_times_check();
            dQuake_c::m_instance->shockMotor(mPlayerNo, (dQuake_c::TYPE_SHOCK_e)10, 0, false);
            player->setBalloonHelpVoice();
            anm_set(1);
            remocon_speed_set();
        }
    }
}

// ---------------------------------------------------------------- 0x801135B0
void daEnHatenaBalloon_c::ButtonPlayerColSet() {
    if (m_7fc == 1) {
        mCc.entry();
    }
}

// ---------------------------------------------------------------- 0x801135D0
void daEnHatenaBalloon_c::break_effect() {
    if (mBalloonType == 0) {
        dGameCom::hideFukidashiTemporarily(mPlayerNo, 0x14, 0);
    }
    mEf::createEffect("Wm_mr_balloonburst", 0, &m_78c, nullptr, nullptr);
    deleteActor(1);
}

// ---------------------------------------------------------------- 0x80113640
bool daEnHatenaBalloon_c::dispInFlyInitCheck(int mode) {
    if (fly_dispin_check()) {
        bool isButton = false;
        if (mBalloonType == 0 && m_814 == 2) {
            isButton = true;
        }
        if (!isButton) {
            mCc.entry();
        }

        if (mode != 0) {
            dEnemyMng_c::m_instance->m_110--;
            if (dEnemyMng_c::m_instance->m_110 > 3) {
                dEnemyMng_c::m_instance->m_110 = 0;
            }
        }

        fly_xspeed_set(true);

        switch (mDirection) {
            case 2:
                mSpeed.y = -1.7f;
                break;
            case 3:
                mSpeed.y = 1.7f;
                break;
        }

        changeState(StateID_Fly);
        return true;
    }
    return false;
}

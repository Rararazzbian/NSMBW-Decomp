// =====================================================================
// d_a_en_hatena_balloon.cpp -- BATCH 8: the state machine.
//
// 18 functions, .text 0x801138D0 - 0x80114470 (2,840 B), in canonical
// address order. All 18 verified byte-exact against the DOL, instruction
// count x 4 == symbol-map size for every one, callee symbol names
// compared, emitted symbol order checked, and the .sdata2 literal pool
// checked per function with wip/hatena_balloon/constchk.py (18/18).
//
// Includes below are the ones THIS batch needs; the lead should merge
// them with the other batches' include block.
// =====================================================================
#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_enemy_manager.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/sLib/s_lib.hpp>

// fn_80112040 -- the TU's one unnamed file-static. Batch 6 owns the
// definition and the name; only executeState_DispFlyMove needs it here.
// Delete this declaration on merge if batch 6's definition precedes it.
static float bg_dispx_get(daEnHatenaBalloon_c *balloon);

// ---------------------------------------------------------------- 0x801138D0
void daEnHatenaBalloon_c::initializeState_DispFlyWait() {
    m_798 = mPos;
    mTimer1 = 10;
    mMaxFallSpeed = -4.0f;
    mAccelF = 0.02f;
    mAccelY = 0.0f;
    mSpeed.y = 0.0f;
    mSpeedF = 0.0f;

    // Rotating 0..3 slot index, shared by every balloon waiting to fly in;
    // create_wait_pos_set() uses it to fan the off-screen wait positions out.
    u32 idx = dEnemyMng_c::m_instance->m_110;
    if (idx > 3) {
        dEnemyMng_c::m_instance->m_110 = 0;
        idx = 0;
    }
    m_7f0 = idx;

    if (m_7f4 == 0) {
        int dir = dBg_c::m_bg_p->m_90009;
        if (m_814 == 0) {
            // The shared `case 0: case 3:` arm is load-bearing: it is what
            // puts the compare chain in the order 1, 0, 3 while leaving the
            // case bodies laid out 2, 3, 0, default. A plain switch emits
            // compares and bodies in the SAME order and is 4 words wrong.
            switch (dir) {
                case 1:
                    mDirection = 2;
                    break;
                case 0:
                case 3:
                    if (dir == 3) {
                        mDirection = 3;
                    } else {
                        mDirection = 0;
                    }
                    break;
                default:
                    mDirection = 1;
                    break;
            }
        } else if (dir == 1 || dir == 3) {
            if (dBgParameter_c::ms_Instance_p->yStart()
                - 0.5f * dBgParameter_c::ms_Instance_p->ySize() > mPos.y) {
                mDirection = 3;
            } else {
                mDirection = 2;
            }
        } else {
            float half = 0.5f * dBgParameter_c::ms_Instance_p->xSize();
            if (half + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x) > mPos.x) {
                mDirection = 1;
            } else {
                mDirection = 0;
            }
        }
    }

    create_wait_pos_set();
    dEnemyMng_c::m_instance->m_110 = dEnemyMng_c::m_instance->m_110 + 1;
}

// ---------------------------------------------------------------- 0x80113A80
void daEnHatenaBalloon_c::finalizeState_DispFlyWait() {}

// ---------------------------------------------------------------- 0x80113A90
void daEnHatenaBalloon_c::executeState_DispFlyWait() {
    if (m_814 == 2) {
        fly_xdisp_check(true);
        fly_ydisp_check(true);
    }
    create_wait_pos_set();

    if (mTimer1 != 0) {
        return;
    }
    if (m_814 == 0) {
        switch (mDirection) {
            case 2:
            case 3:
                fly_ydisp_check(false);
                if (mDirection == 2) {
                    mPos.y = 32.0f + dBgParameter_c::ms_Instance_p->yStart();
                } else {
                    mPos.y = dBgParameter_c::ms_Instance_p->yEnd() - 32.0f;
                }
                break;
            case 0:
            case 1:
                fly_xdisp_check(false);
                if (mDirection == 0) {
                    mPos.x = 32.0f + (dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                                      + dBgParameter_c::ms_Instance_p->xSize());
                } else {
                    mPos.x = dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x) - 32.0f;
                }
                break;
        }
    }
    changeState(StateID_DispFlyMove);
}

// ---------------------------------------------------------------- 0x80113BF0
void daEnHatenaBalloon_c::initializeState_DispFlyMove() {
    if (mBalloonType != 0) {
        return;
    }
    dGameCom::showFukidashi(mPlayerNo, 20);
}

// ---------------------------------------------------------------- 0x80113C10
void daEnHatenaBalloon_c::finalizeState_DispFlyMove() {}

// ---------------------------------------------------------------- 0x80113C20
void daEnHatenaBalloon_c::executeState_DispFlyMove() {
    // bgpY / bgDispY must be named locals, in this order: they are what puts
    // the two loads in the target's order without transposing f0 and f2.
    float bgpY = dBgParameter_c::ms_Instance_p->mPos.y;
    float bgDispY = dBg_c::m_bg_p->m_8feac;
    float yScroll = -(bgDispY - bgpY);
    float xScroll = -(bg_dispx_get(this)
                      - dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x));

    int dir = mDirection;
    switch (dir) {
        case 2:
            fly_xdisp_check(false);
            mSpeed.y = -1.7f;
            if (mSpeed.y > yScroll) {
                mSpeed.y = yScroll;
            }
            break;
        case 3:
            fly_xdisp_check(false);
            mSpeed.y = 1.7f;
            if (mSpeed.y < yScroll) {
                mSpeed.y = yScroll;
            }
            break;
        case 0:
            fly_ydisp_check(false);
            mSpeedF = -1.7f;
            if (mSpeedF > xScroll - 1.0f) {
                mSpeedF = xScroll - 1.0f;
            }
            break;
        case 1:
            fly_ydisp_check(false);
            mSpeedF = 1.7f;
            if (mSpeedF < 1.0f + xScroll) {
                mSpeedF = 1.0f + xScroll;
            }
            break;
    }

    dispInFlyInitCheck(1);
    ButtonPlayerColSet();
    mSpeedMax.x = mSpeedF;
    mSpeed.z = 0.0f;
    calcSpeedX();
    posMove();

    if (m_814 == 2) {
        fly_xdisp_check(true);
        fly_ydisp_check(true);
    }
}

// ---------------------------------------------------------------- 0x80113DC0
void daEnHatenaBalloon_c::initializeState_Fly() {
    mTimer1 = 0;
    mTimer2 = 50;
    m_7cc = -sm_hio_fly_yspeed;
    m_820 = 0;
    m_81e = 1;
}

// ---------------------------------------------------------------- 0x80113DF0
void daEnHatenaBalloon_c::finalizeState_Fly() {}

// ---------------------------------------------------------------- 0x80113E00
void daEnHatenaBalloon_c::executeState_Fly() {
    if (mTimer1 == 0) {
        fly_xspeed_set(false);
        mTimer1 = sm_hio_base_fly_timer_x;
    }

    if (mTimer2 == 0) {
        fly_yspeed_set();
    } else {
        sLib::chase(&mSpeed.y, m_7cc, 0.02f);
    }

    fly_ydisp_check(true);
    fly_xdisp_check(true);
    ButtonPlayerColSet();
    mSpeedMax.x = mSpeedF;
    calcSpeedX();
    remocon_shake_check();
    mSpeed.z = 0.0f;
    posMove();
    player_out_check();
}

// ---------------------------------------------------------------- 0x80113ED0
void daEnHatenaBalloon_c::initializeState_Escape() {
    mCc.release();

    float half = 0.5f * dBgParameter_c::ms_Instance_p->xSize();
    float midX = half + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x);

    if (dEnemyMng_c::m_instance->m_15c == 1) {
        if (midX >= mPos.x) {
            mSpeedF = -0.8f;
        } else {
            mSpeedF = 0.8f;
        }
    } else {
        mSpeedF = -0.8f;
    }
}

// ---------------------------------------------------------------- 0x80113F70
void daEnHatenaBalloon_c::finalizeState_Escape() {}

// ---------------------------------------------------------------- 0x80113F80
void daEnHatenaBalloon_c::executeState_Escape() {
    if (escape_dispout_check()) {
        if (mTimer2 == 0) {
            fly_yspeed_set();
        } else {
            sLib::chase(&mSpeed.y, m_7cc, 0.02f);
        }
        mSpeedMax.x = mSpeedF;
        mSpeed.z = 0.0f;
        calcSpeedX();
        posMove();
    }

    if (dEnemyMng_c::m_instance->m_15c == 0) {
        if (dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x) >= mPos.x) {
            mDirection = 1;
        } else {
            mDirection = 0;
        }
        if (m_814 == 2) {
            m_814 = 0;
        }
        if (fly_dispin_check()) {
            mCc.entry();
            changeState(StateID_Fly);
        } else {
            m_7f4 = 1;
            changeState(StateID_DispFlyWait);
        }
    }

    ButtonPlayerColSet();
    player_out_check();
}

// ---------------------------------------------------------------- 0x801140C0
void daEnHatenaBalloon_c::initializeState_HipAttack() {}

// ---------------------------------------------------------------- 0x801140D0
void daEnHatenaBalloon_c::finalizeState_HipAttack() {}

// ---------------------------------------------------------------- 0x801140E0
void daEnHatenaBalloon_c::executeState_HipAttack() {
    player_set();
    changeState(StateID_SearchSpace);
    fly_ydisp_check(true);
    fly_xdisp_check(true);
}

// ---------------------------------------------------------------- 0x80114140
void daEnHatenaBalloon_c::initializeState_SearchSpace() {
    m_7e0 = 30;
    m_804 = 0;
}

// ---------------------------------------------------------------- 0x80114160
void daEnHatenaBalloon_c::finalizeState_SearchSpace() {
    m_881 = 0;
    mSpeed.x = m_890.x;
    mSpeed.y = m_890.y;
    mSpeed.z = 0.0f;
}

// ---------------------------------------------------------------- 0x80114190
void daEnHatenaBalloon_c::executeState_SearchSpace() {
    if (m_881 == 0) {
        m_884 = mPos;
    }

    int moved = 0;
    if (break_speed_set()) {
        moved = 1;
    }

    if (m_881 == 0) {
        // `m_890 = mPos - m_884;` and not a named difference vector: the
        // by-value operator- temporary lands in the LOW stack slot, which is
        // where the target keeps it. A named local takes the high slot and
        // swaps places with checkPos below.
        m_890 = mPos - m_884;
        m_890.z = 0.0f;
        m_881 = 1;
    }

    bool yHit = fly_ydisp_check(true);
    if (fly_xdisp_check(true) || yHit) {
        moved = 1;
    }

    if (moved != 0) {
        float dz = m_7d4.z;
        mVec3_c checkPos(mPos.x, mPos.y + dz, mPos.z);
        u32 bgRes = pointBgCheck(checkPos, (u32)m_7d4.x, (u32)m_7d4.y, 0);
        int pole = 0;
        if (goalpole_check()) {
            pole = 0xFF;
        }

        int doBreak = 0;
        if (mHitFlag != 0) {
            if (bgRes == 0 && pole == 0) {
                doBreak = 1;
                mHitFlag = 0;
                m_87c = 30;
                m_880 = 1;
            } else {
                float dx = std::fabs(mPos.x - mHitPos.x);
                if (dx <= 2.0f) {
                    float dy = std::fabs(mPos.y - mHitPos.y);
                    if (dy <= 2.0f) {
                        doBreak = 0;
                    }
                }
            }
        }
        if (daPyMng_c::mAllBalloon != 0) {
            doBreak = 0;
        }

        if (doBreak != 0) {
            break_balloon(0);
            break_effect();
            mCc.release();
            m_822 = 0;
        } else {
            changeState(StateID_Fly);
            m_828 = m_7b0;
            m_87c = 30;
            mHitFlag = 0;
        }
    } else if (mLastPos.distTo(mPos) < 0.02f) {
        m_804 = m_804 + 1;
        if (m_804 >= 5) {
            changeState(StateID_Fly);
            m_828 = m_7b0;
            m_804 = 0;
        }
    }
}

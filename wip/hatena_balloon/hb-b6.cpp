#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/bases/d_game_com.hpp>

// ---------------------------------------------------------------- 0x80112040
// Unnamed in the symbol map (fn_80112040), 0x88 bytes.  File-static free
// function with two in-TU callers (fly_xdisp_check, executeState_DispFlyMove),
// so `static` is correct.  Name invented -- see the report.
static float bg_dispx_get(daEnHatenaBalloon_c *balloon) {
    float bgX = dBg_c::m_bg_p->m_8fea8;
    float diff = std::fabs(bgX - dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(balloon->mPos.x));
    if (diff >= dBgParameter_c::ms_Instance_p->mSize.x) {
        return dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(balloon->mPos.x);
    }
    return bgX;
}

// ---------------------------------------------------------------- 0x80112950
void daEnHatenaBalloon_c::fly_yspeed_set() {
    float half = 0.5f * dBgParameter_c::ms_Instance_p->mSize.y;
    float midY = dBgParameter_c::ms_Instance_p->mPos.y - half;
    float unit = half * 0.125f;
    float dist = std::fabs((16.0f + mPos.y) - midY);
    float r = dGameCom::rnd();
    int flip = 0;
    if (dist < 6.0f * unit) {
        if (r < 0.5f) {
            flip = 1;
        }
    } else if (dist < 7.0f * unit) {
        if (r < 0.7f) {
            flip = 1;
        }
    } else {
        flip = 1;
    }

    if (m_81e == 0) {
        m_81e = 1;
        m_7cc = -sm_hio_fly_yspeed;
    } else {
        m_81e = 0;
        m_7cc = sm_hio_fly_yspeed;
    }

    if (flip != 0) {
        m_820 = 1;
        mTimer2 = 100;
        if (m_81e == 0) {
            if (mPos.y < midY) {
                mTimer2 = 150;
            }
        } else {
            if (mPos.y > midY) {
                mTimer2 = 150;
            }
        }
    } else {
        mTimer2 = 100;
        if (m_81e == 0) {
            if (mPos.y > midY) {
                mTimer2 = 150;
            }
        } else {
            if (mPos.y < midY) {
                mTimer2 = 150;
            }
        }
    }
}

// ---------------------------------------------------------------- 0x80112B00
void daEnHatenaBalloon_c::fly_xspeed_set(bool force) {
    if (!force) {
        float half = 0.5f * dBgParameter_c::ms_Instance_p->mSize.x;
        float midX = half + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x);
        float unit = half / 6.0f;
        float dist = std::fabs(mPos.x - midX);
        float r = dGameCom::rnd();
        int flip = 0;
        if (dist < 3.0f * unit) {
            if (r < 0.5f) {
                flip = 1;
            }
        } else if (dist < 5.0f * unit) {
            if (r < 0.7f) {
                flip = 1;
            }
        } else {
            flip = 1;
        }
        if (flip != 0) {
            if (mPos.x < midX) {
                mSpeedF = 0.4f;
            } else {
                mSpeedF = -0.4f;
            }
            return;
        }
    }

    float r2 = dGameCom::rnd();
    if (r2 < 0.8f || force) {
        if (r2 < 0.3f) {
            mSpeedF = 0.4f;
        } else {
            mSpeedF = -0.4f;
        }
    }
}

// ---------------------------------------------------------------- 0x80112C70
bool daEnHatenaBalloon_c::fly_ydisp_check(bool bounce) {
    float scroll = -(dBg_c::m_bg_p->m_8feac - dBgParameter_c::ms_Instance_p->mPos.y);
    if (scroll < -7.0f) {
        scroll = -7.0f;
    }
    if (scroll > 7.0f) {
        scroll = 7.0f;
    }

    float ySpeed = mSpeed.y;
    int hit = 0;
    float top = dBgParameter_c::ms_Instance_p->mPos.y - 24.0f;
    if (mPos.y > top) {
        mPos.y = top;
        hit = 1;
        if (ySpeed > scroll) {
            mSpeed.y = 0.5f * scroll;
        }
    } else {
        float bottom = dBgParameter_c::ms_Instance_p->mPos.y - dBgParameter_c::ms_Instance_p->mSize.y;
        if (mPos.y < bottom) {
            mPos.y = bottom;
            hit = 2;
            if (ySpeed < scroll) {
                mSpeed.y = 0.5f * scroll;
            }
        }
    }

    if (hit != 0 && bounce) {
        if (scroll == 0.0f) {
            mSpeed.y = -ySpeed;
        }
    }
    return hit;
}

// ---------------------------------------------------------------- 0x80112D50
bool daEnHatenaBalloon_c::fly_xdisp_check(bool bounce) {
    float scroll = -(bg_dispx_get(this) - dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x));
    if (scroll < -7.0f) {
        scroll = -7.0f;
    }
    if (scroll > 7.0f) {
        scroll = 7.0f;
    }

    int hit = 0;
    float left = 16.0f + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x);
    if (mPos.x < left) {
        hit = 1;
        float speedF = mSpeedF;
        mPos.x = 16.0f + dBgParameter_c::ms_Instance_p->mPos.x;
        if (speedF < scroll) {
            mSpeedF = 0.5f * scroll;
        }
    } else {
        float right = dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                    + dBgParameter_c::ms_Instance_p->mSize.x - 16.0f;
        if (mPos.x > right) {
            float speedF = mSpeedF;
            hit = 2;
            mPos.x = dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                   + dBgParameter_c::ms_Instance_p->mSize.x - 16.0f;
            if (speedF > scroll) {
                mSpeedF = 0.5f * scroll;
            }
        }
    }

    if (hit != 0 && bounce) {
        if (std::fabs(scroll) < 0.4f) {
            if (hit == 1) {
                mSpeedF = 0.4f;
            } else {
                mSpeedF = -0.4f;
            }
        }
    }
    return hit;
}

// ---------------------------------------------------------------- 0x80112EF0
bool daEnHatenaBalloon_c::fly_dispin_check() {
    if (mPos.y <= dBgParameter_c::ms_Instance_p->mPos.y - 24.0f
        && mPos.y >= dBgParameter_c::ms_Instance_p->mPos.y - dBgParameter_c::ms_Instance_p->mSize.y
        && mPos.x >= 16.0f + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
        && mPos.x <= dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                     + dBgParameter_c::ms_Instance_p->mSize.x - 16.0f) {
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- 0x80112FC0
bool daEnHatenaBalloon_c::escape_dispout_check() {
    // NEGATIVE CONTROL: 47.0f is deliberately wrong (the target uses 48.0f).
    if (mPos.y <= 32.0f + dBgParameter_c::ms_Instance_p->mPos.y
        && mPos.y >= dBgParameter_c::ms_Instance_p->mPos.y
                     - dBgParameter_c::ms_Instance_p->mSize.y - 32.0f
        && mPos.x >= dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x) - 47.0f
        && mPos.x <= 47.0f + (dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                              + dBgParameter_c::ms_Instance_p->mSize.x)) {
        return true;
    }
    return false;
}

/// @brief X/Y nudge applied to the off-screen wait position, indexed by #m_7f0.
static const float l_create_diff[] = { 0.0f, -32.0f, 32.0f, -64.0f };

// ---------------------------------------------------------------- 0x80113740
void daEnHatenaBalloon_c::create_wait_pos_set() {
    if (m_814 != 0) {
        return;
    }

    mPos.x = dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x);
    float top = dBgParameter_c::ms_Instance_p->mPos.y;
    mPos.y = top;

    if (mDirection == 2) {
        mPos.x = mPos.x + 0.5f * dBgParameter_c::ms_Instance_p->mSize.x + l_create_diff[m_7f0];
    } else if (mDirection == 3) {
        mPos.x = mPos.x + 0.5f * dBgParameter_c::ms_Instance_p->mSize.x;
        mPos.x = mPos.x + l_create_diff[m_7f0];
        mPos.y = top - (32.0f + dBgParameter_c::ms_Instance_p->mSize.y);
    } else if (mDirection == 0) {
        mPos.x = mPos.x + (32.0f + dBgParameter_c::ms_Instance_p->mSize.x);
        mPos.y = top - (32.0f + 0.5f * dBgParameter_c::ms_Instance_p->mSize.y) + l_create_diff[m_7f0];
    } else {
        mPos.x = mPos.x - 32.0f;
        mPos.y = top - (32.0f + 0.5f * dBgParameter_c::ms_Instance_p->mSize.y) + l_create_diff[m_7f0];
    }
}

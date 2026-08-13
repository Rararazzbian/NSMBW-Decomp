// Batch 6 of d_a_en_hatena_balloon.cpp -- flight physics + the TU's one file-static.
//
// SPELLING IS LOAD-BEARING HERE.  Four idioms in this file look like they could be
// tidied and cannot be; each was found by sweeping variants against the original:
//
//   * `dBgParameter_c`'s inline accessors (xSize/ySize/xStart/yStart) instead of
//     the open-coded members.  They transpose the FP pair on the surrounding
//     fadds/fmuls.  Hoisting the same value into a named local does NOT work --
//     the local is what produces the wrong operand order.
//   * `half / 8.0f`, not `half * 0.125f`.  CodeWarrior folds the power-of-two
//     divide to a reciprocal multiply, and the folded form carries operand order
//     (numerator, reciprocal).  Sibling fly_xspeed_set uses `half / 6.0f`.
//   * `if (!scroll)`, not `if (scroll == 0.0f)` -- see fly_ydisp_check.
//   * `u32 hit`, not `int hit`, in fly_xdisp_check -- see the note there.
//
// 7 of the 8 functions are byte-exact; fly_ydisp_check is 2 words out.  Details
// in the report and in the comment on that function.

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
    if (std::fabs(bgX - dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(balloon->mPos.x))
        >= dBgParameter_c::ms_Instance_p->xSize()) {
        return dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(balloon->mPos.x);
    }
    return dBg_c::m_bg_p->m_8fea8;
}

// ---------------------------------------------------------------- 0x80112950
void daEnHatenaBalloon_c::fly_yspeed_set() {
    float sy = dBgParameter_c::ms_Instance_p->mSize.y;
    float half = 0.5f * sy;
    float midY = dBgParameter_c::ms_Instance_p->mPos.y - half;
    float dist = std::fabs((16.0f + mPos.y) - midY);
    // `half / 8.0f`, not `half * 0.125f`: MWCC folds the divide to a reciprocal
    // multiply and the folded form emits fmuls(numerator, reciprocal).  Spelling
    // it as an explicit multiply transposes the operands.
    float unit = half / 8.0f;
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
    float sp = 0.4f;
    if (!force) {
        float sx = dBgParameter_c::ms_Instance_p->mSize.x;
        float half = 0.5f * sx;
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
                mSpeedF = sp;
            } else {
                mSpeedF = -sp;
            }
            return;
        }
    }

    float r2 = dGameCom::rnd();
    if (r2 < 0.8f || force) {
        if (r2 < 0.3f) {
            mSpeedF = sp;
        } else {
            mSpeedF = -sp;
        }
    }
}

// ---------------------------------------------------------------- 0x80112C70
// NOT byte-exact: 2 words out of 55.  The original loads dBgParameter_c's mPos.y
// BEFORE dBg_c's m_8feac; this spelling schedules them the other way round.  Same
// registers, same instructions, same count -- purely the order of two adjacent,
// independent lfs.  ~80 source variants were swept without moving it; the axes that
// were eliminated are listed in the report.  Everything else in the function,
// including the `!scroll` below, is confirmed exact.
bool daEnHatenaBalloon_c::fly_ydisp_check(bool bounce) {
    float lim = 7.0f;
    float scroll = -(dBg_c::m_bg_p->m_8feac - dBgParameter_c::ms_Instance_p->mPos.y);
    if (scroll < -lim) {
        scroll = -lim;
    }
    if (scroll > lim) {
        scroll = lim;
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
        // `!scroll`, not `scroll == 0.0f`: the explicit comparison emits
        // fcmpu(0.0, scroll); this emits fcmpu(scroll, 0.0), which is what the
        // original has.  Reversing the operands in the source does not help.
        if (!scroll) {
            mSpeed.y = -ySpeed;
        }
    }
    return hit;
}

// ---------------------------------------------------------------- 0x80112D50
bool daEnHatenaBalloon_c::fly_xdisp_check(bool bounce) {
    float lim = 7.0f;
    float scroll = -(bg_dispx_get(this) - dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x));
    if (scroll < -lim) {
        scroll = -lim;
    }
    if (scroll > lim) {
        scroll = lim;
    }

    // `u32`, not `int`: the original compares `hit == 1` with cmplwi.  With a
    // signed int MWCC emits cmpwi there (and `hit != 0` stays cmpwi either way).
    u32 hit = 0;
    if (mPos.x < 16.0f + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)) {
        hit = 1;
        mPos.x = 16.0f + dBgParameter_c::ms_Instance_p->xStart();
        if (mSpeedF < scroll) {
            mSpeedF = 0.5f * scroll;
        }
    } else if (mPos.x > dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                            + dBgParameter_c::ms_Instance_p->xSize() - 16.0f) {
        mPos.x = dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                     + dBgParameter_c::ms_Instance_p->xSize() - 16.0f;
        // `hit = 2` after the store, unlike `hit = 1` above: the original schedules
        // the `li` late in this branch and early in the other one.
        hit = 2;
        if (mSpeedF > scroll) {
            mSpeedF = 0.5f * scroll;
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
    float by = dBgParameter_c::ms_Instance_p->mPos.y;
    if (mPos.y <= by - 24.0f
        && mPos.y >= by - dBgParameter_c::ms_Instance_p->mSize.y
        && mPos.x >= 16.0f + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)) {
        if (mPos.x <= dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                          + dBgParameter_c::ms_Instance_p->xSize() - 16.0f) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------- 0x80112FC0
bool daEnHatenaBalloon_c::escape_dispout_check() {
    float by = dBgParameter_c::ms_Instance_p->mPos.y;
    if (mPos.y <= 32.0f + by
        && mPos.y >= by - dBgParameter_c::ms_Instance_p->mSize.y - 32.0f
        && mPos.x >= dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x) - 48.0f) {
        if (mPos.x <= 48.0f + (dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                                   + dBgParameter_c::ms_Instance_p->xSize())) {
            return true;
        }
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

    // The three `float x`/`float y` locals below are required.  Adding the
    // l_create_diff term directly to the full expression emits fadds(diff, value);
    // binding the value first emits fadds(value, diff), which is the original.
    // The mDirection == 3 branch is different again: there the original keeps BOTH
    // stores to mPos.x, which only survives if the mPos.y statement sits between
    // them.
    if (mDirection == 2) {
        float x = mPos.x + 0.5f * dBgParameter_c::ms_Instance_p->xSize();
        mPos.x = x + l_create_diff[m_7f0];
    } else if (mDirection == 3) {
        mPos.x = mPos.x + 0.5f * dBgParameter_c::ms_Instance_p->xSize();
        mPos.y = top - (32.0f + dBgParameter_c::ms_Instance_p->ySize());
        mPos.x = mPos.x + l_create_diff[m_7f0];
    } else if (mDirection == 0) {
        mPos.x = mPos.x + (32.0f + dBgParameter_c::ms_Instance_p->xSize());
        float y = top - (32.0f + 0.5f * dBgParameter_c::ms_Instance_p->ySize());
        mPos.y = y + l_create_diff[m_7f0];
    } else {
        mPos.x = mPos.x - 32.0f;
        float y = top - (32.0f + 0.5f * dBgParameter_c::ms_Instance_p->ySize());
        mPos.y = y + l_create_diff[m_7f0];
    }
}

#include <game/bases/d_line_mng.hpp>

STATE_DEFINE(dLineMng_c, Idle);
STATE_DEFINE(dLineMng_c, FallDown);
STATE_DEFINE(dLineMng_c, Left45);
STATE_DEFINE(dLineMng_c, Right45);
STATE_DEFINE(dLineMng_c, Side);
STATE_DEFINE(dLineMng_c, Height);
STATE_DEFINE(dLineMng_c, CornerHeightLine);
STATE_DEFINE(dLineMng_c, CornerSideLine);
STATE_DEFINE(dLineMng_c, Left30Left);
STATE_DEFINE(dLineMng_c, Left30Right);
STATE_DEFINE(dLineMng_c, Right30Left);
STATE_DEFINE(dLineMng_c, Right30Right);
STATE_DEFINE(dLineMng_c, Left60Up);
STATE_DEFINE(dLineMng_c, Left60Down);
STATE_DEFINE(dLineMng_c, Right60Down);
STATE_DEFINE(dLineMng_c, Right60Up);
STATE_DEFINE(dLineMng_c, Circle);
STATE_DEFINE(dLineMng_c, Circle2x2Leftup);
STATE_DEFINE(dLineMng_c, Circle2x2Rightup);
STATE_DEFINE(dLineMng_c, Circle2x2LeftDown);
STATE_DEFINE(dLineMng_c, Circle2x2RightDown);
STATE_DEFINE(dLineMng_c, Circle4x4Rightup);
STATE_DEFINE(dLineMng_c, Circle4x4LeftUp);
STATE_DEFINE(dLineMng_c, Circle4x4LeftDown);
STATE_DEFINE(dLineMng_c, Circle4x4RightDown);

dLineMng_c::dLineMng_c() :
    mStateMgr(*this, sStateID::null)
{}

void dLineMng_c::initializeState_Idle() {}
void dLineMng_c::finalizeState_Idle() {}
void dLineMng_c::executeState_Idle() {}

void dLineMng_c::initializeState_FallDown() {}
void dLineMng_c::finalizeState_FallDown() {}
void dLineMng_c::executeState_FallDown() {}

void dLineMng_c::initializeState_Left45() {}
void dLineMng_c::finalizeState_Left45() {}
void dLineMng_c::executeState_Left45() {}

void dLineMng_c::initializeState_Right45() {}
void dLineMng_c::finalizeState_Right45() {}
void dLineMng_c::executeState_Right45() {}

void dLineMng_c::initializeState_Side() {}
void dLineMng_c::finalizeState_Side() {}
void dLineMng_c::executeState_Side() {}

void dLineMng_c::initializeState_Height() {}
void dLineMng_c::finalizeState_Height() {}
void dLineMng_c::executeState_Height() {}

void dLineMng_c::initializeState_CornerHeightLine() {}
void dLineMng_c::finalizeState_CornerHeightLine() {}
void dLineMng_c::executeState_CornerHeightLine() {}

void dLineMng_c::initializeState_CornerSideLine() {}
void dLineMng_c::finalizeState_CornerSideLine() {}
void dLineMng_c::executeState_CornerSideLine() {}

void dLineMng_c::initializeState_Left30Left() {}
void dLineMng_c::finalizeState_Left30Left() {}
void dLineMng_c::executeState_Left30Left() {}

void dLineMng_c::initializeState_Left30Right() {}
void dLineMng_c::finalizeState_Left30Right() {}
void dLineMng_c::executeState_Left30Right() {}

void dLineMng_c::initializeState_Right30Left() {}
void dLineMng_c::finalizeState_Right30Left() {}
void dLineMng_c::executeState_Right30Left() {}

void dLineMng_c::initializeState_Right30Right() {}
void dLineMng_c::finalizeState_Right30Right() {}
void dLineMng_c::executeState_Right30Right() {}

void dLineMng_c::initializeState_Left60Up() {}
void dLineMng_c::finalizeState_Left60Up() {}
void dLineMng_c::executeState_Left60Up() {}

void dLineMng_c::initializeState_Left60Down() {}
void dLineMng_c::finalizeState_Left60Down() {}
void dLineMng_c::executeState_Left60Down() {}

void dLineMng_c::initializeState_Right60Down() {}
void dLineMng_c::finalizeState_Right60Down() {}
void dLineMng_c::executeState_Right60Down() {}

void dLineMng_c::initializeState_Right60Up() {}
void dLineMng_c::finalizeState_Right60Up() {}
void dLineMng_c::executeState_Right60Up() {}

void dLineMng_c::initializeState_Circle() {}
void dLineMng_c::finalizeState_Circle() {}
void dLineMng_c::executeState_Circle() {}

void dLineMng_c::initializeState_Circle2x2Leftup() {}
void dLineMng_c::finalizeState_Circle2x2Leftup() {}
void dLineMng_c::executeState_Circle2x2Leftup() {}

void dLineMng_c::initializeState_Circle2x2Rightup() {}
void dLineMng_c::finalizeState_Circle2x2Rightup() {}
void dLineMng_c::executeState_Circle2x2Rightup() {}

void dLineMng_c::initializeState_Circle2x2LeftDown() {}
void dLineMng_c::finalizeState_Circle2x2LeftDown() {}
void dLineMng_c::executeState_Circle2x2LeftDown() {}

void dLineMng_c::initializeState_Circle2x2RightDown() {}
void dLineMng_c::finalizeState_Circle2x2RightDown() {}
void dLineMng_c::executeState_Circle2x2RightDown() {}

void dLineMng_c::initializeState_Circle4x4Rightup() {}
void dLineMng_c::finalizeState_Circle4x4Rightup() {}
void dLineMng_c::executeState_Circle4x4Rightup() {}

void dLineMng_c::initializeState_Circle4x4LeftUp() {}
void dLineMng_c::finalizeState_Circle4x4LeftUp() {}
void dLineMng_c::executeState_Circle4x4LeftUp() {}

void dLineMng_c::initializeState_Circle4x4LeftDown() {}
void dLineMng_c::finalizeState_Circle4x4LeftDown() {}
void dLineMng_c::executeState_Circle4x4LeftDown() {}

void dLineMng_c::initializeState_Circle4x4RightDown() {}
void dLineMng_c::finalizeState_Circle4x4RightDown() {}
void dLineMng_c::executeState_Circle4x4RightDown() {}

// ===========================================================================
// author_geom: collision/intersection geometry family
// ===========================================================================

bool dLineMng_c::line_cross_slope_check(const mVec2_c &a, const mVec2_c &b, f32 &slope, f32 &intercept) {
    f32 dx = b.x - a.x;
    f32 dy = b.y - a.y;
    if (dx == 0.0f) {
        return false;
    }
    slope = dy / dx;
    intercept = b.y - slope * b.x;
    return true;
}

bool dLineMng_c::line_cross_range_check(f32 a, f32 b, f32 v) {
    f32 lo, hi;
    if (b >= a) {
        lo = a;
        hi = b;
    } else {
        lo = b;
        hi = a;
    }
    return v >= lo - 0.1f && v <= hi + 0.1f;
}

bool dLineMng_c::line_cross_chk1(f32 p1, f32 p2, const mVec2_c &p3, mVec2_c p4, mVec2_c p5, mVec2_c &out) {
    p4.x -= p3.x;
    p4.y -= p3.y;
    p5.x -= p3.x;
    p5.y -= p3.y;

    f32 slope, intercept;
    if (line_cross_slope_check(p4, p5, slope, intercept)) {
        if (p1 - slope == 0.0f) {
            if (intercept != 0.0f) {
                return false;
            }
            out.x = p5.x;
            out.y = p5.y;
        } else {
            out.x = intercept / (p1 - slope);
            out.y = p1 * out.x;
        }

        if (!(out.x >= 0.1f && out.x <= p2 + 0.1f)) {
            return false;
        }
        if (!line_cross_range_check(p4.x, p5.x, out.x)) {
            return false;
        }
    } else {
        if (!(p5.x >= 0.0f && p5.x < p2)) {
            return false;
        }
        out.x = p5.x;
        out.y = p1 * p5.x;
        if (!line_cross_range_check(p4.y, p5.y, out.y)) {
            return false;
        }
    }

    out.x += p3.x;
    out.y += p3.y;
    return true;
}

bool dLineMng_c::line_cross_chk2(f32 p1, const mVec2_c &p3, mVec2_c p4, mVec2_c p5, f32 &out) {
    p4.x -= p3.x;
    p4.y -= p3.y;
    p5.x -= p3.x;
    p5.y -= p3.y;

    if (p4.x != 0.0f && p5.x != 0.0f) {
        if (p4.x >= 0.0f) {
            if (!(p5.x >= 0.0f)) {
                return false;
            }
        } else if (p5.x < 0.0f) {
            return false;
        }
    }

    f32 slope, intercept;
    if (line_cross_slope_check(p4, p5, slope, intercept)) {
        if (!(intercept >= -0.1f && intercept <= 0.1f + p1)) {
            return false;
        }
        if (!line_cross_range_check(p4.y, p5.y, intercept)) {
            return false;
        }
        out = intercept;
        return true;
    } else {
        if (p5.x != 0.0f) {
            return false;
        }
        if (!(p5.y >= 0.0f && p5.y < p1)) {
            return false;
        }
        out = p5.y;
        return true;
    }
}

bool dLineMng_c::line_cross_chk3(f32 p1, const mVec2_c &p2, const mVec2_c &p3) {
    f32 d3 = p3.x * p3.x + p3.y * p3.y - p1;
    if (d3 == 0.0f) {
        return true;
    }
    f32 d2 = p2.x * p2.x + p2.y * p2.y - p1;
    if (d3 < 0.0f) {
        if (d2 >= 0.0f) {
            goto ok;
        }
        return false;
    }
    if (d2 < 0.0f) {
        goto ok;
    }
    return false;
ok:
    return true;
}

// @unofficial TESTING-ONLY: unnamed file-scope helper (target `fn_800C1EE0`,
// per MAPPING.md's "Unnamed file-scope functions" list). Reconstructed from
// width_cross_chk's caller side and its own body: takes a dLineMng_c* that
// passes straight through from the caller's own `this` (unmodified before
// the call), so it is a free `static` function, not a class member (member
// statics in this unit -- line_cross_chk1 etc. -- carry real mangled names
// in the target; this one has none at all). NOT part of this round's
// assignment; authored only because width_cross_chk cannot be compiled or
// tested without it. See RESULT.md for the friend-declaration caveat.
bool fn_800C1EE0(dLineMng_c *pThis, f32 a, f32 b, const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3, const mVec2_c &origin) {
    mVec2_c out;
    // chk1 needs p2/p3 BY VALUE (it translates them in place); this helper
    // receives them by reference from its caller and makes its OWN local
    // copies here -- confirmed from the target: width_cross_chk passes its
    // own r5/r6 straight through unmodified (no copy at its call site), and
    // fn_800C1EE0's own body stores p2/p3 into ITS OWN stack slots before
    // calling line_cross_chk1.
    mVec2_c p2c = p2;
    mVec2_c p3c = p3;
    bool result = dLineMng_c::line_cross_chk1(a, b, origin, p2c, p3c, out);
    if (result) {
        pThis->mPos.x = out.x;
        pThis->mPos.y = out.y;
        pThis->mUnitBasePos.x = p1.x;
        pThis->mUnitBasePos.y = p1.y;
    }
    return result;
}

bool dLineMng_c::height_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c pt(p1.x + 16.0f, p1.y - 16.0f);
    f32 outY;
    bool result = line_cross_chk2(16.0f, pt, p2, p3, outY);
    if (result) {
        mPos.x = pt.x;
        mPos.y = outY + pt.y;
        mUnitBasePos.x = p1.x;
        mUnitBasePos.y = p1.y;
        if (mSpeed.y < 0.0f) {
            if (mBaseSpeed > 0.0f) {
                change_dir();
            }
        } else {
            if (mBaseSpeed < 0.0f) {
                change_dir();
            }
        }
    }
    return result;
}

bool dLineMng_c::width_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c origin;
    origin.y = p1.y - 16.0f;
    origin.x = p1.x;
    return fn_800C1EE0(this, 0.0f, 16.0f, p1, p2, p3, origin);
}

bool dLineMng_c::lineF_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 8.0f;
    origin.x = p1.x + 8.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(64.0f, p2, p3);
    if (result) {
        mAngle = cM::atan2s(p3.y, p3.x);
        mUnitBasePos.x = p1.x;
        mUnitBasePos.y = p1.y;
        mStateMgr.changeState(StateID_Circle);
    }
    return result;
}

bool dLineMng_c::circle_ul2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 16.0f;
    origin.x = p1.x + 16.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(256.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0xc100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle += 0x3f00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
        change_dir();
            mStateMgr.changeState(StateID_Circle2x2Leftup);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::circle_ur2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 16.0f;
    origin.x = p1.x;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(256.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle -= 0x100;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
        change_dir();
            mStateMgr.changeState(StateID_Circle2x2Rightup);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::circle_dl2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y;
    origin.x = p1.x + 16.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(256.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x8100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle += 0x7f00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle2x2LeftDown);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::circle_dr2_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    p3.x -= p1.x;
    p3.y -= p1.y;
    p2.x -= p1.x;
    p2.y -= p1.y;

    bool result = line_cross_chk3(256.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x4100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle -= 0x4100;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle2x2RightDown);
        } else {
            return false;
        }
    }
    return result;
}

bool dLineMng_c::lineRHUR_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 32.0f;
    origin.x = p1.x;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(1024.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle -= 0x100;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
        change_dir();
            mStateMgr.changeState(StateID_Circle4x4Rightup);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::lineRHUL_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y - 32.0f;
    origin.x = p1.x + 32.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(1024.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0xc100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle += 0x3f00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
        change_dir();
            mStateMgr.changeState(StateID_Circle4x4LeftUp);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::lineRHLL_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    mVec2_c origin;
    origin.y = p1.y;
    origin.x = p1.x + 32.0f;
    p3.x -= origin.x;
    p3.y -= origin.y;
    p2.x -= origin.x;
    p2.y -= origin.y;

    bool result = line_cross_chk3(1024.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x8100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle += 0x7f00;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle4x4LeftDown);
        } else {
            result = false;
        }
    }
    return result;
}

bool dLineMng_c::lineRHLR_cross_chk(const mVec2_c &p1, mVec2_c p2, mVec2_c p3) {
    p3.x -= p1.x;
    p3.y -= p1.y;
    p2.x -= p1.x;
    p2.y -= p1.y;

    bool result = line_cross_chk3(1024.0f, p2, p3);
    if (result) {
        u16 angle = cM::atan2s(p3.y, p3.x);
        angle += 0x4100;
        if (angle < 0x4200) {
            if (angle < 0x100) {
                angle = 0x100;
            } else if (angle >= 0x4100) {
                angle = 0x40ff;
            }
            angle -= 0x4100;
            mAngle = angle;
            mUnitBasePos.x = p1.x;
            mUnitBasePos.y = p1.y;
            mStateMgr.changeState(StateID_Circle4x4RightDown);
        } else {
            return false;
        }
    }
    return result;
}

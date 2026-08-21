// dLineMng_c::{line5,line7,line8,line9,lineA}_cross_chk
// Batch d2_lines_b. Splice in between line4_cross_chk and lineF_cross_chk
// (retail .text/pool order), in this exact order.
//
// All five are BYTE-EXACT against target.txt. Confirmed against the CURRENT
// wip/fix_bigtwo/d_line_mng.cpp base (which already contains line0/line1/
// line3h/line3v/line4 from another batch as of this round) with zero
// regressions elsewhere in the TU.
//
// Key lever: AGGREGATE COPY (AGENT_CONTEXT.md lever 10) for line7/line8/lineA.
// `mVec2_c origin = p1; origin.y -= C;` reproduces the target's load order
// (p1.y and p1.x loaded adjacently, x stored before y, the `a` scalar literal
// reloaded LATE reusing the freed register). The naive member-by-member form
// `origin.y = p1.y - C; origin.x = p1.x;` (as still used by the not-yet-fixed
// sibling `width_cross_chk` in the current base) reorders the stores and
// front-loads the `a` literal -- confirmed NOT to match by direct experiment.

bool dLineMng_c::line5_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = height_cross_chk(p1, p2, p3);
    if (result) {
        mStateMgr.changeState(StateID_Height);
    }
    return result;
}

bool dLineMng_c::line7_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c origin = p1;
    origin.y -= 16.0f;
    bool result = fn_800C1EE0(this, 0.5f, 16.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left30Left);
    }
    return result;
}

bool dLineMng_c::line8_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c origin = p1;
    origin.y -= 8.0f;
    bool result = fn_800C1EE0(this, 0.5f, 16.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left30Right);
    }
    return result;
}

bool dLineMng_c::line9_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = fn_800C1EE0(this, -0.5f, 16.0f, p1, p2, p3, p1);
    if (result) {
        mStateMgr.changeState(StateID_Right30Left);
    }
    return result;
}

bool dLineMng_c::lineA_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    mVec2_c origin = p1;
    origin.y -= 8.0f;
    bool result = fn_800C1EE0(this, -0.5f, 16.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Right30Right);
    }
    return result;
}

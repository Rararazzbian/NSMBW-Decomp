// Deliverable: lineB_cross_chk / lineC_cross_chk / lineD_cross_chk / lineE_cross_chk
// (dLineMng_c). Splice these four, in this order, immediately before
// dLineMng_c::lineF_cross_chk (and after lineA_cross_chk, once written) --
// float-literal-pool ordering pins them there, between init_term_ck_pos and
// lineF_cross_chk.
//
// All four route through the existing file-scope helper fn_800C1EE0 (defined
// earlier in the TU, used already by width_cross_chk), the same shape as the
// other simple "line" family members. Verified byte-exact against
// wip/line_mng_shared/target.txt with the union gate in
// wip/line_mng_shared/tally.py (raw-byte OR canonicalised match), and the
// float-literal and StateID symbol values were independently confirmed (see
// report) so the match is not a zeroed-relocation false positive.

bool dLineMng_c::lineB_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // AGGREGATE COPY (AGENT_CONTEXT lever 10) + compound assignment (lever 11):
    // this is what puts BOTH the member-first fadds/fsubs operand order AND
    // the clustered-then-computed load schedule the target shows. A plain
    // `mVec2_c origin(p1.x + 8.0f, p1.y - 16.0f);` matches the load schedule
    // but leaves the adds/subs literal-first (wrong); a field-by-field
    // `origin.x = p1.x + 8.0f; origin.y = p1.y - 16.0f;` gets neither.
    mVec2_c origin = p1;
    origin.x += 8.0f;
    origin.y -= 16.0f;
    bool result = fn_800C1EE0(this, 2.0f, 8.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left60Up);
    }
    return result;
}

bool dLineMng_c::lineC_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // Here only Y needs an arithmetic op and X is a plain copy, so the
    // ordinary two-argument constructor already produces the exact target
    // shape (no lever-11 operand-order issue arises for a bare copy).
    mVec2_c origin(p1.x, p1.y - 16.0f);
    bool result = fn_800C1EE0(this, 2.0f, 8.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left60Down);
    }
    return result;
}

bool dLineMng_c::lineD_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // Mirror of lineB but only X is computed (+8.0f) and Y is a plain copy;
    // needs the same aggregate-copy + compound-assignment shape as lineB for
    // the member-first fadds.
    mVec2_c origin = p1;
    origin.x += 8.0f;
    bool result = fn_800C1EE0(this, -2.0f, 8.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Right60Down);
    }
    return result;
}

bool dLineMng_c::lineE_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // No origin computation at all -- p1 is passed straight through as both
    // the p1 and origin arguments (confirmed by the target's r7 <- r4 `mr`
    // and its smaller 0x10 stack frame; the three-argument versions above
    // need 0x20 for a real local).
    bool result = fn_800C1EE0(this, -2.0f, 8.0f, p1, p2, p3, p1);
    if (result) {
        mStateMgr.changeState(StateID_Right60Up);
    }
    return result;
}

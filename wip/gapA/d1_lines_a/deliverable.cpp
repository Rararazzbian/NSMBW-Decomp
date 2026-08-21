// Batch d1_lines_a -- line0_cross_chk, line1_cross_chk, line3h_cross_chk,
// line3v_cross_chk, line4_cross_chk (dLineMng_c).
//
// Insertion point: immediately after width_cross_chk() and before
// lineF_cross_chk() in wip/fix_bigtwo/d_line_mng.cpp (retail address order
// 800C20C0 line0 -> 800C2140 line1 -> 800C21B0 line3h -> 800C2210 line3v ->
// 800C2270 line4, then 800C22D0 line5_cross_chk which is another agent's
// function and continues the same address run up to lineF at 800C2750).
//
// All five verified byte-exact against wip/line_mng_shared/target.txt via
// wip/line_mng_shared/tally.py, symbol-checked (not just raw-byte-checked --
// bl targets and StateID_* relocations read back as the correct names, not
// merely zeroed-and-equal).
//
// Signature note: these five take THREE const mVec2_c& (per the shadow
// header, game/bases/d_line_mng.hpp:91-95, and confirmed by the mangled name
// RC7mVec2_cRC7mVec2_cRC7mVec2_c in target.txt) -- NOT the by-value p2/p3 used
// by lineF_cross_chk/circle_*/lineRH* below them. Those mutate p2/p3 (subtract
// an origin) before use; these forward p2/p3 untouched into fn_800C1EE0 /
// width_cross_chk / height_cross_chk, so a const ref is sufficient and is what
// retail actually declared.

bool dLineMng_c::line0_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // AGGREGATE COPY (AGENT_CONTEXT lever 10), not two scalar field writes --
    // measured. `origin.x = p1.x; origin.y = p1.y - 16.0f;` (either field
    // order) compiles to a serial dependency through f0 that also forces the
    // 'a' literal (1.0f) to be hoisted before the prologue's r30/r31 saves;
    // retail defers that load until immediately before the call. Copying the
    // whole struct first frees f0/f1 to hold p1.y/p1.x concurrently, which is
    // what lets the compiler schedule the literal load late, matching retail
    // exactly.
    mVec2_c origin = p1;
    origin.y -= 16.0f;
    bool result = fn_800C1EE0(this, 1.0f, 16.0f, p1, p2, p3, origin);
    if (result) {
        mStateMgr.changeState(StateID_Left45);
    }
    return result;
}

bool dLineMng_c::line1_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    // No origin temp: target passes p1 itself as the 7th (origin) argument --
    // confirmed by `mr r7, r4` in the disassembly with no stfs pair at all,
    // and by the 0x10-byte frame (no stack slot for a synthesized mVec2_c).
    bool result = fn_800C1EE0(this, -1.0f, 16.0f, p1, p2, p3, p1);
    if (result) {
        mStateMgr.changeState(StateID_Right45);
    }
    return result;
}

bool dLineMng_c::line3h_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = width_cross_chk(p1, p2, p3);
    if (result) {
        mStateMgr.changeState(StateID_CornerSideLine);
    }
    return result;
}

bool dLineMng_c::line3v_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = height_cross_chk(p1, p2, p3);
    if (result) {
        mStateMgr.changeState(StateID_CornerHeightLine);
    }
    return result;
}

bool dLineMng_c::line4_cross_chk(const mVec2_c &p1, const mVec2_c &p2, const mVec2_c &p3) {
    bool result = width_cross_chk(p1, p2, p3);
    if (result) {
        mStateMgr.changeState(StateID_Side);
    }
    return result;
}

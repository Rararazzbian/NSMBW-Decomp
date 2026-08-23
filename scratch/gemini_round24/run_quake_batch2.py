import sys, os
sys.path.append(".")
import scratch.gemini_round24.sweep_quake_variants_r32 as sq

variants = []

def add_var(name, code_snippet):
    prefix = """
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
"""
    suffix = """
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
"""
    body = prefix + code_snippet + suffix
    variants.append((name, body))

# 13. in_cond_decl_ternary
add_var("in_cond_decl_ternary", """
    if (fBase_c *base = (mUnk770 == 0 ? (fBase_c*)(mUnk770 >> 31) : fManager_c::searchBaseByID((fBaseID_e)mUnk770))) {
        base->deleteRequest();
    }
""")

# 14. inverted_shift_31
add_var("inverted_shift_31", """
    fBase_c *base = (mUnk770 != 0) ? fManager_c::searchBaseByID((fBaseID_e)mUnk770) : (fBase_c*)(mUnk770 >> 31);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 15. claude_q5_null_then_if
add_var("claude_q5_null_then_if", """
    fBase_c *base = nullptr;
    if (mUnk770 != 0) {
        base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    }
    if (base != nullptr) {
        base->deleteRequest();
    }
""")

# 16. if_else_assign_0
add_var("if_else_assign_0", """
    fBase_c *base;
    if (mUnk770 != 0) {
        base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    } else {
        base = (fBase_c*)(mUnk770 >> 31);
    }
    if (base != nullptr) {
        base->deleteRequest();
    }
""")

# 17. guard_if (no ternary)
add_var("guard_if", """
    if (mUnk770 != 0) {
        fBase_c *base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);
        if (base != nullptr) {
            base->deleteRequest();
        }
    }
""")

# 18. searchBaseByID_direct
add_var("searchBaseByID_direct", """
    fBase_c *base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
""")

# 19. id_var_with_shift
add_var("id_var_with_shift", """
    fBaseID_e id = (fBaseID_e)mUnk770;
    fBase_c *base = (id == 0) ? (fBase_c*)(id >> 31) : fManager_c::searchBaseByID(id);
    if (base != nullptr) {
        base->deleteRequest();
    }
""")

# 20. cast_to_u16_shift
add_var("cast_to_u16_shift", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(u32)(u16)mUnk770 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
""")

print("Testing", len(variants), "variants...")
for name, body in variants:
    res = sq.test_variant(name, body)
    if res and res["ok"]:
        print(f"{name:25s} | words: {res['words']:3s} | frame: {res['frame']:4s} | gpr: {res['gpr']:14s} | li r3,0: {str(res['has_li_r3_0']):5s} | diffs: {res['diffs']}")
    else:
        print(f"{name:25s} | FAILED")

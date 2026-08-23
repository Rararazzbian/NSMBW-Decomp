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

# 1. void* null
add_var("void_ptr_null", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(void*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 2. unrelated class null (dActor_c*)
add_var("dActor_null", """
    dActor_c *base = (mUnk770 == 0) ? (dActor_c*)0 : (dActor_c*)fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 3. int* null
add_var("int_ptr_null", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(int*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 4. (fBase_c*)(u32)0
add_var("u32_zero", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(u32)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 5. (fBase_c*)false
add_var("bool_false", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)false : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 6. (mUnk770 - mUnk770)
add_var("sub_self", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 - mUnk770) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 7. (mUnk770 ^ mUnk770)
add_var("xor_self", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 ^ mUnk770) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 8. (mUnk770 & 0)
add_var("and_zero", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 & 0) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 9. (mUnk770 * 0)
add_var("mul_zero", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 * 0) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 10. (u32)mUnk792
add_var("member_792", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(u32)mUnk792 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 11. inlined ternary arg
add_var("inlined_ternary_arg", """
    fBase_c *base = fManager_c::searchBaseByID((fBaseID_e)(mUnk770 == 0 ? 0 : mUnk770));
    if (base != nullptr) { base->deleteRequest(); }
""")

# 12. shift_31
add_var("shift_31", """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 >> 31) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

print("Testing", len(variants), "variants...")
for name, body in variants:
    res = sq.test_variant(name, body)
    if res and res["ok"]:
        print(f"{name:20s} | words: {res['words']:3s} | frame: {res['frame']:4s} | gpr: {res['gpr']:14s} | li r3,0: {str(res['has_li_r3_0']):5s} | diffs: {res['diffs']}")
    else:
        print(f"{name:20s} | FAILED")

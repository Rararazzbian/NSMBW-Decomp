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
"""
    suffix = """
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
"""
    body = prefix + code_snippet + suffix
    variants.append((name, body))

# 21. store after UnKnownScoreSet
add_var("store_after_score", """
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    mUnk792 = 0;
    mUnk790 = 0;
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 22. store before release
add_var("store_before_release", """
    mUnk792 = 0;
    mUnk790 = 0;
    mCc.release();
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 23. uintptr_t null
add_var("uintptr_null", """
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(unsigned long)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 24. float 0 cast to ptr
add_var("float_0_cast", """
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(int)(float)0.0f : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

# 25. volatile_read_zero
add_var("volatile_read_zero", """
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)*(u32*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) { base->deleteRequest(); }
""")

print("Testing", len(variants), "variants...")
for name, body in variants:
    res = sq.test_variant(name, body)
    if res and res["ok"]:
        print(f"{name:25s} | words: {res['words']:3s} | frame: {res['frame']:4s} | gpr: {res['gpr']:14s} | li r3,0: {str(res['has_li_r3_0']):5s} | diffs: {res['diffs']}")
    else:
        print(f"{name:25s} | FAILED")

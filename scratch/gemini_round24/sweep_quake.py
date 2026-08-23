import sys, re
sys.path.append('.')
from scratch.gemini_round24 import tool

CPP_PATH = 'scratch/gemini_round24/d_enemy_toride_kokoopa.cpp'
with open(CPP_PATH, 'r', encoding='utf-8') as f:
    orig_code = f.read()

target_all = {}
for tf in tool.TARGET_FILES:
    target_all.update(tool.parse_disasm(tf))
t_fn = target_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')

vars = []

# Var 1: l_dieQuake direct assignment
b1 = """void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}
"""
vars.append(("v1_deathData_l_dieQuake", b1))

# Var 2: struct literal
b2 = """void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}
"""
vars.append(("v2_struct_literal", b2))

# Var 3: searchBaseByID inline without base variable
b3 = """void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    mDeathInfo = l_dieQuake;
    mDeathInfo.mDirection = dir;
}
"""
vars.append(("v3_direct_mDeathInfo", b3))

# Var 4: searchBaseByID ternary call
b4 = """void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    if (fBase_c *base = (mUnk770 != 0) ? fManager_c::searchBaseByID((fBaseID_e)mUnk770) : nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}
"""
vars.append(("v4_if_neq0", b4))

# Var 5: (mUnk770 == 0) ternary
b5 = """void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    if (fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770)) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}
"""
vars.append(("v5_if_eq0", b5))

prefix = orig_code[:orig_code.find('void dEnTorideKokoopa_c::setQuakeDead()')]
suffix = orig_code[orig_code.find('void dEnTorideKokoopa_c::setShellDamage'):]

for name, var in vars:
    new_code = prefix + var + suffix
    with open(CPP_PATH, 'w', encoding='utf-8') as f:
        f.write(new_code)
    if not tool.compile_and_disasm():
        print(f"{name}: Compile error")
        continue
    draft_all = tool.parse_disasm(tool.DRAFT_DIS)
    d_fn = draft_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')
    if d_fn:
        diffs = sum(1 for (tb, tt), (db, dt) in zip(t_fn, d_fn) if tb != db and tt != dt) + abs(len(t_fn) - len(d_fn))
        raw_diffs = sum(1 for (tb, _), (db, _) in zip(t_fn, d_fn) if tb != db) + abs(len(t_fn) - len(d_fn))
        print(f"{name}: len T={len(t_fn)} D={len(d_fn)}, {raw_diffs} raw diffs, {diffs} canonical diffs")
        if raw_diffs == 0:
            print(f"*** {name} MATCHES 100%! ***")
            break

# restore
with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(orig_code)
tool.compile_and_disasm()

import sys, itertools
sys.path.append('.')
from scratch.gemini_round24 import tool

CPP_PATH = 'scratch/gemini_round24/d_enemy_toride_kokoopa.cpp'
with open(CPP_PATH, 'r', encoding='utf-8') as f:
    orig_code = f.read()

target_all = {}
for tf in tool.TARGET_FILES:
    target_all.update(tool.parse_disasm(tf))
t_fn = target_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')

prefix = orig_code[:orig_code.find('void dEnTorideKokoopa_c::setQuakeDead()')]
suffix = orig_code[orig_code.find('void dEnTorideKokoopa_c::setShellDamage'):]

mat_variants = [
    "if (mAnmMatClr.mpChildren[1].getObj() != nullptr) { mAnmMatClr.setFrame(0.0f, 1); }",
    "if (*(u32*)((u8*)mAnmMatClr.mpChildren + 0x3C) != 0) { mAnmMatClr.setFrame(0.0f, 1); }",
]

zero_variants = [
    "mUnk792 = 0;\nmUnk790 = 0;",
    "mUnk790 = 0;\nmUnk792 = 0;",
    "mUnk790 = mUnk792 = 0;",
    "mUnk792 = mUnk790 = 0;",
]

base_variants = [
    "fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);\nif (base != nullptr) { base->deleteRequest(); }",
    "fBase_c *base = (mUnk770 != 0) ? fManager_c::searchBaseByID((fBaseID_e)mUnk770) : nullptr;\nif (base != nullptr) { base->deleteRequest(); }",
    "if (fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770)) { base->deleteRequest(); }",
    "if (fBase_c *base = (mUnk770 != 0) ? fManager_c::searchBaseByID((fBaseID_e)mUnk770) : nullptr) { base->deleteRequest(); }",
    "fBase_c *base;\nif (mUnk770 == 0) base = nullptr;\nelse base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);\nif (base != nullptr) { base->deleteRequest(); }",
    "fBase_c *base = nullptr;\nif (mUnk770 != 0) base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);\nif (base != nullptr) { base->deleteRequest(); }",
]

death_variants = [
    "sDeathInfoData deathData = l_dieQuake;\ndeathData.mDirection = dir;\nmDeathInfo = deathData;",
    "sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };\ndeathData.mDirection = dir;\nmDeathInfo = deathData;",
    "sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, dir, 0xFF };\nmDeathInfo = deathData;",
    "mDeathInfo = l_dieQuake;\nmDeathInfo.mDirection = dir;",
]

count = 0
for mv, zv, bv, dv in itertools.product(mat_variants, zero_variants, base_variants, death_variants):
    count += 1
    fn_body = f"""void dEnTorideKokoopa_c::setQuakeDead() {{
    u8 dir = getPl_LRflag(mPos);
    {mv}
    removeCc();
    mCc.release();
    {zv}
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    {bv}
    mActorProperties &= ~8;
    {dv}
}}
"""
    new_code = prefix + fn_body + suffix
    with open(CPP_PATH, 'w', encoding='utf-8') as f:
        f.write(new_code)
    if not tool.compile_and_disasm():
        continue
    draft_all = tool.parse_disasm(tool.DRAFT_DIS)
    d_fn = draft_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')
    if d_fn:
        raw_diffs = sum(1 for (tb, _), (db, _) in zip(t_fn, d_fn) if tb != db) + abs(len(t_fn) - len(d_fn))
        diffs = sum(1 for (tb, tt), (db, dt) in zip(t_fn, d_fn) if tb != db and tt != dt) + abs(len(t_fn) - len(d_fn))
        if len(t_fn) == len(d_fn) or raw_diffs < 20:
            print(f"[{count}] T={len(t_fn)} D={len(d_fn)}, raw={raw_diffs}, can={diffs}")
        if raw_diffs == 0:
            print(f"*** FOUND 100% MATCH at combo {count}! ***")
            print(fn_body)
            break

# restore
with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(orig_code)
tool.compile_and_disasm()

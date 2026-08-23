import sys
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

test_cases = [
    # 1: if with declaration
    """if (fBase_c *base = (mUnk770 == 0) ? (fBase_c*)nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770)) {
        base->deleteRequest();
    }""",

    # 2: block scope
    """{
        fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
        if (base != nullptr) {
            base->deleteRequest();
        }
    }""",

    # 3: base = 0 at end
    """fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    base = nullptr;""",

    # 4: dActor_c cast
    """dActor_c *base = (mUnk770 == 0) ? (dActor_c*)0 : (dActor_c*)fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }""",

    # 5: fBaseID_e cast in ternary condition
    """fBase_c *base = (mUnk770 == 0) ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }""",

    # 6: volatile base
    """fBase_c *base = (mUnk770 == 0) ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base) {
        base->deleteRequest();
    }""",

    # 7: ternary assignment in if condition
    """fBase_c *base;
    if ((base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770)) != nullptr) {
        base->deleteRequest();
    }""",

    # 8: pointer to base actor
    """dBaseActor_c *base = (mUnk770 == 0) ? (dBaseActor_c*)0 : (dBaseActor_c*)fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }""",
]

for i, tc in enumerate(test_cases):
    fn_body = f"""void dEnTorideKokoopa_c::setQuakeDead() {{
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {{
        mAnmMatClr.setFrame(0.0f, 1);
    }}
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    {tc}
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}}
"""
    new_code = prefix + fn_body + suffix
    with open(CPP_PATH, 'w', encoding='utf-8') as f:
        f.write(new_code)
    if not tool.compile_and_disasm():
        print(f"Case {i+1}: Compile error")
        continue
    draft_all = tool.parse_disasm(tool.DRAFT_DIS)
    d_fn = draft_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')
    if d_fn:
        raw_diffs = sum(1 for (tb, _), (db, _) in zip(t_fn, d_fn) if tb != db) + abs(len(t_fn) - len(d_fn))
        diffs = sum(1 for (tb, tt), (db, dt) in zip(t_fn, d_fn) if tb != db and tt != dt) + abs(len(t_fn) - len(d_fn))
        print(f"Case {i+1}: T={len(t_fn)} D={len(d_fn)}, raw={raw_diffs}, can={diffs}")
        if raw_diffs == 0:
            print(f"*** Case {i+1} MATCHES 100%! ***")
            break

# restore
with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(orig_code)
tool.compile_and_disasm()

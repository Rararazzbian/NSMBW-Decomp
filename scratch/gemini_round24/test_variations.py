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

prefix = orig_code[:orig_code.find('void dEnTorideKokoopa_c::setQuakeDead()')]
suffix = orig_code[orig_code.find('void dEnTorideKokoopa_c::setShellDamage'):]

variations = [
    # V1: inline ternary in function arg
    '''    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();''',

    # V2: volatile cast
    '''    fBase_c *volatile base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();''',

    # V3: register keyword
    '''    register fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();''',

    # V4: ternary on searchBaseByID call
    '''    fBase_c *base = (fBase_c*)((mUnk770 == 0) ? 0 : (void*)fManager_c::searchBaseByID((fBaseID_e)mUnk770));
    if (base != 0) base->deleteRequest();''',

    # V5: ternary with integer
    '''    u32 base = (mUnk770 == 0) ? 0 : (u32)fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) ((fBase_c*)base)->deleteRequest();''',

    # V6: dActor_c cast
    '''    dActor_c *act = (mUnk770 == 0) ? 0 : (dActor_c*)fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (act != 0) act->deleteRequest();''',

    # V7: if with assignment
    '''    fBase_c *base;
    if ((base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770)) != 0)
        base->deleteRequest();''',

    # V8: ternary in member call
    '''    fBase_c *base = (mUnk770 == 0) ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base) { ((fBase_c*)base)->deleteRequest(); }''',

    # V9: inline ternary
    '''    if (mUnk770 == 0 ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770)) {
        ((fBase_c*)(mUnk770 == 0 ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770)))->deleteRequest();
    }''',

    # V10: fBase_c* ternary in if
    '''    if (fBase_c *b = (mUnk770 == 0 ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770))) {
        b->deleteRequest();
    }''',

    # V11: ternary with enum cast
    '''    fBase_c *base = (mUnk770 == 0) ? 0 : fManager_c::searchBaseByID(fBaseID_e(mUnk770));
    if (base != 0) base->deleteRequest();''',

    # V12: comma inside if
    '''    fBase_c *base;
    if (base = (mUnk770 == 0 ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770)))
        base->deleteRequest();''',
]

for idx, var in enumerate(variations):
    fn_body = f'''void dEnTorideKokoopa_c::setQuakeDead() {{
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {{
        mAnmMatClr.setFrame(0.0f, 1);
    }}
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
{var}
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}}
'''
    new_code = prefix + fn_body + suffix
    with open(CPP_PATH, 'w', encoding='utf-8') as f:
        f.write(new_code)
    if not tool.compile_and_disasm():
        print(f"V{idx+1}: Compile error")
        continue
    draft_all = tool.parse_disasm(tool.DRAFT_DIS)
    d_fn = draft_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')
    if d_fn:
        raw_diffs = sum(1 for (tb, _), (db, _) in zip(t_fn, d_fn) if tb != db) + abs(len(t_fn) - len(d_fn))
        diffs = sum(1 for (tb, tt), (db, dt) in zip(t_fn, d_fn) if tb != db and tt != dt) + abs(len(t_fn) - len(d_fn))
        print(f"V{idx+1}: T={len(t_fn)} D={len(d_fn)}, raw={raw_diffs}, can={diffs}")
        if raw_diffs == 0:
            print(f"*** V{idx+1} MATCHES 100%! ***")
            break

# restore
with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(orig_code)
tool.compile_and_disasm()

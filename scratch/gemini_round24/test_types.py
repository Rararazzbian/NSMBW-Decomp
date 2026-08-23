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

type_tests = [
    # T1: dir as int
    '''    int dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;''',

    # T2: dir as u32
    '''    u32 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;''',

    # T3: searchBaseByID taking mUnk770 directly (if mUnk770 is fBaseID_e)
    '''    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID(mUnk770);
    if (base != 0) base->deleteRequest();
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;''',

    # T4: inline ternary in deleteRequest
    '''    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? 0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base) { base->deleteRequest(); }
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;''',
]

for idx, tt in enumerate(type_tests):
    fn_code = f"void dEnTorideKokoopa_c::setQuakeDead() {{\n{tt}\n}}\n"
    new_code = prefix + fn_code + suffix
    with open(CPP_PATH, 'w', encoding='utf-8') as f:
        f.write(new_code)
    if not tool.compile_and_disasm():
        print(f"TT{idx+1}: Compile error")
        continue
    draft_all = tool.parse_disasm(tool.DRAFT_DIS)
    d_fn = draft_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')
    if d_fn:
        raw_diffs = sum(1 for (tb, _), (db, _) in zip(t_fn, d_fn) if tb != db) + abs(len(t_fn) - len(d_fn))
        diffs = sum(1 for (tb, tt), (db, dt) in zip(t_fn, d_fn) if tb != db and tt != dt) + abs(len(t_fn) - len(d_fn))
        print(f"TT{idx+1}: T={len(t_fn)} D={len(d_fn)}, raw={raw_diffs}, can={diffs}")
        if raw_diffs == 0:
            print(f"*** TT{idx+1} MATCHES 100%! ***")
            break

# restore
with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(orig_code)
tool.compile_and_disasm()

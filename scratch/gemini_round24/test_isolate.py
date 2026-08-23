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

# Let's test removing each statement one by one to see when r29 disappears
tests = [
    ("No dir", '''
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = 0;
    mDeathInfo = deathData;
    '''),

    ("No anmMatClr", '''
    u8 dir = getPl_LRflag(mPos);
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
    '''),

    ("No deathData", '''
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();
    mActorProperties &= ~8;
    '''),

    ("No UnKnownScoreSet", '''
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
    '''),
]

for label, body in tests:
    fn_code = f"void dEnTorideKokoopa_c::setQuakeDead() {{\n{body}\n}}\n"
    new_code = prefix + fn_code + suffix
    with open(CPP_PATH, 'w', encoding='utf-8') as f:
        f.write(new_code)
    tool.compile_and_disasm()
    draft_all = tool.parse_disasm(tool.DRAFT_DIS)
    d_fn = draft_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')
    print(f"=== {label} ===")
    print(f"Draft len={len(d_fn) if d_fn else 0}")
    if d_fn:
        # Check if r29 is in prologue
        prologue_r29 = any('r29' in t for b, t in d_fn[:10])
        print(f"r29 in prologue: {prologue_r29}")

# restore
with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(orig_code)
tool.compile_and_disasm()

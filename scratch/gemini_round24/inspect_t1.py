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

fn_body = """void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *b = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (b) b->deleteRequest();
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}
"""

new_code = prefix + fn_body + suffix
with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(new_code)
tool.compile_and_disasm()
draft_all = tool.parse_disasm(tool.DRAFT_DIS)
d_fn = draft_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')

print(f"Target: {len(t_fn)} insns, Draft: {len(d_fn)} insns")
for i in range(max(len(t_fn), len(d_fn))):
    tb = t_fn[i][0] if i < len(t_fn) else '        '
    tt = t_fn[i][1] if i < len(t_fn) else ''
    db = d_fn[i][0] if i < len(d_fn) else '        '
    dt = d_fn[i][1] if i < len(d_fn) else ''
    eq = '==' if tb == db else '!='
    print(f"{i:2d} {eq} T: [{tb}] {tt:45s} | D: [{db}] {dt}")

with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(orig_code)
tool.compile_and_disasm()

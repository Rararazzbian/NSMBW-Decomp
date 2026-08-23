import sys, re
sys.path.append('.')
from scratch.gemini_round24 import tool
from tools.auto_decomp import fndiff

CPP_PATH = 'scratch/gemini_round24/d_enemy_toride_kokoopa.cpp'
with open(CPP_PATH, 'r', encoding='utf-8') as f:
    orig_code = f.read()

target_all = {}
for tf in tool.TARGET_FILES:
    target_all.update(tool.parse_disasm(tf))
t_fn = target_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')

prefix = orig_code[:orig_code.find('void dEnTorideKokoopa_c::setQuakeDead()')]
suffix = orig_code[orig_code.find('void dEnTorideKokoopa_c::setShellDamage'):]

def check_ternary_shape(body):
    has_li_r3_0 = any('li r3, 0' in insn or 'li r3, 0x0' in insn for insn in body)
    has_b_after_li = False
    has_shared_cmpwi = False
    for i, insn in enumerate(body):
        if ('li r3, 0' in insn or 'li r3, 0x0' in insn) and i + 1 < len(body):
            if body[i+1].startswith('b '):
                has_b_after_li = True
        if 'cmpwi r3, 0' in insn or 'cmpwi r3, 0x0' in insn:
            has_shared_cmpwi = True
    return has_li_r3_0 and has_b_after_li and has_shared_cmpwi

score_call = 'dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);'

test_chunks = [
    ("T1: (fBase_c*)mUnk770 in false arm", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)mUnk770 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }
"""),

    ("T2: (fBase_c*)(mUnk770 & 0)", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 & 0) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }
"""),

    ("T3: (fBase_c*)((u32)0)", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)((u32)0) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }
"""),

    ("T4: (fBase_c*)((int)0)", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)((int)0) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }
"""),

    ("T5: (fBase_c*)(0L)", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(0L) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }
"""),

    ("T6: (fBase_c*)(0LL)", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(0LL) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }
"""),

    ("T7: (fBase_c*)(void*)0", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(void*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }
"""),

    ("T8: ((fBase_c*)0) without cast in searchBaseByID", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID(mUnk770);
    if (base != 0) {
        base->deleteRequest();
    }
"""),

    ("T9: if (!mUnk770) { base = 0; } else { base = search... }", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base;
    if (!mUnk770) {
        base = (fBase_c*)0;
    } else {
        base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    }
    if (base != 0) {
        base->deleteRequest();
    }
"""),

    ("T10: if (mUnk770) { base = search... } else { base = 0; }", """
    mUnk792 = 0;
    mUnk790 = 0;
    """ + score_call + """
    fBase_c *base;
    if (mUnk770) {
        base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    } else {
        base = (fBase_c*)0;
    }
    if (base != 0) {
        base->deleteRequest();
    }
"""),
]

print(f"{'Variant':<55} | {'Words':<5} | {'Frame':<6} | {'GPR Saves':<14} | {'Ternary':<7} | {'Diffs':<5}")
print("-" * 105)

for name, chunk in test_chunks:
    fn_body = """void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    """ + chunk + """
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}
"""
    new_code = prefix + fn_body + suffix
    with open(CPP_PATH, 'w', encoding='utf-8') as f:
        f.write(new_code)
    ok = tool.compile_and_disasm()
    if not ok:
        print(f"{name:<55} | COMPILE ERROR")
        continue
    draft_all = tool.parse_disasm(tool.DRAFT_DIS)
    d_fn = draft_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')
    if not d_fn:
        print(f"{name:<55} | NOT EMITTED")
        continue
    
    d_insns = [t for _, t in d_fn]
    gpr, fpr, helper = fndiff.saves(d_insns)
    fs = fndiff.frame_size(d_insns)
    t_shape = check_ternary_shape(d_insns)
    
    t_can = fndiff.canonicalise([t for _, t in t_fn])
    d_can = fndiff.canonicalise(d_insns)
    diffs = 0
    for a, b in zip(t_can, d_can):
        if a != b and not fndiff.artifact_pair(a, b):
            diffs += 1
    diffs += abs(len(d_insns) - len(t_fn))
    
    fs_str = f"0x{fs:X}" if fs is not None else "none"
    gpr_str = str(gpr) if gpr else "none"
    t_shape_str = "YES" if t_shape else "NO"
    
    print(f"{name:<55} | {len(d_insns):<5} | {fs_str:<6} | {gpr_str:<14} | {t_shape_str:<7} | {diffs:<5}")

# restore
with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(orig_code)
tool.compile_and_disasm()


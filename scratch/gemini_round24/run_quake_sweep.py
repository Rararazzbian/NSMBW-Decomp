import sys, os, re, subprocess
sys.path.append('.')
from tools.auto_decomp import harness
from scratch.gemini_round24.tool import parse_disasm, EXTRA_INC

BASE_SRC = 'scratch/gemini_round24/d_enemy_toride_kokoopa.cpp'
TEMP_SRC = 'scratch/gemini_round24/test_quake_temp.cpp'
TEMP_OBJ = 'scratch/gemini_round24/test_quake_temp.o'
TEMP_DIS = 'scratch/gemini_round24/test_quake_temp.txt'
TARGET_ALL = 'scratch/gemini_round24/target_all_text.txt'

with open(BASE_SRC, 'r', encoding='utf-8') as fh:
    base_code = fh.read()

def test_quake_var(name, quake_body):
    pat = r'(void dEnTorideKokoopa_c::setQuakeDead\(\)\s*\{)(.*?)(\n\}\s*\nvoid dEnTorideKokoopa_c::setShellDamage)'
    m = re.search(pat, base_code, re.DOTALL)
    if not m:
        print('Error finding setQuakeDead')
        return None
    code = base_code[:m.start(2)] + '\n' + quake_body + base_code[m.end(2):]
    
    with open(TEMP_SRC, 'w', encoding='utf-8') as fh:
        fh.write(code)
        
    ok, err = harness.compile_draft(TEMP_SRC, TEMP_OBJ, extra_inc=EXTRA_INC, module='wiimj2d')
    if not ok:
        print(f'[{name}] COMPILE ERROR:\n{err}')
        return None
    ok_d, err_d = harness.disasm(TEMP_OBJ, TEMP_DIS)
    if not ok_d:
        print(f'[{name}] DISASM ERROR:\n{err_d}')
        return None
        
    cmd = [sys.executable, 'tools/auto_decomp/fndiff.py', TARGET_ALL, TEMP_DIS, 'setQuakeDead__18dEnTorideKokoopa_cFv', '-v']
    proc = subprocess.run(cmd, capture_output=True, text=True)
    out = proc.stdout
    
    d_all = parse_disasm(TEMP_DIS)
    d_insns = d_all.get('setQuakeDead__18dEnTorideKokoopa_cFv', [])
    
    words = len(d_insns)
    frame = 'unknown'
    gprs = []
    has_li_b_cmp = False
    
    for idx, (b, t) in enumerate(d_insns):
        if idx == 0 and 'stwu' in t:
            frame_m = re.search(r'-0x([0-9a-fA-F]+)', t)
            if frame_m:
                frame = '0x' + frame_m.group(1).lower()
        if 'stw r' in t and idx < 10:
            reg_m = re.search(r'stw\s+r(\d+)', t)
            if reg_m and int(reg_m.group(1)) >= 13:
                gprs.append(int(reg_m.group(1)))
        if 'li r3, 0' in t:
            if idx + 1 < len(d_insns) and d_insns[idx+1][1].startswith('b '):
                has_li_b_cmp = True
                
    gprs_sorted = sorted(list(set(gprs)))
    gprs_str = '[' + ', '.join(str(r) for r in gprs_sorted) + ']' if gprs_sorted else 'none'
    
    diff_line = [l for l in out.splitlines() if 'DIFFS' in l or 'MATCH' in l or 'IDENTICAL' in l]
    print(f'=== Variant: {name} ===')
    print(f'  Words: {words}, Frame: {frame}, GPR saves: {gprs_str}, li r3,0 + b -> shared cmp: {has_li_b_cmp}')
    print(f'  Result: {diff_line[0] if diff_line else "NO DIFF LINE"}')
    for idx, (b, t) in enumerate(d_insns):
        if any(k in t for k in ['searchBaseByID', 'deleteRequest', '0x770', 'cmpwi r3']):
            print(f'    {idx:2d}: {t}')
    print()

def make_body(ternary_code):
    return """    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
""" + ternary_code + """
    mActorProperties &= ~8;
    sDeathInfoData deathData = l_dieQuake;
    deathData.mDirection = dir;
    mDeathInfo = deathData;"""

variants = [
    ("QV1: Different pointer type null (dActor_c*)0",
     make_body("""    fBase_c *base = (mUnk770 == 0) ? (dActor_c*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("QV2: Different pointer type null (void*)0",
     make_body("""    fBase_c *base = (mUnk770 == 0) ? (void*)0 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("QV3: Condition tested on local id, passed mUnk770",
     make_body("""    u32 id = mUnk770;
    fBase_c *base = (id == 0) ? (fBase_c*)mUnk770 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("QV4: Condition tested on mUnk770, passed local id",
     make_body("""    fBaseID_e id = (fBaseID_e)mUnk770;
    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)0 : fManager_c::searchBaseByID(id);
    if (base != 0) base->deleteRequest();""")),

    ("QV5: Tested on (id != 0) with (fBase_c*)mUnk770 in false arm",
     make_body("""    fBase_c *base = (mUnk770 != 0) ? fManager_c::searchBaseByID((fBaseID_e)mUnk770) : (fBase_c*)mUnk770;
    if (base != 0) base->deleteRequest();""")),

    ("QV6: Null via pointer variable",
     make_body("""    fBase_c *nullPtr = (fBase_c*)0;
    fBase_c *base = (mUnk770 == 0) ? nullPtr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("QV7: Passed through separate local pointer",
     make_body("""    fBase_c *base = (fBase_c*)mUnk770;
    if (mUnk770 != 0) base = fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("QV8: Condition on (fBaseID_e)mUnk770 == (fBaseID_e)0 with (fBase_c*)mUnk770",
     make_body("""    fBase_c *base = ((fBaseID_e)mUnk770 == (fBaseID_e)0) ? (fBase_c*)mUnk770 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),
]

for name, body in variants:
    test_quake_var(name, body)

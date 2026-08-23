import sys, os, re, subprocess
sys.path.append('.')
from tools.auto_decomp import harness
from scratch.gemini_round24.tool import parse_disasm, EXTRA_INC

BASE_SRC = 'scratch/gemini_round24/d_enemy_toride_kokoopa.cpp'
TEMP_SRC = 'scratch/gemini_round24/test_jump_temp.cpp'
TEMP_OBJ = 'scratch/gemini_round24/test_jump_temp.o'
TEMP_DIS = 'scratch/gemini_round24/test_jump_temp.txt'
TARGET_ALL = 'scratch/gemini_round24/target_all_text.txt'

with open(BASE_SRC, 'r', encoding='utf-8') as fh:
    base_code = fh.read()

def test_variant(name, jump_body, bigjump_body=None):
    if bigjump_body is None:
        bigjump_body = jump_body.replace('mJumpSpeed1', 'mBigJumpSpeed1').replace('mJumpSpeed2', 'mBigJumpSpeed2').replace('mAnmNames[1]', 'mAnmNames[3]')
    
    pat_jump = r'(void dEnTorideKokoopa_c::initializeState_Jump\(\)\s*\{)(.*?)(\n\}\s*\nvoid dEnTorideKokoopa_c::finalizeState_Jump)'
    m = re.search(pat_jump, base_code, re.DOTALL)
    if not m:
        print('Error finding initializeState_Jump')
        return None
    code = base_code[:m.start(2)] + '\n' + jump_body + base_code[m.end(2):]
    
    pat_bigjump = r'(void dEnTorideKokoopa_c::initializeState_BigJump\(\)\s*\{)(.*?)(\n\}\s*\nvoid dEnTorideKokoopa_c::finalizeState_BigJump)'
    m2 = re.search(pat_bigjump, code, re.DOTALL)
    if not m2:
        print('Error finding initializeState_BigJump')
        return None
    code = code[:m2.start(2)] + '\n' + bigjump_body + code[m2.end(2):]
    
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
        
    res = {}
    for fn in ['initializeState_Jump__18dEnTorideKokoopa_cFv', 'initializeState_BigJump__18dEnTorideKokoopa_cFv']:
        cmd = [sys.executable, 'tools/auto_decomp/fndiff.py', TARGET_ALL, TEMP_DIS, fn, '-v']
        proc = subprocess.run(cmd, capture_output=True, text=True)
        res[fn] = proc.stdout
        
    d_all = parse_disasm(TEMP_DIS)
    d_insns = d_all.get('initializeState_Jump__18dEnTorideKokoopa_cFv', [])
    
    print(f'=== Variant: {name} ===')
    for fn in ['initializeState_Jump__18dEnTorideKokoopa_cFv', 'initializeState_BigJump__18dEnTorideKokoopa_cFv']:
        short_fn = 'Jump' if 'Big' not in fn else 'BigJump'
        out = res[fn]
        diff_line = [l for l in out.splitlines() if 'DIFFS' in l or 'MATCH' in l or 'IDENTICAL' in l]
        print(f'  {short_fn}: {diff_line[0] if diff_line else out.strip()}')
        for l in out.splitlines():
            if 'T:' in l and 'naming artifact' not in l:
                print(f'    {l}')
                
    print('  Draft insns around calcJumpRate:')
    in_calc = False
    for b, t in d_insns:
        if 'calcJumpRate' in t:
            in_calc = True
        if in_calc:
            print(f'    {t}')
            if 'stfs' in t and ('0xe8' in t or 'mSpeed' in t):
                in_calc = False
    print()
    return res

if __name__ == '__main__':
    v5 = """    const char *anmName = mpParamJump->mAnmNames[1];
    if (anmName != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmName);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)1);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 0.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }
    int flag = 1;
    if (mpBossLife->isNonDamage() == 0 && mpBossLife->isOneDamage() == 0) {
        flag = 0;
    }
    mVec2_c speed;
    if (flag != 0) {
        speed = mpParamJump->mJumpSpeed1;
    } else {
        speed = mpParamJump->mJumpSpeed2;
    }
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();"""
    test_variant('5-diff starting form', v5)

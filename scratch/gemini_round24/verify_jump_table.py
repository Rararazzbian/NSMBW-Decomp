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

def test_full_jump(name, jump_body, bigjump_body):
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
    
    print(f'=== {name} ===')
    for fn in ['initializeState_Jump__18dEnTorideKokoopa_cFv', 'initializeState_BigJump__18dEnTorideKokoopa_cFv']:
        short_fn = 'Jump' if 'Big' not in fn else 'BigJump'
        out = res[fn]
        diff_line = [l for l in out.splitlines() if 'DIFFS' in l or 'MATCH' in l or 'IDENTICAL' in l]
        print(f'  {short_fn}: {diff_line[0] if diff_line else out.strip()}')
        for l in out.splitlines():
            if 'T:' in l and 'naming artifact' not in l:
                print(f'    {l}')
                
    print('  Disasm (Jump tail):')
    in_calc = False
    for b, t in d_insns:
        if 'calcJumpRate' in t:
            in_calc = True
        if in_calc:
            print(f'    {t}')
            if 'stfs' in t and ('0xe8' in t or 'mSpeed' in t):
                in_calc = False
    print()

def make_jump(anm_idx, sp1, sp2, tail_code):
    return f"""    const char *anmName = mpParamJump->mAnmNames[{anm_idx}];
    if (anmName != nullptr) {{
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmName);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)1);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 0.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }}
    int flag = 1;
    if (mpBossLife->isNonDamage() == 0 && mpBossLife->isOneDamage() == 0) {{
        flag = 0;
    }}
{tail_code}
    jumpEffect();
    jumpSE();"""

v1_tail = """    mVec2_c speed;
    if (flag != 0) {
        speed.x = mpParamJump->mJumpSpeed1.x;
        speed.y = mpParamJump->mJumpSpeed1.y;
    } else {
        speed.x = mpParamJump->mJumpSpeed2.x;
        speed.y = mpParamJump->mJumpSpeed2.y;
    }
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;"""

v1_bj_tail = v1_tail.replace('mJumpSpeed1', 'mBigJumpSpeed1').replace('mJumpSpeed2', 'mBigJumpSpeed2')

test_full_jump("Variant 1 (Component-wise assignment in branches)",
               make_jump(1, 'mJumpSpeed1', 'mJumpSpeed2', v1_tail),
               make_jump(3, 'mBigJumpSpeed1', 'mBigJumpSpeed2', v1_bj_tail))

v2_tail = """    mVec2_c speed = (flag != 0) ? mpParamJump->mJumpSpeed1 : mpParamJump->mJumpSpeed2;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;"""
v2_bj_tail = v2_tail.replace('mJumpSpeed1', 'mBigJumpSpeed1').replace('mJumpSpeed2', 'mBigJumpSpeed2')

test_full_jump("Variant 2 (Ternary struct initialization)",
               make_jump(1, 'mJumpSpeed1', 'mJumpSpeed2', v2_tail),
               make_jump(3, 'mBigJumpSpeed1', 'mBigJumpSpeed2', v2_bj_tail))

v3_tail = """    const mVec2_c &speed = (flag != 0) ? mpParamJump->mJumpSpeed1 : mpParamJump->mJumpSpeed2;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;"""
v3_bj_tail = v3_tail.replace('mJumpSpeed1', 'mBigJumpSpeed1').replace('mJumpSpeed2', 'mBigJumpSpeed2')

test_full_jump("Variant 3 (Reference ternary binding)",
               make_jump(1, 'mJumpSpeed1', 'mJumpSpeed2', v3_tail),
               make_jump(3, 'mBigJumpSpeed1', 'mBigJumpSpeed2', v3_bj_tail))

v4_tail = """    mVec2_c speed;
    if (flag != 0) {
        speed = mpParamJump->mJumpSpeed1;
    } else {
        speed = mpParamJump->mJumpSpeed2;
    }
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    float sx = (mSpeed.y = speed.y, speed.x);
    mSpeed.x = (muki * rate) * sx;"""
v4_bj_tail = v4_tail.replace('mJumpSpeed1', 'mBigJumpSpeed1').replace('mJumpSpeed2', 'mBigJumpSpeed2')

test_full_jump("Variant 4 (Comma operator sequencing for stack read & store)",
               make_jump(1, 'mJumpSpeed1', 'mJumpSpeed2', v4_tail),
               make_jump(3, 'mBigJumpSpeed1', 'mBigJumpSpeed2', v4_bj_tail))

v5_tail = """    mVec2_c speed;
    if (flag != 0) {
        speed = mpParamJump->mJumpSpeed1;
    } else {
        speed = mpParamJump->mJumpSpeed2;
    }
    float rate = calcJumpRate();
    double muki = (double)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;"""
v5_bj_tail = v5_tail.replace('mJumpSpeed1', 'mBigJumpSpeed1').replace('mJumpSpeed2', 'mBigJumpSpeed2')

test_full_jump("Variant 5 (Double-precision cast on direction multiplier)",
               make_jump(1, 'mJumpSpeed1', 'mJumpSpeed2', v5_tail),
               make_jump(3, 'mBigJumpSpeed1', 'mBigJumpSpeed2', v5_bj_tail))

v6_tail = """    mVec2_c speed;
    if (flag != 0) {
        speed = mpParamJump->mJumpSpeed1;
    } else {
        speed = mpParamJump->mJumpSpeed2;
    }
    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = (speed.x * rate) * (float)l_EnMuki[mDirection];"""
v6_bj_tail = v6_tail.replace('mJumpSpeed1', 'mBigJumpSpeed1').replace('mJumpSpeed2', 'mBigJumpSpeed2')

test_full_jump("Variant 6 (Parenthesized product of speed.x and inlined direction cast)",
               make_jump(1, 'mJumpSpeed1', 'mJumpSpeed2', v6_tail),
               make_jump(3, 'mBigJumpSpeed1', 'mBigJumpSpeed2', v6_bj_tail))

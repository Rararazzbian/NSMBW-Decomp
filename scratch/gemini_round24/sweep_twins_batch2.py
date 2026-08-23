import os, sys, re, subprocess
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'gemini_round24')
INC = os.path.join(BASE, 'include')
GAME = os.path.join(BASE, 'game')
TGT = os.path.join(BASE, 'target_all_text.txt')

with open(os.path.join(BASE, 'd_enemy_toride_kokoopa.cpp'), 'r', encoding='utf-8') as f:
    base_code = f.read()

pat_jump = r'void dEnTorideKokoopa_c::initializeState_Jump\(\) \{.*?\n\}'
pat_bigjump = r'void dEnTorideKokoopa_c::initializeState_BigJump\(\) \{.*?\n\}'

orig_jump = re.search(pat_jump, base_code, re.DOTALL).group(0)
orig_bigjump = re.search(pat_bigjump, base_code, re.DOTALL).group(0)

tests = []

def make_fn(speed_assign_block, math_block, anm_idx, speed1_name, speed2_name):
    return f'''void dEnTorideKokoopa_c::initializeState_{'Jump' if anm_idx==1 else 'BigJump'}() {{
    const char *anmName = mpParamJump->mAnmNames[{anm_idx}];
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
    mVec2_c speed;
    if (flag != 0) {{
        speed = mpParamJump->{speed1_name};
    }} else {{
        speed = mpParamJump->{speed2_name};
    }}
    {math_block}
    jumpEffect();
    jumpSE();
}}'''

def add_math(name, math_block):
    j = make_fn('', math_block, 1, 'mJumpSpeed1', 'mJumpSpeed2')
    bj = make_fn('', math_block, 3, 'mBigJumpSpeed1', 'mBigJumpSpeed2')
    tests.append((name, j, bj))

# Math variations
add_math('M1_sx_first_prod', '''float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = sx * (muki * rate);''')

add_math('M2_sx_muki_prod', '''float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (sx * muki) * rate;''')

add_math('M3_sx_rate_prod', '''float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (sx * rate) * muki;''')

add_math('M4_rate_muki_sx', '''float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (rate * muki) * sx;''')

add_math('M5_rate_paren', '''float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = rate * (muki * sx);''')

add_math('M6_muki_paren', '''float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = muki * (rate * sx);''')

add_math('M7_compound_sx', '''float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = muki * rate;
    mSpeed.x *= sx;''')

add_math('M8_sy_sx_rate_muki', '''mSpeed.y = speed.y;
    float sx = speed.x;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * sx;''')

add_math('M9_rate_sy_sx_muki', '''float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float sx = speed.x;
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * sx;''')

add_math('M10_muki_before_call', '''float muki = (float)l_EnMuki[mDirection];
    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;''')

add_math('M11_no_sx_local', '''float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    mSpeed.x = (muki * rate) * speed.x;''')

add_math('M12_no_muki_local', '''float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = ((float)l_EnMuki[mDirection] * rate) * sx;''')

def analyze_twins(txt_path):
    with open(txt_path, 'r', encoding='utf-8') as f:
        content = f.read()
    results = {}
    for fn in ['initializeState_Jump__18dEnTorideKokoopa_cFv', 'initializeState_BigJump__18dEnTorideKokoopa_cFv']:
        m = re.search(r'\.fn ' + fn + r',.*?\n(.*?\n)\.endfn', content, re.DOTALL)
        if not m:
            results[fn] = {}
            continue
        lines = [l.strip() for l in m.group(1).splitlines() if '*/' in l]
        fsubs_l = None
        fmuls_l = []
        lfd_magic = None
        lfs_speedx = None
        lfd_int = None
        for l in lines:
            if 'fsubs' in l: fsubs_l = l.split('*/')[1].strip()
            if 'fmuls' in l: fmuls_l.append(l.split('*/')[1].strip())
            if 'lfd' in l and '@sda21' in l: lfd_magic = l.split('*/')[1].strip()
            if 'lfs' in l and ('0x10(r1)' in l or '(r3)' in l or '(r4)' in l): lfs_speedx = l.split('*/')[1].strip()
            if 'lfd' in l and '0x18(r1)' in l: lfd_int = l.split('*/')[1].strip()
        results[fn] = {
            'words': len(lines),
            'fsubs': fsubs_l,
            'fmuls': fmuls_l,
            'lfd_magic': lfd_magic,
            'lfs_speedx': lfs_speedx,
            'lfd_int': lfd_int
        }
    return results

for name, jbody, bjbody in tests:
    code = base_code.replace(orig_jump, jbody).replace(orig_bigjump, bjbody)
    src = os.path.join(BASE, 'test_twins.cpp')
    obj = os.path.join(BASE, 'test_twins.o')
    txt = os.path.join(BASE, 'test_twins.txt')
    with open(src, 'w', encoding='utf-8') as f:
        f.write(code)
    ok, log = harness.compile_draft(src, obj, extra_inc=(INC, GAME, BASE), module='wiimj2d')
    if not ok:
        print(f'{name:20s} | COMPILE FAIL')
        continue
    harness.disasm(obj, txt)
    r_j = subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'auto_decomp', 'fndiff.py'), TGT, txt, 'initializeState_Jump__18dEnTorideKokoopa_cFv'], capture_output=True, text=True)
    r_bj = subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'auto_decomp', 'fndiff.py'), TGT, txt, 'initializeState_BigJump__18dEnTorideKokoopa_cFv'], capture_output=True, text=True)
    
    j_diff = [l.strip() for l in r_j.stdout.splitlines() if 'DIFFS' in l][0] if 'DIFFS' in r_j.stdout else '?'
    bj_diff = [l.strip() for l in r_bj.stdout.splitlines() if 'DIFFS' in l][0] if 'DIFFS' in r_bj.stdout else '?'
    
    info = analyze_twins(txt)
    j_info = info['initializeState_Jump__18dEnTorideKokoopa_cFv']
    
    print(f'{name:20s} | Jump: {j_diff:30s} | BigJump: {bj_diff:30s}')
    print(f'   Alloc: magic={j_info.get("lfd_magic")} | speedx={j_info.get("lfs_speedx")} | int={j_info.get("lfd_int")}')
    print(f'          fsubs={j_info.get("fsubs")} | fmuls={j_info.get("fmuls")}')

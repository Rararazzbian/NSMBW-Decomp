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

def make_fn(math_block, anm_idx, speed1_name, speed2_name):
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
    j = make_fn(math_block, 1, 'mJumpSpeed1', 'mJumpSpeed2')
    bj = make_fn(math_block, 3, 'mBigJumpSpeed1', 'mBigJumpSpeed2')
    tests.append((name, j, bj))

add_math('T1_sy_first', '''float rate = calcJumpRate();
    f32 sy = speed.y;
    f32 sx = speed.x;
    f32 muki = (f32)l_EnMuki[mDirection];
    mSpeed.y = sy;
    mSpeed.x = (muki * rate) * sx;''')

add_math('T2_compound_order', '''float rate = calcJumpRate();
    f32 sy = speed.y;
    f32 sx = speed.x;
    f32 muki = (f32)l_EnMuki[mDirection];
    mSpeed.y = sy;
    mSpeed.x = muki * rate;
    mSpeed.x *= sx;''')

add_math('T3_sx_declared_top', '''float sx;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    sx = speed.x;
    mSpeed.x = (muki * rate) * sx;''')

add_math('T4_muki_first', '''float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    float sy = speed.y;
    float sx = speed.x;
    mSpeed.y = sy;
    mSpeed.x = (muki * rate) * sx;''')

add_math('T5_speed_members_direct', '''float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * speed.x;''')

add_math('T6_speed_members_compound', '''float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = muki * rate;
    mSpeed.x *= speed.x;''')

add_math('T7_int_cast_shape', '''float rate = calcJumpRate();
    mSpeed.y = speed.y;
    f32 sx = speed.x;
    f32 muki = l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * sx;''')

add_math('T8_int_cast_compound', '''float rate = calcJumpRate();
    mSpeed.y = speed.y;
    f32 sx = speed.x;
    f32 muki = l_EnMuki[mDirection];
    mSpeed.x = muki * rate;
    mSpeed.x *= sx;''')

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
    
    # Check instructions in jump
    with open(txt, 'r', encoding='utf-8') as f:
        content = f.read()
    m = re.search(r'\.fn initializeState_Jump__18dEnTorideKokoopa_cFv,.*?\n(.*?\n)\.endfn', content, re.DOTALL)
    fn_body = m.group(1) if m else ''
    lines = [l.strip() for l in fn_body.splitlines() if '*/' in l]
    fsubs_l, fmuls_l, lfd_magic, lfs_speedx, lfd_int = None, [], None, None, None
    for l in lines:
        if 'fsubs' in l: fsubs_l = l.split('*/')[1].strip()
        if 'fmuls' in l: fmuls_l.append(l.split('*/')[1].strip())
        if 'lfd' in l and '@sda21' in l: lfd_magic = l.split('*/')[1].strip()
        if 'lfs' in l and ('0x10(r1)' in l or '0x14(r1)' in l): 
            if '0x10(r1)' in l: lfs_speedx = l.split('*/')[1].strip()
        if 'lfd' in l and '0x18(r1)' in l: lfd_int = l.split('*/')[1].strip()
        
    print(f'{name:22s} | Jump: {j_diff:30s} | BigJump: {bj_diff:30s}')
    print(f'   Alloc: magic={lfd_magic} | sx={lfs_speedx} | int={lfd_int}')
    print(f'          fsubs={fsubs_l} | fmuls={fmuls_l}')

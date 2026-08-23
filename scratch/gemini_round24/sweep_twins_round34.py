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

variants = []

def add_variant(name, jump_body, bigjump_body):
    variants.append((name, jump_body, bigjump_body))

# Variant J1: const mVec2_c &speed reference
add_variant('J1_const_ref', '''void dEnTorideKokoopa_c::initializeState_Jump() {
    const char *anmName = mpParamJump->mAnmNames[1];
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
    const mVec2_c &speed = (flag != 0) ? mpParamJump->mJumpSpeed1 : mpParamJump->mJumpSpeed2;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();
}''', '''void dEnTorideKokoopa_c::initializeState_BigJump() {
    const char *anmName = mpParamJump->mAnmNames[3];
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
    const mVec2_c &speed = (flag != 0) ? mpParamJump->mBigJumpSpeed1 : mpParamJump->mBigJumpSpeed2;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();
}''')

# Variant J2: const mVec2_c *speed pointer
add_variant('J2_const_ptr', '''void dEnTorideKokoopa_c::initializeState_Jump() {
    const char *anmName = mpParamJump->mAnmNames[1];
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
    const mVec2_c *speed = (flag != 0) ? &mpParamJump->mJumpSpeed1 : &mpParamJump->mJumpSpeed2;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed->y;
    float sx = speed->x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();
}''', '''void dEnTorideKokoopa_c::initializeState_BigJump() {
    const char *anmName = mpParamJump->mAnmNames[3];
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
    const mVec2_c *speed = (flag != 0) ? &mpParamJump->mBigJumpSpeed1 : &mpParamJump->mBigJumpSpeed2;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed->y;
    float sx = speed->x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();
}''')

# Variant J3: pointer selected via if/else
add_variant('J3_ptr_if_else', '''void dEnTorideKokoopa_c::initializeState_Jump() {
    const char *anmName = mpParamJump->mAnmNames[1];
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
    const mVec2_c *speed;
    if (flag != 0) {
        speed = &mpParamJump->mJumpSpeed1;
    } else {
        speed = &mpParamJump->mJumpSpeed2;
    }
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed->y;
    float sx = speed->x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();
}''', '''void dEnTorideKokoopa_c::initializeState_BigJump() {
    const char *anmName = mpParamJump->mAnmNames[3];
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
    const mVec2_c *speed;
    if (flag != 0) {
        speed = &mpParamJump->mBigJumpSpeed1;
    } else {
        speed = &mpParamJump->mBigJumpSpeed2;
    }
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed->y;
    float sx = speed->x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();
}''')

# Variant J4: Declaration order split (lever 12 / 13)
add_variant('J4_decl_order', '''void dEnTorideKokoopa_c::initializeState_Jump() {
    const char *anmName = mpParamJump->mAnmNames[1];
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
    float sx;
    float muki;
    float rate = calcJumpRate();
    muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();
}''', '''void dEnTorideKokoopa_c::initializeState_BigJump() {
    const char *anmName = mpParamJump->mAnmNames[3];
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
        speed = mpParamJump->mBigJumpSpeed1;
    } else {
        speed = mpParamJump->mBigJumpSpeed2;
    }
    float sx;
    float muki;
    float rate = calcJumpRate();
    muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();
}''')

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

for name, jbody, bjbody in variants:
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
    
    print(f'=== {name} ===')
    print(f'  Jump:    {j_diff}')
    print(f'  BigJump: {bj_diff}')
    print(f'  Alloc: magic={j_info.get("lfd_magic")}')
    print(f'         speedx={j_info.get("lfs_speedx")}')
    print(f'         int={j_info.get("lfd_int")}')
    print(f'         fsubs={j_info.get("fsubs")}')
    print(f'         fmuls={j_info.get("fmuls")}')

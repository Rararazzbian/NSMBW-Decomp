import sys, os, re, subprocess
sys.path.append(".")
ROOT = os.path.abspath(".")
BASE = os.path.join(ROOT, "scratch", "gemini_round24")
TARGET_FILE = os.path.join(BASE, "auto_03_800A8710_text.txt")
EXTRA_INC = [os.path.join(BASE, "include")]

sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

base_src = open(os.path.join(ROOT, "scratch", "claude_kokoopa", "k6_sx_late.cpp"), encoding="utf-8").read()

def run_jump_variant(name, jump_snippet):
    body_j = """
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
""" + jump_snippet + """
    jumpEffect();
    jumpSE();
"""
    body_bj = body_j.replace("mJumpSpeed", "mBigJumpSpeed").replace("mAnmNames[1]", "mAnmNames[3]")
    
    src = base_src
    pattern_j = r'void dEnTorideKokoopa_c::initializeState_Jump\(\) \{[\s\S]*?\n\}'
    replacement_j = 'void dEnTorideKokoopa_c::initializeState_Jump() {\n' + body_j.strip() + '\n}'
    src = re.sub(pattern_j, replacement_j, src)
    
    pattern_bj = r'void dEnTorideKokoopa_c::initializeState_BigJump\(\) \{[\s\S]*?\n\}'
    replacement_bj = 'void dEnTorideKokoopa_c::initializeState_BigJump() {\n' + body_bj.strip() + '\n}'
    src = re.sub(pattern_bj, replacement_bj, src)
    
    test_cpp = os.path.join(BASE, "test_jump_temp.cpp")
    test_obj = os.path.join(BASE, "test_jump_temp.o")
    test_dis = os.path.join(BASE, "test_jump_temp.txt")
    
    with open(test_cpp, "w", encoding="utf-8") as f:
        f.write(src)
        
    ok, err = harness.compile_draft(test_cpp, test_obj, extra_inc=EXTRA_INC, module="wiimj2d")
    if not ok:
        print(f"COMPILE ERROR for {name}: {err[:200]}")
        return None
        
    ok_d, err_d = harness.disasm(test_obj, test_dis)
    if not ok_d:
        print(f"DISASM ERROR for {name}: {err_d[:200]}")
        return None
        
    r1 = subprocess.run([sys.executable, os.path.join(ROOT, "tools", "auto_decomp", "fndiff.py"),
                        TARGET_FILE, test_dis, "initializeState_Jump__18dEnTorideKokoopa_cFv", "-v"],
                       capture_output=True, text=True)
    r2 = subprocess.run([sys.executable, os.path.join(ROOT, "tools", "auto_decomp", "fndiff.py"),
                        TARGET_FILE, test_dis, "initializeState_BigJump__18dEnTorideKokoopa_cFv", "-v"],
                       capture_output=True, text=True)
                       
    def parse_fn(out):
        m_diff = re.search(r'DIFFS\s*(\d+)', out)
        diff = int(m_diff.group(1)) if m_diff else -1
        m_w = re.search(r'draft\s*:\s*(\d+)\s*words\s*/\s*frame\s*([0-9a-fx]+|none)\s*/\s*GPR\s*(\[[^\]]*\]|none)', out)
        words = m_w.group(1) if m_w else '?'
        frame = m_w.group(2) if m_w else '?'
        gpr = m_w.group(3) if m_w else '?'
        return diff, words, frame, gpr
        
    d1, w1, f1, g1 = parse_fn(r1.stdout)
    d2, w2, f2, g2 = parse_fn(r2.stdout)
    
    # Parse f2, f3, f4 allocation from disasm of Jump and BigJump
    dis_text = open(test_dis, "r", encoding="utf-8").read()
    
    def get_alloc_detail(fn_name):
        in_f = False
        lines = []
        for line in dis_text.splitlines():
            if fn_name in line:
                in_f = True
            elif in_f and ".endfn" in line:
                break
            elif in_f:
                if any(k in line for k in ["lfd", "lfs", "fsubs", "fmuls", "stfs"]):
                    lines.append(line.strip())
        # find the registers used
        # We want to see: which register gets speed.x (lfs from 0x10(r1)), which gets magic const (lfd from sdata2), which gets int->float (lfd 0x18(r1))
        reg_map = {}
        for l in lines:
            if "0x10(r1)" in l and "lfs" in l:
                m = re.search(r'lfs\s+(f\d+)', l)
                if m: reg_map['speed.x'] = m.group(1)
            elif "0x18(r1)" in l and "lfd" in l:
                m = re.search(r'lfd\s+(f\d+)', l)
                if m: reg_map['int->float'] = m.group(1)
            elif ("@sda21" in l or "@l" in l) and "lfd" in l:
                m = re.search(r'lfd\s+(f\d+)', l)
                if m: reg_map['magic_const'] = m.group(1)
        return reg_map
        
    j_alloc = get_alloc_detail("initializeState_Jump__18dEnTorideKokoopa_cFv")
    bj_alloc = get_alloc_detail("initializeState_BigJump__18dEnTorideKokoopa_cFv")
    
    return {
        'name': name,
        'd1': d1, 'w1': w1, 'f1': f1, 'g1': g1,
        'd2': d2, 'w2': w2, 'f2': f2, 'g2': g2,
        'j_alloc': j_alloc, 'bj_alloc': bj_alloc
    }

tests = [
    ("Component_wise_XY", """
    mVec2_c speed;
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
    mSpeed.x = (muki * rate) * sx;
"""),
    ("Component_wise_YX", """
    mVec2_c speed;
    if (flag != 0) {
        speed.y = mpParamJump->mJumpSpeed1.y;
        speed.x = mpParamJump->mJumpSpeed1.x;
    } else {
        speed.y = mpParamJump->mJumpSpeed2.y;
        speed.x = mpParamJump->mJumpSpeed2.x;
    }
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
"""),
    ("Separate_scalar_locals", """
    float sx, sy;
    if (flag != 0) {
        sx = mpParamJump->mJumpSpeed1.x;
        sy = mpParamJump->mJumpSpeed1.y;
    } else {
        sx = mpParamJump->mJumpSpeed2.x;
        sy = mpParamJump->mJumpSpeed2.y;
    }
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = sy;
    mSpeed.x = (muki * rate) * sx;
"""),
    ("Direct_mSpeedY_assign_in_branches", """
    float sx;
    if (flag != 0) {
        sx = mpParamJump->mJumpSpeed1.x;
        mSpeed.y = mpParamJump->mJumpSpeed1.y;
    } else {
        sx = mpParamJump->mJumpSpeed2.x;
        mSpeed.y = mpParamJump->mJumpSpeed2.y;
    }
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * sx;
"""),
    ("Ptr_select_then_read", """
    const mVec2_c *pSpeed = (flag != 0) ? &mpParamJump->mJumpSpeed1 : &mpParamJump->mJumpSpeed2;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = pSpeed->y;
    float sx = pSpeed->x;
    mSpeed.x = (muki * rate) * sx;
"""),
    ("sx_before_muki_order", """
    mVec2_c speed;
    if (flag != 0) {
        speed = mpParamJump->mJumpSpeed1;
    } else {
        speed = mpParamJump->mJumpSpeed2;
    }
    float rate = calcJumpRate();
    float sx = speed.x;
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    mSpeed.x = (muki * rate) * sx;
"""),
    ("mSpeedY_before_calcJumpRate", """
    mVec2_c speed;
    if (flag != 0) {
        speed = mpParamJump->mJumpSpeed1;
    } else {
        speed = mpParamJump->mJumpSpeed2;
    }
    mSpeed.y = speed.y;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
"""),
    ("sx_before_rate_and_muki", """
    mVec2_c speed;
    if (flag != 0) {
        speed = mpParamJump->mJumpSpeed1;
    } else {
        speed = mpParamJump->mJumpSpeed2;
    }
    float sx = speed.x;
    mSpeed.y = speed.y;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * sx;
""")
]

for name, snippet in tests:
    res = run_jump_variant(name, snippet)
    if res:
        print(f"{name:35s} | Jump: {res['d1']:2d} diffs ({res['w1']}w/{res['f1']}/{res['g1']}) | BigJump: {res['d2']:2d} diffs ({res['w2']}w/{res['f2']}/{res['g2']})")
        print(f"   Jump FPRs:    {res['j_alloc']}")
        print(f"   BigJump FPRs: {res['bj_alloc']}")

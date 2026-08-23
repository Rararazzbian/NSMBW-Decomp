import sys, os, re, subprocess
sys.path.append(".")
ROOT = os.path.abspath(".")
BASE = os.path.join(ROOT, "scratch", "gemini_round24")
TARGET_FILE = os.path.join(BASE, "auto_03_800A8710_text.txt")
EXTRA_INC = [os.path.join(BASE, "include")]

sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

base_src = open(os.path.join(ROOT, "scratch", "claude_kokoopa", "k6_sx_late.cpp"), encoding="utf-8").read()

def test_jump_variant(name, jump_body, big_jump_body=None, custom_header_decl=None):
    if big_jump_body is None:
        big_jump_body = jump_body.replace("mJumpSpeed", "mBigJumpSpeed")
        
    src = base_src
    if custom_header_decl:
        # Put declaration at top of file
        src = custom_header_decl + "\n" + src
        
    # Replace initializeState_Jump
    pattern_j = r'void dEnTorideKokoopa_c::initializeState_Jump\(\) \{[\s\S]*?\n\}'
    replacement_j = 'void dEnTorideKokoopa_c::initializeState_Jump() {\n' + jump_body.strip() + '\n}'
    src = re.sub(pattern_j, replacement_j, src)
    
    # Replace initializeState_BigJump
    pattern_bj = r'void dEnTorideKokoopa_c::initializeState_BigJump\(\) \{[\s\S]*?\n\}'
    replacement_bj = 'void dEnTorideKokoopa_c::initializeState_BigJump() {\n' + big_jump_body.strip() + '\n}'
    src = re.sub(pattern_bj, replacement_bj, src)
    
    test_cpp = os.path.join(BASE, "test_jump_temp.cpp")
    test_obj = os.path.join(BASE, "test_jump_temp.o")
    test_dis = os.path.join(BASE, "test_jump_temp.txt")
    
    with open(test_cpp, "w", encoding="utf-8") as f:
        f.write(src)
        
    ok, err = harness.compile_draft(test_cpp, test_obj, extra_inc=EXTRA_INC, module="wiimj2d")
    if not ok:
        print(f"COMPILE ERROR for {name}: {err[:250]}")
        return None
        
    ok_d, err_d = harness.disasm(test_obj, test_dis)
    if not ok_d:
        print(f"DISASM ERROR for {name}: {err_d[:250]}")
        return None
        
    # Score Jump
    r1 = subprocess.run([sys.executable, os.path.join(ROOT, "tools", "auto_decomp", "fndiff.py"),
                        TARGET_FILE, test_dis, "initializeState_Jump__18dEnTorideKokoopa_cFv", "-v"],
                       capture_output=True, text=True)
    # Score BigJump
    r2 = subprocess.run([sys.executable, os.path.join(ROOT, "tools", "auto_decomp", "fndiff.py"),
                        TARGET_FILE, test_dis, "initializeState_BigJump__18dEnTorideKokoopa_cFv", "-v"],
                       capture_output=True, text=True)
                       
    # Parse results
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
    
    # Extract register allocation for f2, f3, f4 in Jump
    dis_text = open(test_dis, "r", encoding="utf-8").read()
    j_lines = []
    in_j = False
    for line in dis_text.splitlines():
        if "initializeState_Jump__18dEnTorideKokoopa_cFv" in line:
            in_j = True
        elif in_j and ".endfn" in line:
            break
        elif in_j:
            if any(f in line for f in ["lfd", "lfs", "fsubs", "fmuls", "stfs"]):
                j_lines.append(line.strip())
                
    alloc_summary = " | ".join(j_lines[-6:]) if len(j_lines) >= 6 else " | ".join(j_lines)
    
    return {
        'name': name,
        'jump_diffs': d1, 'jump_words': w1, 'jump_frame': f1, 'jump_gpr': g1,
        'bigjump_diffs': d2, 'bigjump_words': w2, 'bigjump_frame': f2, 'bigjump_gpr': g2,
        'alloc_summary': alloc_summary,
        'r1_out': r1.stdout
    }

# Test 0: Baseline k6_sx_late body
k6_body = """
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
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();
"""

k6_big_body = k6_body.replace("mJumpSpeed", "mBigJumpSpeed").replace("mAnmNames[1]", "mAnmNames[3]")

# 1. Baseline s8 (already in d_enemy.hpp)
res0 = test_jump_variant("Baseline_s8_declaration", k6_body, k6_big_body)
if res0:
    print(f"[{res0['name']}] Jump: {res0['jump_diffs']} diffs ({res0['jump_words']}w/{res0['jump_frame']}/{res0['jump_gpr']}), BigJump: {res0['bigjump_diffs']} diffs ({res0['bigjump_words']}w/{res0['bigjump_frame']}/{res0['bigjump_gpr']})")
    print(f"  FP alloc snippet: {res0['alloc_summary']}")

# 2. What if l_EnMuki is declared as s16?
res_s16 = test_jump_variant("Decl_s16", k6_body, k6_big_body, "extern const s16 l_EnMuki[];")
if res_s16:
    print(f"[{res_s16['name']}] Jump: {res_s16['jump_diffs']} diffs, BigJump: {res_s16['bigjump_diffs']} diffs")
    print(f"  FP alloc snippet: {res_s16['alloc_summary']}")

# 3. What if l_EnMuki is declared as int?
res_int = test_jump_variant("Decl_int", k6_body, k6_big_body, "extern const int l_EnMuki[];")
if res_int:
    print(f"[{res_int['name']}] Jump: {res_int['jump_diffs']} diffs, BigJump: {res_int['bigjump_diffs']} diffs")
    print(f"  FP alloc snippet: {res_int['alloc_summary']}")

# 4. What if l_EnMuki is declared as float?
res_float = test_jump_variant("Decl_float", k6_body, k6_big_body, "extern const float l_EnMuki[];")
if res_float:
    print(f"[{res_float['name']}] Jump: {res_float['jump_diffs']} diffs, BigJump: {res_float['bigjump_diffs']} diffs")
    print(f"  FP alloc snippet: {res_float['alloc_summary']}")


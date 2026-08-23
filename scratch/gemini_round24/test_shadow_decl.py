import sys, os, re, subprocess
sys.path.append(".")
ROOT = os.path.abspath(".")
BASE = os.path.join(ROOT, "scratch", "gemini_round24")
TARGET_FILE = os.path.join(BASE, "auto_03_800A8710_text.txt")
EXTRA_INC = [os.path.join(BASE, "include")]

sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

orig_header = open(os.path.join(ROOT, "include", "game", "bases", "d_enemy.hpp"), encoding="utf-8").read()
shadow_header_path = os.path.join(BASE, "include", "game", "bases", "d_enemy.hpp")

base_src = open(os.path.join(ROOT, "scratch", "claude_kokoopa", "k6_sx_late.cpp"), encoding="utf-8").read()

def test_with_decl(type_name, decl_str):
    # Write shadow header
    new_header = orig_header.replace("extern const s8 l_EnMuki[];", decl_str)
    with open(shadow_header_path, "w", encoding="utf-8") as f:
        f.write(new_header)
        
    test_cpp = os.path.join(BASE, "test_jump_temp.cpp")
    test_obj = os.path.join(BASE, "test_jump_temp.o")
    test_dis = os.path.join(BASE, "test_jump_temp.txt")
    
    with open(test_cpp, "w", encoding="utf-8") as f:
        f.write(base_src)
        
    ok, err = harness.compile_draft(test_cpp, test_obj, extra_inc=EXTRA_INC, module="wiimj2d")
    if not ok:
        print(f"COMPILE ERROR for {type_name}: {err[:200]}")
        return None
        
    ok_d, err_d = harness.disasm(test_obj, test_dis)
    if not ok_d:
        print(f"DISASM ERROR for {type_name}: {err_d[:200]}")
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
    
    # Read disasm for Jump around lines 60-75
    dis_text = open(test_dis, "r", encoding="utf-8").read()
    j_lines = []
    in_j = False
    for line in dis_text.splitlines():
        if "initializeState_Jump__18dEnTorideKokoopa_cFv" in line:
            in_j = True
        elif in_j and ".endfn" in line:
            break
        elif in_j:
            if any(f in line for f in ["lfd", "lfs", "fsubs", "fmuls", "stfs", "lbzx", "lhax", "lwzx", "extsb"]):
                j_lines.append(line.strip())
                
    alloc = " | ".join(j_lines)
    print(f"[{type_name}] Jump: {d1} diffs ({w1}w/{f1}/{g1}), BigJump: {d2} diffs ({w2}w/{f2}/{g2})")
    print(f"  Instructions:\n    {alloc}")
    return d1, d2

test_with_decl("s8 (retail matching)", "extern const s8 l_EnMuki[];")
test_with_decl("s16", "extern const s16 l_EnMuki[];")
test_with_decl("int", "extern const int l_EnMuki[];")
test_with_decl("float", "extern const float l_EnMuki[];")

# Clean up shadow header so it doesn't affect other builds
if os.path.exists(shadow_header_path):
    os.remove(shadow_header_path)

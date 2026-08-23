import os, sys, re, subprocess

ROOT = os.path.abspath('.')
BASE = os.path.join(ROOT, 'scratch', 'gemini_round24')
SRC_FILE = os.path.join(BASE, 'd_enemy_toride_kokoopa.cpp')
TARGET_FILE = os.path.join(BASE, 'auto_03_800A8710_text.txt')
EXTRA_INC = [os.path.join(BASE, 'include')]

sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

with open(SRC_FILE, 'r', encoding='utf-8') as f:
    ORIGINAL_SRC = f.read()

def test_variant(name, new_quake_fn_body):
    pattern = r'void dEnTorideKokoopa_c::setQuakeDead\(\) \{[\s\S]*?\n\}'
    replacement = 'void dEnTorideKokoopa_c::setQuakeDead() {\n' + new_quake_fn_body.strip() + '\n}'
    
    modified_src = re.sub(pattern, replacement, ORIGINAL_SRC)
    if modified_src == ORIGINAL_SRC:
        print(f'FAILED TO REPLACE for {name}')
        return None
        
    test_cpp = os.path.join(BASE, 'test_quake_temp.cpp')
    test_obj = os.path.join(BASE, 'test_quake_temp.o')
    test_dis = os.path.join(BASE, 'test_quake_temp.txt')
    
    with open(test_cpp, 'w', encoding='utf-8') as f:
        f.write(modified_src)
        
    ok, err = harness.compile_draft(test_cpp, test_obj, extra_inc=EXTRA_INC, module='wiimj2d')
    if not ok:
        print(f'COMPILE ERROR for {name}: {err[:200]}')
        return {'name': name, 'ok': False, 'err': err}
        
    ok_d, err_d = harness.disasm(test_obj, test_dis)
    if not ok_d:
        print(f'DISASM ERROR for {name}: {err_d[:200]}')
        return {'name': name, 'ok': False, 'err': err_d}
        
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'auto_decomp', 'fndiff.py'),
                        TARGET_FILE, test_dis, 'setQuakeDead__18dEnTorideKokoopa_cFv', '-v'],
                       capture_output=True, text=True)
    
    diff_output = r.stdout
    
    draft_match = re.search(r'draft\s*:\s*(\d+)\s*words\s*/\s*frame\s*([0-9a-fx]+|none)\s*/\s*GPR\s*(\[[^\]]*\]|none)', diff_output)
    words = draft_match.group(1) if draft_match else '?'
    frame = draft_match.group(2) if draft_match else '?'
    gpr = draft_match.group(3) if draft_match else '?'
    
    diffs_match = re.search(r'DIFFS\s*(\d+)', diff_output)
    diffs = int(diffs_match.group(1)) if diffs_match else -1
    
    dis_text = ''
    in_fn = False
    with open(test_dis, 'r', encoding='utf-8') as f:
        for line in f:
            if 'setQuakeDead__18dEnTorideKokoopa_cFv' in line:
                in_fn = True
            elif in_fn and '.endfn' in line:
                break
            elif in_fn:
                dis_text += line
                
    has_li_r3_0 = ('li r3, 0x0' in dis_text or 'li r3, 0\n' in dis_text or 'li r3, 0 ' in dis_text)
    
    res = {
        'name': name,
        'ok': True,
        'words': words,
        'frame': frame,
        'gpr': gpr,
        'diffs': diffs,
        'has_li_r3_0': has_li_r3_0,
        'diff_output': diff_output
    }
    return res

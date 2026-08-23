import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, ROOT)
from tools.auto_decomp.harness import compile_draft, disasm
from tools.auto_decomp.fndiff import read_functions, canonical_name, describe, compare

WORK_DIR = os.path.dirname(os.path.abspath(__file__))
TARGET_TXT = os.path.join(WORK_DIR, 'target.txt')
DRAFT_CPP = os.path.join(WORK_DIR, 'd_p_sw_manager.cpp')
DRAFT_OBJ = os.path.join(WORK_DIR, 'd_p_sw_manager.o')
DRAFT_TXT = os.path.join(WORK_DIR, 'd_p_sw_manager.txt')

EXTRA_INC = [os.path.join(WORK_DIR, 'include')]

def test():
    ok, log = compile_draft(DRAFT_CPP, DRAFT_OBJ, extra_inc=EXTRA_INC, module='wiimj2d')
    if not ok:
        print("COMPILE FAILED:")
        print(log)
        return False

    dok, dlog = disasm(DRAFT_OBJ, DRAFT_TXT)
    if not dok:
        print("DISASM FAILED:")
        print(dlog)
        return False

    tgt = read_functions(TARGET_TXT)
    drf = {canonical_name(k): v for k, v in read_functions(DRAFT_TXT).items()}
    tgt_c = {canonical_name(k): v for k, v in tgt.items()}

    print(f"\nScoreboard ({len(tgt_c)} functions):")
    print("=" * 80)
    matched_count = 0
    for name, body in tgt_c.items():
        if name not in drf:
            print(f"=== {name}\n  NOT IN DRAFT")
            continue
        diff_count = compare(body, drf[name], name, verbose=True)
        if diff_count == 0:
            matched_count += 1
        print("-" * 80)

    print(f"\nSummary: {matched_count} / {len(tgt_c)} functions matched (DIFFS 0)")
    return matched_count == len(tgt_c)

if __name__ == '__main__':
    test()

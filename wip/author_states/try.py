import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

SRC = os.path.join('wip', 'author_states', 'd_line_mng.cpp')
OBJ = os.path.join('wip', 'author_states', 'd_line_mng.o')
TXT = os.path.join('wip', 'author_states', 'd_line_mng.txt')
TARGET = os.path.join('wip', 'line_mng_shared', 'target.txt')
SHADOW = os.path.join('wip', 'line_mng_shared', 'shadow_include')
LOCAL = os.path.join('wip', 'author_states', 'local_shadow')

def build():
    ok, log = H.compile_draft(SRC, OBJ, extra_inc=[LOCAL, SHADOW])
    if not ok:
        print("COMPILE FAILED")
        print(log)
        return False
    ok2, log2 = H.disasm(OBJ, TXT)
    if not ok2:
        print("DISASM FAILED")
        print(log2)
        return False
    return True

def check(*names):
    if not build():
        return
    for name in names:
        matched, report = H.diff_fn(TARGET, TXT, name)
        print("=== %s : %s ===" % (name, "MATCH" if matched else "DIFFER"))
        print(report)
        print()

if __name__ == '__main__':
    check(*sys.argv[1:])

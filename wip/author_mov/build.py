import os, sys
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'd_line_mng.cpp')
OBJ = os.path.join(HERE, 'd_line_mng.o')
DIS = os.path.join(HERE, 'd_line_mng.dis.txt')
TARGET = os.path.join('wip', 'line_mng_shared', 'target.txt')
SHADOW = os.path.join(HERE, 'shadow_include')

FNS = [
    'mov_to_rightlower__10dLineMng_cFUlRC7mVec2_cb',
    'mov_to_rightupper__10dLineMng_cFUlRC7mVec2_cb',
    'mov_to_leftupper__10dLineMng_cFUlRC7mVec2_cb',
    'mov_to_leftlower__10dLineMng_cFUlRC7mVec2_cb',
    'mov_frm_rightupper__10dLineMng_cFRC7mVec2_cb',
    'mov_frm_leftlower__10dLineMng_cFRC7mVec2_cb',
    'mov_frm_rightlower__10dLineMng_cFRC7mVec2_cb',
    'mov_frm_leftupper__10dLineMng_cFRC7mVec2_cb',
]

def main():
    ok, log = H.compile_draft(SRC, OBJ, extra_inc=[SHADOW])
    if not ok:
        print('COMPILE FAILED')
        print(log)
        sys.exit(1)
    ok, log = H.disasm(OBJ, DIS)
    if not ok:
        print('DISASM FAILED')
        print(log)
        sys.exit(1)
    only = sys.argv[1:] or FNS
    total_ok = True
    for fn in only:
        matched, report = H.diff_fn(TARGET, DIS, fn)
        status = 'MATCH' if matched else 'DIFF'
        print('=== %s: %s' % (fn, status))
        if not matched:
            total_ok = False
            print(report)
    sys.exit(0 if total_ok else 2)

if __name__ == '__main__':
    main()

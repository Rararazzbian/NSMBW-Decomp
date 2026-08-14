"""Compile + verify the MERGED d_a_wm_smallcloud.cpp draft, using harness.py's
module-aware flags (module='d_basesNP' -- this is a REL unit, its flags differ
from the DOL's).
"""
import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    src = os.path.join(HERE, 'd_a_wm_smallcloud.cpp')
    obj = os.path.join(HERE, 'draft.o')
    txt = os.path.join(HERE, 'draft.txt')

    extra_inc = [os.path.join(HERE, 'shadow_include'), os.path.join(HERE, 'include')]

    ok, log = H.compile_draft(src, obj, extra_inc=extra_inc, module='d_basesNP')
    if not ok:
        print('COMPILE FAIL')
        print(log)
        sys.exit(1)
    print('compile OK')

    ok, log = H.disasm(obj, txt)
    if not ok:
        print('DISASM FAIL')
        print(log)
        sys.exit(1)
    print('disasm OK')

    sys.path.insert(0, os.path.join(ROOT, 'wip', 'wm_units'))
    import verify_anon as V
    sys.argv = ['verify_anon.py', txt, '0x1797e0', '0x179ff0',
                os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_001797B4_text.o'),
                os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_fn_2_179F40_text.o'),
                os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_00179FC4_text.o')]
    V.main()


if __name__ == '__main__':
    main()

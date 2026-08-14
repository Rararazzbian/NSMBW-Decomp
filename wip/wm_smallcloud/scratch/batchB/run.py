import os
import subprocess
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
MWCC = os.path.join(ROOT, 'compilers', 'Wii', '1.1', 'mwcceppc.exe')
DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
HERE = os.path.dirname(os.path.abspath(__file__))

CFLAGS = ['-c', '-sdata', '0', '-sdata2', '0', '-proc', 'gekko', '-fp', 'hard', '-O4,p',
          '-inline', 'noauto', '-char', 'signed', '-rtti', 'off', '-enum', 'int',
          '-Cpp_exceptions', 'off', '-ipa', 'file', '-enc', 'SJIS', '-DREVOLUTION', '-I-']
INCLUDES = [
    os.path.join(HERE, 'shadow_include'),
    'include', 'include/lib', 'include/lib/MSL', 'include/lib/MSL/internal',
    'include/lib/revolution/BTE/include', 'include/lib/revolution/BTE/stack/include',
    'include/lib/revolution/BTE/stack/btm', 'include/lib/revolution/BTE/bta/include',
    'include/lib/revolution/BTE/bta/sys', 'include/lib/revolution/BTE/gki/common',
    'include/lib/revolution/BTE/gki/platform',
]


def compile_draft(src, obj):
    args = [MWCC] + CFLAGS + [src, '-o', obj]
    for inc in INCLUDES:
        args += ['-i', inc if os.path.isabs(inc) else os.path.join(ROOT, inc)]
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or '') + (p.stderr or '')


def disasm(obj, out):
    p = subprocess.run([DTK, 'elf', 'disasm', obj, out], cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or '') + (p.stderr or '')


def main():
    src = os.path.join(HERE, 'd_a_wm_smallcloud.cpp')
    obj = os.path.join(HERE, 'draft.o')
    txt = os.path.join(HERE, 'draft.txt')
    ok, log = compile_draft(src, obj)
    if not ok:
        print('COMPILE FAIL')
        print(log)
        sys.exit(1)
    print('compile OK')
    ok, log = disasm(obj, txt)
    if not ok:
        print('DISASM FAIL')
        print(log)
        sys.exit(1)
    print('disasm OK')

    sys.path.insert(0, os.path.join(ROOT, 'wip', 'wm_units'))
    import verify_anon as V
    sys.argv = ['verify_anon.py', txt, '0x1797e0', '0x179ff0',
                os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_001797B4_text.o'),
                os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_00179FC4_text.o'),
                os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_fn_2_179F40_text.o')]
    V.main()


if __name__ == '__main__':
    main()

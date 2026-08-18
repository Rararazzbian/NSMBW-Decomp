import os, sys
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

WORK = os.path.join(ROOT, 'wip', 'nand_thread', 'scratch', 'closer_d')
TARGET = os.path.join(ROOT, 'wip', 'nand_thread', 'target_raw.txt')
SYMS = os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')

FUNCS = [
    ('existCheck__13dNandThread_cFv', 0x800CEF80, 0xD8),
    ('cmdExistCheck__13dNandThread_cFv', None, None),
    ('cmdSpaceCheck__13dNandThread_cFv', None, None),
    ('spaceCheck__13dNandThread_cFv', None, None),
    ('save__13dNandThread_cFv', 0x800CF200, 0x17C),
    ('createBanner__13dNandThread_cFv', None, None),
    ('cmdLoad__13dNandThread_cFv', None, None),
    ('load__13dNandThread_cFv', 0x800CF680, 0x284),
    ('checkCRC__13dNandThread_cFv', None, None),
    ('cmdDeleteFile__13dNandThread_cFv', None, None),
    ('deleteFile__13dNandThread_cFv', None, None),
    ('run__13dNandThread_cFv', None, None),
    ('create__13dNandThread_cFPQ23EGG4Heap', None, None),
    ('setNandError__13dNandThread_cFl', None, None),
    ('getSaveData__13dNandThread_cFv', None, None),
]

def main():
    src = os.path.join(WORK, 'd_nand_thread.cpp')
    obj = os.path.join(WORK, 'd_nand_thread.o')
    txt = os.path.join(WORK, 'd_nand_thread.txt')
    ok, log = harness.compile_draft(src, obj)
    if not ok:
        print('COMPILE FAILED')
        print(log[:4000])
        return
    dok, dlog = harness.disasm(obj, txt)
    if not dok:
        print('DISASM FAILED')
        print(dlog[:2000])
        return
    all_ok = True
    for name, addr, size in FUNCS:
        matched, report = harness.diff_fn(TARGET, txt, name)
        tag = 'MATCH' if matched else 'DIFF '
        if not matched:
            all_ok = False
        print('[%s] %s' % (tag, name))
        if not matched:
            print(report)
            print()
    print('\nALL MATCH' if all_ok else '\nSOME DIFFER')

if __name__ == '__main__':
    main()

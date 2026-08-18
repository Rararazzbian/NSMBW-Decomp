import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'd_nand_thread.cpp')
OBJ = os.path.join(HERE, 'd_nand_thread.o')
TXT = os.path.join(HERE, 'd_nand_thread.txt')
TARGET = os.path.join(ROOT, 'wip', 'nand_thread', 'target_raw.txt')

ok, log = harness.compile_draft(SRC, OBJ)
if not ok:
    print("COMPILE FAILED")
    print(log)
    sys.exit(1)
ok, log = harness.disasm(OBJ, TXT)
if not ok:
    print("DISASM FAILED")
    print(log)
    sys.exit(1)

SYMS = os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')
sizes = {}
for line in open(SYMS, encoding='utf-8'):
    if '=' in line and 'size:' in line:
        name = line.split('=')[0].strip()
        size = int(line.split('size:')[1].split()[0], 16)
        sizes[name] = size

for name in ('spaceCheck__13dNandThread_cFv', 'writeBanner__13dNandThread_cFP12NANDFileInfo'):
    matched, report = harness.diff_fn(TARGET, TXT, name)
    got = harness.extract(TXT, name)
    n_bytes = len(got) * 4
    sym_size = sizes.get(name)
    print('=== %s ===' % name)
    print(' emitted size: 0x%X (%d instr)   symbol map size: 0x%X   %s' % (
        n_bytes, len(got), sym_size, 'OK' if n_bytes == sym_size else 'MISMATCH'))
    print(' %s' % ('MATCH' if matched else 'NOT MATCHING'))
    if not matched:
        print(report)
    print()

# emitted order check
order = harness.list_functions(TXT)
print('Emitted order:', order)

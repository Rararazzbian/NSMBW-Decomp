import os, sys
ROOT = '/opt/NSMBW-Decomp'
BASE = '/opt/NSMBW-Decomp/scratch/auto_kokoopa_fumi'
HERE = os.path.join(BASE, 'manifest')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness
harness.MWCC = os.path.join(BASE, 'shims', 'mwcceppc')
harness.DTK = os.path.join(BASE, 'dtk')
TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', 'dol_bases_d_kokoopa_sp_fumi_check', 'target.txt')
src, label = sys.argv[1], sys.argv[2]
incdir = sys.argv[3] if len(sys.argv) > 3 else os.path.join(BASE, 'shadow')
obj, txt = f'{HERE}/{label}.o', f'{HERE}/{label}.txt'
ok, log = harness.compile_draft(os.path.join(HERE, src), obj, extra_inc=(incdir,), module='wiimj2d')
assert ok, log
ok, log = harness.disasm(obj, txt)
assert ok, log
allok = True
for fn in ['operate__20KokoopaSpFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c', '__dt__20KokoopaSpFumiCheck_cFv']:
    m, msg = harness.diff_fn(TARGET, txt, fn)
    print(fn, 'MATCH' if m else msg.splitlines()[0])
    allok &= m
print('ALL MATCH' if allok else 'STILL DIFFING')

import os, sys
ROOT = '/opt/NSMBW-Decomp'
BASE = '/opt/NSMBW-Decomp/scratch/auto_kokoopa_fumi'
HERE = os.path.join(BASE, 'manifest')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness
harness.MWCC = os.path.join(BASE, 'shims', 'mwcceppc')
harness.DTK = os.path.join(BASE, 'dtk')
TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', 'dol_bases_d_kokoopa_sp_fumi_check', 'target.txt')
src = os.path.join(HERE, sys.argv[1])
label = sys.argv[2]
obj, txt = f'{HERE}/{label}.o', f'{HERE}/{label}.txt'
ok, log = harness.compile_draft(src, obj, extra_inc=(f'{BASE}/shadow',), module='wiimj2d')
assert ok, log
ok, log = harness.disasm(obj, txt)
assert ok, log
for fn in ['operate__20KokoopaSpFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c', '__dt__20KokoopaSpFumiCheck_cFv']:
    m, msg = harness.diff_fn(TARGET, txt, fn)
    print(fn, 'MATCH' if m else msg.splitlines()[0])

import os
import sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402
OBJDIR = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')
OUTDIR = os.path.dirname(os.path.abspath(__file__))
o = 'auto_fn_2_164180_text.o'
out = os.path.join(OUTDIR, o + '.dis.txt')
ok, msg = H.disasm(os.path.join(OBJDIR, o), out)
print('%s : %s' % (o, 'OK' if ok else 'FAIL ' + msg))
if ok:
    print(open(out, encoding='utf-8', errors='replace').read())

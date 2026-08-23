import io
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, errors='replace')
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

OBJ = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\bin\compiled\wiimj2d\dol\bases\d_line_mng.o'
OUT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\trial\corpus_line.txt'

ok, log = harness.disasm(OBJ, OUT)
if not ok:
    print('DISASM FAILED:', log[:1500])
    sys.exit(1)

txt = open(OUT, encoding='utf-8', errors='replace').read()
fns = re.split(r'(?m)^\.fn ', txt)
hits = []
for f in fns[1:]:
    name = f.split(',')[0]
    if 'sda21' in f and re.search(r'lfs f1, 0x', f):
        hits.append(name)
print(len(hits), 'functions with lfs f1 member-load + pool load')
for h in hits:
    print(' ', h)

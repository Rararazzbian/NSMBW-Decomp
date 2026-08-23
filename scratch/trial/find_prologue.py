"""Find matched objects whose functions start with the exact dPosShake move() prologue."""
import io
import glob
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, errors='replace')
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
INSN_RE = re.compile(r'\*/\s*(\S.*)$')
FN_RE = re.compile(r'(?m)^\.fn (\S+),')

# signature: insn[0] loads a member into f1, some fmuls f0, f1, f0 within first 6,
# and a pooled lfs into f2 within first 8.
def matches(insns):
    if len(insns) < 8:
        return False
    if not re.match(r'lfs f1, 0x[0-9A-Fa-f]+\(r3\)', insns[0]):
        return False
    try:
        mi = next(i for i in range(6) if insns[i].startswith('fmuls f0, f1, f0'))
    except StopIteration:
        return False
    window = insns[:mi + 4]
    return any(re.match(r'lfs f2, "@\d+"@sda21', x) for x in window)


objs = glob.glob(os.path.join(ROOT, r'bin\compiled\wiimj2d\dol\**\*.o'), recursive=True)
print(len(objs), 'objects')
out_txt = os.path.join(ROOT, r'scratch\trial\_tmp_scan.txt')
hits = []
for n, obj in enumerate(objs):
    ok, _ = harness.disasm(obj, out_txt)
    if not ok:
        continue
    txt = open(out_txt, encoding='utf-8', errors='replace').read()
    for chunk in txt.split('.fn ')[1:]:
        name = chunk.split(',')[0]
        insns = [INSN_RE.search(l).group(1).strip()
                 for l in chunk.splitlines() if INSN_RE.search(l)]
        if matches(insns):
            hits.append((obj, name))
            print('HIT:', os.path.relpath(obj, ROOT), '::', name)
print('total hits:', len(hits))

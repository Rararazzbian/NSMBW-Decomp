import os
import subprocess
import sys
import re

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'wip', 'wm_units'))
from verify_anon import functions

target = functions(os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_fn_2_185AC0_text.o.dis.txt'), with_addr=True)
draft = functions(os.path.join(ROOT, 'scratch', 'gemini_round13', 'd_a_wm_tower_compiled.txt'))

t_name, t_ins = [(name, ins) for addr, name, ins in target if addr == 0x185AC0][0]
d_name, d_ins = [(name, ins) for name, ins in draft if '__sinit' in name][0]

def norm_fixed(instructions):
    out = []
    for line in instructions:
        line = re.sub(r'"?[.\w_@$][^\s,]*"?@(ha|l|sda21|sda2)\b', r'SYM@\1', line)
        line = re.sub(r'^(bl|b) \S+$', r'\1 SYM', line)
        line = re.sub(r'\.L_[0-9A-Fa-f]+', 'LBL', line)
        out.append(line)
    return out

t_n = norm_fixed(t_ins)
d_n = norm_fixed(d_ins)

print(f"Length match: {len(t_n)} == {len(d_n)}")
diffs = [i for i in range(len(t_n)) if t_n[i] != d_n[i]]
print(f"Differences with symbol norm: {len(diffs)}")
for d in diffs:
    print(f"  {d}: {t_n[d]} vs {d_n[d]}")

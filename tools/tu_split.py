"""Flag tu_extent.py ranges that actually contain more than one translation unit.

tu_extent.py delimits TUs by __sinit symbols. A TU with no file-scope static
objects emits no __sinit and is therefore invisible -- it gets silently absorbed
into a neighbour's range. d_a_sink_dokan.cpp was found this way, sitting
undetected between d_a_rot_objs_base and d_a_spin_child_base.

Heuristic: demangle the class name out of every function in a range and count
them. A range dominated by one class is one TU. A range with a *second* class
carrying a double-digit function count is almost certainly two or more TUs, and
that second class is an unlisted target.

Small counts (1-5) are usually inlined helpers or genuinely co-located effect
classes, not separate files -- treat >=6 as the signal and verify before acting.
"""
import re
import subprocess
import collections
import os

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
CLS = re.compile(r'__(\d+)([A-Za-z_0-9]+)')
SUSPECT = 6

funcs = []
with open(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt'),
          encoding='utf-8', errors='replace') as fh:
    for line in fh:
        if ' = .text:' not in line or 'type:function' not in line:
            continue
        addr = int(line.split(' = .text:')[1].split(';')[0], 16)
        funcs.append((addr, line.split(' = ')[0].strip()))
funcs.sort()

out = subprocess.run(['python', os.path.join(HERE, 'tu_extent.py')],
                     capture_output=True, text=True).stdout.splitlines()[1:]

print('%-32s %s' % ('RANGE REPORTED AS', 'CLASSES PRESENT (function count)'))
for row in out:
    f = row.split()
    if len(f) < 5:
        continue
    tu, lo, hi = f[0], int(f[1], 16), int(f[2], 16)
    seen = collections.Counter()
    for addr, name in funcs:
        if lo <= addr < hi:
            m = CLS.search(name)
            if m:
                seen[m.group(2)[:int(m.group(1))]] += 1
    game = {k: v for k, v in seen.items() if k.startswith('d')}
    extra = [k for k, v in game.items() if v >= SUSPECT]
    if len(extra) > 1:
        ranked = sorted(game.items(), key=lambda x: -x[1])[:5]
        print('%-32s %s   <-- %d likely TUs' % (
            tu, ', '.join('%s(%d)' % kv for kv in ranked), len(extra)))

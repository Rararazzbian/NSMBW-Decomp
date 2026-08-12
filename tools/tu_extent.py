"""For each undecompiled actor TU, infer its .text extent.

A TU's __sinit sits inside it. The TU runs from the end of the previous
decompiled slice (or the previous TU's __sinit block) up to and including its
own __sinit and any template instantiations that follow. We bound it by the
neighbouring __sinit symbols and by any decompiled slice ranges in between.
"""
import json
import os

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
TB = 0x80006780

rows = []
with open(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt'),
          encoding='utf-8', errors='replace') as fh:
    for line in fh:
        line = line.strip()
        if not line.startswith('__sinit_'):
            continue
        name, _, rest = line.partition(' = .text:')
        if not rest:
            continue
        rows.append((int(rest.split(';')[0], 16),
                     name[len('__sinit_'):].lstrip('\\'),
                     int(rest.split('size:')[1].split()[0], 16)))
rows.sort()

with open(os.path.join(ROOT, 'slices', 'wiimj2d.json'), encoding='utf-8') as fh:
    slices = json.load(fh)
done = {v['source'].split('/')[-1].replace('.cpp', '_cpp') for v in slices['slices']}
banked = []
for v in slices['slices']:
    t = v['memoryRanges'].get('.text')
    if t:
        a, b = t.split('-')
        banked.append((TB + int(a, 16), TB + int(b, 16)))
banked.sort()

# every function, to find the true end of a TU (its __sinit plus trailing templates)
funcs = []
with open(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt'),
          encoding='utf-8', errors='replace') as fh:
    for line in fh:
        if ' = .text:' not in line or 'type:function' not in line:
            continue
        rest = line.split(' = .text:')[1]
        funcs.append((int(rest.split(';')[0], 16),
                      int(rest.split('size:')[1].split()[0], 16)))
funcs.sort()

print('%-34s %-11s %-11s %8s  %s' % ('TU', 'start', 'end', 'bytes', 'fns'))
for i, (addr, name, size) in enumerate(rows):
    if name in done or not name.startswith('d_a_'):
        continue
    prev_end = rows[i - 1][0] + rows[i - 1][2] if i else TB
    # Any banked slice overlapping [prev_end, addr) pushes the start past it.
    # NOTE: the old test was `prev_end <= lo < addr`, which silently did nothing
    # when a banked slice *straddled* prev_end -- it reported d_a_en_dpakkun_base
    # as starting 1000 bytes inside the banked d_a_en_dpakkun.cpp. Overlap, not
    # containment, is the correct test.
    for lo, hi in banked:
        if lo < addr and hi > prev_end:
            prev_end = max(prev_end, hi)
    end = addr + size                # then absorb trailing template instantiations
    for fa, fs in funcs:
        if fa == end:
            end = fa + fs
    nxt = rows[i + 1][0] if i + 1 < len(rows) else None
    if nxt and end > nxt:
        end = addr + size
    n = sum(1 for fa, fs in funcs if prev_end <= fa < end)
    print('%-34s %-11s %-11s %8d  %d' % (name, hex(prev_end), hex(end), end - prev_end, n))

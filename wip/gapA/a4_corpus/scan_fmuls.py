"""Scan matched-object disassembly for fmuls whose operands are (member load) x (float literal).

Classifies each such fmuls by which operand is in the A (2nd) slot.
"""
import os, re, sys, glob

ASMDIR = sys.argv[1]

fn_re   = re.compile(r'^\.fn\s+(\S+),')
end_re  = re.compile(r'^\.endfn')
ins_re  = re.compile(r'^/\* ([0-9A-F]{8}) [0-9A-F]{8}\s+[0-9A-F ]+\*/\t(.*)$')

lfs_lit = re.compile(r'^lfs\s+(f\d+),\s*"?(@\w+)[^"]*"?@sda21\(r0\)')
lfs_lit2= re.compile(r'^lfs\s+(f\d+),\s*(@\w+)@sda21\(r0\)')
lfs_sda = re.compile(r'^lfs\s+(f\d+),\s*(\S+)@sda21\(r0\)')
lfs_mem = re.compile(r'^lfs\s+(f\d+),\s*(-?0x[0-9a-f]+|\d+)\((r\d+)\)')
fmuls_re= re.compile(r'^fmuls\s+(f\d+),\s*(f\d+),\s*(f\d+)')
deffr   = re.compile(r'^(fmuls|fadds|fsubs|fdivs|fmr|fabs|frsp|fneg|fmadds|fmsubs|fnmsubs|fnmadds|lfd|lfsx|fctiwz|fres|frsqrte|fsel|fmuls\.)\s+(f\d+)')

results = []

for path in sorted(glob.glob(os.path.join(ASMDIR, '*.s'))):
    unit = os.path.basename(path)[:-2].replace('__', '/')
    cur = None
    kind = {}   # freg -> (kindstr, detail, addr)
    for line in open(path, encoding='utf-8', errors='replace'):
        line = line.rstrip('\n')
        m = fn_re.match(line)
        if m:
            cur = m.group(1); kind = {}; continue
        if end_re.match(line):
            cur = None; continue
        m = ins_re.match(line)
        if not m or cur is None:
            continue
        addr, ins = m.group(1), m.group(2).strip()

        mm = fmuls_re.match(ins)
        if mm:
            d, a, c = mm.groups()
            ka = kind.get(a); kc = kind.get(c)
            if ka and kc:
                if ka[0] == 'MEM' and kc[0] == 'LIT':
                    results.append(('MEMBER_IN_A', unit, cur, addr, ins, ka, kc))
                elif ka[0] == 'LIT' and kc[0] == 'MEM':
                    results.append(('CONST_IN_A', unit, cur, addr, ins, ka, kc))
            kind[d] = ('OTHER', '', addr)
            continue

        mm = lfs_sda.match(ins)
        if mm:
            f, sym = mm.groups()
            sym = sym.strip('"')
            kind[f] = ('LIT' if sym.startswith('@') else 'NAMED', sym, addr)
            continue
        mm = lfs_mem.match(ins)
        if mm:
            f, off, base = mm.groups()
            kind[f] = ('MEM', '%s(%s)' % (off, base), addr)
            continue
        mm = deffr.match(ins)
        if mm:
            kind[mm.group(2)] = ('OTHER', '', addr)
            continue
        mm = re.match(r'^lfs\s+(f\d+)', ins)
        if mm:
            kind[mm.group(1)] = ('OTHER', '', addr)

from collections import Counter
c = Counter(r[0] for r in results)
print('TOTALS:', dict(c))
print()
for tag in ('MEMBER_IN_A', 'CONST_IN_A'):
    print('=' * 70)
    print(tag, c[tag])
    print('=' * 70)
    for r in results:
        if r[0] != tag: continue
        _, unit, fn, addr, ins, ka, kc = r
        print('%-38s %-52s %s  %s | A=%s %s  C=%s %s' % (unit, fn, addr, ins, ka[0], ka[1], kc[0], kc[1]))
    print()

import os, time
from pathlib import Path
from collections import Counter

txt = Path('build.ninja').read_text().split('\n')
edges = []
cur = None


def flush():
    global cur
    if cur:
        edges.append(cur)
        cur = None


for line in txt:
    if line.startswith('build '):
        flush()
        body = line[len('build '):]
        outs, rest = body.split(': ', 1)
        parts = rest.split()
        rule = parts[0]
        ins = parts[1:]
        impl = []
        if '|' in ins:
            i = ins.index('|')
            impl = ins[i + 1:]
            ins = ins[:i]
        cur = {'outs': outs.split(), 'rule': rule, 'ins': ins, 'impl': impl}
    elif line.startswith('  ') and cur is not None:
        pass
    else:
        flush()
flush()


def mt(p):
    p = p.replace(chr(92), '/')
    try:
        return os.path.getmtime(p)
    except OSError:
        return None


print('edges by rule:', dict(Counter(e['rule'] for e in edges)))
missing_out = []
missing_in = []
dirty = []
for e in edges:
    if e['rule'] in ('configure', 'decomp_context'):
        continue
    allin = e['ins'] + e['impl']
    for i in allin:
        if mt(i) is None:
            missing_in.append((e['rule'], i))
    outts = []
    for o in e['outs']:
        t = mt(o)
        if t is None:
            missing_out.append((e['rule'], o))
        else:
            outts.append(t)
    if outts and all(mt(i) is not None for i in allin):
        newest_in = max([mt(i) for i in allin] or [0])
        if newest_in > min(outts):
            dirty.append((e['rule'], e['outs'][0], time.ctime(newest_in), time.ctime(min(outts))))

print('\nMISSING INPUTS (%d):' % len(missing_in))
for r, i in missing_in[:60]:
    print('  ', r, i)
print('\nMISSING OUTPUTS (%d):' % len(missing_out))
for r, o in missing_out[:60]:
    print('  ', r, o)
print('\nDIRTY EDGES (input newer than output) (%d):' % len(dirty))
for d in dirty[:60]:
    print('  ', d)

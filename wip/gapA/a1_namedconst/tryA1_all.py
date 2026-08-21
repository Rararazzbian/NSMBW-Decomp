"""Does the named-constant shape that fixes Left30Left also fix the eight
siblings? Global substitution of 0.8910065f across the whole TU, then a
per-function verdict for every executeState_* in the file.
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'wip', 'gapA', 'gapA_all.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')
ANCHOR = 'void dLineMng_c::initializeState_Left30Left()'

SHAPES = {
    'lit':    ('', '0.8910065f'),
    'arr1':   ('static const f32 kCos27a[1] = { 0.8910065f };\n\n', 'kCos27a[0]'),
    'extern': ('extern const f32 kCos27;\n\n', 'kCos27'),
}


def parse(path):
    fns, cur = {}, None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"')
            fns[cur] = []
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns


tgt = parse(TARGET)
NAMES = sorted(n for n in tgt if n.startswith('executeState_'))

for tag, (prelude, expr) in SHAPES.items():
    src = open(BASE, encoding='utf-8').read()
    n = src.count('0.8910065f')
    src = src.replace('0.8910065f', expr)
    at = src.index(ANCHOR)
    src = src[:at] + prelude + src[at:]
    p = os.path.join(HERE, 'all_%s.cpp' % tag)
    open(p, 'w', encoding='utf-8').write(src)
    obj = os.path.join(HERE, 'all_%s.o' % tag)
    ok, log = harness.compile_draft(p, obj, extra_inc=[INC])
    if not ok:
        print(tag, 'COMPILE FAILED'); print(log[-800:]); continue
    txt = os.path.join(HERE, 'all_%s.txt' % tag)
    ok, _ = harness.disasm(obj, txt)
    d = parse(txt)
    good = []
    for fn in NAMES:
        t, dr = tgt[fn], d.get(fn)
        if dr is None:
            continue
        eq = ([b for b, _ in dr] == [b for b, _ in t] or
              harness.canonicalise([x for _, x in dr]) == harness.canonicalise([x for _, x in t]))
        good.append((fn, len(t), len(dr), eq))
    print('=== shape %-7s (%d sites)  %d/%d match' %
          (tag, n, sum(1 for g in good if g[3]), len(good)))
    for fn, lt, ld, eq in good:
        print('    %-52s %3d/%3d %s' % (fn, ld, lt, 'MATCH' if eq else '-'))

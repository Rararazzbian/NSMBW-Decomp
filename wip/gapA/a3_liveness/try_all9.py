"""Apply the winning L1c5 shape to ALL NINE 30/60-degree executeState siblings.

    mSpeed.C = mBaseSpeed * K;      ->      mSpeed.C = mBaseSpeed;
                                            mSpeed.C *= K;

and report per-function byte/canonical match against retail.
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'wip', 'gapA', 'gapA_all.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')

FNS = ['executeState_Left30Left', 'executeState_Left30Right',
       'executeState_Right30Left', 'executeState_Right30Right',
       'executeState_Left60Up', 'executeState_Left60Down',
       'executeState_Right60Down', 'executeState_Right60Up']
MANGLED = {f: f + '__10dLineMng_cFv' for f in FNS}

PAT = re.compile(r'^(\s*)mSpeed\.([xy]) = mBaseSpeed \* (-?0\.8910065f);$', re.M)


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


def main():
    src = open(BASE, encoding='utf-8').read()
    new, n = PAT.subn(lambda m: '%smSpeed.%s = mBaseSpeed;\n%smSpeed.%s *= %s;'
                      % (m.group(1), m.group(2), m.group(1), m.group(2), m.group(3)), src)
    print('rewrote %d assignment sites' % n)
    out_src = os.path.join(HERE, 'all9.cpp')
    open(out_src, 'w', encoding='utf-8').write(new)

    obj = os.path.join(HERE, 'all9.o')
    ok, log = harness.compile_draft(out_src, obj, extra_inc=[INC])
    if not ok:
        sys.exit('COMPILE FAILED\n' + log[-2000:])
    txt = os.path.join(HERE, 'all9.txt')
    ok, log = harness.disasm(obj, txt)
    if not ok:
        sys.exit('DISASM FAILED\n' + log[-2000:])

    D, T = parse(txt), parse(TARGET)
    for f in FNS:
        m = MANGLED[f]
        d, t = D.get(m), T.get(m)
        if d is None or t is None:
            print('%-32s MISSING (draft=%s target=%s)' % (f, d is not None, t is not None))
            continue
        by = [x for x, _ in d] == [x for x, _ in t]
        cn = harness.canonicalise([x for _, x in d]) == harness.canonicalise([x for _, x in t])
        tag = 'MATCH' if (by or cn) else 'differs'
        print('%-32s len %d/%d  bytes=%s canon=%s  %s'
              % (f, len(d), len(t), by, cn, tag))
        if not (by or cn):
            shown = 0
            for i in range(max(len(d), len(t))):
                a = t[i][1] if i < len(t) else '--'
                b = d[i][1] if i < len(d) else '--'
                if a != b and harness.canonicalise([a]) != harness.canonicalise([b]):
                    print('      %-4d %-38s %s' % (i, a, b))
                    shown += 1
                    if shown > 12:
                        print('      ...')
                        break


main()

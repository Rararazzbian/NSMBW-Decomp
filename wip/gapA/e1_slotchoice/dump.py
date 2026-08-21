"""Dump / diff one function's instruction stream: retail target vs a draft .txt.

Usage:
    python dump.py <fn-substring>                 # target only, full stream
    python dump.py <fn-substring> <draft.txt>     # index-aligned diff
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')


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
            mi = re.match(r'/\* ([0-9A-F]+)\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1), mi.group(2).strip(), mi.group(3).strip()))
    return fns


def pick(fns, sub):
    ks = [k for k in fns if sub in k]
    if not ks:
        raise SystemExit('no fn matching %r; have e.g. %s' % (sub, list(fns)[:5]))
    if len(ks) > 1:
        exact = [k for k in ks if k.startswith(sub)]
        ks = exact or ks
    return ks[0]


def main():
    sub = sys.argv[1]
    t = parse(TARGET)
    k = pick(t, sub)
    tgt = t[k]
    if len(sys.argv) < 3:
        print('=== TARGET %s   (%d words)' % (k, len(tgt)))
        for i, (a, b, txt) in enumerate(tgt):
            print('%4d  %s  %-8s  %s' % (i, a, b, txt))
        return
    d = parse(os.path.abspath(sys.argv[2]))
    dk = pick(d, sub)
    dft = d[dk]
    print('=== %s   target %dw  draft %dw' % (k, len(tgt), len(dft)))
    ndiff = 0
    for i in range(max(len(tgt), len(dft))):
        tw = tgt[i][1] if i < len(tgt) else '--'
        tx = tgt[i][2] if i < len(tgt) else '--'
        dw = dft[i][1] if i < len(dft) else '--'
        dx = dft[i][2] if i < len(dft) else '--'
        same = (tw == dw)
        if not same:
            ndiff += 1
        flag = '   ' if same else '>>>'
        if '--all' in sys.argv or not same:
            print('%s %4d  T %-36s | D %s' % (flag, i, tx, dx))
    print('   %d differing words' % ndiff)


if __name__ == '__main__':
    main()

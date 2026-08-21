"""Apply the winning mul-equals shape to EVERY sibling and score the whole TU.

Rewrites, throughout wip/gapA/gapA_all.cpp:
    mSpeed.x = <expr> * <lit>;      ->   mSpeed.x = <expr>;
                                         mSpeed.x *= <lit>;
only where <expr> is mBaseSpeed / -mBaseSpeed, then diffs every function that
target.txt also defines.
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'wip', 'gapA', 'gapA_all.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')

PAT = re.compile(r'^([ \t]*)mSpeed\.x = (-?mBaseSpeed) \* (-?[0-9.]+f);[ \t]*$', re.M)


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
                fns[cur].append(mi.group(2).strip())
    return fns


def norm(lines):
    mp, out = {}, []
    for l in lines:
        l = re.sub(r'("@[^"]+"|\.L_[0-9A-Fa-f]+|\.\.\.bss\.0|@[0-9]+)',
                   lambda m: mp.setdefault(m.group(0), 'S%d' % len(mp)), l)
        out.append(l)
    return out


def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else 'fixed'
    src = open(BASE, encoding='utf-8').read()
    n = 0
    if mode == 'fixed':
        def rep(m):
            global_n = m.group(1)
            return '%smSpeed.x = %s;\n%smSpeed.x *= %s;' % (
                m.group(1), m.group(2), m.group(1), m.group(3))
        src, n = PAT.subn(rep, src)
    print('rewrote %d sites (mode=%s)' % (n, mode))

    out_src = os.path.join(HERE, 'sweep_%s.cpp' % mode)
    open(out_src, 'w', encoding='utf-8').write(src)
    obj = os.path.join(HERE, 'sweep_%s.o' % mode)
    ok, log = harness.compile_draft(out_src, obj, extra_inc=[INC])
    if not ok:
        sys.exit('COMPILE FAILED\n' + log[-2000:])
    txt = os.path.join(HERE, 'sweep_%s.txt' % mode)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        sys.exit('DISASM FAILED')

    draft, target = parse(txt), parse(TARGET)
    ok_n = bad_n = 0
    for fn in sorted(target):
        if fn not in draft:
            continue
        t, d = norm(target[fn]), norm(draft[fn])
        if t == d:
            ok_n += 1
        else:
            bad_n += 1
            diffs = sum(1 for i in range(max(len(t), len(d)))
                        if (t[i] if i < len(t) else None) != (d[i] if i < len(d) else None))
            print('  MISS %-52s len %d/%d  diffs %d' % (fn, len(draft[fn]), len(target[fn]), diffs))
    print('MATCH %d / %d' % (ok_n, ok_n + bad_n))


main()

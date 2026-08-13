"""Generic variant sweeper.

sweep(fnkey, old_block, [new_block, ...]) -- for each new_block, write a copy of
the base source with old_block replaced, compile, diff ONLY fnkey, report
diff-line count.  Keeps the best.
"""
import os
import sys
import difflib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402
import b5cmp  # noqa: E402
import run as R  # noqa: E402

BASE = os.path.join(os.path.dirname(HERE), 'hb-b6.cpp')


def base_text(path=None):
    with open(path or BASE, encoding='utf-8') as fh:
        return fh.read()


def score(src_text, fnkey, tag='sw', ctx=4, quiet=True):
    """Compile src_text, return (ndiff, report). ndiff=-1 on compile failure."""
    p = os.path.join(HERE, tag + '.cpp')
    with open(p, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write(src_text)
    txt, log = R.build(p, tag)
    if txt is None:
        return -1, log
    tgt, drf = None, None
    for t, d in R.PAIRS:
        if fnkey in t or fnkey in d:
            tgt, drf = t, d
            break
    if tgt is None:
        return -2, 'no such fn %s' % fnkey
    a = b5cmp.extract_exact(R.TARGET, tgt, R.RENAME)
    b = b5cmp.extract_exact(txt, drf)
    if b is None:
        return -3, 'DRAFT MISSING %s' % drf
    if a == b:
        return 0, 'MATCH (%d insns)' % len(a)
    d = list(difflib.unified_diff(a, b, 'target', 'draft', lineterm='', n=ctx))
    n = len([l for l in d if (l.startswith('+') or l.startswith('-'))
             and not l.startswith('+++') and not l.startswith('---')])
    return n, ('target=%d draft=%d\n' % (len(a), len(b))) + '\n'.join(d)


def sweep(fnkey, old, variants, base=None, show_best=True, ctx=4):
    txt = base_text(base)
    if old not in txt:
        print('!! anchor block not found')
        return
    res = []
    for i, v in enumerate(variants):
        n, rep = score(txt.replace(old, v, 1), fnkey, tag='sw%d' % (i % 4))
        res.append((n if n >= 0 else 9999, i, v, rep))
        print('  v%-2d  %s' % (i, 'MATCH' if n == 0 else
                               ('ERR' if n < 0 else 'diff=%d' % n)))
        if n < 0:
            print(rep[-1500:])
    res.sort(key=lambda r: r[0])
    print('\nBEST: v%d  score=%s' % (res[0][1], res[0][0]))
    if show_best:
        print(res[0][2])
        print(res[0][3])
    return res

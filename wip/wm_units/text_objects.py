"""List EVERY split object covering a unit's `.text` range.

Why this exists
---------------
A per-function count is only as good as the range and the object list it was
measured on, and this project has now been bitten twice by an object nobody
passed in:

  d_a_wm_koopa_castle.cpp  reported 15/16 for a round. Its `__sinit`
                           (`fn_2_191C30`, 58 instructions) lives in
                           `auto_fn_2_191C30_text.o`, which was absent from the
                           unit's own `diffdump.py` TARGET_OBJS. Nothing built
                           on that map could see the function, so it was never
                           measured and the count silently described a range
                           smaller than the unit. It was open and differing.

  d_a_wm_ghost.cpp         reported 11/11 against one object. Its `__sinit` and
                           array destructor sit in two further objects; with all
                           three it is 13/13. The 11/11 was not wrong, it was
                           incomplete -- and incomplete in the direction that
                           flatters the unit.

dtk does not split on unit boundaries. It emits one object per contiguous run it
could resolve, so a single unit's `.text` is routinely spread over two or three
objects, and a lone function -- almost always the compiler-generated `__sinit`
-- frequently gets an `auto_fn_2_*_text.o` of its very own.

`verify_anon.py` cannot warn about this. Given a range and a list of objects it
reports on the target functions it was handed; a function in an object nobody
passed simply does not exist as far as it is concerned, and its absence looks
exactly like a smaller unit.

So run this BEFORE quoting a count, and pass every object it names.

Usage
-----
    python wip/wm_units/text_objects.py <module> <lo> <hi>

e.g.
    python wip/wm_units/text_objects.py d_basesNP 0x1910d0 0x191d40

Prints the objects to pass, a ready-to-paste `verify_anon.py` command line, and
the target functions in range that each object accounts for -- so a function
with no covering object is visible rather than silently missing.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def split_objects(module):
    """[(start_addr, path)] for every `.text` split object, sorted by address.

    Two naming forms exist and both matter: `auto_00_<ADDR>_text.o` for a normal
    run, and `auto_fn_2_<ADDR>_text.o` for a single function dtk could not merge
    into a neighbour. Matching only the first form is exactly how koopa_castle's
    `__sinit` went unmeasured.
    """
    d = os.path.join(ROOT, 'bin', 'dtkspl', module, 'obj')
    if not os.path.isdir(d):
        raise SystemExit('no split object dir: %s' % d)
    out = []
    for name in os.listdir(d):
        if not name.endswith('_text.o'):
            continue
        m = (re.fullmatch(r'auto_\d+_([0-9A-Fa-f]+)_text\.o', name)
             or re.fullmatch(r'auto_fn_2_([0-9A-Fa-f]+)_text\.o', name))
        if m:
            out.append((int(m.group(1), 16), os.path.join('bin', 'dtkspl', module, 'obj', name)))
    out.sort()
    return out


def target_functions(module, lo, hi):
    """[(addr, size, name)] of target functions in [lo, hi) from dtk's map."""
    path = os.path.join(ROOT, 'bin', 'dtk', module + '_symbols.txt')
    pat = re.compile(r'^(\S+)\s*=\s*\.text:(0x[0-9A-Fa-f]+);.*?size:(0x[0-9A-Fa-f]+)')
    out = []
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = pat.match(line.strip())
            if m:
                a = int(m.group(2), 16)
                if lo <= a < hi:
                    out.append((a, int(m.group(3), 16), m.group(1)))
    out.sort()
    return out


def main():
    if len(sys.argv) != 4:
        print(__doc__)
        return 1
    module, lo, hi = sys.argv[1], int(sys.argv[2], 0), int(sys.argv[3], 0)
    objs = split_objects(module)
    if not objs:
        raise SystemExit('no *_text.o found for %s' % module)

    # Each object covers from its own start up to the next object's start.
    spans = []
    for i, (addr, path) in enumerate(objs):
        end = objs[i + 1][0] if i + 1 < len(objs) else 1 << 32
        spans.append((addr, end, path))

    covering = [s for s in spans if s[0] < hi and s[1] > lo]
    fns = target_functions(module, lo, hi)

    print('range %#x-%#x  -- %d target function(s), %d covering object(s)\n'
          % (lo, hi, len(fns), len(covering)))

    for start, end, path in covering:
        mine = [f for f in fns if start <= f[0] < end]
        print('%s' % path)
        print('    covers %#x-%#x, %d function(s) in range%s'
              % (start, min(end, hi), len(mine),
                 ':' if mine else ' -- NONE, you may not need this one'))
        for a, sz, nm in mine:
            print('        %#010x %-24s %#x' % (a, nm, sz))

    orphans = [f for f in fns
               if not any(s[0] <= f[0] < s[1] for s in covering)]
    if orphans:
        print('\nNO COVERING OBJECT for %d function(s) -- these cannot be verified '
              'from bin/dtkspl at all:' % len(orphans))
        for a, sz, nm in orphans:
            print('    %#010x %-24s %#x' % (a, nm, sz))
        print('Read their bytes out of original/%s.rel instead, at file offset '
              '0xF0 + address.' % module)

    print('\nPass ALL of these to verify_anon.py:\n')
    print('python wip/wm_units/verify_anon.py <draft.txt> %#x %#x \\\n  %s'
          % (lo, hi, ' \\\n  '.join(s[2].replace('\\', '/') for s in covering)))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

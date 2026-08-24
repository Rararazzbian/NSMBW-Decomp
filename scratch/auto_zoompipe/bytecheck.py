"""Byte-level verification: compare raw hex bytes of every function in the
draft listing against the target listing. fndiff alone is not sufficient.

    python3 scratch/auto_zoompipe/bytecheck.py <draft.txt>
"""
import re
import sys

TARGET = ('/opt/NSMBW-Decomp/tools/auto_decomp/work/'
          'dol_bases_d_a_zoom_pipe_base/target.txt')

FNS = [
    'init__16daZoomPipeBase_cFUi',
    'execute__16daZoomPipeBase_cFv',
]

LINE = re.compile(r'^/\*\s*[0-9A-Fa-f]{8}\s+[0-9A-Fa-f]{8}\s+(([0-9A-Fa-f]{2})( [0-9A-Fa-f]{2})*)\s*\*/')


def words(path, fn):
    out, on = [], False
    for line in open(path, encoding='utf-8', errors='replace'):
        s = line.rstrip('\n')
        if s.startswith('.fn '):
            on = (s.split()[1].rstrip(',') == fn)
            continue
        if s.startswith('.endfn'):
            on = False
            continue
        if on:
            m = LINE.match(s.strip())
            if m:
                out.extend(m.group(1).split())
    return out


def main():
    draft = sys.argv[1]
    bad = 0
    for fn in FNS:
        t = words(TARGET, fn)
        d = words(draft, fn)
        if not t or not d:
            print('%-40s EXTRACT FAILED target=%d draft=%d' % (fn, len(t), len(d)))
            bad += 1
            continue
        if len(t) != len(d):
            print('%-40s LENGTH %d vs %d bytes' % (fn, len(t), len(d)))
            bad += 1
            continue
        diffs = [(i, a, b) for i, (a, b) in enumerate(zip(t, d)) if a != b]
        if diffs:
            print('%-40s %d differing bytes (of %d)' % (fn, len(diffs), len(t)))
            for i, a, b in diffs[:12]:
                print('   byte %d: target %s draft %s' % (i, a, b))
            bad += 1
        else:
            print('%-40s BYTE-IDENTICAL (%d bytes)' % (fn, len(t)))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()

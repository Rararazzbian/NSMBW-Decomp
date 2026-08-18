"""Scan every landed slice in every module for a .data object placed AFTER
the last vtable in that TU's own .data range, or a .rodata word placed after
whatever __sinit's constant pool would be (heuristically: after the last
float/double/vec-shaped object in a contiguous run at the END of the range).

This is a precedent search for: 'can source cause MWCC to emit data after
its own vtable pool / __sinit pool'.
"""
import json
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

MODULES = ['d_basesNP', 'd_en_bossNP', 'd_enemiesNP', 'd_profileNP', 'wiimj2d']

SYM_RE = re.compile(
    r'^(\S+)\s*=\s*(\.\w+):0x([0-9A-Fa-f]+);\s*//\s*(.*)$')


def load_symbols(module):
    path = os.path.join(ROOT, 'bin', 'dtk', '%s_symbols.txt' % module)
    out = {}  # section -> list of (addr, name, size, attrs, weak)
    with open(path, encoding='utf-8') as fh:
        for line in fh:
            m = SYM_RE.match(line.strip())
            if not m:
                continue
            name, sec, addr, rest = m.groups()
            addr = int(addr, 16)
            size_m = re.search(r'size:0x([0-9A-Fa-f]+)', rest)
            size = int(size_m.group(1), 16) if size_m else 0
            weak = 'weak' in rest
            out.setdefault(sec, []).append((addr, name, size, rest, weak))
    for sec in out:
        out[sec].sort()
    return out


def load_slices(module):
    path = os.path.join(ROOT, 'slices', '%s.json' % module)
    with open(path, encoding='utf-8') as fh:
        d = json.load(fh)
    return d['slices']


def main():
    for module in MODULES:
        symbols = load_symbols(module)
        slices = load_slices(module)
        for entry in slices:
            src = entry.get('source', '?')
            ranges = entry.get('memoryRanges', {})
            for sec in ('.data', '.rodata'):
                rng = ranges.get(sec)
                if not rng:
                    continue
                lo_s, hi_s = rng.split('-')
                lo, hi = int(lo_s, 16), int(hi_s, 16)
                syms = [s for s in symbols.get(sec, []) if lo <= s[0] < hi]
                if not syms:
                    continue
                syms.sort()
                if sec == '.data':
                    # find index of the LAST vtable symbol
                    vt_idx = [i for i, s in enumerate(syms) if '__vt__' in s[1]]
                    if not vt_idx:
                        continue
                    last_vt = vt_idx[-1]
                    after = syms[last_vt + 1:]
                    if after:
                        print('=== %s :: %s [%s] ===' % (module, src, sec))
                        print('  last vtable: %#x %s' % (syms[last_vt][0], syms[last_vt][1]))
                        for a in after:
                            print('  AFTER: %#x size=%#x weak=%s %s // %s' % (a[0], a[2], a[4], a[1], a[3]))
                elif sec == '.rodata':
                    # heuristic: does the range's tail include a lone small
                    # (<=8 byte) float/double object separated from a
                    # preceding run, i.e. the LAST symbol in range is not the
                    # largest, or there's a gap before it
                    pass

    print('scan complete')


if __name__ == '__main__':
    main()

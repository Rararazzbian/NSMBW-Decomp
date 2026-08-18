"""For every landed slice's .rodata range, check whether the range's
declared hi bound exceeds the end of the last symbol found within it --
i.e. trailing bytes not covered by ANY named symbol. That is the shape of
Case 2's problem: a word after the constant pool with no use in the TU.
"""
import json, os, re

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
MODULES = ['d_basesNP', 'd_en_bossNP', 'd_enemiesNP', 'd_profileNP', 'wiimj2d']
SYM_RE = re.compile(r'^(\S+)\s*=\s*(\.\w+):0x([0-9A-Fa-f]+);\s*//\s*(.*)$')

def load_symbols(module):
    path = os.path.join(ROOT, 'bin', 'dtk', '%s_symbols.txt' % module)
    out = {}
    with open(path, encoding='utf-8') as fh:
        for line in fh:
            m = SYM_RE.match(line.strip())
            if not m: continue
            name, sec, addr, rest = m.groups()
            addr = int(addr, 16)
            size_m = re.search(r'size:0x([0-9A-Fa-f]+)', rest)
            size = int(size_m.group(1), 16) if size_m else 0
            out.setdefault(sec, []).append((addr, name, size, rest))
    for sec in out:
        out[sec].sort()
    return out

def load_slices(module):
    path = os.path.join(ROOT, 'slices', '%s.json' % module)
    with open(path, encoding='utf-8') as fh:
        return json.load(fh)['slices']

def main():
    for module in MODULES:
        symbols = load_symbols(module)
        for entry in load_slices(module):
            src = entry.get('source', '?')
            ranges = entry.get('memoryRanges', {})
            rng = ranges.get('.rodata')
            if not rng:
                continue
            lo_s, hi_s = rng.split('-')
            lo, hi = int(lo_s, 16), int(hi_s, 16)
            syms = [s for s in symbols.get('.rodata', []) if lo <= s[0] < hi]
            if not syms:
                continue
            last_end = max(a + sz for a, _n, sz, _r in syms)
            if last_end < hi:
                print('%s :: %s -- gap %#x..%#x (%d bytes) after last symbol, range end %#x' %
                      (module, src, last_end, hi, hi - last_end, hi))
    print('done')

if __name__ == '__main__':
    main()

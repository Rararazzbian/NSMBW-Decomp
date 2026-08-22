"""Refinement of weak_scan.py: add the condition that actually makes the hazard bite.

AGENT_CONTEXT's rule has TWO halves:
  (a) the weak symbol's only other definition lives in an un-landed TU, AND
  (b) the landed unit REFERENCES it.

Unreferenced weak symbols are never placed (AGENT_CONTEXT, "Unreferenced weak
symbols are not placed by the linker"), so (b) is what separates a real
placement from the normal, benign .text over-claim.

This adds (b): a weak symbol counts only if some .rela* entry in the SAME object
names it.
"""
import json
import os
import re
import struct
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
STB_WEAK = 2
SHN_UNDEF = 0


def parse(path):
    b = open(path, 'rb').read()
    e_shoff, = struct.unpack_from('>I', b, 0x20)
    e_shentsize, e_shnum, e_shstrndx = struct.unpack_from('>HHH', b, 0x2E)
    secs = []
    for i in range(e_shnum):
        o = e_shoff + i * e_shentsize
        f = struct.unpack_from('>10I', b, o)
        secs.append(dict(name=f[0], type=f[1], off=f[4], size=f[5], link=f[6], entsize=f[9]))
    syms = []
    symidx_name = {}
    for si, s in enumerate(secs):
        if s['type'] != 2:
            continue
        strtab = secs[s['link']]
        for i in range(s['size'] // 16):
            o = s['off'] + i * 16
            st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack_from('>IIIBBH', b, o)
            end = b.index(b'\0', strtab['off'] + st_name)
            nm = b[strtab['off'] + st_name:end].decode('utf-8', 'replace')
            syms.append((nm, st_info >> 4, st_shndx))
            symidx_name[i] = nm
    referenced = set()
    for s in secs:
        if s['type'] != 4:          # SHT_RELA
            continue
        n = s['size'] // 12
        for i in range(n):
            o = s['off'] + i * 12
            r_offset, r_info, r_addend = struct.unpack_from('>III', b, o)
            si = r_info >> 8
            if si in symidx_name:
                referenced.add(symidx_name[si])
    return syms, referenced


def load_symbol_map(module):
    path = os.path.join(ROOT, 'bin', 'dtk', module + '_symbols.txt')
    pat = re.compile(r'^(\S+)\s*=\s*(\.\w+):(0x[0-9A-Fa-f]+);(?:.*size:(0x[0-9A-Fa-f]+))?')
    out = defaultdict(list)
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = pat.match(line.strip())
            if m:
                out[m.group(1)].append((m.group(2), int(m.group(3), 16)))
    return out


for module in ['wiimj2d', 'd_profileNP', 'd_basesNP', 'd_enemiesNP', 'd_en_bossNP']:
    sf = json.load(open(os.path.join(ROOT, 'slices', module + '.json'), encoding='utf-8'))
    meta = sf['meta']['sections']
    base = 0
    if sf['meta']['type'] == 'DOL':
        a = meta['.text']['addr']
        base = int(a, 16) if isinstance(a, str) else a
    claims = {}
    for s in sf['slices']:
        r = s.get('memoryRanges', {}).get('.text')
        if r:
            lo, hi = (int(x, 16) for x in r.split('-'))
            claims[s['source']] = (lo + base, hi + base)

    provider = defaultdict(list)
    refs = {}
    for s in sf['slices']:
        src = s['source']
        obj = os.path.join(ROOT, 'bin', 'compiled', module,
                           os.path.splitext(src)[0].replace('/', os.sep) + '.o')
        if not os.path.exists(obj):
            continue
        syms, referenced = parse(obj)
        refs[src] = referenced
        for nm, bind, shndx in syms:
            if bind == STB_WEAK and shndx != SHN_UNDEF and nm:
                provider[nm].append(src)

    symmap = load_symbol_map(module)
    hits = []
    for nm, srcs in provider.items():
        if len(srcs) != 1:
            continue
        src = srcs[0]
        if nm not in refs.get(src, ()):
            continue                       # unreferenced weak -> never placed
        ents = [a for sec, a in symmap.get(nm, []) if sec == '.text']
        if not ents:
            continue
        lo, hi = claims.get(src, (0, 0))
        if all(not (lo <= a < hi) for a in ents):
            hits.append((nm, src, ents[0], lo, hi))
    print('=' * 74)
    print('MODULE %s' % module)
    print('  weak-DEFINED symbols: %d ; sole landed provider: %d ; '
          'sole provider AND self-referenced AND retail places it outside the claim: %d'
          % (len(provider), sum(1 for v in provider.values() if len(v) == 1), len(hits)))
    for nm, src, a, lo, hi in sorted(hits):
        print('    RISK %-58s %s  retail@%#x  claim %#x-%#x' % (nm[:58], src, a, lo, hi))

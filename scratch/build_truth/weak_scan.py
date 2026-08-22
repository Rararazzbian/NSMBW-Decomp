"""Scan the LANDED set for the AGENT_CONTEXT hazard:

  a landed unit REFERENCES a weak inline function whose ONLY other definition
  lives in an UN-LANDED TU, so the linker has nothing to weak-dedupe against,
  the unit's own copy is PLACED, and every downstream byte shifts.

Method (all read-only, no build):
  1. Parse the .symtab of every compiled object listed in build.ninja.
  2. Collect symbols that are WEAK and DEFINED (st_shndx != SHN_UNDEF).
  3. For each weak symbol, count how many landed objects in that MODULE define
     it -- the linker can only dedupe among linkable objects, i.e. the landed
     ones (fillers are opaque byte blobs).
  4. Look the symbol up in the module's dtk symbol map.  If retail places it at
     an address that lies OUTSIDE the defining unit's own claimed .text range,
     the retail copy belongs to a different (un-landed) TU -> the landed unit's
     own copy has to be placed somewhere it does not own -> RISK.
"""
import json
import os
import re
import struct
import sys
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

STB_WEAK = 2
STB_GLOBAL = 1
SHN_UNDEF = 0


def read_syms(path):
    b = open(path, 'rb').read()
    if b[:4] != b'\x7fELF':
        return []
    e_shoff, = struct.unpack_from('>I', b, 0x20)
    e_shentsize, e_shnum, e_shstrndx = struct.unpack_from('>HHH', b, 0x2E)
    secs = []
    for i in range(e_shnum):
        o = e_shoff + i * e_shentsize
        name, typ, flags, addr, off, size, link, info, align, entsize = struct.unpack_from('>10I', b, o)
        secs.append(dict(name=name, type=typ, off=off, size=size, link=link, entsize=entsize))
    out = []
    for s in secs:
        if s['type'] != 2:          # SHT_SYMTAB
            continue
        strtab = secs[s['link']]
        n = s['size'] // 16
        for i in range(n):
            o = s['off'] + i * 16
            st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack_from('>IIIBBH', b, o)
            end = b.index(b'\0', strtab['off'] + st_name)
            nm = b[strtab['off'] + st_name:end].decode('utf-8', 'replace')
            out.append((nm, st_info >> 4, st_info & 0xf, st_shndx, st_value, st_size))
    return out


def load_symbol_map(module):
    path = os.path.join(ROOT, 'bin', 'dtk', module + '_symbols.txt')
    pat = re.compile(r'^(\S+)\s*=\s*(\.\w+):(0x[0-9A-Fa-f]+);(?:.*size:(0x[0-9A-Fa-f]+))?')
    out = {}
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = pat.match(line.strip())
            if m:
                out.setdefault(m.group(1), []).append(
                    (m.group(2), int(m.group(3), 16), int(m.group(4), 16) if m.group(4) else 0))
    return out


sys.path.insert(0, os.path.join(ROOT, 'tools'))

MODULES = {
    'wiimj2d': 'wiimj2d',
    'd_profileNP': 'd_profileNP',
    'd_basesNP': 'd_basesNP',
    'd_enemiesNP': 'd_enemiesNP',
    'd_en_bossNP': 'd_en_bossNP',
}

for module in MODULES:
    slicefile = json.load(open(os.path.join(ROOT, 'slices', module + '.json'), encoding='utf-8'))
    meta = slicefile['meta']['sections']
    is_dol = slicefile['meta']['type'] == 'DOL'
    textbase = 0
    if is_dol:
        a = meta['.text']['addr']
        textbase = int(a, 16) if isinstance(a, str) else a

    claims = {}
    for s in slicefile['slices']:
        r = s.get('memoryRanges', {}).get('.text')
        if r:
            lo, hi = (int(x, 16) for x in r.split('-'))
            claims[s['source']] = (lo + textbase, hi + textbase)

    provider = defaultdict(list)      # weak sym -> [source]
    for s in slicefile['slices']:
        src = s['source']
        obj = os.path.join(ROOT, 'bin', 'compiled', module,
                           os.path.splitext(src)[0].replace('/', os.sep) + '.o')
        if not os.path.exists(obj):
            print('  !! missing object', obj)
            continue
        for nm, bind, typ, shndx, val, size in read_syms(obj):
            if bind == STB_WEAK and shndx != SHN_UNDEF and nm:
                provider[nm].append(src)

    symmap = load_symbol_map(module)
    sole = {k: v[0] for k, v in provider.items() if len(v) == 1}
    risk = []
    unknown = 0
    for nm, src in sorted(sole.items()):
        ents = symmap.get(nm)
        if not ents:
            unknown += 1
            continue
        lo, hi = claims.get(src, (0, 0))
        for sec, addr, size in ents:
            if sec != '.text':
                continue
            if not (lo <= addr < hi):
                risk.append((nm, src, addr, lo, hi))
    print('=' * 74)
    print('MODULE %s: %d landed objects, %d distinct weak DEFINED symbols, '
          '%d with a SOLE landed provider' % (module, len(claims), len(provider), len(sole)))
    print('   %d sole-provider weak symbols are not named anywhere in the dtk map '
          '(deadstripped in retail, or an unnamed local) -- not checkable, not a risk signal'
          % unknown)
    if risk:
        print('   *** %d sole-provider weak symbols that retail places OUTSIDE the defining '
              'unit\'s .text claim:' % len(risk))
        for nm, src, addr, lo, hi in risk[:40]:
            print('       %-60s  %s  retail@%#x  claim %#x-%#x' % (nm[:60], src, addr, lo, hi))
    else:
        print('   no sole-provider weak symbol is placed outside its defining unit\'s claim.')

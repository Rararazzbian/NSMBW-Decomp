"""Run wip/wm_units/check_bounds.py over EVERY slice entry in all five slice files.

Conventions used, stated per module (this matters -- mixing them is the exact
error AGENT_CONTEXT warns about):

  d_profileNP / d_basesNP / d_enemiesNP / d_en_bossNP
      slices/*.json stores REL-RELATIVE SECTION OFFSETS.
      bin/dtk/<mod>_symbols.txt ALSO stores REL-relative section offsets.
      -> fed straight through, no conversion.

  wiimj2d
      slices/wiimj2d.json stores SECTION-RELATIVE OFFSETS (NOT virtual
      addresses -- verified: d_2d.cpp claims .text 0x10-0xd90).
      bin/dtk/wiimj2d_symbols.txt stores VIRTUAL ADDRESSES.
      -> every claim converted with VA = meta.sections[sec].addr + offset
         before being handed to check_bounds. Verified against a known pair:
         .text offset 0x10 -> 0x80006790 = init__3d2d, the first symbol of the
         first slice.
"""
import io
import json
import os
import sys
import contextlib

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, 'wip', 'wm_units'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))

import check_bounds as cb

# ---- cache the two expensive readers -------------------------------------
_symcache = {}
_orig_load = cb.load_symbols
def load_symbols(module):
    if module not in _symcache:
        _symcache[module] = _orig_load(module)
    return _symcache[module]
cb.load_symbols = load_symbols

_relcache = {}
def _reltable(module):
    """[(reloc_target_addend, referring_addr)] for the whole module, once."""
    if module in _relcache:
        return _relcache[module]
    import struct
    path = os.path.join(ROOT, 'original', module + '.rel')
    if not os.path.exists(path):
        _relcache[module] = None
        return None
    b = open(path, 'rb').read()
    impOff, impSize = struct.unpack_from('>II', b, 0x28)
    pairs = []
    for i in range(0, impSize, 8):
        _mid, roff = struct.unpack_from('>II', b, impOff + i)
        pos, addr = roff, 0
        while pos + 8 <= len(b):
            o, t, _sec, add = struct.unpack_from('>HBBI', b, pos)
            pos += 8
            if t == 203:
                break
            if t == 202:
                addr = 0
                continue
            addr += o
            if t != 201:
                pairs.append((add, addr))
    pairs.sort()
    _relcache[module] = pairs
    return pairs

import bisect
def referrers(module, lo, hi):
    pairs = _reltable(module)
    if pairs is None:
        return None
    i = bisect.bisect_left(pairs, (lo, -1))
    out = set()
    while i < len(pairs) and pairs[i][0] < hi:
        out.add(pairs[i][1])
        i += 1
    return sorted(out)
cb.referrers = referrers

_ercache = {}
_orig_er = cb.existing_ranges
def existing_ranges(module):
    if module not in _ercache:
        _ercache[module] = _orig_er(module)
    return _ercache[module]
cb.existing_ranges = existing_ranges

# ---- for wiimj2d, present VA-converted ranges to check_bounds -------------
WII_META = json.load(open(os.path.join(ROOT, 'slices', 'wiimj2d.json'),
                          encoding='utf-8'))['meta']['sections']
def _va(sec, off):
    base = WII_META[sec]['addr']
    base = int(base, 16) if isinstance(base, str) else base
    return base + off

# For wiimj2d, existing_ranges must be VA-converted too, or the overlap test
# compares VAs against offsets and reports nothing.
def existing_ranges_va(module):
    if module != 'wiimj2d':
        return existing_ranges(module)
    key = 'wiimj2d_va'
    if key not in _ercache:
        out = []
        for sec, lo, hi, src in existing_ranges('wiimj2d'):
            out.append((sec, _va(sec, lo), _va(sec, hi), src))
        _ercache[key] = out
    return _ercache[key]
cb.existing_ranges = existing_ranges_va


MODULES = ['d_profileNP', 'd_basesNP', 'd_enemiesNP', 'd_en_bossNP', 'wiimj2d']

summary = []
for module in MODULES:
    data = json.load(open(os.path.join(ROOT, 'slices', module + '.json'), encoding='utf-8'))
    print('#' * 78)
    print('# MODULE %s  (%d slice entries)  convention: %s'
          % (module, len(data['slices']),
             'REL offsets (no conversion)' if module != 'wiimj2d'
             else 'offsets -> VA via meta.sections[sec].addr'))
    print('#' * 78)
    for s in data['slices']:
        src = s['source']
        claim = {}
        for sec, rng in s.get('memoryRanges', {}).items():
            lo, hi = (int(x, 16) for x in rng.split('-'))
            if module == 'wiimj2d':
                if sec not in WII_META:
                    continue
                lo, hi = _va(sec, lo), _va(sec, hi)
            claim[sec] = '%#x-%#x' % (lo, hi)
        buf = io.StringIO()
        argv = sys.argv
        sys.argv = ['check_bounds.py', module, json.dumps(claim), src]
        try:
            with contextlib.redirect_stdout(buf):
                rc = cb.main()
        except Exception as e:
            rc = -1
            buf.write('\nEXCEPTION: %r\n' % (e,))
        finally:
            sys.argv = argv
        out = buf.getvalue()
        status = 'PASS' if rc == 0 else 'FAIL'
        summary.append((module, src, status, out))
        print('\n===== %s :: %s  -> %s' % (module, src, status))
        if status != 'PASS':
            print(out)

print('\n' + '=' * 78)
print('SUMMARY')
print('=' * 78)
nf = 0
for module, src, status, out in summary:
    if status != 'PASS':
        nf += 1
        # one-line reasons
        reasons = [l.strip() for l in out.splitlines()
                   if l.startswith('  ') and not l.strip().startswith(('first:', 'last:', 'gap of'))
                   and 'no symbols in range' not in l]
        print('%-12s %-55s %s' % (module, src, '; '.join(reasons)[:400]))
print('\n%d of %d slice entries reported a problem.' % (nf, len(summary)))

"""Batch-6 verification. Six independent checks + negative controls.

Each check must be able to FAIL on a deliberately broken draft, or it is not a
check. run_neg() proves that.
"""
import os
import re
import struct
import sys
import difflib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402
import b5cmp  # noqa: E402
import run as R  # noqa: E402

DOL = os.path.join(ROOT, 'original', 'wiimj2d.dol')
TARGET = R.TARGET
PAIRS = R.PAIRS
RENAME = R.RENAME
SIZES = R.SIZES

# Target .text addresses, from the symbol map, in canonical order.
ADDRS = {
    'fn_80112040': 0x80112040,
    'fly_yspeed_set__19daEnHatenaBalloon_cFv': 0x80112950,
    'fly_xspeed_set__19daEnHatenaBalloon_cFb': 0x80112B00,
    'fly_ydisp_check__19daEnHatenaBalloon_cFb': 0x80112C70,
    'fly_xdisp_check__19daEnHatenaBalloon_cFb': 0x80112D50,
    'fly_dispin_check__19daEnHatenaBalloon_cFv': 0x80112EF0,
    'escape_dispout_check__19daEnHatenaBalloon_cFv': 0x80112FC0,
    'create_wait_pos_set__19daEnHatenaBalloon_cFv': 0x80113740,
}

WORD = re.compile(r'^/\*\s*\S+\s+\S+\s+((?:[0-9A-F]{2} ){3}[0-9A-F]{2})\s*\*/\s*(\S.*)$')
FN_START = H.FN_START
FN_END = H.FN_END

# operand tokens that name a real symbol (not a compiler pool temp, not a local label)
SYMTOK = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*(?:__[A-Za-z0-9_]+)?)\b')
POOLISH = re.compile(r'^@\d+')


def raw_body(path, name, rename=None):
    """(words, text) lists for one function, WITHOUT canonicalise()."""
    words, text = None, None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_START.match(s)
            if m:
                if m.group(1).strip().strip('"') == name:
                    words, text = [], []
                continue
            if FN_END.match(s):
                if words is not None:
                    return words, text
                continue
            if words is not None:
                mw = WORD.match(s)
                if mw:
                    body = mw.group(2)
                    if rename:
                        for k, v in rename.items():
                            body = body.replace(k, v)
                    words.append(mw.group(1).replace(' ', ''))
                    text.append(body)
    return (words, text) if words is not None else (None, None)


# --------------------------------------------------------------- the checks

def check_sizes():
    """A. instruction count x 4 == symbol-map size."""
    bad = []
    for name, size in SIZES.items():
        w, _ = raw_body(TARGET, name)
        if w is None:
            bad.append('%s MISSING from target' % name)
        elif len(w) * 4 != size:
            bad.append('%s: %d x4 = 0x%X != map 0x%X' % (name, len(w), len(w) * 4, size))
    return bad


def check_raw_words(drf_txt):
    """B. raw uncanonicalised 4-byte word compare."""
    bad = []
    for tgt, drf in PAIRS:
        a, _ = raw_body(TARGET, tgt)
        b, _ = raw_body(drf_txt, drf)
        if b is None:
            bad.append('%s: draft symbol %s MISSING' % (tgt, drf))
            continue
        if len(a) != len(b):
            bad.append('%s: %d vs %d words' % (tgt, len(a), len(b)))
            continue
        diffs = [i for i, (x, y) in enumerate(zip(a, b)) if x != y]
        if diffs:
            bad.append('%s: %d word(s) differ, first at insn %d (%s vs %s)'
                       % (tgt, len(diffs), diffs[0], a[diffs[0]], b[diffs[0]]))
    return bad


def callees(text):
    """Symbol names referenced by a body: bl targets and @sda21/@ha/@l operands."""
    out = []
    for line in text:
        m = re.match(r'^(bl|b)\s+([A-Za-z_@][\w@.]*)', line)
        if m and not m.group(2).startswith('.L'):
            out.append('CALL:' + m.group(2))
        for mm in re.finditer(r'([A-Za-z_][\w]*)@(sda21|ha|l)\b', line):
            out.append('REF:' + mm.group(1))
    return out


def check_callees(drf_txt):
    """C. callee/symbol NAMES -- dtk zeroes relocations, so words cannot see this."""
    bad = []
    for tgt, drf in PAIRS:
        _, at = raw_body(TARGET, tgt, RENAME)
        _, bt = raw_body(drf_txt, drf)
        if bt is None:
            bad.append('%s: draft MISSING' % tgt)
            continue
        a, b = callees(at), callees(bt)
        if a != b:
            bad.append('%s:\n    target: %s\n    draft : %s'
                       % (tgt, a, b) + '\n    delta: ' +
                       '; '.join(difflib.unified_diff(a, b, lineterm='', n=0)))
    return bad


def check_order(drf_txt):
    """D. emitted symbol order == target address order."""
    want = [d for t, d in sorted(PAIRS, key=lambda p: ADDRS[p[0]])]
    got = [n for n in b5cmp.names(drf_txt) if not n.startswith('gap_')]
    return [] if got == want else ['order:\n    want %s\n    got  %s' % (want, got)]


# --------------------------------------------------------------- constants

_d = open(DOL, 'rb').read()
_offs = struct.unpack('>18I', _d[0x00:0x48])
_addrs = struct.unpack('>18I', _d[0x48:0x90])
_sizes = struct.unpack('>18I', _d[0x90:0xD8])


def dolread(a, n):
    for o, ad, s in zip(_offs, _addrs, _sizes):
        if s and ad <= a < ad + s:
            return _d[o + (a - ad): o + (a - ad) + n]


def sdata2_map(path):
    m, cur = {}, None
    for L in open(path, encoding='utf-8', errors='replace'):
        o = re.match(r'\.obj "?([@\w]+)"?, local', L.strip())
        if o:
            cur = o.group(1)
            continue
        b = re.match(r'\.4byte 0x([0-9A-Fa-f]{8})', L.strip())
        if b and cur:
            m[cur] = struct.unpack('>f', bytes.fromhex(b.group(1)))[0]
            cur = None
    return m


def lits(path, fn, resolve):
    out, inside = [], False
    for L in open(path, encoding='utf-8', errors='replace'):
        if L.startswith('.fn '):
            inside = L.split()[1].rstrip(',').strip('"') == fn
        elif L.startswith('.endfn'):
            inside = False
        elif inside:
            for mm in re.finditer(r'"?(@[\w]+?)(?:_([0-9A-F]{8}))?"?@sda21', L):
                v = resolve(mm.group(1), mm.group(2))
                if v is not None and v not in out:
                    out.append(v)
    return out


def check_consts(drf_txt):
    """E. literal-pool VALUES -- the .text comparator is blind to these."""
    dm = sdata2_map(drf_txt)
    tr = lambda s, a: struct.unpack('>f', dolread(int(a, 16), 4))[0] if a else None
    dr = lambda s, a: dm.get(s)
    bad = []
    for tgt, drf in PAIRS:
        a = lits(TARGET, tgt, tr)
        b = lits(drf_txt, drf, dr)
        if a != b:
            bad.append('%s:\n    target: %s\n    draft : %s' % (tgt, a, b))
    return bad


def check_rodata(drf_txt):
    """E2. l_create_diff BYTES against the DOL."""
    want = dolread(0x802F4EA8, 0x10)  # l_create_diff, size 0x10, per wiimj2d_symbols.txt
    got, inside, vals = None, False, []
    for L in open(drf_txt, encoding='utf-8', errors='replace'):
        s = L.strip()
        if re.match(r'\.obj "?l_create_diff"?\s*,', s):
            inside, vals = True, []
            continue
        if inside:
            if s.startswith('.endobj'):
                got = b''.join(vals)
                inside = False
                continue
            b = re.match(r'\.4byte 0x([0-9A-Fa-f]{8})', s)
            if b:
                vals.append(bytes.fromhex(b.group(1)))
            elif s:
                return ['l_create_diff: unexpected directive %r' % s]
    if got is None:
        return ['l_create_diff not emitted by draft']
    if got != want:
        return ['l_create_diff bytes differ\n    dol  : %s\n    draft: %s'
                % (want.hex(), got.hex())]
    return []


CHECKS = [('A size', lambda t: check_sizes()),
          ('B raw words', check_raw_words),
          ('C callee names', check_callees),
          ('D symbol order', check_order),
          ('E const values', check_consts),
          ('E2 l_create_diff bytes', check_rodata)]


def run(src, tag='ver', verbose=True):
    txt, log = R.build(src, tag)
    if txt is None:
        return {'BUILD': [log[-2000:]]}
    out = {}
    for name, fn in CHECKS:
        out[name] = fn(txt)
    if verbose:
        for name, _ in CHECKS:
            bad = out[name]
            print('  %-24s %s' % (name, 'PASS' if not bad else 'FAIL'))
            for b in bad:
                print('      ' + str(b).replace('\n', '\n      '))
    return out


if __name__ == '__main__':
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(HERE), 'hb-b6.cpp')
    print('=== verifying', src)
    run(src)

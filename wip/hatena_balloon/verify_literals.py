"""Compare the DATA a function references, not just its instruction words.

The .text comparator canonicalises every pool reference to a positional marker,
so it cannot see a literal's value at all: two functions each loading one float
compare equal whatever those floats are.  This resolves each reference on both
sides -- target symbol -> DOL address, draft symbol -> emitted object bytes --
and compares the bytes.  It also checks the target<->draft symbol mapping is a
BIJECTION, which is what makes the "only the numbering differs" claim testable
rather than assumed.
"""
import sys, os, re, struct

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
S = r'C:\Users\Razz\AppData\Local\Temp\claude\C--Users-Razz-Documents-Projects-NSMBW-Decomp\a82a73ff-4c16-4614-ab34-6dd919c467b3\scratchpad\hb2'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
os.chdir(ROOT)
import harness

DOL = os.path.join(ROOT, 'original', 'wiimj2d.dol')
_d = open(DOL, 'rb').read()
_offs = struct.unpack('>18I', _d[0:72])
_addrs = struct.unpack('>18I', _d[72:144])
_sizes = struct.unpack('>18I', _d[144:216])


def dol_read(va, n):
    for o, a, s in zip(_offs, _addrs, _sizes):
        if s and a <= va < a + s:
            return _d[o + (va - a): o + (va - a) + n]
    return None


SYMS = {}
for line in open(os.path.join(ROOT, 'syms.txt'), encoding='utf-8', errors='replace'):
    m = re.match(r'^([\w@$.]+)\s*=\s*(0x[0-9A-Fa-f]+)', line.strip())
    if m:
        SYMS[m.group(1)] = int(m.group(2), 16)

FN = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')
SYMREF = re.compile(r'(?:"([^"]+)"|([A-Za-z_.][\w.$]*))@(?:sda21|ha|l)\b')
LOCAL_L = re.compile(r'\.L_[0-9A-Fa-f]+')


def raw_body(path, name):
    cur, body = None, None
    for line in open(path, encoding='utf-8', errors='replace'):
        t = line.rstrip()
        m = FN.match(t)
        if m:
            cur = re.sub(r'_[0-9A-Fa-f]{8}$', '', m.group(1))
            body = [] if cur == name else None
            continue
        if t.startswith('.endfn'):
            if cur == name and body is not None:
                return body
            cur, body = None, None
            continue
        mi = INSN.match(t)
        if mi and body is not None:
            body.append(mi.group(1))
    return None


def skeleton(line):
    syms = []

    def sub(m):
        syms.append(m.group(1) or m.group(2))
        return '%S%@'
    return LOCAL_L.sub('.L', SYMREF.sub(sub, line)), syms


def parse_objects(path):
    """symbol -> (section, offset, bytes, mask, relocs), plus definition order.

    A `.4byte <symbol>` line is a RELOCATION: the object holds four zero bytes
    and the linker fills the address in.  Counting it as zero bytes silently
    shifted every following object by four and turned the whole .data compare
    into garbage, so it is recorded as four masked-out bytes plus the referenced
    symbol, which lets the caller chase the pointer through the DOL instead.
    """
    objs, order = {}, []
    sec, off, cur, buf = None, 0, None, bytearray()
    mask, relocs = bytearray(), {}
    for line in open(path, encoding='utf-8', errors='replace'):
        t = line.strip()
        m = re.match(r'^\.section\s+([\w.]+)', t)
        if m:
            sec = m.group(1)
            continue
        m = re.match(r'^\.(data|rodata|sdata2|sdata|text|bss)\s*$', t)
        if m:
            sec = '.' + m.group(1)
            continue
        m = re.match(r'^#\s*\.(\w+):0x([0-9A-Fa-f]+)\s*\|\s*size:', t)
        if m:
            off = int(m.group(2), 16)
            continue
        m = re.match(r'^\.obj\s+"?([^",]+)"?\s*,', t)
        if m:
            cur, buf, mask, relocs = m.group(1), bytearray(), bytearray(), {}
            continue
        if t.startswith('.endobj') and cur is not None:
            objs[cur] = (sec, off, bytes(buf), bytes(mask), relocs)
            order.append(cur)
            cur = None
            continue
        m = re.match(r'^\.sym\s+"?([^",]+)"?\s*,', t)
        if m:
            objs.setdefault(m.group(1), (sec, off, b'', b'', {}))
            continue
        if cur is not None:
            m = re.match(r'^\.4byte\s+"?([A-Za-z_.@][^",]*?)"?\s*$', t)
            if m and not m.group(1).startswith('0x'):
                relocs[len(buf)] = m.group(1)
                buf += b'\0\0\0\0'
                mask += b'\0\0\0\0'
                continue
            for w in re.findall(r'0x([0-9A-Fa-f]+)', t):
                b = int(w, 16).to_bytes((len(w) + 1) // 2, 'big')
                buf += b
                mask += b'\xff' * len(b)
    return objs, order


def target_addr(sym):
    m = re.match(r'^@\d+_([0-9A-Fa-f]{8})$', sym) or re.match(r'^lbl_([0-9A-Fa-f]{8})$', sym)
    if m:
        return int(m.group(1), 16)
    return SYMS.get(sym)


def run(objtxt, tgt, fns, quiet=False):
    objs, order = parse_objects(objtxt)
    fwd, bad = {}, []
    for name in fns:
        t, d = raw_body(tgt, name), raw_body(objtxt, name)
        if t is None or d is None:
            bad.append('%s: MISSING BODY' % name)
            continue
        if len(t) != len(d):
            bad.append('%s: length %d vs %d' % (name, len(d), len(t)))
            continue
        for i, (a, b) in enumerate(zip(t, d)):
            sa, la = skeleton(a)
            sb, lb = skeleton(b)
            if sa != sb or len(la) != len(lb):
                bad.append('%s[%d]: %r vs %r' % (name, i, a, b))
                continue
            for x, y in zip(la, lb):
                if fwd.setdefault(y, x) != x:
                    bad.append('%s[%d]: draft %s maps to both %s and %s' % (name, i, y, fwd[y], x))
    rev = {}
    for y, x in fwd.items():
        if x in rev:
            bad.append('NOT A BIJECTION: target %s <- draft %s and %s' % (x, rev[x], y))
        rev[x] = y

    def masked_cmp(got, data, mask):
        if got is None or len(got) != len(data):
            return False
        return all(g == d for g, d, k in zip(got, data, mask) if k)

    checked = 0
    for dsym, tsym in sorted(fwd.items(), key=lambda kv: kv[1]):
        if dsym.startswith('...'):
            continue
        if dsym not in objs:
            if dsym == tsym:
                if not quiet:
                    print('  --  %-26s external, name identical on both sides' % tsym)
                continue
            bad.append('draft symbol %s not in object data' % dsym)
            continue
        sec, off, data, mask, relocs = objs[dsym]
        va = target_addr(tsym)
        if va is None:
            bad.append('cannot resolve target address of %s' % tsym)
            continue
        if not data:
            bad.append('VACUOUS: %s -> %s has no comparable bytes' % (dsym, tsym))
            continue
        got = dol_read(va, len(data))
        if not masked_cmp(got, data, mask):
            bad.append('LITERAL MISMATCH %s -> %s @%08X: dol %s != obj %s'
                       % (dsym, tsym, va, got.hex() if got else None, data.hex()))
            checked += 1
            continue
        # A relocated word is zero in the object; chase it through the DOL and
        # compare what it POINTS AT, otherwise the entry is checked vacuously.
        detail = data.hex()
        for roff, rsym in sorted(relocs.items()):
            ptr = struct.unpack('>I', got[roff:roff + 4])[0]
            if rsym in objs and objs[rsym][2]:
                want = objs[rsym][2]
                deref = dol_read(ptr, len(want))
                if deref != want:
                    bad.append('POINTER MISMATCH %s+0x%x -> %08X: dol %s != obj %s (%s)'
                               % (tsym, roff, ptr, deref.hex() if deref else None, want.hex(), rsym))
                else:
                    detail += ' [+0x%x -> %08X = %s ok]' % (roff, ptr, want.rstrip(b'\0').decode('latin1'))
            else:
                detail += ' [+0x%x -> %08X = %s, unresolved]' % (roff, ptr, rsym)
        if not quiet:
            print('  ok  %-26s @%08X %-8s = %s' % (tsym, va, sec, detail))
        checked += 1

    base_t = None
    for y, x in fwd.items():
        if y.startswith('...data'):
            base_t = x
    if base_t is not None:
        tva = target_addr(base_t)
        end = 0
        for n in order:
            sec, off, data, mask, relocs = objs[n]
            if sec != '.data':
                continue
            if n.startswith('__vt__'):
                break
            end = off + len(data)
        end = (end + 3) & ~3
        ours, omask = bytearray(end), bytearray(end)
        for n in order:
            sec, off, data, mask, relocs = objs[n]
            if sec == '.data' and off < end:
                ours[off:off + len(data)] = data
                omask[off:off + len(mask)] = mask
        theirs = dol_read(tva, end)
        if theirs is None or any(g != d for g, d, k in zip(theirs, ours, omask) if k):
            bad.append('STRING BASE MISMATCH @%08X\n    dol %s\n    obj %s'
                       % (tva, theirs.hex() if theirs else None, bytes(ours).hex()))
        elif not quiet:
            nmask = sum(1 for k in omask if not k)
            print('  ok  %-26s @%08X %-8s = 0x%x bytes identical (%d relocated bytes masked)'
                  % (base_t, tva, '.data', end, nmask))
        checked += 1
    return checked, bad


FNS = ['preDraw__19daEnHatenaBalloon_cFv',
       'draw__19daEnHatenaBalloon_cFv',
       'model_set__19daEnHatenaBalloon_cFv',
       'item_draw_calc__19daEnHatenaBalloon_cFP7mVec3_c',
       'anm_set__19daEnHatenaBalloon_cFi']

if __name__ == '__main__':
    src = sys.argv[1]
    quiet = '--quiet' in sys.argv
    obj, txt = os.path.join(S, 'lv.o'), os.path.join(S, 'lv.txt')
    ok, out = harness.compile_draft(src, obj, extra_inc=(os.path.join(S, 'inc'),))
    if not ok:
        print('COMPILE FAILED\n', out[-3000:])
        sys.exit(2)
    harness.disasm(obj, txt)
    n, bad = run(txt, os.path.join(S, 'target.txt'), FNS, quiet)
    print('\n%d data references checked against original/wiimj2d.dol' % n)
    if bad:
        print('FAILURES:')
        for b in bad:
            print('  ' + b)
        sys.exit(1)
    print('ALL LITERALS, STRINGS AND SYMBOL MAPPINGS VERIFIED')

"""Verify literal-pool VALUES per function -- the .text comparator cannot see them."""
import re, struct, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DOL = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol'
d = open(DOL, 'rb').read()
offs = struct.unpack('>18I', d[0x00:0x48]); addrs = struct.unpack('>18I', d[0x48:0x90]); sizes = struct.unpack('>18I', d[0x90:0xD8])
def dolread(a, n):
    for o, ad, s in zip(offs, addrs, sizes):
        if s and ad <= a < ad + s:
            return d[o + (a - ad): o + (a - ad) + n]
def sdata2_map(path):
    """Parse '.obj \"@N\", local' followed by .4byte -> value."""
    m = {}; cur = None
    for L in open(path, encoding='utf-8', errors='replace'):
        o = re.match(r'\.obj "?([@\w]+)"?, local', L.strip())
        if o: cur = o.group(1); continue
        b = re.match(r'\.4byte 0x([0-9A-Fa-f]{8})', L.strip())
        if b and cur:
            m[cur] = struct.unpack('>f', bytes.fromhex(b.group(1)))[0]; cur = None
    return m
def lits(path, fn, resolve):
    """Ordered distinct literal values referenced inside function fn."""
    out = []; inside = False
    for L in open(path, encoding='utf-8', errors='replace'):
        if L.startswith('.fn '):
            inside = L.split()[1].rstrip(',') == fn
        elif L.startswith('.endfn'):
            inside = False
        elif inside:
            for mm in re.finditer(r'"?(@[\w]+?)(?:_([0-9A-F]{8}))?"?@sda21', L):
                v = resolve(mm.group(1), mm.group(2))
                if v is not None and v not in out:
                    out.append(v)
    return out
TGT = 'dis/hb_a.txt'; DRF = 'b6/v_show.txt'
dm = sdata2_map(DRF)
tr = lambda s, a: struct.unpack('>f', dolread(int(a, 16), 4))[0] if a else None
dr = lambda s, a: dm.get(s)
PAIRS = [('fn_80112040', 'bg_dispx_get__FP19daEnHatenaBalloon_c'),
         ('fly_yspeed_set__19daEnHatenaBalloon_cFv',) * 2,
         ('fly_xspeed_set__19daEnHatenaBalloon_cFb',) * 2,
         ('fly_ydisp_check__19daEnHatenaBalloon_cFb',) * 2,
         ('fly_xdisp_check__19daEnHatenaBalloon_cFb',) * 2,
         ('fly_dispin_check__19daEnHatenaBalloon_cFv',) * 2,
         ('escape_dispout_check__19daEnHatenaBalloon_cFv',) * 2,
         ('create_wait_pos_set__19daEnHatenaBalloon_cFv',) * 2]
bad = 0
for t, f in PAIRS:
    a = lits(TGT, t, tr); b = lits(DRF, f, dr)
    ok = a == b
    bad += not ok
    print('%-46s %s' % (t.split('__')[0], 'CONSTANTS OK' if ok else 'CONSTANT MISMATCH'))
    if not ok:
        print('    target:', a); print('    draft :', b)
print('\n%d/%d functions have matching constant pools' % (len(PAIRS) - bad, len(PAIRS)))

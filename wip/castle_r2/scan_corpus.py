"""Disassemble every banked (already-matching) object and find the source idiom
that produces the TARGET's deferred-store tail:

    fadds ...                      <- all three adds first
    stfs fA, 0x4(r3)
    stfs fB, 0x0(r3)
    stfs fC, 0x8(r3)
    blr

versus the early-store tail that `dBaseActor_c::getCenterPos` (byte-exact,
`return mPos + mCenterOffs;`) and our draft both produce.
"""
import sys, os, re, subprocess

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
CACHE = os.path.join(ROOT, 'wip', 'castle_r2', '_corpus')
os.makedirs(CACHE, exist_ok=True)

objs = []
for dirpath, _, files in os.walk(os.path.join(ROOT, 'bin', 'compiled')):
    for f in files:
        if f.endswith('.o'):
            objs.append(os.path.join(dirpath, f))

print('objects:', len(objs))

FN = re.compile(r'^\.fn (\S+),')
INS = re.compile(r'^/\* [0-9A-F]{8} [0-9A-F]{8}  (?:[0-9A-F]{2} ){4}\*/\t(.*)$')

hits_defer = []
hits_early = []
for o in objs:
    out = os.path.join(CACHE, os.path.relpath(o, os.path.join(ROOT, 'bin', 'compiled')).replace(os.sep, '__') + '.txt')
    if not os.path.exists(out) or os.path.getsize(out) == 0:
        subprocess.run([DTK, 'elf', 'disasm', o, out], capture_output=True)
    try:
        lines = open(out, encoding='utf-8', errors='replace').read().splitlines()
    except OSError:
        continue
    cur = None
    body = []
    for ln in lines:
        m = FN.match(ln)
        if m:
            cur = m.group(1); body = []; continue
        if ln.startswith('.endfn'):
            if cur and body:
                # look at the tail
                tail = [b.split('/*')[0].strip() for b in body]
                # find every run of three consecutive stfs to r3 offsets 0/4/8
                for i in range(len(tail) - 3):
                    w = tail[i:i+3]
                    if all(re.match(r'stfs f\d+, 0x[048]\(r3\)$', x) for x in w):
                        offs = [re.search(r'0x([048])\(', x).group(1) for x in w]
                        if sorted(offs) == ['0', '4', '8']:
                            prev = tail[i-1] if i else ''
                            rec = (os.path.basename(o), cur, ''.join(offs), prev, len(tail))
                            if ''.join(offs) == '408':
                                hits_defer.append(rec)
                            else:
                                hits_early.append(rec)
                        break
            cur = None; body = []
            continue
        m2 = INS.match(ln)
        if m2 and cur:
            body.append(m2.group(1))

print('\n== DEFERRED (target order y,x,z = 4,0,8) ==')
for h in hits_defer:
    print('  %-34s %-60s prev=%-24s len=%d' % h[:2] + '' if False else '  %-30s %-58s prev=%-22s len=%d' % (h[0], h[1], h[3], h[4]))
print('\n== OTHER three-store tails ==')
for h in hits_early:
    print('  %-30s %-58s order=%s prev=%-22s len=%d' % (h[0], h[1], h[2], h[3], h[4]))

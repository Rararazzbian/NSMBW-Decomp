"""Side-by-side raw-content diff of one target function vs one draft function."""
import sys, os, re, difflib
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))

def load(path, name):
    out = []
    inside = False
    for line in open(path, encoding='utf-8', errors='replace'):
        s = line.strip()
        if s.startswith('.fn ') or s.startswith('.obj '):
            fn = s.split(None, 1)[1].split(',')[0].strip()
            inside = (fn == name)
            continue
        if s.startswith('.endfn') or s.startswith('.endobj'):
            if inside:
                break
            continue
        if not inside:
            continue
        m = re.match(r'/\*.*?\*/\s*(.*)$', s)
        if m:
            t = m.group(1).strip()
            if t:
                out.append(t)
        elif s and not s.startswith('#') and not s.startswith('.'):
            out.append(s)
    return out

def canon(t):
    # normalise relocation symbol names and local labels
    t = re.sub(r'\b(lbl_2_\w+|fn_2_\w+|\.L_\w+|@?\d+@?[\w$]*)\b', 'SYM', t)
    return t

tgt_txt, tgt_fn, drf_txt, drf_fn = sys.argv[1:5]
A = load(tgt_txt, tgt_fn)
B = load(drf_txt, drf_fn)
print('target %s: %d insns    draft %s: %d insns' % (tgt_fn, len(A), drf_fn, len(B)))
sm = difflib.SequenceMatcher(None, [canon(x) for x in A], [canon(x) for x in B])
ndiff = 0
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal':
        for k in range(i1, i2):
            print('    %4d  %-44s | %s' % (k, A[k], B[j1 + (k - i1)]))
    else:
        n = max(i2 - i1, j2 - j1)
        ndiff += n
        for k in range(n):
            a = A[i1 + k] if i1 + k < i2 else ''
            b = B[j1 + k] if j1 + k < j2 else ''
            print(' >> %4d  %-44s | %s' % (i1 + k, a, b))
print('content-aligned differing blocks: %d lines' % ndiff)

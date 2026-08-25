"""Raw byte comparison of target vs draft listings, position by position."""
import re
import sys

def fn_bytes(path, name):
    lines = open(path).read().splitlines()
    start = None
    for i, l in enumerate(lines):
        if re.match(r'\.fn\s+%s\b' % re.escape(name), l):
            start = i + 1
            break
    if start is None:
        return None
    words = []
    for l in lines[start:]:
        if l.startswith('.endfn'):
            break
        m = re.match(r'/\*\s*[0-9A-Fa-f]{8}\s+[0-9A-Fa-f]{8}\s+((?:[0-9A-Fa-f]{2} ?)+)\s*\*/', l)
        if not m:
            continue
        hx = m.group(1).replace(' ', '')
        for j in range(0, len(hx), 8):
            words.append(hx[j:j+8])
    return words

target, draft, name = sys.argv[1], sys.argv[2], sys.argv[3]
t = fn_bytes(target, name)
d = fn_bytes(draft, name)
if t is None or d is None:
    print('MISSING function', name); sys.exit(1)
print('%s: target %dw, draft %dw' % (name, len(t), len(d)))
if len(t) != len(d):
    print('WORD COUNT MISMATCH'); sys.exit(1)
diffs = [(i, t[i], d[i]) for i in range(len(t)) if t[i] != d[i]]
print('byte diffs:', len(diffs))
for i, a, b in diffs[:20]:
    print('  word %3d: target %s draft %s' % (i, a, b))
sys.exit(1 if diffs else 0)

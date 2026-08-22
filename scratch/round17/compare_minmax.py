import os
BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
with open(os.path.join(BASE, 'target_8007E17C.txt')) as f:
    t = f.readlines()
with open(os.path.join(BASE, 'draft_disasm.txt')) as f:
    d = f.readlines()

def get_fn(lines, name):
    start = None
    for i, line in enumerate(lines):
        if name in line and '.fn' in line:
            start = i
        if start is not None and '.endfn' in line and i > start:
            return lines[start:i+1]
    return None

def instrs(fn):
    out = []
    for l in fn:
        s = l.strip()
        if not s or s.startswith('#') or s.startswith('.fn') or s.startswith('.endfn'):
            continue
        out.append(s)
    return out

t_fn = get_fn(t, 'ProcMain__17dBgActorManager_cFv')
d_fn = get_fn(d, 'ProcMain__17dBgActorManager_cFv')
t_i = instrs(t_fn)
d_i = instrs(d_fn)
print('Target instrs:', len(t_i))
print('Draft instrs:', len(d_i))

t_loop = None
d_loop = None
for i, s in enumerate(t_i):
    if 'L_8007E5DC:' in s:
        t_loop = i
        break
for i, s in enumerate(d_i):
    if 'L_0000059C:' in s:
        d_loop = i
        break
print('Target loop start idx:', t_loop)
print('Draft loop start idx:', d_loop)

def find_first(lines, name, start=0):
    for i in range(start, len(lines)):
        if name in lines[i]:
            return i
    return None

t_gs = find_first(t_i, 'getSize', t_loop)
d_gs = find_first(d_i, 'getSize', d_loop)
print('First getSize: target', t_gs, 'draft', d_gs)
if t_gs and d_gs:
    print()
    print('=== Target around mMin/mMax (getSize region) ===')
    for s in t_i[t_gs-10:t_gs+45]:
        print(s)
    print()
    print('=== Draft around mMin/mMax (getSize region) ===')
    for s in d_i[d_gs-10:d_gs+45]:
        print(s)

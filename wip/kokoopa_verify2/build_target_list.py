import sys, os
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
files = [
    os.path.join(ROOT, 'wip', 'kokoopa_verify2', 'target_1_800A8710.txt'),
    os.path.join(ROOT, 'wip', 'kokoopa_verify2', 'target_2_sinit.txt'),
    os.path.join(ROOT, 'wip', 'kokoopa_verify2', 'target_3_800B03D8.txt'),
]

all_fns = []
for f in files:
    fns = harness.list_functions(f, with_size=True)
    print(f, '->', len(fns), 'entries')
    for name, size in fns:
        all_fns.append((name, size, f))

print('TOTAL entries (incl pad_*):', len(all_fns))
total_size = sum(s for _, s, _ in all_fns)
print('TOTAL size (incl pad_*):', total_size, hex(total_size))

# Exclude pad_ prefixed (alignment fillers, not real functions)
real = [(n, s, f) for n, s, f in all_fns if not n.startswith('pad_')]
print('TOTAL entries excl pad_*:', len(real))
print('TOTAL size excl pad_*:', sum(s for _, s, _ in real))

with open(os.path.join(ROOT, 'wip', 'kokoopa_verify2', 'all_functions.txt'), 'w', encoding='utf-8') as fh:
    for n, s, f in all_fns:
        fh.write('%s\t%d\t%s\n' % (n, s, os.path.basename(f)))

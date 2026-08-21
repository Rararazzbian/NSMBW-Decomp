import sys, os
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
WIP = os.path.join(ROOT, 'wip', 'kokoopa_verify2')

def raw_extract(path, name):
    body = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = harness.FN_START.match(s)
            if m:
                body = [] if m.group(1).strip('"') == name else None
                continue
            if harness.FN_END.match(s):
                if body is not None:
                    return body
                continue
            if body is not None:
                mi = harness.INSN.match(s)
                if mi:
                    body.append(mi.group(1).strip())
    return body

fn = sys.argv[1] if len(sys.argv) > 1 else '__ct__18dEnTorideKokoopa_cFv'
target_path = os.path.join(WIP, sys.argv[2] if len(sys.argv) > 2 else 'target_1_800A8710.txt')
draft_path = os.path.join(WIP, 'd_enemy_toride_kokoopa.txt')

t = raw_extract(target_path, fn)
d = raw_extract(draft_path, fn)
print('TARGET: %d instructions' % (len(t) if t else -1))
print('DRAFT : %d instructions' % (len(d) if d else -1))

print('\n--- TARGET bl/blr calls ---')
for i, line in enumerate(t or []):
    if line.startswith('bl '):
        print('  [%3d] %s' % (i, line))

print('\n--- DRAFT bl/blr calls ---')
for i, line in enumerate(d or []):
    if line.startswith('bl '):
        print('  [%3d] %s' % (i, line))

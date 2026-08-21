import sys, os
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
WIP = os.path.join(ROOT, 'wip', 'kokoopa_verify2')
DRAFT_TXT = os.path.join(WIP, 'd_enemy_toride_kokoopa.txt')

TARGET_FILES = {
    'target_1_800A8710.txt': os.path.join(WIP, 'target_1_800A8710.txt'),
    'target_2_sinit.txt': os.path.join(WIP, 'target_2_sinit.txt'),
    'target_3_800B03D8.txt': os.path.join(WIP, 'target_3_800B03D8.txt'),
}

fn_list = []
with open(os.path.join(WIP, 'true_target_list.txt'), encoding='utf-8') as fh:
    for line in fh:
        addr, size, name, srcfile = line.rstrip('\n').split('\t')
        fn_list.append((int(addr, 16), int(size), name, srcfile))

matched = []
unmatched = []
draft_missing = []
for addr, size, name, srcfile in fn_list:
    target_txt = TARGET_FILES[srcfile]
    ok, report = harness.diff_fn(target_txt, DRAFT_TXT, name)
    if ok:
        matched.append((addr, size, name))
    else:
        if report.startswith('DRAFT MISSING'):
            draft_missing.append((addr, size, name))
        unmatched.append((addr, size, name, report))

total_fn = len(fn_list)
total_bytes = sum(s for _, s, _, _ in fn_list)
matched_bytes = sum(s for _, s, _ in matched)

print('=== SUMMARY ===')
print('Total target functions: %d' % total_fn)
print('Total target bytes: %d' % total_bytes)
print('Matched functions: %d (%.2f%%)' % (len(matched), 100.0 * len(matched) / total_fn))
print('Matched bytes: %d (%.2f%%)' % (matched_bytes, 100.0 * matched_bytes / total_bytes))
print('Draft-missing (never emitted) functions: %d' % len(draft_missing))
print()

with open(os.path.join(WIP, 'matched_list.txt'), 'w', encoding='utf-8') as fh:
    for addr, size, name in matched:
        fh.write('0x%08X\t%d\t%s\n' % (addr, size, name))

with open(os.path.join(WIP, 'unmatched_list.txt'), 'w', encoding='utf-8') as fh:
    for addr, size, name, report in unmatched:
        fh.write('0x%08X\t%d\t%s\n' % (addr, size, name))

with open(os.path.join(WIP, 'draft_missing_list.txt'), 'w', encoding='utf-8') as fh:
    for addr, size, name in draft_missing:
        fh.write('0x%08X\t%d\t%s\n' % (addr, size, name))

print('Draft-missing functions (peer never emitted a definition):')
for addr, size, name in draft_missing:
    print('  0x%08X %5d %s' % (addr, size, name))

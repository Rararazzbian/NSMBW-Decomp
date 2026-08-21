import sys, json
sys.path.insert(0, 'tools/auto_decomp')
import harness

# Build one combined target text covering the whole TU, in true dtk raw format.
parts = [
    open('scratch/gemini_round16/auto_03_800A8710_text.txt', encoding='utf-8', errors='replace').read(),
    open('wip/kokoopa_verify/sinit_retail_raw.txt', encoding='utf-8', errors='replace').read(),
    open('scratch/gemini_round16/auto_03_800B03D8_text.txt', encoding='utf-8', errors='replace').read(),
]
combined_path = 'wip/kokoopa_verify/target_combined.txt'
open(combined_path, 'w', encoding='utf-8').write('\n'.join(parts))

in_range = json.load(open('wip/kokoopa_verify/target_fns_in_range.json', encoding='utf-8'))
# in_range: list of [addr, size, name]

draft_path = 'wip/kokoopa_verify/draft.txt'

results = []
matched_bytes = 0
matched_count = 0
draft_missing = []
mismatched = []
target_missing = []
matched_list = []

for addr, size, name in in_range:
    ok, report = harness.diff_fn(combined_path, draft_path, name)
    if ok:
        matched_count += 1
        matched_bytes += size
        results.append((addr, size, name, 'MATCH'))
        matched_list.append((addr, size, name))
    else:
        if report.startswith('DRAFT MISSING'):
            draft_missing.append((addr, size, name))
            results.append((addr, size, name, 'NOT_ATTEMPTED'))
        elif report.startswith('TARGET MISSING'):
            target_missing.append((addr, size, name))
            results.append((addr, size, name, 'TARGET_MISSING'))
        else:
            mismatched.append((addr, size, name, report))
            results.append((addr, size, name, 'MISMATCH'))

total_fns = len(in_range)
total_bytes = sum(s for a, s, n in in_range)

print('=== TOTALS ===')
print('Total functions in TU (0x800A8710-0x800B0A20, excl gaps):', total_fns)
print('Total bytes:', total_bytes)
print()
print('MATCHED     :', matched_count, 'functions,', matched_bytes, 'bytes')
print('MISMATCHED  :', len(mismatched), 'functions (draft emits it, bytes differ)')
print('NOT ATTEMPTED (not emitted by draft):', len(draft_missing))
print('TARGET MISSING (name not found in target -- naming issue):', len(target_missing))
print()
print('Match pct by count: %.2f%%' % (matched_count / total_fns * 100))
print('Match pct by bytes: %.2f%%' % (matched_bytes / total_bytes * 100))

print('\n=== MISMATCHES (draft attempted, bytes differ) ===')
for addr, size, name, report in mismatched:
    print('0x%08X (%d B) %s' % (addr, size, name))

print('\n=== TARGET MISSING (name lookup failed) ===')
for addr, size, name in target_missing:
    print('0x%08X (%d B) %s' % (addr, size, name))

json.dump({
    'total_fns': total_fns, 'total_bytes': total_bytes,
    'matched_count': matched_count, 'matched_bytes': matched_bytes,
    'matched_list': matched_list,
    'mismatched': [[a, s, n] for a, s, n, r in mismatched],
    'draft_missing': draft_missing,
    'target_missing': target_missing,
}, open('wip/kokoopa_verify/full_verify_results.json', 'w', encoding='utf-8'), indent=2)

print('\n=== MATCHED (sorted by size, smallest first) ===')
for addr, size, name in sorted(matched_list, key=lambda t: t[1]):
    print('%4d B  0x%08X  %s' % (size, addr, name))

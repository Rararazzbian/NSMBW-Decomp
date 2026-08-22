import re
from pathlib import Path
from collections import Counter, defaultdict

txt = Path('scratch/build_truth/check_bounds_all.txt').read_text(encoding='utf-8', errors='replace')
blocks = txt.split('===== ')
CATS = [
    ('START_MID_OBJECT', 'START is '),
    ('START_OVERLAP', 'START overlaps '),
    ('END_CUTS_SHORT', 'END cuts '),
    ('OVERLAPS_SLICE', 'OVERLAPS '),
    ('FAMILY_DATA_PROFILE', '.data begins at a PROFILE symbol'),
    ('FAMILY_DATA_0x24', '.data begins at a 0x24 object'),
    ('FAMILY_TEXT_0x1c', 'is 0x1c -- array-destructor sized'),
    ('OWNERSHIP_UNREFERENCED', 'is NEVER referenced from anywhere'),
]
counts = Counter()
per_cat = defaultdict(list)
for b in blocks[1:]:
    head = b.split('\n', 1)[0]
    if '-> FAIL' not in head:
        continue
    name = head.replace(' -> FAIL', '').strip()
    seen = set()
    for cat, needle in CATS:
        n = b.count(needle)
        if n:
            counts[cat] += n
            seen.add(cat)
            per_cat[cat].append((name, n))
print('FAILURE CATEGORIES ACROSS ALL 182 SLICE ENTRIES')
for cat, _ in CATS:
    print('  %-24s %4d occurrence(s) in %d unit(s)' % (cat, counts[cat], len(per_cat[cat])))
print()
for cat in ('START_MID_OBJECT', 'START_OVERLAP', 'END_CUTS_SHORT', 'OVERLAPS_SLICE'):
    print('--- %s ---' % cat)
    if not per_cat[cat]:
        print('   (none)')
    for name, n in per_cat[cat]:
        print('   %s  x%d' % (name, n))
    print()
print('--- units flagged ONLY by family heuristics and/or ownership ---')
onlysoft = 0
for b in blocks[1:]:
    head = b.split('\n', 1)[0]
    if '-> FAIL' not in head:
        continue
    hard = any(nd in b for c, nd in CATS[:4])
    if not hard:
        onlysoft += 1
print('   %d of the %d failing entries have NO hard (address-arithmetic) finding' %
      (onlysoft, sum(1 for b in blocks[1:] if '-> FAIL' in b.split(chr(10), 1)[0])))

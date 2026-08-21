import sys
sys.path.insert(0, 'tools/auto_decomp')
import harness

target_path = 'wip/kokoopa_verify/sinit_retail_raw.txt'
draft_path = 'wip/kokoopa_verify/draft.txt'

target_name = r'__sinit_\d_enemy_toride_kokoopa_cpp'
draft_name = r'__sinit_\draft_cpp'

want = harness.extract(target_path, target_name)
got = harness.extract(draft_path, draft_name)

print('target instr count:', None if want is None else len(want))
print('draft  instr count:', None if got is None else len(got))

if want is None or got is None:
    sys.exit('one side missing')

if want == got:
    print('EXACT MATCH (%d instructions, canonicalised)' % len(want))
else:
    print('MISMATCH')
    n = 0
    for i in range(max(len(want), len(got))):
        a = want[i] if i < len(want) else '<none>'
        b = got[i] if i < len(got) else '<none>'
        if a != b:
            print('%4d | want: %-50s got: %s' % (i, a, b))
            n += 1
            if n > 60:
                print('... truncated')
                break
    print('\ntotal diffs shown (capped): %d' % n)

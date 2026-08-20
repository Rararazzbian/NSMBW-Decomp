"""Diff two differently-named functions (target anon vs draft real name)."""
import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

target_txt = sys.argv[1]
draft_txt = sys.argv[2]
target_name = sys.argv[3]
draft_name = sys.argv[4]

want = H.extract(target_txt, target_name)
got = H.extract(draft_txt, draft_name)
if want is None:
    print('TARGET MISSING:', target_name); sys.exit(1)
if got is None:
    print('DRAFT MISSING:', draft_name); sys.exit(1)

print('size: target %d, draft %d' % (len(want), len(got)))
n = 0
for i in range(max(len(want), len(got))):
    a = want[i] if i < len(want) else '<none>'
    b = got[i] if i < len(got) else '<none>'
    if a != b:
        n += 1
        print('  %3d | want: %-50s got: %s' % (i, a, b))
print('total differing:', n)

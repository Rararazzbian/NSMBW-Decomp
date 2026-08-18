import sys, os
sys.path.insert(0, os.path.join('wip', 'wm_units'))
from verify_anon import norm, functions

draft = dict(functions('wip/wm_units/agent_sandpillar/probe/test.txt'))
target_funcs = functions(sys.argv[3], with_addr=True)

want_addr = int(sys.argv[1], 0)
draft_name = sys.argv[2]

t = None
for addr, name, ins in target_funcs:
    if addr == want_addr:
        t = ins
        break
if t is None:
    print('not found'); sys.exit(1)

d = draft[draft_name]
tn = norm(t)
dn = norm(d)

import difflib
sm = difflib.SequenceMatcher(a=tn, b=dn)
for op, i1, i2, j1, j2 in sm.get_opcodes():
    if op == 'equal':
        for k in range(i1, i2):
            print('  %-60s | %s' % (t[i1+(k-i1)], d[j1+(k-i1)]))
    else:
        ta = t[i1:i2]; da = d[j1:j2]
        for k in range(max(len(ta), len(da))):
            tl = ta[k] if k < len(ta) else ''
            dl = da[k] if k < len(da) else ''
            print('X %-60s | %s' % (tl, dl))

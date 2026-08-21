import sys, os
sys.path.insert(0, 'wip/line_mng_shared')
sys.path.insert(0, os.path.join('tools','auto_decomp'))
import tally as T
d = T.parse('wip/fix_bigtwo/_tally/d.txt')
t = T.parse('wip/line_mng_shared/target.txt')
for name in ['mov_to_rightupper__10dLineMng_cFUlRC7mVec2_cb',
             'mov_to_leftupper__10dLineMng_cFUlRC7mVec2_cb',
             'mov_to_leftlower__10dLineMng_cFUlRC7mVec2_cb']:
    dn = len(d.get(name, []))
    tn = len(t.get(name, []))
    m = T.matched(d[name], t[name]) if name in d and name in t else None
    print(name, 'draft_len', dn, 'target_len', tn, 'matched', m)

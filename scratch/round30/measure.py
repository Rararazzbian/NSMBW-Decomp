import re
import sys
sys.path.insert(0, 'tools/auto_decomp')
import harness
T = 'scratch/round29/d_bg_ctr/target.txt'
pairs = [
    ('baseline', 'scratch/round29/d_bg_ctr/calc_v1_decl_order.txt'),
    ('shape2', 'scratch/round29/d_bg_ctr/calc_round29_shape2.txt'),
    ('round29', 'scratch/round29/d_bg_ctr/calc_round29.txt'),
]
names = ['calc__9dBg_ctr_cFv', 'fn_80080670__9dBg_ctr_cFP7mVec3_cf', 'revisePos__9dBg_ctr_cFv', 'addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c']
for label, draft in pairs:
    print(label)
    for name in names:
        ok, report = harness.diff_fn(T, draft, name)
        print(name, 'MATCH' if ok else report.splitlines()[0])
for path in [T] + [x[1] for x in pairs]:
    text = open(path, encoding='utf-8').read()
    print('DETAIL', path)
    for name in names:
        m = re.search(r'\.fn .*?' + re.escape(name) + r'.*?\n(.*?)\.endfn', text, re.S)
        body = m.group(1) if m else ''
        count = len(re.findall(r'^/\*', body, re.M))
        saved = sorted(set(re.findall(r'\b(?:stfd|psq_st) f(\d+)', body)), key=int)
        frame = re.search(r'stwu r1, -(0x[0-9a-f]+)\(r1\)', body)
        print(name, count, saved, frame.group(1) if frame else 'leaf')

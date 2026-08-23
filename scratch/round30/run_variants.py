import sys
sys.path.insert(0, 'tools/auto_decomp')
import harness
inc = ('scratch/round30/d_bg_ctr',)
target = 'scratch/round29/d_bg_ctr/target.txt'
for name in ('calc_order', 'calc_store', 'calc_both'):
    src = 'scratch/round30/d_bg_ctr/' + name + '.cpp'
    obj = 'scratch/round30/d_bg_ctr/' + name + '.o'
    txt = 'scratch/round30/d_bg_ctr/' + name + '.txt'
    ok, log = harness.compile_draft(src, obj, extra_inc=inc)
    print(name, 'compile', ok)
    if not ok:
        open('scratch/round30/d_bg_ctr/' + name + '.compile.log', 'w').write(log)
        continue
    dok, dlog = harness.disasm(obj, txt)
    print('disasm', dok)
    print(harness.diff_fn(target, txt, 'calc__9dBg_ctr_cFv')[1].splitlines()[0])

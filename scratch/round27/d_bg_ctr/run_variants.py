import os
import sys
sys.path.insert(0, os.path.join(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp', 'tools', 'auto_decomp'))
import harness

base = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round27\d_bg_ctr'
target = os.path.join(base, 'target.txt')
for name in ('calc_v1_decl_order', 'calc_v2_named_reads', 'calc_v3_split_assign'):
    src = os.path.join(base, name + '.cpp')
    obj = os.path.join(base, name + '.o')
    dis = os.path.join(base, name + '.txt')
    ok, log = harness.compile_draft(src, obj, extra_inc=(os.path.join(base, 'shadow'),))
    print(name, 'COMPILE', ok)
    if not ok:
        print(log)
        continue
    ok, log = harness.disasm(obj, dis)
    print(name, 'DISASM', ok)
    ok, report = harness.diff_fn(target, dis, 'calc__9dBg_ctr_cFv')
    print(report.splitlines()[0] if report else 'NO REPORT')

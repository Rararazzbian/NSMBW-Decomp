import os
import sys
sys.path.insert(0, os.path.join(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp', 'tools', 'auto_decomp'))
import harness

base = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round27\d_bg_ctr'
target = os.path.join(base, 'target.txt')

def diff(obj_name, fn_name):
    dis = os.path.join(base, obj_name + '.txt')
    ok, report = harness.diff_fn(target, dis, fn_name)
    print(obj_name, 'MATCH' if ok else 'DIFF', report.splitlines()[0] if report else 'NO REPORT')

def build(name, source=None):
    source = source or name
    src = os.path.join(base, source + '.cpp')
    obj = os.path.join(base, name + '.o')
    dis = os.path.join(base, name + '.txt')
    ok, log = harness.compile_draft(src, obj, extra_inc=(os.path.join(base, 'shadow'),))
    print(name, 'COMPILE', ok)
    if not ok:
        print(log)
        return False
    ok, log = harness.disasm(obj, dis)
    print(name, 'DISASM', ok)
    return True

for name in ('calc_v1_decl_order', 'calc_v2_named_reads', 'calc_v3_split_assign'):
    if build(name):
        diff(name, 'calc__9dBg_ctr_cFv')

if build('round27_gate'):
    diff('round27_gate', 'fn_80080E40')

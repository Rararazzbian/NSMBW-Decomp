import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import fndiff
import harness

target_file = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', 'dol_bases_d_panel_obj_list', 'target.txt')
target_fns = fndiff.read_functions(target_file)

print(f"Total functions in target.txt: {len(target_fns)}")

scoped_fns = []
for name, body in target_fns.items():
    cname = fndiff.canonical_name(name)
    desc = fndiff.describe(body)
    is_dpanel = 'dPanelObjList_c' in name
    tag = "[IN SCOPE]" if is_dpanel else "[OUT OF SCOPE]"
    print(f"{tag:15s} {name:60s} -> {desc}")
    if is_dpanel:
        scoped_fns.append((name, body))

print(f"\nIn-scope dPanelObjList_c functions: {len(scoped_fns)}")

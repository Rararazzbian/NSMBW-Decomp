import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))

import harness
import fndiff

src = os.path.join(HERE, 'd_panel_obj_list.cpp')
obj = os.path.join(HERE, 'd_panel_obj_list.o')
txt = os.path.join(HERE, 'd_panel_obj_list.txt')
tgt = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', 'dol_bases_d_panel_obj_list', 'target.txt')

tgt_fns = fndiff.read_functions(tgt)
drf_fns = fndiff.read_functions(txt)

print("Target getScale:")
for line in tgt_fns.get('getScale__15dPanelObjList_cCFv', []):
    print(" ", line)

print("\nDraft getScale:")
for line in drf_fns.get('getScale__15dPanelObjList_cCFv', []):
    print(" ", line)
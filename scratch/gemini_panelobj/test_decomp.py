import os
import sys
import subprocess

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))

import harness
import fndiff

src = os.path.join(HERE, 'd_panel_obj_list.cpp')
obj = os.path.join(HERE, 'd_panel_obj_list.o')
txt = os.path.join(HERE, 'd_panel_obj_list.txt')
tgt = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', 'dol_bases_d_panel_obj_list', 'target.txt')

ok, log = harness.compile_draft(src, obj, extra_inc=[HERE])
if not ok:
    print("Compilation failed:")
    print(log)
    sys.exit(1)

dok, dlog = harness.disasm(obj, txt)
if not dok:
    print("Disasm failed:")
    print(dlog)
    sys.exit(1)

print("=== FNDIFF REPORT ===")
tgt_fns = fndiff.read_functions(tgt)
drf_fns = {fndiff.canonical_name(k): v for k, v in fndiff.read_functions(txt).items()}
tgt_canon = {fndiff.canonical_name(k): v for k, v in tgt_fns.items()}

fn_names = [
    "__ct__15dPanelObjList_cFv",
    "__dt__15dPanelObjList_cFv",
    "getValue__15dPanelObjList_cCFv",
    "isChange__15dPanelObjList_cCFv",
    "setChange__15dPanelObjList_cFb",
    "getPosX__15dPanelObjList_cCFv",
    "getPosY__15dPanelObjList_cCFv",
    "getPosZ__15dPanelObjList_cCFv",
    "setPosXY__15dPanelObjList_cFff",
    "setPos__15dPanelObjList_cFfff",
    "getType__15dPanelObjList_cCFv",
    "setScaleFoot__15dPanelObjList_cFf",
    "setScaleAngle__15dPanelObjList_cFfs",
    "getScale__15dPanelObjList_cCFv",
    "getAngleF__15dPanelObjList_cCFv",
    "getAngleS__15dPanelObjList_cCFv",
    "getParts__15dPanelObjList_cCFv",
]

matched_count = 0
for idx, name in enumerate(fn_names, 1):
    cname = fndiff.canonical_name(name)
    if cname not in tgt_canon:
        print(f"[{idx:2d}/17] {name}: NOT IN TARGET")
        continue
    if cname not in drf_fns:
        print(f"[{idx:2d}/17] {name}: NOT IN DRAFT")
        continue
    
    diff_cnt = fndiff.compare(tgt_canon[cname], drf_fns[cname], name, verbose=True)
    if diff_cnt == 0:
        matched_count += 1

print(f"\nScore: {matched_count} / {len(fn_names)} (DIFFS 0)")

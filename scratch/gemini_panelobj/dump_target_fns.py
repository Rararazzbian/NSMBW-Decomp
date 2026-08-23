import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import fndiff

target_file = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', 'dol_bases_d_panel_obj_list', 'target.txt')
target_fns = fndiff.read_functions(target_file)

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

for idx, name in enumerate(fn_names, 1):
    body = target_fns.get(name)
    desc = fndiff.describe(body) if body else "MISSING"
    print(f"[{idx:2d}/17] {name}")
    print(f"       {desc}")
    if body:
        for line in body:
            print(f"         {line}")
    print()

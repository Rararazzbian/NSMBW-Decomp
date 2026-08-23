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

base_cpp = """#include "d_panel_obj_list.hpp"

int dPanelObjList_c::getType() const { return mType; }

f32 dPanelObjList_c::getScale() const {
    f32 scale = 1.0f;
    if ((u32)(getType() - 1) <= 1) {
        scale = mScale;
    }
    return scale;
}

s16 dPanelObjList_c::getAngleS() const {
    s16 angle = 0;
    if ((u32)(getType() - 2) <= 1) {
        angle = mAngle;
    }
    return angle;
}
"""

with open(src, 'w', encoding='utf-8', newline='\n') as f:
    f.write(base_cpp)

ok, log = harness.compile_draft(src, obj, extra_inc=[HERE])
dok, dlog = harness.disasm(obj, txt)
drf_fns = fndiff.read_functions(txt)

print("\nDraft getScale (with f32 scale = 1.0f):")
for line in drf_fns.get('getScale__15dPanelObjList_cCFv', []):
    print(" ", line)

print("\nDraft getAngleS (with s16 angle = 0):")
for line in drf_fns.get('getAngleS__15dPanelObjList_cCFv', []):
    print(" ", line)
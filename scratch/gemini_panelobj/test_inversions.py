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
tgt_canon = {fndiff.canonical_name(k): v for k, v in tgt_fns.items()}

scale_tests = [
    ("gt_1_return_1", """f32 dPanelObjList_c::getScale() const {
    if ((u32)(getType() - 1) > 1) {
        return 1.0f;
    }
    return mScale;
}"""),
    ("gt_1_var", """f32 dPanelObjList_c::getScale() const {
    if ((u32)(getType() - 1) > 1) return 1.0f;
    return mScale;
}"""),
    ("neq_1_2", """f32 dPanelObjList_c::getScale() const {
    if (getType() != 1 && getType() != 2) {
        return 1.0f;
    }
    return mScale;
}"""),
    ("not_or", """f32 dPanelObjList_c::getScale() const {
    if (!(getType() == 1 || getType() == 2)) {
        return 1.0f;
    }
    return mScale;
}"""),
]

angle_tests = [
    ("gt_1_return_0", """s16 dPanelObjList_c::getAngleS() const {
    if ((u32)(getType() - 2) > 1) {
        return 0;
    }
    return mAngle;
}"""),
    ("neq_2_3", """s16 dPanelObjList_c::getAngleS() const {
    if (getType() != 2 && getType() != 3) {
        return 0;
    }
    return mAngle;
}"""),
    ("not_or", """s16 dPanelObjList_c::getAngleS() const {
    if (!(getType() == 2 || getType() == 3)) {
        return 0;
    }
    return mAngle;
}"""),
]

base_cpp = """#include "d_panel_obj_list.hpp"

dPanelObjList_c::dPanelObjList_c() {
    mpPrev = 0;
    mpNext = 0;
    mValue = 0;
    mType = 0;
    mChange = 1;
    mPosX = 0.0f;
    mPosY = 0.0f;
    mScale = 1.0f;
    mAngle = 0;
    mParts = 0;
}

dPanelObjList_c::~dPanelObjList_c() {
}

u16 dPanelObjList_c::getValue() const {
    return mValue;
}

bool dPanelObjList_c::isChange() const {
    return mChange != 0;
}

void dPanelObjList_c::setChange(bool change) {
    mChange = change;
}

f32 dPanelObjList_c::getPosX() const {
    return mPosX;
}

f32 dPanelObjList_c::getPosY() const {
    return mPosY;
}

f32 dPanelObjList_c::getPosZ() const {
    return mPosZ;
}

void dPanelObjList_c::setPosXY(f32 x, f32 y) {
    mPosX = x;
    mPosY = y;
}

void dPanelObjList_c::setPos(f32 x, f32 y, f32 z) {
    mPosX = x;
    mPosY = y;
    mPosZ = z;
}

int dPanelObjList_c::getType() const {
    return mType;
}

void dPanelObjList_c::setScaleFoot(f32 scale) {
    mType = 1;
    mScale = scale;
    mAngle = 0;
    mParts = 0;
}

void dPanelObjList_c::setScaleAngle(f32 scale, s16 angle) {
    mType = 2;
    mScale = scale;
    mAngle = angle;
    mParts = 0;
}

SCALE_BODY

f32 dPanelObjList_c::getAngleF() const {
    return getAngleS() * 0.0000958738019107841f;
}

ANGLE_BODY

u8 dPanelObjList_c::getParts() const {
    if (getType() == 3) {
        return mParts;
    }
    return 0;
}
"""

for s_name, s_code in scale_tests:
    for a_name, a_code in angle_tests:
        code = base_cpp.replace('SCALE_BODY', s_code).replace('ANGLE_BODY', a_code)
        with open(src, 'w', encoding='utf-8', newline='\n') as f:
            f.write(code)
        
        ok, log = harness.compile_draft(src, obj, extra_inc=[HERE])
        if not ok:
            continue
        dok, dlog = harness.disasm(obj, txt)
        if not dok:
            continue
        
        drf_fns = {fndiff.canonical_name(k): v for k, v in fndiff.read_functions(txt).items()}
        s_diff = fndiff.compare(tgt_canon['getScale__15dPanelObjList_cCFv'], drf_fns['getScale__15dPanelObjList_cCFv'], 'getScale', verbose=False)
        a_diff = fndiff.compare(tgt_canon['getAngleS__15dPanelObjList_cCFv'], drf_fns['getAngleS__15dPanelObjList_cCFv'], 'getAngleS', verbose=False)
        print(f"Scale [{s_name:18s}] diff={s_diff} | Angle [{a_name:18s}] diff={a_diff}")
        if s_diff == 0 and a_diff == 0:
            print(f"*** 17/17 MATCH WITH {s_name} AND {a_name} ***")
            sys.exit(0)
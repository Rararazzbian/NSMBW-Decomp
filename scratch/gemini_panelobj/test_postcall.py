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

scale_variants = [
    # 1
    """f32 dPanelObjList_c::getScale() const {
    int type = getType();
    f32 scale = 1.0f;
    if ((u32)(type - 1) <= 1) {
        scale = mScale;
    }
    return scale;
}""",
    # 2
    """f32 dPanelObjList_c::getScale() const {
    int type = getType();
    return ((u32)(type - 1) <= 1) ? mScale : 1.0f;
}""",
    # 3
    """f32 dPanelObjList_c::getScale() const {
    int type = getType();
    if ((u32)(type - 1) <= 1) {
        return mScale;
    }
    return 1.0f;
}""",
    # 4
    """f32 dPanelObjList_c::getScale() const {
    int type = getType() - 1;
    f32 scale = 1.0f;
    if ((u32)type <= 1) {
        scale = mScale;
    }
    return scale;
}""",
    # 5
    """f32 dPanelObjList_c::getScale() const {
    int type = getType() - 1;
    return ((u32)type <= 1) ? mScale : 1.0f;
}""",
    # 6
    """f32 dPanelObjList_c::getScale() const {
    int type = getType() - 1;
    if ((u32)type <= 1) {
        return mScale;
    }
    return 1.0f;
}""",
    # 7
    """f32 dPanelObjList_c::getScale() const {
    int type = getType();
    f32 scale = 1.0f;
    if (type == 1 || type == 2) {
        scale = mScale;
    }
    return scale;
}""",
    # 8
    """f32 dPanelObjList_c::getScale() const {
    int type = getType();
    return (type == 1 || type == 2) ? mScale : 1.0f;
}""",
]

angle_variants = [
    # 1
    """s16 dPanelObjList_c::getAngleS() const {
    int type = getType();
    s16 angle = 0;
    if ((u32)(type - 2) <= 1) {
        angle = mAngle;
    }
    return angle;
}""",
    # 2
    """s16 dPanelObjList_c::getAngleS() const {
    int type = getType();
    return ((u32)(type - 2) <= 1) ? mAngle : 0;
}""",
    # 3
    """s16 dPanelObjList_c::getAngleS() const {
    int type = getType();
    if ((u32)(type - 2) <= 1) {
        return mAngle;
    }
    return 0;
}""",
    # 4
    """s16 dPanelObjList_c::getAngleS() const {
    int type = getType() - 2;
    s16 angle = 0;
    if ((u32)type <= 1) {
        angle = mAngle;
    }
    return angle;
}""",
    # 5
    """s16 dPanelObjList_c::getAngleS() const {
    int type = getType() - 2;
    return ((u32)type <= 1) ? mAngle : 0;
}""",
    # 6
    """s16 dPanelObjList_c::getAngleS() const {
    int type = getType() - 2;
    if ((u32)type <= 1) {
        return mAngle;
    }
    return 0;
}""",
    # 7
    """s16 dPanelObjList_c::getAngleS() const {
    int type = getType();
    s16 angle = 0;
    if (type == 2 || type == 3) {
        angle = mAngle;
    }
    return angle;
}""",
    # 8
    """s16 dPanelObjList_c::getAngleS() const {
    int type = getType();
    return (type == 2 || type == 3) ? mAngle : 0;
}""",
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

for s_idx, s_code in enumerate(scale_variants, 1):
    for a_idx, a_code in enumerate(angle_variants, 1):
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
        print(f"Scale #{s_idx} diff={s_diff} | Angle #{a_idx} diff={a_diff}")
        if s_diff == 0 and a_diff == 0:
            print(f"*** 17/17 PERFECT MATCH: Scale #{s_idx}, Angle #{a_idx} ***")
            sys.exit(0)
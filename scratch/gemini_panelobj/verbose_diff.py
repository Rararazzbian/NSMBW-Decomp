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

# Let's test ternary_range
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

f32 dPanelObjList_c::getScale() const {
    return ((u32)(getType() - 1) <= 1) ? mScale : 1.0f;
}

f32 dPanelObjList_c::getAngleF() const {
    return getAngleS() * 0.0000958738019107841f;
}

s16 dPanelObjList_c::getAngleS() const {
    return ((u32)(getType() - 2) <= 1) ? mAngle : 0;
}

u8 dPanelObjList_c::getParts() const {
    if (getType() == 3) {
        return mParts;
    }
    return 0;
}
"""

with open(src, 'w', encoding='utf-8', newline='\n') as f:
    f.write(base_cpp)

ok, log = harness.compile_draft(src, obj, extra_inc=[HERE])
dok, dlog = harness.disasm(obj, txt)
drf_fns = {fndiff.canonical_name(k): v for k, v in fndiff.read_functions(txt).items()}

print("=== getScale verbose diff ===")
fndiff.compare(tgt_canon['getScale__15dPanelObjList_cCFv'], drf_fns['getScale__15dPanelObjList_cCFv'], 'getScale', verbose=True)

print("\n=== getAngleS verbose diff ===")
fndiff.compare(tgt_canon['getAngleS__15dPanelObjList_cCFv'], drf_fns['getAngleS__15dPanelObjList_cCFv'], 'getAngleS', verbose=True)
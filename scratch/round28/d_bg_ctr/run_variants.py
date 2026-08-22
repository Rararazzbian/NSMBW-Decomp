import os, shutil, subprocess, sys
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'round28', 'd_bg_ctr')
SRC = os.path.join(BASE, 'd_bg_ctr.cpp')
SHADOW = os.path.join(BASE, 'shadow')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

def build(label, transform):
    text = open(SRC, encoding='utf-8').read()
    text = transform(text)
    path = os.path.join(BASE, label + '.cpp')
    obj = os.path.join(BASE, label + '.o')
    dis = os.path.join(BASE, label + '.txt')
    open(path, 'w', encoding='utf-8', newline='\n').write(text)
    ok, log = harness.compile_draft(path, obj, extra_inc=(SHADOW,))
    print(label, 'COMPILE', ok)
    if log: print(log)
    if not ok: return
    ok, log = harness.disasm(obj, dis)
    print(label, 'DISASM', ok, log)
    for target, draft in [
        ('calc__9dBg_ctr_cFv', 'calc__9dBg_ctr_cFv'),
        ('revisePos__9dBg_ctr_cFv', 'revisePos__9dBg_ctr_cFv'),
        ('addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c', 'addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c'),
        ('fn_80080E40', None), ('fn_8007FFA0', None), ('fn_80080900', None)]:
        want = harness.extract(os.path.join(BASE, 'target.txt'), target)
        got = harness.extract(dis, draft or target)
        if got is None and draft is None:
            for line in open(dis, encoding='utf-8', errors='replace'):
                if line.startswith('.fn ' + target + '__'):
                    got = harness.extract(dis, line.split()[1].rstrip(',')); break
        print(' ', target, len(want or []), len(got or []), 'MATCH' if want == got else 'DIFFER')

build('v_calc_decl_f29', lambda s: s.replace(
'''        f32 cos = nw4r::math::CosIdx(rot);\n        f32 sin = nw4r::math::SinIdx(rot);\n\n        f32 cx = mCenter.x;''',
'''        f32 cos;\n        f32 sin;\n        f32 cx = mCenter.x;\n        f32 cy = mCenter.y;\n        f32 ox = mOffset2.x;\n        f32 oy = mOffset2.y;\n        f32 px = mPos.x;\n        f32 py = mPos.y;\n        cos = nw4r::math::CosIdx(rot);\n        sin = nw4r::math::SinIdx(rot);\n\n        f32 unused;''').replace(
'''        f32 cy = mCenter.y;\n        f32 ox = mOffset2.x;\n        f32 oy = mOffset2.y;\n        f32 px = mPos.x;\n        f32 py = mPos.y;\n\n        f32 cxc''',
'''        f32 cxc'''))

build('v_revise_order', lambda s: s.replace(
'''    f32 f0 = rawF32(this, 0x9C);\n    f32 f1 = rawF32(actor, 0xB4);\n    f32 f3 = rawF32(actor, 0xB0);\n    f32 f4 = f1 - f0;\n    f32 f2 = rawF32(this, 0x98);\n    f1 = rawF32(actor, 0xAC);\n    f0 = rawF32(this, 0x94);\n    f2 = f3 - f2;\n    f1 = f1 - f0;''', '''    f32 oldY = rawF32(this, 0x9C);\n    f32 actorY = rawF32(actor, 0xB4);\n    f32 actorZ = rawF32(actor, 0xB0);\n    f32 oldZ = rawF32(this, 0x98);\n    f32 actorX = rawF32(actor, 0xAC);\n    f32 oldX = rawF32(this, 0x94);\n    f32 f4 = actorY - oldY;\n    f32 f2 = actorZ - oldZ;\n    f32 f1 = actorX - oldX;'''))

build('v_dokan_target_shape', lambda s: s.replace(
'''    f32 dx = out->x - mPos.x;
    f32 dy = out->y - mPos.y;

    f32 len = EGG::Math<f32>::sqrt(dx * dx + dy * dy);

    s16 angle = (s16)(nw4r::math::Atan2Idx(dy, dx) + m_c4);

    f32 cos = nw4r::math::CosIdx(angle);
    f32 sin = nw4r::math::SinIdx(angle);

    f32 corrected_dx = len * cos - dx;
    f32 corrected_dy = len * sin - dy;

    out->x = dScStage_c::getLoopPosX(mPos.x + len * nw4r::math::CosIdx(angle));
    out->y = mPos.y + len * nw4r::math::SinIdx(angle);''', '''    f32 dy = out->y - mPos.y;
    f32 dx = out->x - mPos.x;
    f32 len = EGG::Math<f32>::sqrt(dx * dx + dy * dy);
    s16 angle = (s16)(nw4r::math::Atan2Idx(dy, dx) + m_c4);
    s16 angle2 = angle;
    f32 cos = nw4r::math::CosIdx(angle);
    f32 sin = nw4r::math::SinIdx(angle);
    f32 x = len * cos;
    f32 y = len * sin;
    out->x = dScStage_c::getLoopPosX(mPos.x + x);
    out->y = mPos.y + y;'''))

build('v_fn809_liveness', lambda s: s.replace(
'''    // edge segment and result on stack
    nw4r::math::SEGMENT3 edgeSeg;
    f32 distSqResult;
    f32 *retDistSq = &distSqResult;
    int found = 0;''', '''    // edge segment and result on stack
    nw4r::math::SEGMENT3 edgeSeg;
    nw4r::math::SEGMENT3 edgeStart;
    nw4r::math::SEGMENT3 edgeEnd;
    mVec3_c hitResult;
    mVec3_c angleResult;
    f32 distSqResult;
    f32 *retDistSq = &distSqResult;
    int found = 0;'''))

build('v_filter_dc_first', lambda s: s.replace(
'''    if (m_d4 != 0) {\n        return false;\n    }\n\n    if (!(*(const u8 *)((const u8 *)bc + 0xE5) & m_dd)) {''', '''    if (*(const u8 *)((const u8 *)bc + 0xDC) != 0) {\n        return false;\n    }\n\n    if (m_d4 != 0) {\n        return false;\n    }\n\n    if (!(*(const u8 *)((const u8 *)bc + 0xE5) & m_dd)) {'''))

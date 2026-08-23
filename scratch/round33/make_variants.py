from pathlib import Path

base = Path(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round33\d_bg_ctr')
source = (base / 'w1_direct_reads.cpp').read_text()
center_old = '''    mVec3_c centerPos = mpActor->getCenterPos();
    dScStage_c::getLoopPosX(centerPos.x);

    mPos.x = centerPos.x;
    mPos.y = centerPos.y;'''
center_variants = {
    'v3_center_components': '''    mVec3_c centerPos = mpActor->getCenterPos();
    f32 centerX = centerPos.x;
    f32 centerY = centerPos.y;
    dScStage_c::getLoopPosX(centerX);

    mPos.x = centerX;
    mPos.y = centerY;''',
    'v4_center_x_spill': '''    mVec3_c centerPos = mpActor->getCenterPos();
    f32 centerX = centerPos.x;
    dScStage_c::getLoopPosX(centerX);
    centerPos.x = centerX;

    mPos.x = centerPos.x;
    mPos.y = centerPos.y;''',
    'v5_trig_decl_order': center_old,
    'v6_trig_split_assign': center_old,
}
trig_old = '''        f32 cos = nw4r::math::CosIdx(rot);
        f32 sin = nw4r::math::SinIdx(rot);'''
trig_variants = {
    'v5_trig_decl_order': '''        f32 sin;
        f32 cos = nw4r::math::CosIdx(rot);
        sin = nw4r::math::SinIdx(rot);''',
    'v6_trig_split_assign': '''        f32 cos;
        f32 sin;
        cos = nw4r::math::CosIdx(rot);
        sin = nw4r::math::SinIdx(rot);''',
}
for name, center in center_variants.items():
    if name in ('v5_trig_decl_order', 'v6_trig_split_assign'):
        text = source
    else:
        text = source.replace(center_old, center, 1)
        if text == source:
            raise RuntimeError(name + ': center block not found')
    if name in trig_variants:
        text = text.replace(trig_old, trig_variants[name], 1)
    (base / (name + '.cpp')).write_text(text)

from pathlib import Path

base = Path(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round33\d_bg_ctr')
source = (base / 'w1_direct_reads.cpp').read_text()
old = '''    mVec3_c centerPos = mpActor->getCenterPos();
    dScStage_c::getLoopPosX(centerPos.x);

    mPos.x = centerPos.x;
    mPos.y = centerPos.y;'''
variants = {
    'v7_center_address': '''    mVec3_c centerPos = mpActor->getCenterPos();
    mVec3_c *centerPtr = &centerPos;
    dScStage_c::getLoopPosX(centerPtr->x);

    mPos.x = centerPtr->x;
    mPos.y = centerPtr->y;''',
    'v8_center_pointer_helper': '''    mVec3_c centerPos = mpActor->getCenterPos();
    mVec3_c *centerPtr = &centerPos;
    dScStage_c::getLoopPosX(centerPtr->x);

    mPos.x = centerPtr->x;
    mPos.y = centerPtr->y;''',
    'v9_center_cast_cursor': '''    mVec3_c centerPos = mpActor->getCenterPos();
    const f32 *centerValues = reinterpret_cast<const f32 *>(&centerPos);
    dScStage_c::getLoopPosX(*(f32 *)centerValues);

    mPos.x = centerValues[0];
    mPos.y = centerValues[1];''',
    'v10_center_separate_assign': '''    mVec3_c centerPos;
    centerPos = mpActor->getCenterPos();
    dScStage_c::getLoopPosX(centerPos.x);

    mPos.x = centerPos.x;
    mPos.y = centerPos.y;''',
}
for name, block in variants.items():
    text = source.replace(old, block, 1)
    if text == source:
        raise RuntimeError(name + ': source block not found')
    (base / (name + '.cpp')).write_text(text)

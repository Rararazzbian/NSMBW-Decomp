from pathlib import Path

base = Path(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round33\d_bg_ctr')
source = (base / 'w1_direct_reads.cpp').read_text()
old = '''        f32 px = mPos.x;
        f32 py = mPos.y;

        f32 cxc = mCenter.x * cos;'''
variants = {
    'v11_center_else_direct': '''        f32 px = centerPos.x;
        f32 py = centerPos.y;

        f32 cxc = mCenter.x * cos;''',
    'v12_center_else_pointer': '''        mVec3_c *centerPtr = &centerPos;
        f32 px = centerPtr->x;
        f32 py = centerPtr->y;

        f32 cxc = mCenter.x * cos;''',
    'v13_center_else_cast': '''        const f32 *centerValues = reinterpret_cast<const f32 *>(&centerPos);
        f32 px = centerValues[0];
        f32 py = centerValues[1];

        f32 cxc = mCenter.x * cos;''',
    'v14_center_a0_direct': '''        f32 px = mPos.x;
        f32 py = mPos.y;

        f32 cxc = mCenter.x * cos;''',
}
for name, block in variants.items():
    text = source.replace(old, block, 1)
    if name == 'v14_center_a0_direct':
        text = text.replace('''        m_a0.x = (mScratch[0].x + mScratch[2].x) * 0.5f;
        m_a0.y = (mScratch[0].y + mScratch[2].y) * 0.5f;''', '''        m_a0.x = (centerPos.x + mScratch[2].x) * 0.5f;
        m_a0.y = (centerPos.y + mScratch[2].y) * 0.5f;''', 1)
    (base / (name + '.cpp')).write_text(text)

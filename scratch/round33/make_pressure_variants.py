from pathlib import Path

base = Path(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round33\d_bg_ctr')
source = (base / 'w1_direct_reads.cpp').read_text()
old = '''        mScratch[0].x = px + (cxc - cys);
        mScratch[0].y = py + (cyc + cxs);

        mScratch[1].x = px + (cxc - oys);
        mScratch[1].y = py + (oyc + cxs);

        mScratch[2].x = px + (oxc - oys);
        mScratch[2].y = py + (oyc + oxs);

        mScratch[3].x = px + (oxc - cys);
        mScratch[3].y = py + (cyc + oxs);'''
variants = {
    'v15_corner_at_time': '''        mScratch[0].x = px + (mCenter.x * cos - mCenter.y * sin);
        mScratch[0].y = py + (mCenter.y * cos + mCenter.x * sin);
        mScratch[1].x = px + (mCenter.x * cos - mOffset2.y * sin);
        mScratch[1].y = py + (mOffset2.y * cos + mCenter.x * sin);
        mScratch[2].x = px + (mOffset2.x * cos - mOffset2.y * sin);
        mScratch[2].y = py + (mOffset2.y * cos + mOffset2.x * sin);
        mScratch[3].x = px + (mOffset2.x * cos - mCenter.y * sin);
        mScratch[3].y = py + (mCenter.y * cos + mOffset2.x * sin);''',
    'v16_hoist_all_products': '''        f32 p0x = mCenter.x * cos - mCenter.y * sin;
        f32 p0y = mCenter.y * cos + mCenter.x * sin;
        f32 p1x = mCenter.x * cos - mOffset2.y * sin;
        f32 p1y = mOffset2.y * cos + mCenter.x * sin;
        f32 p2x = mOffset2.x * cos - mOffset2.y * sin;
        f32 p2y = mOffset2.y * cos + mOffset2.x * sin;
        f32 p3x = mOffset2.x * cos - mCenter.y * sin;
        f32 p3y = mCenter.y * cos + mOffset2.x * sin;
        mScratch[0].x = px + p0x;
        mScratch[0].y = py + p0y;
        mScratch[1].x = px + p1x;
        mScratch[1].y = py + p1y;
        mScratch[2].x = px + p2x;
        mScratch[2].y = py + p2y;
        mScratch[3].x = px + p3x;
        mScratch[3].y = py + p3y;''',
    'v17_group_by_factor': '''        f32 cx = mCenter.x * cos;
        f32 cy = mCenter.y * cos;
        f32 ox = mOffset2.x * cos;
        f32 oy = mOffset2.y * cos;
        f32 sx = mCenter.x * sin;
        f32 sy = mCenter.y * sin;
        f32 tx = mOffset2.x * sin;
        f32 ty = mOffset2.y * sin;
        mScratch[0].x = px + (cx - sy);
        mScratch[0].y = py + (cy + sx);
        mScratch[1].x = px + (cx - ty);
        mScratch[1].y = py + (oy + sx);
        mScratch[2].x = px + (ox - ty);
        mScratch[2].y = py + (oy + tx);
        mScratch[3].x = px + (ox - sy);
        mScratch[3].y = py + (cy + tx);''',
    'v18_named_sums': '''        f32 s0x = cxc - cys;
        f32 s0y = cyc + cxs;
        f32 s1x = cxc - oys;
        f32 s1y = oyc + cxs;
        f32 s2x = oxc - oys;
        f32 s2y = oyc + oxs;
        f32 s3x = oxc - cys;
        f32 s3y = cyc + oxs;
        mScratch[0].x = px + s0x;
        mScratch[0].y = py + s0y;
        mScratch[1].x = px + s1x;
        mScratch[1].y = py + s1y;
        mScratch[2].x = px + s2x;
        mScratch[2].y = py + s2y;
        mScratch[3].x = px + s3x;
        mScratch[3].y = py + s3y;''',
}
for name, block in variants.items():
    text = source.replace(old, block, 1)
    if text == source:
        raise RuntimeError(name + ': arithmetic block not found')
    (base / (name + '.cpp')).write_text(text)

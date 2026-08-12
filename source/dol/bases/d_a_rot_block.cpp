#include <game/bases/d_a_rot_block.hpp>
#include <game/framework/f_manager.hpp>

void daRotBlock_c::rot_block_init(f32 width, f32 height) {
    mPos.x += 0.5f * width;
    mPos.y += -0.5f * height;

    f32 size = width > height ? width : height;
    size *= 1.5f;

    mVisibleAreaOffset.set(0.0f, 0.0f);
    mVisibleAreaSize.set(size, size);

    mObjBgData[0].mTopLeft = mVec2_c(0.5f * -width, 0.5f * height);
    mObjBgData[0].mBottomRight = mVec2_c(0.5f * width, 0.5f * -height);

    setBgData(mBgCtr, mBgCtr + 1, mObjBgData);
}

bool daRotBlock_c::scroll_out_check() {
    return false;
}

daRotBlock_c::~daRotBlock_c() {}


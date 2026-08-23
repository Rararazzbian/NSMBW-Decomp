#include <game/bases/d_gx_state_save.hpp>

GXStateSave_c::GXStateSave_c() : mMask(0) {}

GXStateSave_c::~GXStateSave_c() {}

void GXStateSave_c::save(unsigned long mask) {
    if (mask & 1) {
        GXGetVtxDescv(mVtxDesc);
    }
    if (mask & 2) {
        GXGetVtxAttrFmtv(GX_VTXFMT0, mVtxAttrFmt);
    }
    if (mask & 4) {
        GXGetProjectionv(reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(this) + 0x28C));
    }
    if (mask & 8) {
        GXGetViewportv(mViewport);
    }
    if (mask & 16) {
        GXGetCullMode(&mCullMode);
    }
    if (mask & 32) {
        GXGetScissor(&mScissor[0], &mScissor[1], &mScissor[2], &mScissor[3]);
    }
    if (mask & 64) {
        mColorUpdate = EGG::StateGX::s_cacheGX[0xC];
    }
    if (mask & 128) {
        mAlphaUpdate = EGG::StateGX::s_cacheGX[0xD];
    }
    if (mask & 256) {
        mDither = EGG::StateGX::s_cacheGX[0xE];
    }
    mMask |= mask;
}

void GXStateSave_c::restore() {
    if (mMask & 1) {
        GXSetVtxDescv(mVtxDesc);
    }
    if (mMask & 2) {
        GXSetVtxAttrFmtv(GX_VTXFMT0, mVtxAttrFmt);
    }
    if (mMask & 128) {
        GXSetCullMode(mCullMode);
    }
    if (mMask & 4) {
        GXSetProjectionv(mProjection);
    }
    if (mMask & 8) {
        float* viewport = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(this) + 0x2A8);
        GXSetViewport(viewport[0], viewport[1], viewport[2], viewport[3], viewport[4], viewport[5]);
    }
    if (mMask & 32) {
        unsigned long* scissor = reinterpret_cast<unsigned long*>(reinterpret_cast<unsigned char*>(this) + 0x2C0);
        GXSetScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
    }
    if (mMask & 64) {
        GXSetColorUpdate(reinterpret_cast<unsigned char*>(this)[0x2D4] != 0);
    }
    if (mMask & 128) {
        GXSetAlphaUpdate(reinterpret_cast<unsigned char*>(this)[0x2D5] != 0);
    }
    if (mMask & 256) {
        GXSetDither(reinterpret_cast<unsigned char*>(this)[0x2D6] != 0);
    }
    mMask = 0;
}

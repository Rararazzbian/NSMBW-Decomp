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
        EGG::StateGX::GXGetScissor_(&mScissor[0], &mScissor[1], &mScissor[2], &mScissor[3]);
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
    if (mMask & 16) {
        GXSetCullMode(mCullMode);
    }
    if (mMask & 4) {
        EGG::StateGX::GXSetProjectionv_(mProjection);
    }
    if (mMask & 8) {
        EGG::StateGX::GXSetViewport_(mViewport[0], mViewport[1], mViewport[2], mViewport[3], mViewport[4], mViewport[5]);
    }
    if (mMask & 32) {
        EGG::StateGX::GXSetScissor_(mScissor[0], mScissor[1], mScissor[2], mScissor[3]);
    }
    if (mMask & 64) {
        EGG::StateGX::GXSetColorUpdate_(mColorUpdate != 0);
    }
    if (mMask & 128) {
        EGG::StateGX::GXSetAlphaUpdate_(mAlphaUpdate != 0);
    }
    if (mMask & 256) {
        EGG::StateGX::GXSetDither_(mDither != 0);
    }
    mMask = 0;
}

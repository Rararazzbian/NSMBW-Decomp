#pragma once

#include <revolution/GX/GXAttr.h>
#include <revolution/GX/GXGeometry.h>
#include <revolution/GX/GXPixel.h>
#include <revolution/GX/GXTransform.h>

namespace EGG {
namespace StateGX {
extern u8 s_cacheGX[0x14];
void GXGetScissor_(unsigned long *x, unsigned long *y, unsigned long *w, unsigned long *h);
void GXSetProjectionv_(const f32 *mtx);
void GXSetViewport_(f32 x, f32 y, f32 w, f32 h, f32 nearZ, f32 farZ);
void GXSetScissor_(unsigned long x, unsigned long y, unsigned long w, unsigned long h);
void GXSetColorUpdate_(bool update);
void GXSetAlphaUpdate_(bool update);
void GXSetDither_(bool dither);
}
}

class GXStateSave_c {
public:
    GXStateSave_c();
    ~GXStateSave_c();
    void save(unsigned long mask);
    void restore();

private:
    unsigned long mMask;
    GXVtxAttrFmtList mVtxAttrFmt[27];
    GXVtxDescList mVtxDesc[27];
    f32 mProjection[7];
    f32 mViewport[6];
    unsigned long mScissor[4];
    GXCullMode mCullMode;
    GXBool mColorUpdate;
    GXBool mAlphaUpdate;
    GXBool mDither;
};

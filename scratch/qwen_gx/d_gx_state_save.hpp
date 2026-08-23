#ifndef GAME_BASES_D_GX_STATE_SAVE_HPP
#define GAME_BASES_D_GX_STATE_SAVE_HPP

#include <revolution/GX/GXAttr.h>
#include <revolution/GX/GXGeometry.h>
#include <revolution/GX/GXPixel.h>
#include <revolution/GX/GXTransform.h>

namespace EGG {
namespace StateGX {
extern unsigned char s_cacheGX[0x10];
void GXGetScissor_(u32 *x, u32 *y, u32 *w, u32 *h);
void GXSetProjectionv_(const f32 *mtx);
void GXSetViewport_(f32 x, f32 y, f32 w, f32 h, f32 nearZ, f32 farZ);
void GXSetScissor_(u32 x, u32 y, u32 w, u32 h);
void GXSetColorUpdate_(GXBool update);
void GXSetAlphaUpdate_(GXBool update);
void GXSetDither_(GXBool dither);
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
    u32 mScissor[4];
    GXCullMode mCullMode;
    GXBool mColorUpdate;
    GXBool mAlphaUpdate;
    GXBool mDither;
};

#endif

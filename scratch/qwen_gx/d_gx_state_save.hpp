#ifndef GAME_BASES_D_GX_STATE_SAVE_HPP
#define GAME_BASES_D_GX_STATE_SAVE_HPP

#include <revolution/GX/GXAttr.h>
#include <revolution/GX/GXGeometry.h>
#include <revolution/GX/GXPixel.h>
#include <revolution/GX/GXTransform.h>

class GXStateSave_c {
public:
    GXStateSave_c();
    virtual ~GXStateSave_c();
    void save(unsigned long mask);
    void restore();

private:
    unsigned long mMask;
    GXVtxAttrFmtList mVtxAttrFmt[32];
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

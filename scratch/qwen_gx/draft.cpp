#include <game/bases/d_gx_state_save.hpp>

class GXStateSave_c {
public:
    GXStateSave_c();
    ~GXStateSave_c();
    void save(unsigned long mask);
    void restore();

private:
    unsigned long mMask;
    unsigned char mData[0x2D3];
};

GXStateSave_c::GXStateSave_c() : mMask(0) {}

GXStateSave_c::~GXStateSave_c() {}

void GXStateSave_c::save(unsigned long mask) {
    if (mask & 1) {
        GXGetVtxDescv(reinterpret_cast<GXVtxDescList*>(reinterpret_cast<unsigned char*>(this) + 0x1B4));
    }
    if (mask & 2) {
        GXGetVtxAttrFmtv(0, reinterpret_cast<GXVtxAttrFmtList*>(reinterpret_cast<unsigned char*>(this) + 4));
    }
    if (mask & 4) {
        GXGetProjectionv(reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(this) + 0x28C));
    }
    if (mask & 8) {
        GXGetViewport(reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(this) + 0x2A8));
    }
    if (mask & 16) {
        *reinterpret_cast<GXCullMode*>(reinterpret_cast<unsigned char*>(this) + 0x2D0) = GXGetCullMode();
    }
    if (mask & 32) {
        GXGetScissor(reinterpret_cast<unsigned long*>(reinterpret_cast<unsigned char*>(this) + 0x2C0),
                     reinterpret_cast<unsigned long*>(reinterpret_cast<unsigned char*>(this) + 0x2C4),
                     reinterpret_cast<unsigned long*>(reinterpret_cast<unsigned char*>(this) + 0x2C8),
                     reinterpret_cast<unsigned long*>(reinterpret_cast<unsigned char*>(this) + 0x2CC));
    }
    if (mask & 64) {
        reinterpret_cast<unsigned char*>(this)[0x2D4] = GXGetColorUpdate();
    }
    if (mask & 128) {
        reinterpret_cast<unsigned char*>(this)[0x2D5] = GXGetAlphaUpdate();
    }
    if (mask & 256) {
        reinterpret_cast<unsigned char*>(this)[0x2D6] = GXGetDither();
    }
    mMask |= mask;
}

void GXStateSave_c::restore() {
    if (mMask & 1) {
        GXSetVtxDescv(reinterpret_cast<GXVtxDescList*>(reinterpret_cast<unsigned char*>(this) + 0x1B4));
    }
    if (mMask & 2) {
        GXSetVtxAttrFmtv(0, reinterpret_cast<GXVtxAttrFmtList*>(reinterpret_cast<unsigned char*>(this) + 4));
    }
    if (mMask & 128) {
        GXSetCullMode(*reinterpret_cast<GXCullMode*>(reinterpret_cast<unsigned char*>(this) + 0x2D0));
    }
    if (mMask & 4) {
        GXSetProjection(reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(this) + 0x28C));
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

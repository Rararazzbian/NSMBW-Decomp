#include <revolution/GX/GXAttr.h>

#include <revolution/GX/GXHardware.h>
#include <revolution/GX/GXHardwareCP.h>

void GXSetArray(GXAttr attr, const void *base, u8 stride) {
    u32 idx;

    if (attr == GX_VA_NBT) {
        attr = GX_VA_NRM;
    }
    idx = attr - GX_VA_POS;

    GX_CP_LOAD_REG(idx | GX_CP_REG_ARRAYBASE, (u32)base & 0x3FFFFFFF);
    GX_CP_LOAD_REG(idx | GX_CP_REG_ARRAYSTRIDE, stride);
}

void GXInvalidateVtxCache(void) {
    WGPIPE.c = GX_FIFO_CMD_INVAL_VTX;
}

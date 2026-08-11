#include <revolution/GX/GXTransform.h>

#include <revolution/GX/GXHardware.h>
#include <revolution/GX/GXHardwareBP.h>
#include <revolution/GX/GXHardwareCP.h>
#include <revolution/GX/GXHardwareXF.h>
#include <revolution/GX/GXInit.h>

#define GX_SCISSOR_BIAS 0x156

void GXSetScissor(u32 x, u32 y, u32 w, u32 h) {
    u32 tx = x + GX_SCISSOR_BIAS;
    u32 ty = y + GX_SCISSOR_BIAS;

    gxdt->scissorTL = GX_BITSET(gxdt->scissorTL, 21, 11, ty);
    gxdt->scissorTL = GX_BITSET(gxdt->scissorTL, 9, 11, tx);

    gxdt->scissorBR = GX_BITSET(gxdt->scissorBR, 21, 11, ty + h - 1);
    gxdt->scissorBR = GX_BITSET(gxdt->scissorBR, 9, 11, tx + w - 1);

    GX_BP_LOAD_REG(gxdt->scissorTL);
    GX_BP_LOAD_REG(gxdt->scissorBR);
    gxdt->lastWriteWasXF = FALSE;
}

void GXGetScissor(u32 *x, u32 *y, u32 *w, u32 *h) {
    u32 tl = gxdt->scissorTL;
    u32 br = gxdt->scissorBR;
    u32 tx = GX_BITGET(tl, 21, 11);
    u32 ty = GX_BITGET(tl, 9, 11);

    *x = ty - GX_SCISSOR_BIAS;
    *y = tx - GX_SCISSOR_BIAS;
    *w = GX_BITGET(br, 9, 11) - ty + 1;
    *h = GX_BITGET(br, 21, 11) - tx + 1;
}

void GXSetScissorBoxOffset(u32 ox, u32 oy) {
    u32 reg = 0;

    reg = GX_BITSET(reg, 22, 10, (ox + GX_SCISSOR_BIAS) >> 1);
    reg = GX_BITSET(reg, 12, 10, (oy + GX_SCISSOR_BIAS) >> 1);
    reg = GX_BITSET(reg, 0, 8, GX_BP_REG_SCISSOROFFSET);

    GX_BP_LOAD_REG(reg);
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetClipMode(GXClipMode mode) {
    GX_XF_LOAD_REG(GX_XF_REG_CLIPDISABLE, mode);
    gxdt->lastWriteWasXF = TRUE;
}

void __GXSetMatrixIndex(GXAttr index) {
    if (index < GX_VA_TEX4MTXIDX) {
        GX_CP_LOAD_REG(0x30, gxdt->matrixIndex0);
        GX_XF_LOAD_REG(0x1018, gxdt->matrixIndex0);
    } else {
        GX_CP_LOAD_REG(0x40, gxdt->matrixIndex1);
        GX_XF_LOAD_REG(0x1019, gxdt->matrixIndex1);
    }

    gxdt->lastWriteWasXF = TRUE;
}

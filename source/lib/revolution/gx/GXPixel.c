#include <revolution/GX/GXPixel.h>

#include <revolution/GX/GXHardware.h>
#include <revolution/GX/GXHardwareBP.h>
#include <revolution/GX/GXInit.h>

void GXSetColorUpdate(GXBool enable) {
    u32 reg = gxdt->blendMode;

    GX_BP_SET_BLENDMODE_COLOR_UPDATE(reg, enable);
    GX_BP_LOAD_REG(reg);
    gxdt->blendMode = reg;
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetAlphaUpdate(GXBool enable) {
    u32 reg = gxdt->blendMode;

    GX_BP_SET_BLENDMODE_ALPHA_UPDATE(reg, enable);
    GX_BP_LOAD_REG(reg);
    gxdt->blendMode = reg;
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetZMode(GXBool enableTest, GXCompare func, GXBool enableUpdate) {
    u32 reg = gxdt->zMode;

    GX_BP_SET_ZMODE_TEST_ENABLE(reg, enableTest);
    GX_BP_SET_ZMODE_COMPARE(reg, func);
    GX_BP_SET_ZMODE_UPDATE_ENABLE(reg, enableUpdate);
    GX_BP_LOAD_REG(reg);
    gxdt->zMode = reg;
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetZCompLoc(GXBool beforeTex) {
    GX_BP_SET_ZCONTROL_BEFORE_TEX(gxdt->zControl, beforeTex);
    GX_BP_LOAD_REG(gxdt->zControl);
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetPixelFmt(GXPixelFmt fmt, GXZFmt16 zFmt) {
    static u32 p2f[] = {0, 1, 2, 3, 4, 4, 4, 5};
    u32 prev = gxdt->zControl;

    gxdt->zControl = GX_BITSET(gxdt->zControl, 29, 3, p2f[fmt]);
    gxdt->zControl = GX_BITSET(gxdt->zControl, 26, 3, zFmt);

    if (prev != gxdt->zControl) {
        GX_BP_LOAD_REG(gxdt->zControl);
        gxdt->genMode = GX_BITSET(gxdt->genMode, 22, 1, fmt == GX_PF_RGBA565_Z16);
        gxdt->gxDirtyFlags |= 4;
    }

    if (p2f[fmt] == 4) {
        gxdt->dstAlpha = GX_BITSET(gxdt->dstAlpha, 21, 2, fmt - GX_PF_Y8);
        gxdt->dstAlpha = GX_BITSET(gxdt->dstAlpha, 0, 8, GX_BP_REG_DSTALPHA);
        GX_BP_LOAD_REG(gxdt->dstAlpha);
    }

    gxdt->lastWriteWasXF = FALSE;
}

void GXSetDither(GXBool enable) {
    u32 reg = gxdt->blendMode;

    GX_BP_SET_BLENDMODE_DITHER(reg, enable);
    GX_BP_LOAD_REG(reg);
    gxdt->blendMode = reg;
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetDstAlpha(GXBool enable, u8 alpha) {
    u32 reg = gxdt->dstAlpha;

    reg = GX_BITSET(reg, 24, 8, alpha);
    reg = GX_BITSET(reg, 23, 1, enable);
    GX_BP_LOAD_REG(reg);
    gxdt->dstAlpha = reg;
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetFieldMask(GXBool enableEven, GXBool enableOdd) {
    u32 reg = 0;

    reg = GX_BITSET(reg, 31, 1, enableOdd);
    reg = GX_BITSET(reg, 30, 1, enableEven);
    reg = GX_BITSET(reg, 0, 8, GX_BP_REG_FIELDMASK);

    GX_BP_LOAD_REG(reg);
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetFieldMode(GXBool texLOD, GXBool adjustAR) {
    gxdt->linePtWidth = GX_BITSET(gxdt->linePtWidth, 9, 1, adjustAR);
    GX_BP_LOAD_REG(gxdt->linePtWidth);
    __GXFlushTextureState();

    GX_BP_LOAD_REG(texLOD | 0x68000000);
    __GXFlushTextureState();
}

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

#include <revolution/GX/GXTev.h>

#include <revolution/GX/GXHardware.h>
#include <revolution/GX/GXHardwareBP.h>
#include <revolution/GX/GXInit.h>

void GXSetTevColorIn(GXTevStageID stage, GXTevColorArg a, GXTevColorArg b,
                     GXTevColorArg c, GXTevColorArg d) {
    u32 reg = gxdt->tevc[stage];

    reg = GX_BITSET(reg, 16, 4, a);
    reg = GX_BITSET(reg, 20, 4, b);
    reg = GX_BITSET(reg, 24, 4, c);
    reg = GX_BITSET(reg, 28, 4, d);

    GX_BP_LOAD_REG(reg);
    gxdt->tevc[stage] = reg;
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetTevAlphaIn(GXTevStageID stage, GXTevAlphaArg a, GXTevAlphaArg b,
                     GXTevAlphaArg c, GXTevAlphaArg d) {
    u32 reg = gxdt->teva[stage];

    reg = GX_BITSET(reg, 16, 3, a);
    reg = GX_BITSET(reg, 19, 3, b);
    reg = GX_BITSET(reg, 22, 3, c);
    reg = GX_BITSET(reg, 25, 3, d);

    GX_BP_LOAD_REG(reg);
    gxdt->teva[stage] = reg;
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetTevColorOp(GXTevStageID stage, GXTevOp op, GXTevBias bias,
                     GXTevScale scale, GXBool clamp, GXTevRegID outReg) {
    u32 reg = gxdt->tevc[stage];

    reg = GX_BITSET(reg, 13, 1, op);

    if (op <= GX_TEV_SUB) {
        reg = GX_BITSET(reg, 10, 2, scale);
        reg = GX_BITSET(reg, 14, 2, bias);
    } else {
        reg = GX_BITSET(reg, 10, 2, (u32)op >> 1);
        reg |= 0x00030000;
    }

    reg = GX_BITSET(reg, 12, 1, clamp);
    reg = GX_BITSET(reg, 8, 2, outReg);

    GX_BP_LOAD_REG(reg);
    gxdt->tevc[stage] = reg;
    gxdt->lastWriteWasXF = FALSE;
}

void GXSetTevAlphaOp(GXTevStageID stage, GXTevOp op, GXTevBias bias,
                     GXTevScale scale, GXBool clamp, GXTevRegID outReg) {
    u32 reg = gxdt->teva[stage];

    reg = GX_BITSET(reg, 13, 1, op);

    if (op <= GX_TEV_SUB) {
        reg = GX_BITSET(reg, 10, 2, scale);
        reg = GX_BITSET(reg, 14, 2, bias);
    } else {
        reg = GX_BITSET(reg, 10, 2, (u32)op >> 1);
        reg |= 0x00030000;
    }

    reg = GX_BITSET(reg, 12, 1, clamp);
    reg = GX_BITSET(reg, 8, 2, outReg);

    GX_BP_LOAD_REG(reg);
    gxdt->teva[stage] = reg;
    gxdt->lastWriteWasXF = FALSE;
}

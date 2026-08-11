#include <revolution/BASE/PPCArch.h>

asm u32 PPCMfmsr(void) {
    nofralloc
    mfmsr r3
    blr
}

asm void PPCMtmsr(u32 val) {
    nofralloc
    mtmsr r3
    blr
}

asm u32 PPCMfhid0(void) {
    nofralloc
    mfspr r3, HID0
    blr
}

asm void PPCMthid0(u32 val) {
    nofralloc
    mtspr HID0, r3
    blr
}

asm u32 PPCMfl2cr(void) {
    nofralloc
    mfspr r3, L2CR
    blr
}

asm void PPCMtl2cr(u32 val) {
    nofralloc
    mtspr L2CR, r3
    blr
}

asm void PPCMtdec(u32 val) {
    nofralloc
    mtdec r3
    blr
}

asm void PPCSync(void) {
    nofralloc
    sc
    blr
}

asm void PPCHalt(void) {
    nofralloc
    sync
_hloop:
    nop
    li r3, 0
    nop
    b _hloop
}

asm void PPCMtmmcr0(u32 val) {
    nofralloc
    mtspr MMCR0, r3
    blr
}

asm void PPCMtmmcr1(u32 val) {
    nofralloc
    mtspr MMCR1, r3
    blr
}

asm void PPCMtpmc1(u32 val) {
    nofralloc
    mtspr PMC1, r3
    blr
}

asm void PPCMtpmc2(u32 val) {
    nofralloc
    mtspr PMC2, r3
    blr
}

asm void PPCMtpmc3(u32 val) {
    nofralloc
    mtspr PMC3, r3
    blr
}

asm void PPCMtpmc4(u32 val) {
    nofralloc
    mtspr PMC4, r3
    blr
}

asm u32 PPCMffpscr(void) {
    nofralloc
    stwu r1, -0x20(r1)
    stfd f31, 0x18(r1)
    mffs f31
    stfd f31, 0x8(r1)
    lfd f31, 0x18(r1)
    lwz r3, 0xc(r1)
    addi r1, r1, 0x20
    blr
}

asm void PPCMtfpscr(u32 val) {
    nofralloc
    stwu r1, -0x20(r1)
    stfd f31, 0x18(r1)
    li r4, 0
    stw r4, 0x8(r1)
    stw r3, 0xc(r1)
    lfd f31, 0x8(r1)
    mtfsf 255, f31
    lfd f31, 0x18(r1)
    addi r1, r1, 0x20
    blr
}

asm u32 PPCMfhid2(void) {
    nofralloc
    mfspr r3, 920 // HID2; CW's built-in HID2 is SPR 979, not Broadway's 920
    blr
}

asm void PPCMthid2(u32 val) {
    nofralloc
    mtspr 920, r3 // HID2; see PPCMfhid2
    blr
}

asm u32 PPCMfwpar(void) {
    nofralloc
    sync
    mfspr r3, WPAR
    blr
}

asm void PPCMtwpar(u32 val) {
    nofralloc
    mtspr WPAR, r3
    blr
}

void PPCDisableSpeculation(void) {
    PPCMthid0(PPCMfhid0() | HID0_SPD);
}

asm void PPCSetFpNonIEEEMode(void) {
    nofralloc
    mtfsb1 29
    blr
}

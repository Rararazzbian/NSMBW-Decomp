#include <revolution/OS/OSCache.h>

#include <revolution/BASE/PPCArch.h>
#include <revolution/OS/OSInterrupt.h>

asm void DCEnable(void) {
    nofralloc
    sync
    mfspr r3, HID0
    ori r3, r3, 0x4000
    mtspr HID0, r3
    blr
}

asm void DCInvalidateRange(const void *buf, u32 len) {
    nofralloc
    cmplwi r4, 0
    blelr
    clrlwi r5, r3, 27
    add r4, r4, r5
    addi r4, r4, 0x1f
    srwi r4, r4, 5
    mtctr r4
_dcinv:
    dcbi r0, r3
    addi r3, r3, 0x20
    bdnz _dcinv
    blr
}

asm void DCFlushRange(const void *buf, u32 len) {
    nofralloc
    cmplwi r4, 0
    blelr
    clrlwi r5, r3, 27
    add r4, r4, r5
    addi r4, r4, 0x1f
    srwi r4, r4, 5
    mtctr r4
_dcflush:
    dcbf r0, r3
    addi r3, r3, 0x20
    bdnz _dcflush
    sc
    blr
}

asm void DCStoreRange(const void *buf, u32 len) {
    nofralloc
    cmplwi r4, 0
    blelr
    clrlwi r5, r3, 27
    add r4, r4, r5
    addi r4, r4, 0x1f
    srwi r4, r4, 5
    mtctr r4
_dcstore:
    dcbst r0, r3
    addi r3, r3, 0x20
    bdnz _dcstore
    sc
    blr
}

asm void DCFlushRangeNoSync(const void *buf, u32 len) {
    nofralloc
    cmplwi r4, 0
    blelr
    clrlwi r5, r3, 27
    add r4, r4, r5
    addi r4, r4, 0x1f
    srwi r4, r4, 5
    mtctr r4
_dcflushns:
    dcbf r0, r3
    addi r3, r3, 0x20
    bdnz _dcflushns
    blr
}

asm void DCStoreRangeNoSync(const void *buf, u32 len) {
    nofralloc
    cmplwi r4, 0
    blelr
    clrlwi r5, r3, 27
    add r4, r4, r5
    addi r4, r4, 0x1f
    srwi r4, r4, 5
    mtctr r4
_dcstorens:
    dcbst r0, r3
    addi r3, r3, 0x20
    bdnz _dcstorens
    blr
}

asm void DCZeroRange(const void *buf, u32 len) {
    nofralloc
    cmplwi r4, 0
    blelr
    clrlwi r5, r3, 27
    add r4, r4, r5
    addi r4, r4, 0x1f
    srwi r4, r4, 5
    mtctr r4
_dczero:
    dcbz r0, r3
    addi r3, r3, 0x20
    bdnz _dczero
    blr
}

asm void ICInvalidateRange(const void *buf, u32 len) {
    nofralloc
    cmplwi r4, 0
    blelr
    clrlwi r5, r3, 27
    add r4, r4, r5
    addi r4, r4, 0x1f
    srwi r4, r4, 5
    mtctr r4
_icinv:
    icbi r0, r3
    addi r3, r3, 0x20
    bdnz _icinv
    sync
    isync
    blr
}

asm void ICFlashInvalidate(void) {
    nofralloc
    mfspr r3, HID0
    ori r3, r3, 0x800
    mtspr HID0, r3
    blr
}

asm void ICEnable(void) {
    nofralloc
    isync
    mfspr r3, HID0
    ori r3, r3, 0x8000
    mtspr HID0, r3
    blr
}

asm void __LCEnable(void) {
    nofralloc
    mfmsr r5
    ori r5, r5, 0x1000
    mtmsr r5
    lis r3, 0x8000
    li r4, 0x400
    mtctr r4
_lcflush:
    dcbt r0, r3
    dcbst r0, r3
    addi r3, r3, 0x20
    bdnz _lcflush
    mfspr r4, 920
    oris r4, r4, 0x100f
    mtspr 920, r4
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    lis r3, 0xe000
    ori r3, r3, 0x2
    mtdbatl 3, r3
    ori r3, r3, 0x1fe
    mtdbatu 3, r3
    isync
    lis r3, 0xe000
    li r6, 0x200
    mtctr r6
    li r6, 0
_lczero:
    dcbz_l r6, r3
    addi r3, r3, 0x20
    bdnz _lczero
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    blr
}

void LCEnable(void) {
    BOOL enabled = OSDisableInterrupts();
    __LCEnable();
    OSRestoreInterrupts(enabled);
}

asm void LCDisable(void) {
    nofralloc
    lis r3, 0xe000
    li r4, 0x200
    mtctr r4
_lcdis:
    dcbi r0, r3
    addi r3, r3, 0x20
    bdnz _lcdis
    mfspr r4, 920 // HID2
    rlwinm r4, r4, 0, 4, 2
    mtspr 920, r4
    blr
}

asm void LCLoadBlocks(void *dst, const void *src, u32 blocks) {
    nofralloc
    extrwi r6, r5, 5, 25
    clrlwi r4, r4, 3
    or r6, r6, r4
    mtspr 922, r6 // DMA_U
    clrlslwi r6, r5, 30, 2
    or r6, r6, r3
    ori r6, r6, 0x12
    mtspr 923, r6 // DMA_L
    blr
}

asm void LCStoreBlocks(void *dst, const void *src, u32 blocks) {
    nofralloc
    extrwi r6, r5, 5, 25
    clrlwi r3, r3, 3
    or r6, r6, r3
    mtspr 922, r6 // DMA_U
    clrlslwi r6, r5, 30, 2
    or r6, r6, r4
    ori r6, r6, 0x2
    mtspr 923, r6 // DMA_L
    blr
}

u32 LCStoreData(void *dst, const void *src, u32 len) {
    u32 blocks = (len + 31) >> 5;
    u32 queued = (blocks + 127) >> 7;

    while (blocks != 0) {
        if (blocks < 128) {
            LCStoreBlocks(dst, src, blocks);
            blocks = 0;
        } else {
            LCStoreBlocks(dst, src, 0);
            blocks -= 128;
            dst = (u8 *)dst + 0x1000;
            src = (const u8 *)src + 0x1000;
        }
    }

    return queued;
}

asm u32 LCQueueLength(void) {
    nofralloc
    mfspr r4, 920 // HID2
    extrwi r3, r4, 4, 4
    blr
}

asm void LCQueueWait(u32 len) {
    nofralloc
_lcwait:
    mfspr r4, 920 // HID2
    extrwi r4, r4, 4, 4
    cmpw r4, r3
    bgt _lcwait
    blr
}

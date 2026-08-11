#include <revolution/OS/OSCache.h>

#include <revolution/BASE/PPCArch.h>

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

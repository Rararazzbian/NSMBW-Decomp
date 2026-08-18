#pragma once
#include <types.h>

// dPyEffect_c ? sizeof must be 0x13C (316 bytes)
// vtable at 0x80317E14
// Member layout from fn_800D2BB0 disasm:
//   0x118: float pos_x
//   0x11C: float pos_y
//   0x120: float pos_z
//   0x124: float scl_x
//   0x128: float scl_y
//   0x12C: float scl_z
//   0x130: u8   layer
//   0x134: int  effId
//   0x138: int  active
// 0x13C - 4 (vptr) = 0x138 of data
class dPyEffect_c {
public:
    virtual ~dPyEffect_c() {}
    virtual void update() {}
    virtual void fn_800D2BB0(float, int, void*, u8) { return; }
    u8 pad_data[0x138];
};
static_assert(sizeof(dPyEffect_c) == 0x13C, "dPyEffect_c size wrong");

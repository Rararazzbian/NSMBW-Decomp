#pragma once

#include <lib/revolution/OS.h>

namespace EGG {

class Heap;

class Thread {
public:
    /// @note `unsigned long`, not `u32`. u32 is `unsigned int` here and would
/// mangle Ui; the symbol is __ct__Q23EGG6ThreadFUliiPQ23EGG4Heap.
    Thread(unsigned long stackSize, int msgCount, int priority, Heap *heap);
    Thread(OSThread *, int);

    /// @note run(), onEnter() and onExit() MUST keep their inline bodies. The
    /// original emits weak out-of-line copies of onEnter/onExit at the tail of
    /// a deriving TU's .text; declared without bodies, MWCC emits nothing and
    /// that TU's .text comes up 0x10 short. Proven both ways by compiled test
    /// fixture. @unofficial
    virtual ~Thread();
    virtual void *run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}

    static void initialize();

    /// @note 0x4 vtable pointer + 0x48 = sizeof 0x4C, unchanged. d_system.cpp
    /// allocates one with `li r3, 0x4c` and is byte-identical either way.
    u8 mPad[0x48];
};

} // namespace EGG

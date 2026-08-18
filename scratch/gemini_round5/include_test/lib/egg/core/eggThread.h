#pragma once

#include <lib/revolution/OS.h>

namespace EGG {

class Heap;

class Thread {
public:
    Thread(u8* stack, u32 stackSize, int msgCount, int priority);
    Thread(OSThread*, int);
    virtual ~Thread();
    virtual void* run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}

    static void initialize();

    u8 mPad[0x48];
};

} // namespace EGG

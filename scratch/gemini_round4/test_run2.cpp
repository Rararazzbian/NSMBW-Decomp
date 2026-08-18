
#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>

namespace EGG {
class Heap;
class Thread {
public:
    Thread(u32 stackSize, int msgCount, int priority, Heap* heap);
    virtual ~Thread();
    virtual void* run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}
};
}

class dNandThread_c : public EGG::Thread {
public:
    dNandThread_c(int priority, EGG::Heap* heap);
    virtual ~dNandThread_c();
    virtual void* run();
};

dNandThread_c::dNandThread_c(int priority, EGG::Heap* heap) : EGG::Thread(0x4000, 0, priority, heap) {}
dNandThread_c::~dNandThread_c() {}
void* dNandThread_c::run() { return 0; }

// What if there is a function taking EGG::Thread* and calling run()?
void dummy(EGG::Thread* t) {
    t->run();
}

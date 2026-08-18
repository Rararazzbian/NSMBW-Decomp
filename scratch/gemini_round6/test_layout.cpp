
#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggThread.h>

typedef struct OSCond {
    OSThreadQueue queue;
} OSCond;

namespace EGG {
class Mutex {
public:
    Mutex() {}
    virtual ~Mutex() {}
};
}

class mMutex : public EGG::Mutex {
public:
    mMutex() {}
    virtual ~mMutex() {}
    OSMutex mOSMutex;
    OSCond mOSCond;
};

class TestA : public EGG::Thread {
public:
    mMutex mMutex;
};

class TestB : public EGG::Thread {
public:
    u32 mUnknownField;
    mMutex mMutex;
};

size_t sizeA = sizeof(TestA);
size_t offA = offsetof(TestA, mMutex);
size_t sizeB = sizeof(TestB);
size_t offB = offsetof(TestB, mMutex);

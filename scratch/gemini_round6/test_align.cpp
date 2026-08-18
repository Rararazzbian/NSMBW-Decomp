
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

size_t align_Thread = __alignof__(EGG::Thread);
size_t align_Mutex = __alignof__(mMutex);
size_t align_OSMutex = __alignof__(OSMutex);
size_t align_OSCond = __alignof__(OSCond);
size_t align_OSThreadQueue = __alignof__(OSThreadQueue);

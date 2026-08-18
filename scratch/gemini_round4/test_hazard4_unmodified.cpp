
#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>
#include <lib/egg/core/eggThread.h>

// Forward declare EGG::Mutex since eggMutex.h doesn't exist
namespace EGG {
class Mutex {
public:
    virtual ~Mutex() {}
};
}

typedef struct OSCond {
    OSThreadQueue queue;
} OSCond;

class mMutex : public EGG::Mutex {
public:
    virtual ~mMutex() {}
    OSMutex mOSMutex;
    OSCond mOSCond;
};

class dNandThread_c : public EGG::Thread {
public:
    dNandThread_c(int priority, void* heap);
    virtual ~dNandThread_c();
    virtual void* run();

    mMutex mMutex;
    u32 mCommand;
    u32 mStatus;
    u8 mFileExists;
    u8 mPad[3];
};

STATIC_ASSERT(sizeof(EGG::Thread) == 0x4C); // Because eggThread.h lacks virtual destructor
STATIC_ASSERT(sizeof(mMutex) == 0x24);
STATIC_ASSERT(sizeof(dNandThread_c) == 0x80);
STATIC_ASSERT(offsetof(dNandThread_c, mMutex) == 0x50);
STATIC_ASSERT(offsetof(dNandThread_c, mCommand) == 0x74);
STATIC_ASSERT(offsetof(dNandThread_c, mStatus) == 0x78);
STATIC_ASSERT(offsetof(dNandThread_c, mFileExists) == 0x7C);

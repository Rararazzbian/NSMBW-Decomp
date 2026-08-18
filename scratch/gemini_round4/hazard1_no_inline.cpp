
#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>

namespace EGG {
class Heap;
class Mutex {
public:
    virtual ~Mutex() {}
};

class Thread {
public:
    Thread(u32 stackSize, int msgCount, int priority, Heap* heap);
    virtual ~Thread();
    virtual void* run();
    virtual void onEnter();
    virtual void onExit();
    u8 mPad[0x4c];
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
    dNandThread_c(int priority, EGG::Heap* heap);
    virtual ~dNandThread_c();
    virtual void* run();

    bool cmdExistCheck();
    void existCheck();
    bool cmdSpaceCheck();
    void spaceCheck();
    bool cmdSave(const void* src);
    u32 save();
    void createBanner();
    void writeBanner(NANDFileInfo* fileInfo);
    bool cmdLoad();
    u32 load();
    void checkCRC();
    bool cmdDeleteFile();
    void deleteFile();
    void setNandError(s32 error);
    void* getSaveData();

    static dNandThread_c* create(EGG::Heap* heap);
    static dNandThread_c* m_instance;

    mMutex mMutex;
    u32 mCommand;
    u32 mStatus;
    u8 mFileExists;
    u8 mPad[3];
};

dNandThread_c* dNandThread_c::m_instance;

dNandThread_c::dNandThread_c(int priority, EGG::Heap* heap) : EGG::Thread(0x4000, 0, priority, heap) {}
dNandThread_c::~dNandThread_c() {}
bool dNandThread_c::cmdExistCheck() { return false; }
void dNandThread_c::existCheck() {}
bool dNandThread_c::cmdSpaceCheck() { return false; }
void dNandThread_c::spaceCheck() {}
bool dNandThread_c::cmdSave(const void* src) { return false; }
u32 dNandThread_c::save() { return 0; }
void dNandThread_c::createBanner() {}
void dNandThread_c::writeBanner(NANDFileInfo* fileInfo) {}
bool dNandThread_c::cmdLoad() { return false; }
u32 dNandThread_c::load() { return 0; }
void dNandThread_c::checkCRC() {}
bool dNandThread_c::cmdDeleteFile() { return false; }
void dNandThread_c::deleteFile() {}
void* dNandThread_c::run() { return 0; }
dNandThread_c* dNandThread_c::create(EGG::Heap* heap) { return 0; }
void dNandThread_c::setNandError(s32 error) {}
void* dNandThread_c::getSaveData() { return 0; }

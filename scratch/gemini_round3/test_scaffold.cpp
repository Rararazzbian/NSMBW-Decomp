#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>

// Forward declaration of EGG classes
namespace EGG {
    class Heap;

    class Mutex {
    public:
        virtual ~Mutex();
    };

    class Thread {
    public:
        Thread(u32 stackSize, int msgCount, int priority, Heap* heap);
        virtual ~Thread();
        virtual void* run();
        virtual void onEnter();
        virtual void onExit();

        u8 mPad[0x4c]; // 0x50 total size including vtable
    };
}

typedef struct OSCond {
    OSThreadQueue queue; // 0x08
} OSCond;

extern "C" {
void OSInitCond(OSCond* cond);
void OSWaitCond(OSCond* cond, OSMutex* mutex);
void OSSignalCond(OSCond* cond);

s32 NANDSimpleSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* stageBuf, u32 stageBufSize);
s32 NANDSimpleSafeClose(NANDFileInfo* info);
s32 NANDSimpleSafeCancel(NANDFileInfo* info);
}

class mMutex : public EGG::Mutex {
public:
    virtual ~mMutex();

    OSMutex mOSMutex; // 0x04, size 0x18
    OSCond mOSCond;   // 0x1C, size 0x08
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

    mMutex mMutex;       // 0x50 (size 0x24)
    u32 mCommand;        // 0x74 (size 0x04)
    u32 mStatus;         // 0x78 (size 0x04)
    u8 mFileExists;      // 0x7C (size 0x01)
    u8 mPad[3];          // 0x7D (size 0x03)
};

STATIC_ASSERT(sizeof(EGG::Mutex) == 0x4);
STATIC_ASSERT(sizeof(mMutex) == 0x24);
STATIC_ASSERT(sizeof(EGG::Thread) == 0x50);
STATIC_ASSERT(sizeof(dNandThread_c) == 0x80);
STATIC_ASSERT(offsetof(dNandThread_c, mMutex) == 0x50);
STATIC_ASSERT(offsetof(dNandThread_c, mCommand) == 0x74);
STATIC_ASSERT(offsetof(dNandThread_c, mStatus) == 0x78);
STATIC_ASSERT(offsetof(dNandThread_c, mFileExists) == 0x7C);


#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>
#include <string.h>

// Forward declarations
namespace EGG {
class Heap;
class Mutex {
public:
    virtual ~Mutex() {}
};

class Thread {
public:
    Thread(u32 stackSize, int msgCount, int priority, Heap* heap);
    Thread(OSThread*, int);
    virtual ~Thread();
    virtual void* run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}

    static void initialize();

    u8 mPad_04[0x04];
    OSThread* mOSThread; // 0x08
    u8 mPad_0C[0x44];   // total size = 0x50
};
}

typedef struct OSCond {
    OSThreadQueue queue;
} OSCond;

extern "C" {
void OSInitCond(OSCond* cond);
void OSWaitCond(OSCond* cond, OSMutex* mutex);
void OSSignalCond(OSCond* cond);

s32 NANDSimpleSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* stageBuf, u32 stageBufSize);
s32 NANDSimpleSafeClose(NANDFileInfo* info);
s32 NANDSimpleSafeCancel(NANDFileInfo* info);

void* setCurrentHeap__5mHeapFPQ23EGG4Heap(void* heap);
const void* getMsg__10dMessage_cFUlUl(u32, u32);
void* getRes__6dRes_cCFPCcPCc(void*, const char*, const char*);
void* getSaveGame__10dSaveMng_cFSc(void*, s8);
void* getTempGame__10dSaveMng_cFSc(void*, s8);
u32 calcCRC32__4sCrcFPCvUl(const void*, u32);
}

class mMutex : public EGG::Mutex {
public:
    virtual ~mMutex() {}

    OSMutex mOSMutex; // 0x04
    OSCond  mOSCond;  // 0x1C
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

    mMutex mMutex;       // 0x50
    u32 mCommand;        // 0x74
    u32 mStatus;         // 0x78
    u8 mFileExists;      // 0x7C
    u8 mPad[3];          // 0x7D
};

STATIC_ASSERT(sizeof(EGG::Mutex) == 0x4);
STATIC_ASSERT(sizeof(mMutex) == 0x24);
STATIC_ASSERT(sizeof(EGG::Thread) == 0x50);
STATIC_ASSERT(sizeof(dNandThread_c) == 0x80);
STATIC_ASSERT(offsetof(dNandThread_c, mMutex) == 0x50);
STATIC_ASSERT(offsetof(dNandThread_c, mCommand) == 0x74);
STATIC_ASSERT(offsetof(dNandThread_c, mStatus) == 0x78);
STATIC_ASSERT(offsetof(dNandThread_c, mFileExists) == 0x7C);

// Anonymous namespace data
namespace {
const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
u8 l_tmpSave[0x3FA0];
}

dNandThread_c* dNandThread_c::m_instance;

// All 24 functions with stubs referencing required objects

dNandThread_c::dNandThread_c(int priority, EGG::Heap* heap)
    : EGG::Thread(0x4000, 0, priority, heap)
{
    OSInitMutex(&mMutex.mOSMutex);
    OSInitCond(&mMutex.mOSCond);
    mCommand = 0;
    m_instance = this;
}

dNandThread_c::~dNandThread_c() {
    m_instance = 0;
}

bool dNandThread_c::cmdExistCheck() {
    OSTryLockMutex(&mMutex.mOSMutex);
    mStatus = 0;
    mFileExists = 0;
    mCommand = 1;
    OSSignalCond(&mMutex.mOSCond);
    OSUnlockMutex(&mMutex.mOSMutex);
    return true;
}

void dNandThread_c::existCheck() {
    u8 type;
    NANDGetType(sc_GAME_FILE, &type);
    NANDGetType(sc_BANNER_FILE, &type);
    mFileExists = 1;
}

bool dNandThread_c::cmdSpaceCheck() {
    OSLockMutex(&mMutex.mOSMutex);
    mCommand = 2;
    OSSignalCond(&mMutex.mOSCond);
    OSUnlockMutex(&mMutex.mOSMutex);
    return true;
}

void dNandThread_c::spaceCheck() {
    u32 answer;
    NANDCheck(0x3FA0, 1, &answer);
}

bool dNandThread_c::cmdSave(const void* src) {
    OSLockMutex(&mMutex.mOSMutex);
    mCommand = 4;
    memcpy(l_tmpSave, src, 0x3FA0);
    OSSignalCond(&mMutex.mOSCond);
    OSUnlockMutex(&mMutex.mOSMutex);
    return true;
}

u32 dNandThread_c::save() {
    NANDFileInfo info;
    NANDSimpleSafeOpen(sc_GAME_FILE, &info, 2, l_safeCopyBuf, sizeof(l_safeCopyBuf));
    NANDWrite(&info, l_tmpSave, sizeof(l_tmpSave));
    NANDSimpleSafeClose(&info);
    createBanner();
    return 1;
}

void dNandThread_c::createBanner() {
    NANDFileInfo info;
    NANDCreate(sc_TEMP_BANNER_FILE, 0x3F, 0);
    NANDOpen(sc_TEMP_BANNER_FILE, &info, 2);
    writeBanner(&info);
    NANDClose(&info);
    NANDMove(sc_TEMP_BANNER_FILE, sc_BANNER_FILE);
}

void dNandThread_c::writeBanner(NANDFileInfo* fileInfo) {
    static u8 a_banner[0xF0A0];
    static const char* c_icon_res = "save_icon.bti";
    
    NANDInitBanner((NANDBanner*)a_banner, 0, (const wchar_t*)L"SMNP", (const wchar_t*)c_icon_res);
    getRes__6dRes_cCFPCcPCc(0, "save_banner", "save_banner_EU.bti");
    NANDWrite(fileInfo, a_banner, 0x72A0);
}

bool dNandThread_c::cmdLoad() {
    OSLockMutex(&mMutex.mOSMutex);
    mCommand = 5;
    OSSignalCond(&mMutex.mOSCond);
    OSUnlockMutex(&mMutex.mOSMutex);
    return true;
}

u32 dNandThread_c::load() {
    NANDFileInfo info;
    NANDSimpleSafeOpen(sc_GAME_FILE, &info, 1, l_safeCopyBuf, sizeof(l_safeCopyBuf));
    NANDRead(&info, l_tmpSave, sizeof(l_tmpSave));
    NANDSimpleSafeClose(&info);
    checkCRC();
    return 1;
}

void dNandThread_c::checkCRC() {
    calcCRC32__4sCrcFPCvUl(l_tmpSave, sizeof(l_tmpSave));
}

bool dNandThread_c::cmdDeleteFile() {
    OSLockMutex(&mMutex.mOSMutex);
    mCommand = 3;
    OSSignalCond(&mMutex.mOSCond);
    OSUnlockMutex(&mMutex.mOSMutex);
    return true;
}

void dNandThread_c::deleteFile() {
    NANDDelete(sc_BANNER_FILE);
    NANDDelete(sc_GAME_FILE);
}

void* dNandThread_c::run() {
    OSLockMutex(&mMutex.mOSMutex);
    while (mCommand != 6) {
        OSWaitCond(&mMutex.mOSCond, &mMutex.mOSMutex);
        switch (mCommand) {
        case 1: existCheck(); break;
        case 2: spaceCheck(); break;
        case 3: deleteFile(); break;
        case 4: save(); break;
        case 5: load(); break;
        }
    }
    OSUnlockMutex(&mMutex.mOSMutex);
    return 0;
}

dNandThread_c* dNandThread_c::create(EGG::Heap* heap) {
    void* oldHeap = setCurrentHeap__5mHeapFPQ23EGG4Heap(heap);
    dNandThread_c* instance = new dNandThread_c(OSGetThreadPriority() - 1, 0);
    setCurrentHeap__5mHeapFPQ23EGG4Heap(oldHeap);
    OSResumeThread(instance->mOSThread);
    return instance;
}

void dNandThread_c::setNandError(s32 error) {
    switch (error) {
    case 0:   mStatus = 0; break;
    case -1:  mStatus = 1; break;
    case -2:  mStatus = 2; break;
    case -3:  mStatus = 3; break;
    case -4:  mStatus = 4; break;
    case -5:  mStatus = 5; break;
    case -6:  mStatus = 6; break;
    case -7:  mStatus = 7; break;
    case -8:  mStatus = 8; break;
    case -9:  mStatus = 9; break;
    case -10: mStatus = 10; break;
    case -11: mStatus = 11; break;
    case -12: mStatus = 12; break;
    case -13: mStatus = 13; break;
    case -14: mStatus = 14; break;
    default:  mStatus = 15; break;
    }
}

void* dNandThread_c::getSaveData() {
    return l_tmpSave;
}

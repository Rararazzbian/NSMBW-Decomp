import os
import sys

sys.path.append(os.path.abspath('.'))
from tools.auto_decomp.harness import compile_draft, disasm, diff_fn

draft_src = """#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>

extern "C" void* memcpy(void*, const void*, size_t);

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
        virtual void* run() { return 0; }
        virtual void onEnter() {}
        virtual void onExit() {}

        u8 mPad[0x4c];
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
}

class mMutex : public EGG::Mutex {
public:
    virtual ~mMutex() {}

    OSMutex mOSMutex; // 0x04
    OSCond mOSCond;   // 0x1C
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
    u8 mPad[3];
};

static u8 l_tmpSave[0x3FA0];

void* dNandThread_c::getSaveData() {
    return l_tmpSave;
}

bool dNandThread_c::cmdExistCheck() {
    if (OSTryLockMutex(&mMutex.mOSMutex)) {
        mStatus = 0;
        mFileExists = 0;
        mCommand = 1;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

bool dNandThread_c::cmdSpaceCheck() {
    if (OSTryLockMutex(&mMutex.mOSMutex)) {
        mStatus = 0;
        mCommand = 2;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

bool dNandThread_c::cmdDeleteFile() {
    if (OSTryLockMutex(&mMutex.mOSMutex)) {
        mStatus = 0;
        mCommand = 3;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

bool dNandThread_c::cmdLoad() {
    if (OSTryLockMutex(&mMutex.mOSMutex)) {
        mStatus = 0;
        mCommand = 5;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

bool dNandThread_c::cmdSave(const void* src) {
    if (OSTryLockMutex(&mMutex.mOSMutex)) {
        mStatus = 0;
        mCommand = 4;
        memcpy(l_tmpSave, src, 0x3FA0);
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}
"""

with open('scratch/gemini_round3/test_fn_match.cpp', 'w') as f:
    f.write(draft_src)

src_path = os.path.abspath('scratch/gemini_round3/test_fn_match.cpp')
obj_path = os.path.abspath('scratch/gemini_round3/test_fn_match.o')
txt_path = os.path.abspath('scratch/gemini_round3/test_fn_match.txt')
target_txt = os.path.abspath('scratch/gemini_round3/d_nand_thread_disasm.txt')

ok, log = compile_draft(src_path, obj_path)
print("Compile:", ok)
if not ok:
    print(log)
else:
    dok, dlog = disasm(obj_path, txt_path)
    print("Disasm:", dok)
    
    test_fns = [
        'getSaveData__13dNandThread_cFv',
        'cmdExistCheck__13dNandThread_cFv',
        'cmdSpaceCheck__13dNandThread_cFv',
        'cmdDeleteFile__13dNandThread_cFv',
        'cmdLoad__13dNandThread_cFv',
        'onExit__Q23EGG6ThreadFv',
        'onEnter__Q23EGG6ThreadFv',
        'run__Q23EGG6ThreadFv',
    ]
    for fn in test_fns:
        m, rep = diff_fn(target_txt, txt_path, fn)
        print(f"Diff {fn}: {'MATCH' if m else 'DIFF'}")
        if not m:
            print("  ", rep.splitlines()[:4])

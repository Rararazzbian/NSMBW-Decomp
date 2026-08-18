import os, subprocess

src = '''#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggThread.h>
#include <string.h>

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
    OSCond  mOSCond;
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

    u8 mPad4C[4];
    mMutex mMutex;       // 0x50
    u32 mCommand;        // 0x74
    u32 mStatus;         // 0x78
    u8 mFileExists;      // 0x7C
    u8 mPad[3];          // 0x7D
};

dNandThread_c* dNandThread_c::m_instance;

dNandThread_c::dNandThread_c(int priority, EGG::Heap* heap)
    : EGG::Thread(0x4000, 0, priority, heap)
{
    OSInitMutex(&mMutex.mOSMutex);
    OSInitCond(&mMutex.mOSCond);
    mCommand = 0;
    m_instance = this;
}
'''

with open('scratch/gemini_round6/test_ctor.cpp', 'w') as f:
    f.write(src)

MWCC = 'compilers/Wii/1.1/mwcceppc.exe'
CFLAGS = ['-c', '-proc', 'gekko', '-fp', 'hard', '-O4', '-inline', 'noauto',
          '-Cpp_exceptions', 'off', '-enum', 'int', '-RTTI', 'off', '-ipa', 'file',
          '-enc', 'SJIS', '-DREVOLUTION', '-I-']
INCLUDES = ['-Iinclude', '-Iinclude/lib', '-Iinclude/lib/MSL', '-Iinclude/lib/MSL/internal',
            '-Iinclude/lib/revolution/BTE/include', '-Iinclude/lib/revolution/BTE/stack/include',
            '-Iinclude/lib/revolution/BTE/stack/btm', '-Iinclude/lib/revolution/BTE/bta/include',
            '-Iinclude/lib/revolution/BTE/bta/sys', '-Iinclude/lib/revolution/BTE/gki/common',
            '-Iinclude/lib/revolution/BTE/gki/platform']

res = subprocess.run([MWCC] + CFLAGS + INCLUDES + ['-o', 'scratch/gemini_round6/test_ctor.o', 'scratch/gemini_round6/test_ctor.cpp'], capture_output=True, text=True)
print('compile stdout:', res.stdout)
print('compile stderr:', res.stderr)

DTK = 'bin/dtk-windows-x86_64.exe'
subprocess.run([DTK, 'elf', 'disasm', 'scratch/gemini_round6/test_ctor.o', 'scratch/gemini_round6/test_ctor.txt'])
with open('scratch/gemini_round6/test_ctor.txt') as f:
    print(f.read()[:2000])

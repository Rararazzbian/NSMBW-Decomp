import os
import subprocess

os.makedirs('scratch/gemini_round5/include_full/game/bases', exist_ok=True)
os.makedirs('scratch/gemini_round5/include_full/lib/egg/core', exist_ok=True)
os.makedirs('scratch/gemini_round5/include_full/lib/revolution/NAND', exist_ok=True)
os.makedirs('scratch/gemini_round5/include_full/lib/revolution/OS', exist_ok=True)

# 1. OSCond declarations in OSMutex.h or OSCond.h
with open('scratch/gemini_round5/include_full/lib/revolution/OS/OSMutex.h', 'w') as f:
    f.write("""#ifndef RVL_SDK_OS_MUTEX_H
#define RVL_SDK_OS_MUTEX_H
#include <revolution/OS/OSThread.h>
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSMutex {
    OSThreadQueue queue;  // at 0x0
    OSThread* thread;     // at 0x8
    s32 lock;             // at 0xC
    struct OSMutex* next; // at 0x10
    struct OSMutex* prev; // at 0x14
} OSMutex;

typedef struct OSCond {
    OSThreadQueue queue;  // at 0x0
} OSCond;

void OSInitMutex(OSMutex* mutex);
void OSLockMutex(OSMutex* mutex);
void OSUnlockMutex(OSMutex* mutex);
void __OSUnlockAllMutex(OSThread* thread);
BOOL OSTryLockMutex(OSMutex* mutex);

void OSInitCond(OSCond* cond);
void OSWaitCond(OSCond* cond, OSMutex* mutex);
void OSSignalCond(OSCond* cond);

#ifdef __cplusplus
}
#endif
#endif
""")

# 2. eggThread.h
with open('scratch/gemini_round5/include_full/lib/egg/core/eggThread.h', 'w') as f:
    f.write("""#pragma once

#include <lib/revolution/OS.h>

namespace EGG {

class Heap;

class Thread {
public:
    Thread(u32 stackSize, int msgCount, int priority, Heap* heap);
    Thread(OSThread*, int);
    virtual ~Thread();
    virtual void* run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}

    static void initialize();

    u8 mPad[0x48];
};

} // namespace EGG
""")

# 3. NANDOpenClose.h
with open('scratch/gemini_round5/include_full/lib/revolution/NAND/NANDOpenClose.h', 'w') as f:
    f.write("""#ifndef RVL_SDK_NAND_OPEN_CLOSE_H
#define RVL_SDK_NAND_OPEN_CLOSE_H
#include <revolution/NAND/nand.h>
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

s32 NANDOpen(const char* path, NANDFileInfo* info, u8 mode);
s32 NANDPrivateOpen(const char* path, NANDFileInfo* info, u8 mode);
s32 NANDOpenAsync(const char* path, NANDFileInfo* info, u8 mode,
                  NANDAsyncCallback callback, NANDCommandBlock* block);
s32 NANDPrivateOpenAsync(const char* path, NANDFileInfo* info, u8 mode,
                         NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDClose(NANDFileInfo* info);
s32 NANDCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback,
                   NANDCommandBlock* block);

s32 NANDPrivateSafeOpenAsync(const char* path, NANDFileInfo* info, u8 access,
                             void* buffer, u32 bufferSize,
                             NANDAsyncCallback callback,
                             NANDCommandBlock* block);
s32 NANDSafeCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback,
                       NANDCommandBlock* block);

s32 NANDSimpleSafeOpen(const char* path, NANDFileInfo* info, u8 mode, void* buffer, u32 bufferSize);
s32 NANDSimpleSafeClose(NANDFileInfo* info);
s32 NANDSimpleSafeCancel(NANDFileInfo* info);

#ifdef __cplusplus
}
#endif
#endif
""")

# 4. d_nand_thread.hpp
hpp_content = """#pragma once

#include <types.h>
#include <revolution/OS.h>
#include <revolution/NAND.h>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggThread.h>

// Forward declarations & primitives

namespace EGG {

/**
 * @brief Thread mutex synchronization primitive.
 * @unofficial
 */
class Mutex {
public:
    Mutex() {}
    virtual ~Mutex() {}
};

} // namespace EGG

/**
 * @brief Game-level OS mutex wrapper.
 * @unofficial
 */
class mMutex : public EGG::Mutex {
public:
    mMutex() {}
    virtual ~mMutex() {}

    OSMutex mOSMutex;       ///< 0x04..0x1B: Embedded OS mutex (size 0x18)
    OSCond mOSCond;         ///< 0x1C..0x23: Condition variable (size 0x08)
};

/**
 * @brief Dedicated background thread for asynchronous NAND flash filesystem operations.
 */
class dNandThread_c : public EGG::Thread {
public:
    enum Status_e {
        STATUS_IDLE = 0,
        STATUS_BUSY = 1,
        STATUS_ERROR = 2
    };

    enum Command_e {
        CMD_NONE = 0,
        CMD_EXIST_CHECK = 1,
        CMD_SPACE_CHECK = 2,
        CMD_LOAD = 3,
        CMD_SAVE = 4,
        CMD_DELETE_FILE = 5
    };

    dNandThread_c(int msgCount, EGG::Heap *heap);
    virtual ~dNandThread_c();

    virtual void *run();

    void cmdExistCheck();
    bool existCheck();

    void cmdSpaceCheck();
    bool spaceCheck();

    bool cmdSave(const void *saveData);
    bool save();

    bool createBanner();
    bool writeBanner(NANDFileInfo *fileInfo);

    void cmdLoad();
    bool load();

    bool checkCRC();

    void cmdDeleteFile();
    bool deleteFile();

    void setNandError(s32 err);
    void *getSaveData();

    static void create(EGG::Heap *heap);

    // Layout
    u8 mPad_4c[0x4];        ///< 0x4C: Padding / unmeasured region
    mMutex mMutex;          ///< 0x50: Synchronization mutex & condition variable (size 0x24)
    int mCommand;           ///< 0x74: Active NAND command ID
    int mStatus;            ///< 0x78: Execution status / error code
    bool mFileExists;       ///< 0x7C: Flag indicating file presence
    u8 mPad_7d[0x3];        ///< 0x7D: 4-byte struct alignment padding

    static dNandThread_c *m_instance;
};

STATIC_ASSERT(sizeof(EGG::Mutex) == 0x4);
STATIC_ASSERT(sizeof(mMutex) == 0x24);
STATIC_ASSERT(sizeof(EGG::Thread) == 0x4C);
STATIC_ASSERT(sizeof(dNandThread_c) == 0x80);
STATIC_ASSERT(offsetof(dNandThread_c, mMutex) == 0x50);
STATIC_ASSERT(offsetof(dNandThread_c, mCommand) == 0x74);
STATIC_ASSERT(offsetof(dNandThread_c, mStatus) == 0x78);
STATIC_ASSERT(offsetof(dNandThread_c, mFileExists) == 0x7C);
"""

with open('scratch/gemini_round5/d_nand_thread.hpp', 'w') as f:
    f.write(hpp_content)

with open('scratch/gemini_round5/include_full/game/bases/d_nand_thread.hpp', 'w') as f:
    f.write(hpp_content)

# 5. d_nand_thread_test.cpp
scaffold_cpp = """#include <game/bases/d_nand_thread.hpp>
#include <cstddef>
#include <cstring>

namespace {
const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
u8 l_tmpSave[0x3FA0];
}

dNandThread_c *dNandThread_c::m_instance = nullptr;

dNandThread_c::dNandThread_c(int msgCount, EGG::Heap *heap)
    : EGG::Thread(0x4000, 0, msgCount, heap) {
}

dNandThread_c::~dNandThread_c() {}

void *dNandThread_c::run() { return nullptr; }

void dNandThread_c::cmdExistCheck() {}
bool dNandThread_c::existCheck() { return false; }

void dNandThread_c::cmdSpaceCheck() {}
bool dNandThread_c::spaceCheck() { return false; }

bool dNandThread_c::cmdSave(const void *saveData) { return false; }
bool dNandThread_c::save() { return false; }

bool dNandThread_c::createBanner() { return false; }
bool dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static u8 a_banner[0xF0A0];
    static const char *c_icon_res = "save_icon.bti";
    const char *smnp = "SMNP";
    return false;
}

void dNandThread_c::cmdLoad() {}
bool dNandThread_c::load() { return false; }

bool dNandThread_c::checkCRC() { return false; }

void dNandThread_c::cmdDeleteFile() {}
bool dNandThread_c::deleteFile() { return false; }

void dNandThread_c::setNandError(s32 err) {}
void *dNandThread_c::getSaveData() { return nullptr; }

void dNandThread_c::create(EGG::Heap *heap) {}
"""

with open('scratch/gemini_round5/d_nand_thread_test.cpp', 'w') as f:
    f.write(scaffold_cpp)

mwcc = 'compilers/Wii/1.1/mwcceppc.exe'
dtk = os.path.abspath('bin/dtk-windows-x86_64.exe')
includes = [
    '-Iscratch/gemini_round5/include_full',
    '-Iinclude', '-Iinclude/lib', '-Iinclude/lib/MSL', '-Iinclude/lib/MSL/internal',
    '-Iinclude/lib/revolution/BTE/include', '-Iinclude/lib/revolution/BTE/stack/include',
    '-Iinclude/lib/revolution/BTE/stack/btm', '-Iinclude/lib/revolution/BTE/bta/include',
    '-Iinclude/lib/revolution/BTE/bta/sys', '-Iinclude/lib/revolution/BTE/gki/common',
    '-Iinclude/lib/revolution/BTE/gki/platform'
]
cflags = ['-c', '-proc', 'gekko', '-fp', 'hard', '-O4', '-inline', 'noauto',
          '-Cpp_exceptions', 'off', '-enum', 'int', '-RTTI', 'off', '-ipa', 'file',
          '-enc', 'SJIS', '-DREVOLUTION', '-I-']

cmd = [mwcc] + cflags + includes + ['-o', 'scratch/gemini_round5/d_nand_thread_test.o', 'scratch/gemini_round5/d_nand_thread_test.cpp']
p = subprocess.run(cmd, capture_output=True)
print('STDOUT:', p.stdout.decode('cp1252', errors='ignore'))
print('STDERR:', p.stderr.decode('cp1252', errors='ignore'))
print('Scaffold compile return code:', p.returncode)

# Disassemble scaffold
if p.returncode == 0:
    subprocess.run([dtk, 'elf', 'disasm', os.path.abspath('scratch/gemini_round5/d_nand_thread_test.o'), os.path.abspath('scratch/gemini_round5/d_nand_thread_test_disasm.txt')], check=True)
    print('Disassembly generated successfully!')

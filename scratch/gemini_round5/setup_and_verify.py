import os
import subprocess
import shutil

shutil.rmtree('scratch/gemini_round5/include_full', ignore_errors=True)
os.makedirs('scratch/gemini_round5/include_full/game/bases', exist_ok=True)
os.makedirs('scratch/gemini_round5/include_full/lib/egg/core', exist_ok=True)
os.makedirs('scratch/gemini_round5/include_full/egg/core', exist_ok=True)
os.makedirs('scratch/gemini_round5/include_full/revolution/OS', exist_ok=True)
os.makedirs('scratch/gemini_round5/include_full/revolution/NAND', exist_ok=True)

# 1. OSMutex.h
with open('scratch/gemini_round5/include_full/revolution/OS/OSMutex.h', 'w') as f:
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
egg_thread_content = """#pragma once

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
"""
with open('scratch/gemini_round5/include_full/lib/egg/core/eggThread.h', 'w') as f:
    f.write(egg_thread_content)
with open('scratch/gemini_round5/include_full/egg/core/eggThread.h', 'w') as f:
    f.write(egg_thread_content)

# 3. NANDOpenClose.h
nand_open_close_content = """#ifndef RVL_SDK_NAND_OPEN_CLOSE_H
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
"""
with open('scratch/gemini_round5/include_full/revolution/NAND/NANDOpenClose.h', 'w') as f:
    f.write(nand_open_close_content)

# 4. d_nand_thread.hpp
with open('scratch/gemini_round5/d_nand_thread.hpp') as f1, open('scratch/gemini_round5/include_full/game/bases/d_nand_thread.hpp', 'w') as f2:
    f2.write(f1.read())

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

if p.returncode == 0:
    subprocess.run([dtk, 'elf', 'disasm', os.path.abspath('scratch/gemini_round5/d_nand_thread_test.o'), os.path.abspath('scratch/gemini_round5/d_nand_thread_test_disasm.txt')], check=True)
    print('Disassembly generated successfully!')

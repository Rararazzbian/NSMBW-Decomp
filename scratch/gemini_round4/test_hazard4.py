import os
import subprocess

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"
MWCC = os.path.join(ROOT, "compilers", "Wii", "1.1", "mwcceppc.exe")
DTK = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")
CFLAGS = ['-c', '-proc', 'gekko', '-fp', 'hard', '-O4', '-inline', 'noauto', '-Cpp_exceptions', 'off', '-enum', 'int', '-RTTI', 'off', '-ipa', 'file', '-enc', 'SJIS', '-DREVOLUTION', '-I-']
INCLUDES = ['include', 'include/lib', 'include/lib/MSL', 'include/lib/MSL/internal',
            'include/lib/revolution/BTE/include', 'include/lib/revolution/BTE/stack/include',
            'include/lib/revolution/BTE/stack/btm', 'include/lib/revolution/BTE/bta/include',
            'include/lib/revolution/BTE/bta/sys', 'include/lib/revolution/BTE/gki/common',
            'include/lib/revolution/BTE/gki/platform']

# Test A: Using the repo's UNMODIFIED include/lib/egg/core/eggThread.h
code_unmodified = r"""
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
"""

# Test B: Using the PROPOSED include/lib/egg/core/eggThread.h
code_proposed = r"""
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
    Thread(OSThread*, int);
    virtual ~Thread();
    virtual void* run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}

    static void initialize();

    u8 mPad[0x4c]; // 0x4c bytes after vtable ptr
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

    mMutex mMutex;
    u32 mCommand;
    u32 mStatus;
    u8 mFileExists;
    u8 mPad[3];
};

STATIC_ASSERT(sizeof(EGG::Thread) == 0x50); // 0x04 vptr + 0x4C data = 0x50
STATIC_ASSERT(sizeof(mMutex) == 0x24);
STATIC_ASSERT(sizeof(dNandThread_c) == 0x80);
STATIC_ASSERT(offsetof(dNandThread_c, mMutex) == 0x50);
STATIC_ASSERT(offsetof(dNandThread_c, mCommand) == 0x74);
STATIC_ASSERT(offsetof(dNandThread_c, mStatus) == 0x78);
STATIC_ASSERT(offsetof(dNandThread_c, mFileExists) == 0x7C);
"""

def test_compile(code_str, filename):
    src_path = os.path.join(ROOT, "scratch", "gemini_round4", filename + ".cpp")
    obj_path = os.path.join(ROOT, "scratch", "gemini_round4", filename + ".o")
    with open(src_path, "w", encoding="ascii") as f:
        f.write(code_str)
    args = [MWCC] + CFLAGS + [src_path, "-o", obj_path]
    for inc in INCLUDES: args += ["-i", inc]
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or "") + (p.stderr or "")

ok_unmod, err_unmod = test_compile(code_unmodified, "test_hazard4_unmodified")
print(f"Test with current repo eggThread.h: {'PASS' if ok_unmod else 'FAIL'}")
if not ok_unmod: print(err_unmod)

ok_prop, err_prop = test_compile(code_proposed, "test_hazard4_proposed")
print(f"Test with proposed eggThread.h: {'PASS' if ok_prop else 'FAIL'}")
if not ok_prop: print(err_prop)

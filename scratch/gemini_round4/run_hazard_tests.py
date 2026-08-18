import os
import subprocess
import re

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"
MWCC = os.path.join(ROOT, "compilers", "Wii", "1.1", "mwcceppc.exe")
DTK = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")

CFLAGS = [
    "-c", "-proc", "gekko", "-fp", "hard", "-O4", "-inline", "noauto",
    "-Cpp_exceptions", "off", "-enum", "int", "-RTTI", "off", "-ipa", "file",
    "-enc", "SJIS", "-DREVOLUTION", "-I-"
]

INCLUDES = [
    os.path.join(ROOT, "include"),
    os.path.join(ROOT, "include", "lib"),
    os.path.join(ROOT, "include", "lib", "MSL"),
    os.path.join(ROOT, "include", "lib", "MSL", "internal"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "include"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "stack", "include"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "stack", "btm"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "bta", "include"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "bta", "sys"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "gki", "common"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "gki", "platform")
]

def compile_code(cpp_code, obj_path, extra_incs=()):
    src_path = obj_path.replace(".o", ".cpp")
    with open(src_path, "w", encoding="ascii") as f:
        f.write(cpp_code)
    
    args = [MWCC] + CFLAGS + [src_path, "-o", obj_path]
    for inc in list(extra_incs) + INCLUDES:
        args += ["-i", inc]
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or "") + (p.stderr or "")

def disasm(obj_path, txt_path):
    p = subprocess.run([DTK, "elf", "disasm", obj_path, txt_path], cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or "") + (p.stderr or "")

def get_symbols(txt_path):
    fn_list = []
    obj_list = []
    with open(txt_path, "r", encoding="ascii", errors="replace") as f:
        for line in f:
            line = line.strip()
            if line.startswith(".fn "):
                parts = line.split()
                fn_list.append(parts[1].rstrip(","))
            elif line.startswith(".obj "):
                parts = line.split()
                obj_list.append(parts[1].rstrip(","))
    return fn_list, obj_list

def run_tests():
    out_dir = os.path.join(ROOT, "scratch", "gemini_round4")
    os.makedirs(out_dir, exist_ok=True)

    # -------------------------------------------------------------
    # HAZARD 1: EGG::Thread weak virtuals (with inline vs without inline)
    # -------------------------------------------------------------
    print("=== TESTING HAZARD 1: EGG::Thread weak virtuals ===")

    # Test 1A: With inline bodies in EGG::Thread
    code_1a = """
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
    virtual void* run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}
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
"""
    obj_1a = os.path.join(out_dir, "hazard1_inline.o")
    txt_1a = os.path.join(out_dir, "hazard1_inline.txt")
    ok, err = compile_code(code_1a, obj_1a)
    print(f"1A (with inline bodies) Compile: {ok}")
    if not ok: print(err)
    disasm(obj_1a, txt_1a)
    fns_1a, objs_1a = get_symbols(txt_1a)
    print(f"1A Functions emitted ({len(fns_1a)}): {fns_1a}")

    # Test 1B: WITHOUT inline bodies in EGG::Thread
    code_1b = code_1a.replace(
        "virtual void* run() { return 0; }\n    virtual void onEnter() {}\n    virtual void onExit() {}",
        "virtual void* run();\n    virtual void onEnter();\n    virtual void onExit();"
    )
    obj_1b = os.path.join(out_dir, "hazard1_no_inline.o")
    txt_1b = os.path.join(out_dir, "hazard1_no_inline.txt")
    ok, err = compile_code(code_1b, obj_1b)
    print(f"1B (without inline bodies) Compile: {ok}")
    if not ok: print(err)
    disasm(obj_1b, txt_1b)
    fns_1b, objs_1b = get_symbols(txt_1b)
    print(f"1B Functions emitted ({len(fns_1b)}): {fns_1b}")

    print("\n--- HAZARD 1 COMPARISON ---")
    weak_virtuals = ["onExit__Q23EGG6ThreadFv", "onEnter__Q23EGG6ThreadFv", "run__Q23EGG6ThreadFv"]
    print("In 1A (inline bodies):")
    for wv in weak_virtuals:
        present = any(wv in f for f in fns_1a)
        print(f"  {wv}: {'PRESENT' if present else 'ABSENT'}")
    print("In 1B (no inline bodies):")
    for wv in weak_virtuals:
        present = any(wv in f for f in fns_1b)
        print(f"  {wv}: {'PRESENT' if present else 'ABSENT'}")

if __name__ == "__main__":
    run_tests()

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

code = r"""
#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>

extern void use(const void*);

namespace {
const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
u8 l_tmpSave[0x3FA0];
}

class dNandThread_c {
public:
    void writeBanner(NANDFileInfo* fileInfo);
};

void dNandThread_c::writeBanner(NANDFileInfo* fileInfo) {
    static u8 a_banner[0xF0A0];
    static const char* c_icon_res = "save_icon.bti";
    
    use(sc_TEMP_BANNER_FILE);
    use(sc_BANNER_FILE);
    use(sc_GAME_FILE);
    use(l_safeCopyBuf);
    use(l_tmpSave);
    use(a_banner);
    use(c_icon_res);
    use("SMNP");
    use("save_banner_EU.bti");
    use("save_banner");
}
"""

src_path = os.path.join(ROOT, "scratch", "gemini_round4", "d_nand_thread_scope.cpp")
obj_path = os.path.join(ROOT, "scratch", "gemini_round4", "d_nand_thread_scope.o")
txt_path = os.path.join(ROOT, "scratch", "gemini_round4", "d_nand_thread_scope.txt")

with open(src_path, "w", encoding="ascii") as f:
    f.write(code)

args = [MWCC] + CFLAGS + [src_path, "-o", obj_path]
for inc in INCLUDES: args += ["-i", inc]
p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
print("Compile output:", p.stdout, p.stderr)
subprocess.run([DTK, "elf", "disasm", obj_path, txt_path], cwd=ROOT)

with open(txt_path, "r", encoding="ascii", errors="replace") as f:
    print(f.read())

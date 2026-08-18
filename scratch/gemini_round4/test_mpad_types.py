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

test_mpad_code = r"""
#include <types.h>
#include <lib/revolution/WPAD.h>
#include <lib/revolution/PAD.h>
#include <lib/revolution/MEM.h>
#include <lib/revolution/GX.h>
#include <lib/revolution/MTX.h>
#include <lib/nw4r/ut/ut_TextWriterBase.h>
#include <lib/nw4r/ut/ut_List.h>
#include <lib/egg/core/eggController.h>

namespace mPad {
    enum CH_e {
        CH_0,
        CH_1,
        CH_2,
        CH_3
    };

    struct PadAdditionalData_t {
        PadAdditionalData_t();
        ~PadAdditionalData_t();
        u8 mData[0x18];
    };

    void create();
    void beginPad();
    void endPad();
    void setCurrentChannel(CH_e ch);
    u32 getBatteryLevel_ch(CH_e ch);
    void setWPADInfo(CH_e ch, const WPADInfo& info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    void getWPADInfoCb(s32 chan, s32 result);
    bool getWPADInfoAsync(CH_e ch);
    void setGetWPADInfoInterval(u32 interval);
    u32 getGetWPADInfoInterval();

    extern EGG::CoreController* g_core[4];
    extern PadAdditionalData_t g_PadAdditionalData[4];
    extern WPADInfo s_WPADInfo[4];
    extern WPADInfo s_WPADInfoTmp[4];

    extern void* g_padMg;
    extern u32 g_currentCoreID;
    extern EGG::CoreController* g_currentCore;
    extern u8 g_IsConnected;
    extern u32 g_PadFrame;
    extern u32 s_WPADInfoAvailable;
    extern u32 s_GetWPADInfoInterval;
    extern u32 s_GetWPADInfoCount;
}

namespace mTex {
    class base_c {
    public:
        void init(int, int, int, int);
        int getTileNo(int, int) const;
        int getIdInTile(int, int) const;
        int xyToDotId(int, int) const;

        int mWidth;
        int mHeight;
        int mFormat;
        int mTileSize;
    };

    class edit4b_c : public base_c {
    public:
        void init(int width, int height, u8* buffer);
        virtual void set(int x, int y, u8 color, bool flag);
        void endEdit();
        virtual ~edit4b_c();

        u8* mBuffer;
    };
}

STATIC_ASSERT(sizeof(mPad::PadAdditionalData_t) == 0x18);
STATIC_ASSERT(sizeof(mTex::base_c) == 0x10);
STATIC_ASSERT(sizeof(mTex::edit4b_c) == 0x18);
"""

src_path = os.path.join(ROOT, "scratch", "gemini_round4", "test_mpad_types.cpp")
obj_path = os.path.join(ROOT, "scratch", "gemini_round4", "test_mpad_types.o")

with open(src_path, "w", encoding="ascii") as f:
    f.write(test_mpad_code)

args = [MWCC] + CFLAGS + [src_path, "-o", obj_path]
for inc in INCLUDES: args += ["-i", inc]
p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
print("mPad types test compile:", "PASS" if p.returncode == 0 else "FAIL")
if p.returncode != 0:
    print(p.stdout, p.stderr)

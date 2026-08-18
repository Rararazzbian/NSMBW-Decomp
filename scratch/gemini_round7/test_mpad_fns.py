import os, sys, subprocess

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.append(os.path.join(ROOT, 'tools', 'auto_decomp'))

import harness

src_code = r'''#include <types.h>
#include <revolution/OS.h>
#include <revolution/WPAD.h>
#include <revolution/GX.h>
#include <revolution/MEM.h>
#include <revolution/MTX.h>

namespace EGG {
    class CoreController {
    public:
        void sceneReset();
    };
    class CoreControllerMgr {
    public:
        static CoreControllerMgr *sInstance;
        CoreController *getNthController(int);
    };
    class CoreStatus {
    public:
        void init();
    };
}

namespace mPad {
    enum CH_e {
        MPAD_CH_0 = 0,
        MPAD_CH_1 = 1,
        MPAD_CH_2 = 2,
        MPAD_CH_3 = 3
    };

    struct PadAdditionalData_t {
        float mData[6];

        PadAdditionalData_t() {}
        ~PadAdditionalData_t() {}
    };

    void create();
    void beginPad();
    void endPad();
    void setCurrentChannel(CH_e ch);
    s8 getBatteryLevel_ch(CH_e ch);
    void setWPADInfo(CH_e ch, const WPADInfo &info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    void getWPADInfoAsync(CH_e ch);
    void setGetWPADInfoInterval(u32 interval);
    u32 getGetWPADInfoInterval();

    extern EGG::CoreControllerMgr *g_padMg;
    extern u32 g_currentCoreID;
    extern EGG::CoreController *g_currentCore;
    extern u8 g_IsConnected[4];
    extern u32 g_PadFrame;
    extern u8 s_WPADInfoAvailable[4];
    extern u32 s_GetWPADInfoInterval;
    extern u32 s_GetWPADInfoCount;

    extern EGG::CoreController *g_core[4];
    extern PadAdditionalData_t g_PadAdditionalData[4];
    extern WPADInfo s_WPADInfo[4];
    extern WPADInfo s_WPADInfoTmp[4];
}

extern "C" void getWPADInfoCb(s32 chan, s32 result);

void mPad::create() {
    g_padMg = EGG::CoreControllerMgr::sInstance;
    initWPADInfo();
    beginPad();
    endPad();
}

void mPad::setCurrentChannel(CH_e ch) {
    g_currentCoreID = ch;
    g_currentCore = g_core[ch];
}

s8 mPad::getBatteryLevel_ch(CH_e ch) {
    if (s_WPADInfoAvailable[ch] == 0) {
        return -1;
    }
    return s_WPADInfo[ch].battery;
}

void mPad::setWPADInfo(CH_e ch, const WPADInfo &info) {
    s_WPADInfo[ch] = info;
    s_WPADInfoAvailable[ch] = 1;
}

void mPad::clearWPADInfo(CH_e ch) {
    WPADInfo *info = &s_WPADInfo[ch];
    info->dpd = 0;
    info->attach = 0;
    info->lowBat = 0;
    info->nearempty = 0;
    info->battery = 0;
    info->led = 0;
    info->protocol = 0;
    info->firmware = 0;
    s_WPADInfoAvailable[ch] = 0;
}

void mPad::initWPADInfo() {
    for (int i = 0; i < 4; i++) {
        clearWPADInfo((CH_e)i);
    }
}

extern "C" void getWPADInfoCb(s32 chan, s32 result) {
    if (mPad::s_GetWPADInfoInterval == 0) {
        return;
    }
    if (result == 0) {
        mPad::setWPADInfo((mPad::CH_e)chan, mPad::s_WPADInfoTmp[chan]);
    } else if (result == -1) {
        mPad::clearWPADInfo((mPad::CH_e)chan);
    }
}

void mPad::setGetWPADInfoInterval(u32 interval) {
    s_GetWPADInfoInterval = interval;
    if (interval == 0) {
        initWPADInfo();
    }
}

u32 mPad::getGetWPADInfoInterval() {
    return s_GetWPADInfoInterval;
}
'''

src_path = os.path.join(ROOT, 'scratch', 'gemini_round7', 'test_mpad_match.cpp')
obj_path = os.path.join(ROOT, 'scratch', 'gemini_round7', 'test_mpad_match.o')
txt_path = os.path.join(ROOT, 'scratch', 'gemini_round7', 'test_mpad_match.txt')

with open(src_path, 'w', encoding='utf-8') as f:
    f.write(src_code)

res = harness.compile_draft(src_path, obj_path)
print("Compile:", res)

DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
subprocess.run([DTK, 'elf', 'disasm', obj_path, txt_path], capture_output=True, text=True)

with open(txt_path) as f:
    print(f.read())

import os, sys, subprocess

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.append(os.path.join(ROOT, 'tools', 'auto_decomp'))

import harness

scaffold_cpp = r'''#include <types.h>
#include <revolution/OS.h>
#include <revolution/WPAD.h>
#include <revolution/GX.h>
#include <revolution/MEM.h>
#include <revolution/MTX.h>
#include <lib/nw4r/ut/ut_list.h>
#include <lib/nw4r/ut/ut_Font.h>
#include <lib/nw4r/ut/ut_TextWriterBase.h>
#include <lib/nw4r/ut/ut_CharWriter.h>
#include <lib/egg/core/eggController.h>

// Forward declarations
namespace EGG {
    class CoreController;
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

// -------------------------------------------------------------
// mPad namespace
// -------------------------------------------------------------
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

    // Functions
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

    // Globals in .sbss
    EGG::CoreControllerMgr *g_padMg;
    u32 g_currentCoreID;
    EGG::CoreController *g_currentCore;
    u8 g_IsConnected[4];
    u32 g_PadFrame;
    u8 s_WPADInfoAvailable[4];
    u32 s_GetWPADInfoInterval;
    u32 s_GetWPADInfoCount;

    // Globals in .bss
    EGG::CoreController *g_core[4];
    PadAdditionalData_t g_PadAdditionalData[4];
    WPADInfo s_WPADInfo[4];
    WPADInfo s_WPADInfoTmp[4];
}

// File-scope callback
void getWPADInfoCb(s32 chan, s32 result);

// -------------------------------------------------------------
// mPrint namespace & MyPrintBase template
// -------------------------------------------------------------
namespace mPrint {
    template <typename T>
    class MyPrintBase {
    public:
        struct MyText {
            nw4r::ut::Link mLink;
        };

        nw4r::ut::Font *mFont;
        nw4r::ut::List mList;
        u8 mVisible;

        MyPrintBase();
        ~MyPrintBase();

        void Initialize(void *buffer, u32 size, const nw4r::ut::Font &font);
        void SetFont(const nw4r::ut::Font &font);
        const nw4r::ut::Font *GetFont() const;
        void SetVisible(bool visible);
        bool IsVisible() const;
        void VRegisterf(int x, int y, u32 col, u32 bgCol, float scale, bool wrap, const T *fmt, va_list args);
        void Reset();
        void Flush();
        void Flush(int x1, int y1, int x2, int y2);
        void Register(int x, int y, u32 col, u32 bgCol, float scale, bool wrap, const T *str, int len);
        MyText *GetFirstText();
        MyText *GetNextText(MyText *curr);
        void Unregister(MyText *text);
        void SetBuffer(void *buffer, u32 size);
    };
}

// -------------------------------------------------------------
// mTex namespace
// -------------------------------------------------------------
namespace mTex {
    class base_c {
    public:
        int mWidth;
        int mHeight;
        int mTileWidth;
        int mTileHeight;
        int mTileSize;
        int mTileCountX;
        int mTileCountY;
        int mTotalTiles;

        void init(int w, int h, int tw, int th);
        int getTileNo(int x, int y) const;
        int getIdInTile(int x, int y) const;
        int xyToDotId(int x, int y) const;
    };

    class edit4b_c : public base_c {
    public:
        u8 *mpData;

        virtual ~edit4b_c();
        virtual bool set(int x, int y, u8 val, bool sync);

        void init(int w, int h, u8 *data);
        void endEdit();
    };
}

// Explicit instantiations or implementations
'''

with open('scratch/gemini_round7/scaffold_base.hpp', 'w') as f:
    f.write(scaffold_cpp)

print("Wrote scaffold_base.hpp")

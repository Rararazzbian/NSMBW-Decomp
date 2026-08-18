#include <types.h>
#include <revolution/OS.h>
#include <revolution/WPAD.h>
#include <revolution/GX.h>
#include <revolution/MEM.h>
#include <revolution/MTX.h>
#include <lib/nw4r/ut/ut_list.h>
#include <lib/nw4r/ut/ut_Font.h>
#include <lib/nw4r/ut/ut_TextWriterBase.h>
#include <lib/nw4r/ut/ut_CharWriter.h>

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

    // Globals in .sbss (defined here)
    EGG::CoreControllerMgr *g_padMg;
    u32 g_currentCoreID;
    EGG::CoreController *g_currentCore;
    u8 g_IsConnected[4];
    u32 g_PadFrame;
    u8 s_WPADInfoAvailable[4];
    u32 s_GetWPADInfoInterval;
    u32 s_GetWPADInfoCount;

    // Globals in .bss (defined here)
    EGG::CoreController *g_core[4];
    PadAdditionalData_t g_PadAdditionalData[4];
    WPADInfo s_WPADInfo[4];
    WPADInfo s_WPADInfoTmp[4];
}

void getWPADInfoCb(s32 chan, s32 result);

namespace mPrint {
    template <typename T>
    class MyPrintBase {
    public:
        struct MyText {
            nw4r::ut::Link mLink;
        };

        const nw4r::ut::Font *mFont;
        nw4r::ut::List mList;
        u8 mVisible;

        MyPrintBase() {
            mFont = 0;
            mVisible = 0;
            nw4r::ut::List_Init(&mList, 0x14);
        }
        ~MyPrintBase() {}

        void Initialize(void *buffer, u32 size, const nw4r::ut::Font &font) {
            SetBuffer(buffer, size);
            SetFont(font);
            nw4r::ut::List_Init(&mList, 0x14);
        }
        void SetFont(const nw4r::ut::Font &font) { mFont = &font; }
        const nw4r::ut::Font *GetFont() const { return mFont; }
        void SetVisible(bool visible) { mVisible = visible; }
        bool IsVisible() const { return mVisible; }
        void VRegisterf(int x, int y, u32 col, u32 bgCol, float scale, bool wrap, const T *fmt, va_list args);
        void Reset();
        void Flush();
        void Flush(int x1, int y1, int x2, int y2);
        void Register(int x, int y, u32 col, u32 bgCol, float scale, bool wrap, const T *str, int len);
        MyText *GetFirstText() { return (MyText*)nw4r::ut::List_GetFirst(&mList); }
        MyText *GetNextText(MyText *curr) { return (MyText*)nw4r::ut::List_GetNext(&mList, curr); }
        void Unregister(MyText *text) {
            nw4r::ut::List_Remove(&mList, text);
            // MEMFreeToExpHeap...
        }
        void SetBuffer(void *buffer, u32 size) {
            // MEMCreateExpHeapEx...
        }
    };
}

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

// -------------------------------------------------------------
// Function implementations (Scaffold)
// -------------------------------------------------------------

void mPad::create() {
    g_padMg = EGG::CoreControllerMgr::sInstance;
    initWPADInfo();
    beginPad();
    endPad();
}

void mPad::beginPad() {}
void mPad::endPad() {}
void mPad::setCurrentChannel(CH_e ch) {}
s8 mPad::getBatteryLevel_ch(CH_e ch) { return 0; }
void mPad::setWPADInfo(CH_e ch, const WPADInfo &info) {}
void mPad::clearWPADInfo(CH_e ch) {}
void mPad::initWPADInfo() {}
void getWPADInfoCb(s32 chan, s32 result) {}
void mPad::getWPADInfoAsync(CH_e ch) {}
void mPad::setGetWPADInfoInterval(u32 interval) {}
u32 mPad::getGetWPADInfoInterval() { return 0; }

template <typename T>
void mPrint::MyPrintBase<T>::VRegisterf(int x, int y, u32 col, u32 bgCol, float scale, bool wrap, const T *fmt, va_list args) {}

template <typename T>
void mPrint::MyPrintBase<T>::Reset() {}

template <typename T>
void mPrint::MyPrintBase<T>::Flush() {}

template <typename T>
void mPrint::MyPrintBase<T>::Flush(int x1, int y1, int x2, int y2) {}

template <typename T>
void mPrint::MyPrintBase<T>::Register(int x, int y, u32 col, u32 bgCol, float scale, bool wrap, const T *str, int len) {}

// Explicit instantiations of template methods
template class mPrint::MyPrintBase<char>;
template class mPrint::MyPrintBase<wchar_t>;

void mTex::base_c::init(int w, int h, int tw, int th) {}
int mTex::base_c::getTileNo(int x, int y) const { return 0; }
int mTex::base_c::getIdInTile(int x, int y) const { return 0; }
int mTex::base_c::xyToDotId(int x, int y) const { return 0; }

mTex::edit4b_c::~edit4b_c() {}
bool mTex::edit4b_c::set(int x, int y, u8 val, bool sync) { return false; }
void mTex::edit4b_c::init(int w, int h, u8 *data) {}
void mTex::edit4b_c::endEdit() {}

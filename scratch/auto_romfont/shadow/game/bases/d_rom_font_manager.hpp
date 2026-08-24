#pragma once

#include <lib/egg/core/eggHeap.h>
#include <lib/nw4r/ut/ut_RomFont.h>

/// @unofficial The game's own TextWriter (ctor at 0x80106FF0, not yet
/// decompiled); only its sDefaultFont static is needed here.
class dRomFontMgr_c;

class TextWriter {
public:
    static dRomFontMgr_c *sDefaultFont;
};

/// @brief Owns the system RomFont and hands it to TextWriter.
class dRomFontMgr_c {
public:
    dRomFontMgr_c() : mBuffer(NULL), mLoaded(false) {}

    nw4r::ut::RomFont mFont;  ///< 0x00. @unofficial
    void *mBuffer;            ///< 0x1C font buffer from the heap. @unofficial
    bool mLoaded;             ///< 0x20 set once the font is loaded. @unofficial

    static void createInstance(EGG::Heap *heap);
    void load_resource(EGG::Heap *heap);

    static dRomFontMgr_c *m_pInstance;
};

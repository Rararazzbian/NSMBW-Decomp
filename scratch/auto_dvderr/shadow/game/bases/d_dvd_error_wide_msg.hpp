#pragma once
#include <types.h>
#include <game/bases/d_lang_loc_string.hpp>

/// @brief Localization state driving the DVD error screens.
/// @unofficial Declared here because this unit is the first to reference
/// LangLoc's constructor; relocate when its own TU lands. Layout from
/// __ct__7LangLocFv (fields at +4/+8/+C/+10; +0 never written) and from how
/// fn_80107DA0 uses it. sizeof == 0x14 (LangLocString member sits at 0xF4).
class LangLoc {
public:
    LangLoc();

    /* 0x000 */ int m000;
    /* 0x004 */ int m004;
    /* 0x008 */ int m008;
    /* 0x00C */ u8 m00c;
    /* 0x010 */ LangLocString* mp010;
};

/// @brief Localized wide ("insert disc" / "disc could not be read") message
/// table for ::dDvdErr_c's error screen.
/// @unofficial Member names inferred; offsets measured from __ct and
/// initialize. Each mpWide slot holds "<sMsgBase prefix><mpMsgs message>";
/// the [7][6]/[7][2] shapes are what reproduce retail's zeroing loop.
class dDvdErrorWideMsg_c {
public:
    dDvdErrorWideMsg_c();
    void initialize(const char** msgs);

    /// [language][variant] format prefixes shared by all slots.
    static const char* sMsgBase[14][3];

    /* 0x000 */ char* mpWide[7][6];       ///< snprintf'd "<prefix><msg>" slots
    /* 0x0A8 */ const char* mpMsgs[7][2]; ///< current per-language message
    /* 0x0E0 */ LangLoc mLangLoc;
    /* 0x0F4 */ LangLocString mLocStr;
};

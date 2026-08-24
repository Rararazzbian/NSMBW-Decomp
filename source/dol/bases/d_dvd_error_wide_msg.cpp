#include <game/bases/d_dvd_error_wide_msg.hpp>

// String literals referenced by sMsgBase. Owned by the (unlanded)
// d_dvd_error.cpp TU; pinned by address so this unit emits no literals.
extern const char dvdStr_80321F70[];
extern const char dvdStr_80321F98[];
extern const char dvdStr_80321FE8[];
extern const char dvdStr_80322004[];
extern const char dvdStr_80322050[];
extern const char dvdStr_8032206C[];
extern const char dvdStr_803220C0[];
extern const char dvdStr_803220E0[];
extern const char dvdStr_80322148[];
extern const char dvdStr_803221A8[];
extern const char dvdStr_803221C4[];
extern const char dvdStr_80322234[];
extern const char dvdStr_803222A4[];
extern const char dvdStr_803222C4[];
extern const char dvdStr_80322338[];
extern const char dvdStr_80322354[];
extern const char dvdStr_803223C8[];
extern const char dvdStr_803223E8[];

// .data:0x80322448, size 0xA8 -- the last row is what dtk attributes to the
// anonymous lbl_803224E4; retail reads it as row 13 of this table.
const char* dDvdErrorWideMsg_c::sMsgBase[14][3] = {
    {dvdStr_80321F70, dvdStr_80321F98, dvdStr_80321F70},  // JP
    {dvdStr_80321FE8, dvdStr_80322004, dvdStr_80321FE8},  // EN
    {dvdStr_80322050, dvdStr_8032206C, dvdStr_80322050},  // EN "Disc"
    {dvdStr_803220C0, dvdStr_803220E0, dvdStr_803220C0},  // FR
    {dvdStr_803220C0, dvdStr_80322148, dvdStr_803220C0},  // FR (alt text)
    {dvdStr_803221A8, dvdStr_803221C4, dvdStr_803221A8},  // ES
    {dvdStr_803221A8, dvdStr_80322234, dvdStr_803221A8},  // ES (alt text)
    {dvdStr_803222A4, dvdStr_803222C4, dvdStr_803222A4},  // DE
    {dvdStr_80322338, dvdStr_80322354, dvdStr_80322338},  // IT
    {dvdStr_80322050, dvdStr_8032206C, dvdStr_80322050},  // EN "Disc" (2)
    {dvdStr_803223C8, dvdStr_803223E8, dvdStr_803223C8},  // Game Disc
    {dvdStr_803223C8, dvdStr_803223E8, dvdStr_803223C8},
    {dvdStr_803223C8, dvdStr_803223E8, dvdStr_803223C8},
    {dvdStr_803223C8, dvdStr_803223E8, dvdStr_803223C8},
};

// Unnamed retail helpers in this class's original TU (d_dvd_error.cpp).
extern "C" void fn_80107AF0(dDvdErrorWideMsg_c*, int, int);
extern "C" void fn_80107DA0(LangLoc*, int, LangLocString*);

dDvdErrorWideMsg_c::dDvdErrorWideMsg_c() {
    const char** m = &mpMsgs[0][0];
    char** w = &mpWide[0][0];
    for (s16 i = 0; i < 7; i++) {
        m[0] = NULL;
        for (s16 j = 0; j < 3; j++) {
            w[j] = NULL;
        }
        m[1] = NULL;
        m += 2;
        for (s16 j = 3; j < 6; j++) {
            w[j] = NULL;
        }
        w += 6;
    }
}

void dDvdErrorWideMsg_c::initialize(const char** msgs) {
    const char** slot = &mpMsgs[0][0];
    const char** p = msgs;
    for (u32 i = 0; i < 14; i++) {
        *slot = *p;
        for (u32 j = 0; j < 3; j++) {
            fn_80107AF0(this, i, j);
        }
        slot++;
        p++;
    }
    fn_80107DA0(&mLangLoc, 2, &mLocStr);
}

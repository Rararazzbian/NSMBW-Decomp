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
extern const char dvdStr_80322148[];
extern const char dvdStr_803221A8[];
extern const char dvdStr_803221C4[];
extern const char dvdStr_803222A4[];
extern const char dvdStr_803222C4[];
extern const char dvdStr_80322338[];
extern const char dvdStr_80322354[];
extern const char dvdStr_803223C8[];
extern const char dvdStr_803223E8[];

const char* dDvdErrorWideMsg_c::sMsgBase[14][3] = {
    {dvdStr_80321F70, dvdStr_80321F98, dvdStr_80321F70},  // JP
    {dvdStr_80321FE8, dvdStr_80322004, dvdStr_80321FE8},  // EN
    {dvdStr_80322050, dvdStr_8032206C, dvdStr_80322050},  // EN "Disc"
    {dvdStr_803220C0, dvdStr_80322148, dvdStr_803220C0},  // FR
    {dvdStr_803220C0, dvdStr_80322148, dvdStr_803220C0},  // FR (dup)
    {dvdStr_803221A8, dvdStr_803221C4, dvdStr_803221A8},  // ES
    {dvdStr_803221A8, dvdStr_803221C4, dvdStr_803221A8},  // ES (dup)
    {dvdStr_803222A4, dvdStr_803222C4, dvdStr_803222A4},  // DE
    {dvdStr_80322338, dvdStr_80322354, dvdStr_80322338},  // IT
    {dvdStr_80322050, dvdStr_8032206C, dvdStr_80322050},  // EN "Disc" (dup)
    {dvdStr_803223C8, dvdStr_803223E8, dvdStr_803223C8},  // Game Disc
    {dvdStr_803223C8, dvdStr_803223E8, dvdStr_803223C8},
    {dvdStr_803223C8, dvdStr_803223E8, dvdStr_803223C8},
    {dvdStr_803223C8, dvdStr_803223E8, dvdStr_803223C8},
};

// Unnamed retail helpers between this class and LangLocString's ctor.
extern "C" void fn_80107AF0(dDvdErrorWideMsg_c*, int, int);
extern "C" void fn_80107DA0(LangLoc*, int, LangLocString*);

dDvdErrorWideMsg_c::dDvdErrorWideMsg_c() {
    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 3; j++) {
            mpWideMsg[i][j] = NULL;
        }
        mpMsg[i] = NULL;
    }
}

void dDvdErrorWideMsg_c::initialize(const char** msgs) {
    for (int i = 0; i < 14; i++) {
        mpMsg[i] = *(msgs++);
        for (int j = 0; j < 3; j++) {
            fn_80107AF0(this, i, j);
        }
    }
    fn_80107DA0(&mLangLoc, 2, &mLocStr);
}

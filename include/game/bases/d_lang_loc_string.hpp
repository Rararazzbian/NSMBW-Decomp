#pragma once
#include <types.h>

/// @brief Language/localization code-string table.
/// @unofficial Only ::LangLocString's constructor exists in wiimj2d.dol;
/// nothing here constructs or reads the class, so every member below is an
/// inferred char* slot named by its offset.
class LangLocString {
public:
    LangLocString();

    const char *m000;  ///< "JPJpn" @unofficial
    const char *m004;  ///< "USEng" @unofficial
    const char *m008;  ///< "EUEng" @unofficial
    const char *m00c;  ///< "USFre" @unofficial
    const char *m010;  ///< "EUFre" @unofficial
    const char *m014;  ///< "USSpa" @unofficial
    const char *m018;  ///< "EUSpa" @unofficial
    const char *m01c;  ///< "EUGer" @unofficial
    const char *m020;  ///< "EUIta" @unofficial
    const char *m024;  ///< "EUDut" @unofficial
    const char *m028;  ///< "CHCsm" @unofficial
    const char *m02c;  ///< "CHCtr" @unofficial
    const char *m030;  ///< "KRKor" @unofficial
    const char *m034;  ///< "CMCmn" @unofficial
    const char *m038;  ///< "_JpnJP" @unofficial
    const char *m03c;  ///< "_EngUS" @unofficial
    const char *m040;  ///< "_EngEU" @unofficial
    const char *m044;  ///< "_FreUS" @unofficial
    const char *m048;  ///< "_FreEU" @unofficial
    const char *m04c;  ///< "_SpaUS" @unofficial
    const char *m050;  ///< "_SpaEU" @unofficial
    const char *m054;  ///< "_GerEU" @unofficial
    const char *m058;  ///< "_ItaEU" @unofficial
    const char *m05c;  ///< "_DutEU" @unofficial
    const char *m060;  ///< "_CsmCH" @unofficial
    const char *m064;  ///< "_CtrCH" @unofficial
    const char *m068;  ///< "_KorKR" @unofficial
    const char *m06c;  ///< "_CmnCM" @unofficial
    const char *m070;  ///< "Jpn" @unofficial
    const char *m074;  ///< "Eng" @unofficial
    const char *m078;  ///< "Fre" @unofficial
    const char *m07c;  ///< "Spa" @unofficial
    const char *m080;  ///< "Ger" @unofficial
    const char *m084;  ///< "Ita" @unofficial
    const char *m088;  ///< "Dut" @unofficial
    const char *m08c;  ///< "Csm" @unofficial
    const char *m090;  ///< "Ctr" @unofficial
    const char *m094;  ///< "Kor" @unofficial
    const char *m098;  ///< "Cmn" @unofficial
    const char *m09c;  ///< "JP" @unofficial
    const char *m0a0;  ///< "US" @unofficial
    const char *m0a4;  ///< "EU" @unofficial
    const char *m0a8;  ///< "CH" @unofficial
    const char *m0ac;  ///< "KR" @unofficial
    const char *m0b0;  ///< "CM" @unofficial
};

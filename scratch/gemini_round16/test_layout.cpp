#include <game/bases/d_enemy_toride_kokoopa.hpp>
#include <cstddef>

#define CHECK_OFFSET(type, member, expected)     STATIC_ASSERT(offsetof(type, member) == expected)

// Offset assertions
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk600, 0x600);
CHECK_OFFSET(dEnTorideKokoopa_c, mMdlKokoopa, 0x604);
CHECK_OFFSET(dEnTorideKokoopa_c, mAnmChrKokoopa, 0x644);
CHECK_OFFSET(dEnTorideKokoopa_c, mAnmMatClr, 0x67C);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk6A8, 0x6A8);
CHECK_OFFSET(dEnTorideKokoopa_c, mAnmTexPat, 0x6AC);
CHECK_OFFSET(dEnTorideKokoopa_c, mMdlShell, 0x6D8);
CHECK_OFFSET(dEnTorideKokoopa_c, mAnmChrShell, 0x718);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk750, 0x750);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk754, 0x754);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk758, 0x758);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk75C, 0x75C);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk760, 0x760);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk764, 0x764);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk768, 0x768);
CHECK_OFFSET(dEnTorideKokoopa_c, mPad76C, 0x76C);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk770, 0x770);
CHECK_OFFSET(dEnTorideKokoopa_c, mBlitzPos, 0x774);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk780, 0x780);
CHECK_OFFSET(dEnTorideKokoopa_c, mLookatPos, 0x784);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk790, 0x790);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk792, 0x792);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk794, 0x794);
CHECK_OFFSET(dEnTorideKokoopa_c, mAtkCnt, 0x798);
CHECK_OFFSET(dEnTorideKokoopa_c, mCc, 0x79C);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk840, 0x840);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk844, 0x844);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk848, 0x848);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnk84C, 0x84C);
CHECK_OFFSET(dEnTorideKokoopa_c, mLevelEffect1, 0x850);
CHECK_OFFSET(dEnTorideKokoopa_c, mLevelEffect2, 0x978);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnkAA0, 0xAA0);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnkAA4, 0xAA4);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnkAB0, 0xAB0);
CHECK_OFFSET(dEnTorideKokoopa_c, mPadABC, 0xABC);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnkAC0, 0xAC0);
CHECK_OFFSET(dEnTorideKokoopa_c, mPadAC2, 0xAC2);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnkAC4, 0xAC4);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnkAC8, 0xAC8);
CHECK_OFFSET(dEnTorideKokoopa_c, mScaleSpeed, 0xACC);
CHECK_OFFSET(dEnTorideKokoopa_c, mRootJntIdx, 0xAD0);
CHECK_OFFSET(dEnTorideKokoopa_c, mShellJntIdx, 0xAD4);
CHECK_OFFSET(dEnTorideKokoopa_c, mRootJntPos, 0xAD8);
CHECK_OFFSET(dEnTorideKokoopa_c, mShellJntPos, 0xAE4);
CHECK_OFFSET(dEnTorideKokoopa_c, mUnkAF0, 0xAF0);
CHECK_OFFSET(dEnTorideKokoopa_c, mLevelEffects, 0xAF4);
CHECK_OFFSET(dEnTorideKokoopa_c, mVoiceParam, 0xE6C);

STATIC_ASSERT(sizeof(dEnTorideKokoopa_c) == 0xE70);

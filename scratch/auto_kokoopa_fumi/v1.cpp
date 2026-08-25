#include <game/bases/d_actor.hpp>
#include <game/bases/d_en_fumi_check.hpp>
#include <types.h>

// Pinned anonymous pool literals (.sdata2)
extern const float l_poolZero_8042C768;
extern const float l_poolFour_8042C76C;
extern const float l_poolTen_8042C770;

/// @unofficial Read-only view of the fields the fumi checks touch on the
/// player actor. The leading data block places the class vptr at +0x60,
/// matching the real player layout.
class FumiPlayerView {
public:
    u8 mLead[0x60];

    virtual ~FumiPlayerView();
    virtual void vf08();
    virtual void vf0C();
    virtual void vf10();
    virtual void vf14();
    virtual void vf18();
    virtual void vf1C();
    virtual void vf20();
    virtual void vf24();
    virtual void vf28();
    virtual void vf2C();
    virtual void vf30();
    virtual void vf34();
    virtual void vf38();
    virtual void vf3C();
    virtual void vf40();
    virtual void vf44();
    virtual void vf48();
    virtual void vf4C();
    virtual void vf50();
    virtual void vf54();
    virtual void vf58();
    virtual void vf5C();
    virtual void vf60();
    virtual void vf64();
    virtual const s8 *getDamageTable(); ///< vtable slot at +0x6C.

public:
    u8 mPad1[0xB0 - 0x64];
    float mB0; ///< @unofficial
    u8 mPad2[0xEC - 0xB4];
    float mEC; ///< @unofficial
    u8 mPad3[0x1074 - 0xF0];
    u32 mFlagA; ///< @unofficial
    u32 mFlagB; ///< @unofficial
    u8 mPad4[0x1090 - 0x107C];
    s32 mMode;  ///< @unofficial
};

/// @unofficial Read-only view of the enemy-side fields written by the check.
class FumiEnemyView {
public:
    u8 mPad[0xB0];
    float mB0; ///< @unofficial
    u8 mPad2[0xEC - 0xB4];
    float mEC; ///< @unofficial
    u8 mPad3[0x504 - 0xF0];
    u16 mFumiVals[0x80]; ///< @unofficial damage values, indexed by table entry.
};

KokoopaSpFumiCheck_c::~KokoopaSpFumiCheck_c() {}

bool KokoopaSpFumiCheck_c::operate(int &out, dEn_c *en, FumiCcInfo_c &info) {
    out = 0;
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;

    if (((pl->mFlagA | pl->mFlagB) != 0) && pl->mEC > l_poolZero_8042C768) {
        out = 0;
        return true;
    }

    if (((dBc_c *) ((u8 *) pl + 0x1EC))->isFoot() == 0 && ev->mEC > l_poolZero_8042C768) {
        if (pl->mMode == 3) {
            if (pl->mB0 >= ev->mB0 + l_poolFour_8042C76C) {
                const s8 *tbl = pl->getDamageTable();
                s8 idx = tbl[0];
                ev->mFumiVals[idx] = 0x18;
                out = 1;
                return true;
            }
        } else {
            if (pl->mB0 >= ev->mB0 + l_poolTen_8042C770) {
                const s8 *tbl = pl->getDamageTable();
                s8 idx = tbl[0];
                ev->mFumiVals[idx] = 0x18;
                out = 1;
                return true;
            }
        }
    }
    return false;
}

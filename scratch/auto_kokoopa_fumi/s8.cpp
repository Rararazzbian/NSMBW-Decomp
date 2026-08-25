#include <game/bases/d_enemy.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_en_fumi_check.hpp>
#include <types.h>

// Pinned anonymous pool literals (.sdata2)
extern const float l_poolZero_8042C768;
extern const float l_poolFour_8042C76C;
extern const float l_poolTen_8042C770;

KokoopaSpFumiCheck_c::~KokoopaSpFumiCheck_c() {}

bool KokoopaSpFumiCheck_c::operate(int &out, dEn_c *en, FumiCcInfo_c &info) {
    out = 0;
    daPlBase_c *pl = (daPlBase_c *) info.mCc2->getOwner();

    if (((*(u32 *) ((u8 *) pl + 0x1074) | *(u32 *) ((u8 *) pl + 0x1078)) != 0) &&
        pl->mSpeed.y > l_poolZero_8042C768) {
        out = 0;
        return true;
    }

    if (!pl->mBc.isFoot() && en->mSpeed.y > l_poolZero_8042C768) {
        if (*(s32 *) ((u8 *) pl + 0x1090) == 3) {
            if (pl->mPos.y >= l_poolFour_8042C76C + en->mPos.y) {
                int plrNo = pl->getPlrNo();
                en->mNoHitPlayer.mTimer[plrNo] = 24;
                out = 1;
                return true;
            }
        } else {
            if (pl->mPos.y >= l_poolTen_8042C770 + en->mPos.y) {
                int plrNo = pl->getPlrNo();
                en->mNoHitPlayer.mTimer[plrNo] = 24;
                out = 1;
                return true;
            }
        }
    }
    return false;
}

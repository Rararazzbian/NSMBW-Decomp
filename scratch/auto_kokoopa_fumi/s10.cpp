#include <game/bases/d_enemy.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_en_fumi_check.hpp>
#include <types.h>

bool KokoopaSpFumiCheck_c::operate(int &out, dEn_c *en, FumiCcInfo_c &info) {
    out = 0;
    daPlBase_c *pl = (daPlBase_c *) info.mCc2->getOwner();

    if (((*(u32 *) ((u8 *) pl + 0x1074) | *(u32 *) ((u8 *) pl + 0x1078)) != 0) &&
        pl->mSpeed.y > 0.0f) {
        out = 0;
        return true;
    }

    if (!pl->mBc.isFoot() && en->mSpeed.y > 0.0f) {
        if (*(s32 *) ((u8 *) pl + 0x1090) == 3) {
            if (pl->mPos.y >= 4.0f + en->mPos.y) {
                int plrNo = pl->getPlrNo();
                en->mNoHitPlayer.mTimer[plrNo] = 24;
                out = 1;
                return true;
            }
        } else {
            if (pl->mPos.y >= 10.0f + en->mPos.y) {
                int plrNo = pl->getPlrNo();
                en->mNoHitPlayer.mTimer[plrNo] = 24;
                out = 1;
                return true;
            }
        }
    }
    return false;
}

KokoopaSpFumiCheck_c::~KokoopaSpFumiCheck_c() {}

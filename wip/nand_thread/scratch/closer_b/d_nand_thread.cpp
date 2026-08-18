#include <types.h>
#include <string.h>
#include <game/bases/d_nand_thread.hpp>
#include <game/bases/d_message.hpp>
#include <game/bases/d_res_mng.hpp>

namespace {
inline void setIconSpeed(NANDBanner *banner, int frame, int speed) {
    banner->iconSpeed = (banner->iconSpeed & ~(3 << (frame * 2))) | (speed << (frame * 2));
}
} // namespace

// -----------------------------------------------------------------------
// spaceCheck -- 34/37 instructions. See CLOSE_B.md for the full record of
// what was tried (24 source variants across this batch and Batch 2, all
// converging on the identical single divergence below).
// -----------------------------------------------------------------------
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (answer & 5) {
                mError = 7;
            } else if (answer & 0xa) {
                mError = 8;
            }
        }
    }
}

// -----------------------------------------------------------------------
// writeBanner -- 63/66 instructions. See CLOSE_B.md for the register-
// pressure / bitfield-store gap and everything tried to close it.
// -----------------------------------------------------------------------
s32 dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static NANDBanner a_banner;
    static const char *c_icon_res = "save_icon.bti";

    const wchar_t *title = dMessage_c::getMsg(0x15f91, 1);
    const wchar_t *subtitle = dMessage_c::getMsg(0x15f91, 0);
    NANDInitBanner(&a_banner, 0, title, subtitle);

    nw4r::g3d::ResFile bannerRes = dResMng_c::m_instance->getRes("save_banner", "save_banner_EU.bti");
    const u8 *bannerBase = (const u8 *)bannerRes.ptr();
    memcpy(a_banner.bannerTexture, bannerBase + *(const u32 *)(bannerBase + 0x1c), sizeof(a_banner.bannerTexture));

    nw4r::g3d::ResFile iconRes = dResMng_c::m_instance->getRes("save_banner", c_icon_res);
    const u8 *iconBase = (const u8 *)iconRes.ptr();
    memcpy(a_banner.iconTexture, iconBase + *(const u32 *)(iconBase + 0x1c), 0x1200);

    setIconSpeed(&a_banner, 0, 2);

    return NANDWrite(fileInfo, &a_banner, 0x72a0);
}

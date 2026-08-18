#include <types.h>
#include <string.h>
#include <game/bases/d_nand_thread.hpp>
#include <game/bases/d_message.hpp>
#include <game/bases/d_res_mng.hpp>

namespace {
inline void setIconSpeed(NANDBanner *banner, int frame, int speed) {
    banner->iconSpeed = (banner->iconSpeed & ~(3 << (frame * 2))) | (speed << (frame * 2));
}

const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
u8 l_tmpSave[0x3FA0];
} // namespace

bool dNandThread_c::cmdSave(const void *saveData) {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 4;
        memcpy(l_tmpSave, saveData, sizeof(l_tmpSave));
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

s32 dNandThread_c::save() {
    setNandError(NANDCreate(sc_GAME_FILE, 0x3c, 0));
    if (mError == 0) {
        NANDFileInfo info;
        setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 2, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
        if (mError == 0) {
            s32 written = NANDWrite(&info, l_tmpSave, sizeof(l_tmpSave));
            if (written < 0) {
                setNandError(written);
                if (mError != 0) {
                    if (mError != 6) {
                        setNandError(NANDSimpleSafeCancel(&info));
                        if (mError == 0)
                            return 2;
                    }
                    return 1;
                }
            }
            setNandError(NANDSimpleSafeClose(&info));
            if (mError == 0)
                return createBanner();
            return 1;
        }
        if (mError != 6)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }
    return 1;
}

bool dNandThread_c::createBanner() {
    setNandError(NANDCreate(sc_TEMP_BANNER_FILE, 0x3c, 0));
    if (mError != 0)
        return true;

    NANDFileInfo info;
    setNandError(NANDOpen(sc_TEMP_BANNER_FILE, &info, 2));
    if (mError != 0)
        return true;

    u32 written = writeBanner(&info);
    if (written != 0x72a0) {
        setNandError(written);
        return true;
    }

    setNandError(NANDClose(&info));
    if (mError != 0)
        return true;

    char homeDir[0x40] = {0};
    setNandError(NANDGetHomeDir(homeDir));
    if (mError != 0)
        return true;

    setNandError(NANDMove(sc_TEMP_BANNER_FILE, homeDir));
    return mError != 0;
}

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

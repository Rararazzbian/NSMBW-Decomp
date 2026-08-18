#include <types.h>
#include <game/bases/d_nand_thread.hpp>
#include <game/sLib/s_Crc.hpp>

namespace {
const char sc_GAME_FILE[] = "wiimj2d.sav";
u8 l_safeCopyBuf[0x4000];
char l_tmpSave[0x3FA0];
} // namespace

s32 dNandThread_c::load() {
    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 1, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    bool ok1 = (mError == 0);
    if (!ok1) {
        bool eq6a = (mError == 6);
        if (eq6a)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    u32 length;
    setNandError(NANDGetLength(&info, &length));
    bool ok2 = (mError == 0);
    if (!ok2) {
        bool eq6b = (mError == 6);
        if (eq6b)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    if (length != sizeof(l_tmpSave)) {
        setNandError(NANDSimpleSafeClose(&info));
        bool ok3 = (mError == 0);
        if (ok3) {
            mError = 6;
        }
        return 1;
    }

    s32 written = NANDRead(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        bool ok4 = (mError == 0);
        if (!ok4) {
            bool eq6c = (mError == 6);
            if (eq6c) {
                setNandError(NANDSimpleSafeCancel(&info));
                bool ok5 = (mError == 0);
                if (ok5)
                    return 2;
            }
            return 1;
        }
    }

    setNandError(NANDSimpleSafeClose(&info));
    bool ok6 = (mError == 0);
    if (!ok6) {
        return 1;
    }

    if (l_tmpSave[0] != "SMNP"[0] || l_tmpSave[1] != "SMNP"[1] ||
        l_tmpSave[2] != "SMNP"[2]) {
        mError = 6;
        return 1;
    }
    if (l_tmpSave[3] != "SMNP"[3]) {
        mError = 6;
        return 1;
    }

    if (!checkCRC()) {
        mError = 6;
        return 1;
    }
    return 0;
}

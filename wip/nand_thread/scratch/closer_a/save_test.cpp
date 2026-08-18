#include <types.h>
#include <game/bases/d_nand_thread.hpp>

namespace {
const char sc_GAME_FILE[] = "wiimj2d.sav";
u8 l_safeCopyBuf[0x4000];
char l_tmpSave[0x3FA0];
} // namespace

s32 dNandThread_c::save() {
    setNandError(NANDCreate(sc_GAME_FILE, 0x3c, 0));
    bool ok1 = (mError == 0);
    if (!ok1) {
        return 1;
    }

    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 2, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    bool ok2 = (mError == 0);
    if (!ok2) {
        bool eq6 = (mError == 6);
        if (eq6)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    s32 written = NANDWrite(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        bool ok3 = (mError == 0);
        if (!ok3) {
            bool eq6b = (mError == 6);
            if (eq6b) {
                setNandError(NANDSimpleSafeCancel(&info));
                bool ok4 = (mError == 0);
                if (ok4)
                    return 2;
            }
            return 1;
        }
    }
    setNandError(NANDSimpleSafeClose(&info));
    bool ok5 = (mError == 0);
    if (ok5)
        return createBanner();
    return 1;
}

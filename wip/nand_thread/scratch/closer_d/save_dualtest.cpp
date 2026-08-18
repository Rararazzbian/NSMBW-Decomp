#include <game/bases/d_nand_thread.hpp>
#include <lib/revolution/NAND.h>

namespace {
const char sc_GAME_FILE[] = "wiimj2d.sav";
u8 l_safeCopyBuf[0x4000];
char l_tmpSave[0x3FA0];
} // namespace

// Testing the coordinator's "two live bools from one plain, CSE-able read"
// idea, applied at full function scale (not an isolated probe), to see
// whether register pressure / function size changes whether the FIRST
// (standalone) test of a chain also materialises.
s32 dNandThread_c::save() {
    setNandError(NANDCreate(sc_GAME_FILE, 0x3c, 0));
    bool ok1 = (mError == 0);
    if (!ok1) {
        return 1;
    }

    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 2, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    int e2 = mError;
    bool ok2 = (e2 == 0);
    bool eq6 = (e2 == 6);
    if (!ok2) {
        if (eq6)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    s32 written = NANDWrite(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        int e3 = mError;
        bool ok3 = (e3 == 0);
        bool eq6b = (e3 == 6);
        if (!ok3) {
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
    if (!ok5)
        return 1;
    return createBanner();
}

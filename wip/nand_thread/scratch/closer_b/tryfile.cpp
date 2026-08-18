#include <game/bases/d_nand_thread.hpp>


bool dNandThread_c::spaceCheck() {
    s32 pad = 0;
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err + pad);
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

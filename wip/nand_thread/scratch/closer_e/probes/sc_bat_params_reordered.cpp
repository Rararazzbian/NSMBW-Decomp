#include <game/bases/d_nand_thread.hpp>

bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(&answer, 3, 2);
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

#include <game/bases/d_nand_thread.hpp>


bool dNandThread_c::spaceCheck() {
    s32 err;
    u32 answer;
    answer = 0xFFFFFFFF;
    err = NANDCheck(3, 2, &answer);
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

#include <game/bases/d_nand_thread.hpp>

bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    NANDCheck(3, 2, &answer);
    setNandError(0);
    if (mError == 0) {
        if (answer & 5) {
            mError = 7;
        } else if (answer & 0xa) {
            mError = 8;
        }
    }
}

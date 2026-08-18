
#include <game/bases/d_nand_thread.hpp>
bool dNandThread_c::setNandError(long err) {
    switch (err) {
    default:
        mError = (err != 0);
        break;
    }
    return mError != 0;
}


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

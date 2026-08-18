#include <game/bases/d_nand_thread.hpp>

void dNandThread_c::cmdExistCheck() {
    if (OSTryLockMutex(&mMutex.mOSMutex)) {
        mError = 0;
        mFileExists = false;
        mState = 1;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
    }
}

#include <game/bases/d_nand_thread.hpp>

// ---- proposed, not-yet-decompiled dependency (see report) ----------------
class sCrc {
public:
    static unsigned long calcCRC32(const void *p, unsigned long len);
};

// ---- stand-ins for objects owned by the LEAD / other batches -------------
// These are NOT part of the final source; they exist only so this TU
// compiles in isolation. See BATCH4.md for the types actually needed.
namespace {
const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
char l_tmpSave[0x3FA0];
} // namespace

dNandThread_c *dNandThread_c::m_instance;

// ---------------------------------------------------------------- cmdLoad
bool dNandThread_c::cmdLoad() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 5;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- load
s32 dNandThread_c::load() {
    NANDFileInfo info;
    long err = NANDSimpleSafeOpen(sc_GAME_FILE, &info, 1, l_safeCopyBuf,
                                   sizeof(l_safeCopyBuf));
    setNandError(err);
    if (mError) {
        if (mError != 6) {
            err = NANDSimpleSafeCancel(&info);
            setNandError(err);
        }
        return 1;
    }

    u32 length;
    err = NANDGetLength(&info, &length);
    setNandError(err);
    if (mError != 0) {
        if (mError != 6) {
            err = NANDSimpleSafeCancel(&info);
            setNandError(err);
        }
        return 1;
    }

    if (length != sizeof(l_tmpSave)) {
        err = NANDSimpleSafeClose(&info);
        setNandError(err);
        if (mError != 0) {
            return 1;
        }
        mError = 6;
        return 1;
    }

    err = NANDRead(&info, l_tmpSave, sizeof(l_tmpSave));
    if (err < 0) {
        setNandError(err);
        if (mError != 0) {
            if (mError != 6) {
                err = NANDSimpleSafeCancel(&info);
                setNandError(err);
                if (mError != 0) {
                    return 2;
                }
            }
            return 1;
        }
    }

    err = NANDSimpleSafeClose(&info);
    setNandError(err);
    if (mError != 0) {
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

// ---------------------------------------------------------------- checkCRC
bool dNandThread_c::checkCRC() {
    if (*(unsigned long *)(l_tmpSave + 0x69c) != sCrc::calcCRC32(l_tmpSave + 4, 0x698)) {
        return false;
    }
    char *dataA = l_tmpSave + 0x6a0;
    char *p = l_tmpSave;
    char *dataB = l_tmpSave + 0x2320;
    for (int i = 0; i < 3; i++) {
        unsigned long crcA = sCrc::calcCRC32(dataA, 0x97c);
        if (crcA != *(unsigned long *)(p + 0x101c)) {
            return false;
        }
        unsigned long crcB = sCrc::calcCRC32(dataB, 0x97c);
        if (crcB != *(unsigned long *)(p + 0x2c9c)) {
            return false;
        }
        dataA += 0x980;
        p += 0x980;
        dataB += 0x980;
    }
    return true;
}

// ---------------------------------------------------------------- cmdDeleteFile
bool dNandThread_c::cmdDeleteFile() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 3;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- deleteFile
void dNandThread_c::deleteFile() {
    long err = NANDDelete(sc_BANNER_FILE);
    setNandError(err);
    if (mError == 0) {
        err = NANDDelete(sc_GAME_FILE);
        setNandError(err);
        if (mError != 0) {
            return;
        }
    }
}

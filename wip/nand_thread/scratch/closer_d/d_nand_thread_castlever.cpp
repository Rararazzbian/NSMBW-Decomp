#include <game/bases/d_nand_thread.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/sLib/s_Crc.hpp>
#include <lib/revolution/NAND.h>
#include <lib/revolution/OS.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Regression-test stand-in: everybody else's proven-matching functions,
// assembled into one TU so the closer_d lever (a per-read volatile CAST
// inside save()/load() only, not a header change) can be checked against
// every function that touches mError, not just save()/load(). Uses the REAL
// header unmodified -- plain `int mError`, no shadow include needed.
namespace {
const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
char l_tmpSave[0x3FA0];
} // namespace

dNandThread_c *dNandThread_c::m_instance;

// ---------------------------------------------------------------- batch 1
void *dNandThread_c::run() {
    OSLockMutex(&mMutex.mOSMutex);
    for (;;) {
        mState = 0;
        OSWaitCond(&mMutex.mOSCond, &mMutex.mOSMutex);
        switch (mState) {
        case 1:
            existCheck();
            break;
        case 2:
            spaceCheck();
            break;
        case 4:
            while (save() == 2) {}
            break;
        case 5:
            while (load() == 2) {}
            break;
        case 3:
            deleteFile();
            break;
        case 6:
            OSUnlockMutex(&mMutex.mOSMutex);
            return 0;
        }
    }
}

void dNandThread_c::create(EGG::Heap *heap) {
    EGG::Heap *prevHeap = mHeap::setCurrentHeap(heap);
    dNandThread_c *thread = new dNandThread_c(OSGetThreadPriority(OSGetCurrentThread()) - 1, nullptr);
    mHeap::setCurrentHeap(prevHeap);
    OSResumeThread(*(OSThread **)((u8 *)thread + 0x8));
}

void dNandThread_c::setNandError(long err) {
    switch (err) {
    case NAND_RESULT_NOEXISTS:
    case NAND_RESULT_EXISTS:
    case NAND_RESULT_OK:
        mError = 0;
        break;
    case NAND_RESULT_CORRUPT:
        mError = 1;
        break;
    case NAND_RESULT_MAXBLOCKS:
        mError = 2;
        break;
    case NAND_RESULT_MAXFILES:
        mError = 3;
        break;
    case NAND_RESULT_BUSY:
    case NAND_RESULT_ALLOC_FAILED:
        mError = 4;
        break;
    case NAND_RESULT_AUTHENTICATION:
    case NAND_RESULT_ECC_CRIT:
        mError = 6;
        break;
    default:
        mError = 5;
        break;
    }
}

void *dNandThread_c::getSaveData() {
    return l_tmpSave;
}

// ---------------------------------------------------------------- batch 2
bool dNandThread_c::cmdExistCheck() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mFileExists = false;
        mState = 1;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

bool dNandThread_c::existCheck() {
    u8 count = 0;
    u8 type;

    s32 err = NANDGetType(sc_GAME_FILE, &type);
    setNandError(err);
    if (mError == 0) {
        if (err == 0 && type == 1) {
            count = 1;
        }

        err = NANDGetType(sc_BANNER_FILE, &type);
        setNandError(err);
        if (mError == 0) {
            if (err == 0 && type == 1) {
                count++;
            }

            if (count == 2) {
                mFileExists = true;
            }
        }
    }
}

bool dNandThread_c::cmdSpaceCheck() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 2;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
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

// ---------------------------------------------------------------- batch 3
bool dNandThread_c::createBanner() {
    setNandError(NANDCreate(sc_TEMP_BANNER_FILE, 0x3c, 0));
    if (mError != 0)
        return true;

    NANDFileInfo info;
    setNandError(NANDOpen(sc_TEMP_BANNER_FILE, &info, 2));
    if (mError != 0)
        return true;

    u32 written = writeBanner(&info);
    if (written != 0x72a0) {
        setNandError(written);
        return true;
    }

    setNandError(NANDClose(&info));
    if (mError != 0)
        return true;

    char homeDir[0x40] = {0};
    setNandError(NANDGetHomeDir(homeDir));
    if (mError != 0)
        return true;

    setNandError(NANDMove(sc_TEMP_BANNER_FILE, homeDir));
    return mError != 0;
}

// ---------------------------------------------------------------- batch 4
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

// ---------------------------------------------------------------- closer_d
//
// THE LEVER: `*(volatile int *)&mError` at the read site, NOT a `volatile`
// header qualifier. mError stays a plain `int` in the real header, untouched.
//
// Proof (see CLOSE_D.md for the full A/B): a bare `bool ok = (mError == K);`
// on a plain field always folds to `cmpwi` under MWCC -O4, regardless of the
// surrounding arm shape (block-fallthrough like existCheck(), or an early
// guard-clause return like save()/load()). The materialising `cntlzw`+`srwi.`
// idiom appears ONLY when the read feeding that bool is opaque to the
// optimiser (a volatile access, or an external call's raw return -- see
// cmdSpaceCheck's `OSTryLockMutex`), *combined* with storing the comparison
// into a named bool before branching on it. A cast-qualified read at the use
// site supplies that opacity locally, without qualifying the member globally,
// so existCheck() (which never uses a bool intermediate) is provably
// unaffected -- its source is not touched by this change at all.
//
// The `== 6` polarity (not `!= 6`) is CLOSE_A's finding, carried over
// unchanged: NANDSimpleSafeCancel is only called on the FALLTHROUGH side of
// each `beq`, taken when mError != 6, so the guard is `if (mError == 6)`.
//
// Residual: a *chained* pair (mError==0 immediately followed, on the failure
// path only, by mError==6) shares ONE load in target but this lever forces a
// second volatile read for the second test -- volatile semantics forbid
// merging two textual reads even with nothing between them but the branch.
// Capturing the first read into a plain local so the second test can reuse it
// was tried and confirmed to collapse BOTH tests back to `cmpwi` (the
// opacity does not survive being copied into an ordinary variable). This is
// the same residual CLOSE_A already characterised under the header-volatile
// approach; it persists unchanged here. See CLOSE_D.md.
s32 dNandThread_c::save() {
    setNandError(NANDCreate(sc_GAME_FILE, 0x3c, 0));
    bool ok1 = (*(volatile int *)&mError == 0);
    if (!ok1) {
        return 1;
    }

    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 2, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    bool ok2 = (*(volatile int *)&mError == 0);
    if (!ok2) {
        bool eq6 = (*(volatile int *)&mError == 6);
        if (eq6)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    s32 written = NANDWrite(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        bool ok3 = (*(volatile int *)&mError == 0);
        if (!ok3) {
            bool eq6b = (*(volatile int *)&mError == 6);
            if (eq6b) {
                setNandError(NANDSimpleSafeCancel(&info));
                bool ok4 = (*(volatile int *)&mError == 0);
                if (ok4)
                    return 2;
            }
            return 1;
        }
    }
    setNandError(NANDSimpleSafeClose(&info));
    bool ok5 = (*(volatile int *)&mError == 0);
    if (!ok5)
        return 1;
    return createBanner();
}

s32 dNandThread_c::load() {
    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 1, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    bool ok1 = (*(volatile int *)&mError == 0);
    if (!ok1) {
        bool eq6a = (*(volatile int *)&mError == 6);
        if (eq6a)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    u32 length;
    setNandError(NANDGetLength(&info, &length));
    bool ok2 = (*(volatile int *)&mError == 0);
    if (!ok2) {
        bool eq6b = (*(volatile int *)&mError == 6);
        if (eq6b)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    if (length != sizeof(l_tmpSave)) {
        setNandError(NANDSimpleSafeClose(&info));
        bool ok3 = (*(volatile int *)&mError == 0);
        if (ok3) {
            mError = 6;
        }
        return 1;
    }

    s32 written = NANDRead(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        bool ok4 = (*(volatile int *)&mError == 0);
        if (!ok4) {
            bool eq6c = (*(volatile int *)&mError == 6);
            if (eq6c) {
                setNandError(NANDSimpleSafeCancel(&info));
                bool ok5 = (*(volatile int *)&mError == 0);
                if (ok5)
                    return 2;
            }
            return 1;
        }
    }

    setNandError(NANDSimpleSafeClose(&info));
    bool ok6 = (*(volatile int *)&mError == 0);
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

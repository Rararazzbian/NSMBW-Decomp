#include <game/bases/d_nand_thread.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/bases/d_message.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/sLib/s_Crc.hpp>
#include <lib/revolution/NAND.h>
#include <lib/revolution/OS.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Regression-test stand-in: everybody else's proven-matching functions,
// assembled into one TU so a header change (volatile mError) can be checked
// against every function that touches mError, not just save()/load().
namespace {
const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
char l_tmpSave[0x3FA0];
inline void setIconSpeed(NANDBanner *banner, int frame, int speed) {
    banner->iconSpeed = (banner->iconSpeed & ~(3 << (frame * 2))) | (speed << (frame * 2));
}
} // namespace

dNandThread_c *dNandThread_c::m_instance;

dNandThread_c::dNandThread_c(int msgCount, EGG::Heap *heap)
    : EGG::Thread(0x4000, 0, msgCount, heap) {
    mState = 0;
    m_instance = this;

    u8 *saveGame;
    u8 *tempGame;
    u8 buf[0x3fa0] ALIGN(32);
    dSaveMng_c *saveMng = dSaveMng_c::m_instance;
    memcpy(buf, &saveMng->mHeader, sizeof(dMj2dHeader_c));
    saveGame = buf + sizeof(dMj2dHeader_c);
    tempGame = buf + sizeof(dMj2dHeader_c) + 3 * sizeof(dMj2dGame_c);

    for (s8 i = 0; i < 3; i++) {
        memcpy(saveGame, saveMng->getSaveGame(i), sizeof(dMj2dGame_c));
        memcpy(tempGame, saveMng->getTempGame(i), sizeof(dMj2dGame_c));
        tempGame += sizeof(dMj2dGame_c);
        saveGame += sizeof(dMj2dGame_c);
    }

    memcpy(l_tmpSave, buf, sizeof(l_tmpSave));
}

dNandThread_c::~dNandThread_c() {
    m_instance = nullptr;
}

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
bool dNandThread_c::cmdSave(const void *saveData) {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 4;
        memcpy(l_tmpSave, saveData, sizeof(l_tmpSave));
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

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

s32 dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static NANDBanner a_banner;
    static const char *c_icon_res = "save_icon.bti";

    const wchar_t *title = dMessage_c::getMsg(0x15f91, 1);
    const wchar_t *subtitle = dMessage_c::getMsg(0x15f91, 0);
    NANDInitBanner(&a_banner, 0, title, subtitle);

    nw4r::g3d::ResFile bannerRes = dResMng_c::m_instance->getRes("save_banner", "save_banner_EU.bti");
    const u8 *bannerBase = (const u8 *)bannerRes.ptr();
    memcpy(a_banner.bannerTexture, bannerBase + *(const u32 *)(bannerBase + 0x1c), sizeof(a_banner.bannerTexture));

    nw4r::g3d::ResFile iconRes = dResMng_c::m_instance->getRes("save_banner", c_icon_res);
    const u8 *iconBase = (const u8 *)iconRes.ptr();
    memcpy(a_banner.iconTexture, iconBase + *(const u32 *)(iconBase + 0x1c), 0x1200);

    setIconSpeed(&a_banner, 0, 2);

    return NANDWrite(fileInfo, &a_banner, 0x72a0);
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

// ---------------------------------------------------------------- closer_a
//
// Every `bool x = (mError OP k); if (...)` below is load-bearing, not style:
// with mError plain `int`, MWCC folds all of these back down to a bare
// `cmpwi`. With mError `volatile` (see the header note), the identical shape
// produces the target's `cntlzw`+`srwi.` materialisation exactly. See
// CLOSE_A.md for the A/B proof.
//
// The `== 6` polarity (not `!= 6`, which both prior batches assumed) is
// proven directly from the branch target at every one of the five save()/
// load() occurrences: NANDSimpleSafeCancel is only called on the FALLTHROUGH
// side of the `beq`, which is taken when mError != 6. See CLOSE_A.md.
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

s32 dNandThread_c::load() {
    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 1, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    bool ok1 = (mError == 0);
    if (!ok1) {
        bool eq6a = (mError == 6);
        if (eq6a)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    u32 length;
    setNandError(NANDGetLength(&info, &length));
    bool ok2 = (mError == 0);
    if (!ok2) {
        bool eq6b = (mError == 6);
        if (eq6b)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    if (length != sizeof(l_tmpSave)) {
        setNandError(NANDSimpleSafeClose(&info));
        bool ok3 = (mError == 0);
        if (ok3) {
            mError = 6;
        }
        return 1;
    }

    s32 written = NANDRead(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        bool ok4 = (mError == 0);
        if (!ok4) {
            bool eq6c = (mError == 6);
            if (eq6c) {
                setNandError(NANDSimpleSafeCancel(&info));
                bool ok5 = (mError == 0);
                if (ok5)
                    return 2;
            }
            return 1;
        }
    }

    setNandError(NANDSimpleSafeClose(&info));
    bool ok6 = (mError == 0);
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

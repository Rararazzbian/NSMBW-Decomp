#include <game/bases/d_nand_thread.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/mLib/m_heap.hpp>
#include <lib/revolution/NAND.h>
#include <lib/revolution/OS.h>

// ---------------------------------------------------------------------------
// Batch-1 scratch stand-in for objects this batch does not own. LEAD owns the
// real definitions at file top of d_nand_thread.cpp; these exist here only so
// this TU compiles standalone. Do not bank this block.
namespace {
    u8 l_safeCopyBuf[0x4000];
    u8 l_tmpSave[0x3fa0];
}

dNandThread_c *dNandThread_c::m_instance;
// ---------------------------------------------------------------------------

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
        saveGame += sizeof(dMj2dGame_c);
        tempGame += sizeof(dMj2dGame_c);
    }

    memcpy(l_tmpSave, buf, sizeof(l_tmpSave));
}

dNandThread_c::~dNandThread_c() {
    m_instance = nullptr;
}

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
    // EGG::Thread does not yet name the field at +0x8 (inside mPad); it holds
    // the OSThread handle. Unofficial cast pending eggThread.h attribution.
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

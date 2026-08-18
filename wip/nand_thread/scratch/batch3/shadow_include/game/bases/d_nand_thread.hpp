#pragma once

#include <types.h>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggThread.h>
#include <lib/revolution/NAND.h>
#include <lib/revolution/OS.h>

namespace EGG {

/// @brief A synchronisation primitive base.
/// @unofficial Reconstructed from __vt__Q23EGG5Mutex (0x80317D6C), which has a
/// single virtual slot holding this class's destructor.
class Mutex {
public:
    Mutex() {}
    virtual ~Mutex() {}
};

} // namespace EGG

/// @brief An EGG::Mutex paired with the OS condition variable it is waited on.
/// @unofficial Reconstructed from dNandThread_c's constructor, which stores the
/// vtable at +0x00, passes +0x04 to OSInitMutex and +0x1C to OSInitCond.
class mMutex : public EGG::Mutex {
public:
    mMutex() {}
    virtual ~mMutex() {}

    OSMutex mOSMutex; ///< [0x04] size 0x18
    OSCond mOSCond;   ///< [0x1C] size 0x08
};

/// @brief The background thread that performs all NAND save-file I/O.
class dNandThread_c : public EGG::Thread {
public:
    dNandThread_c(int msgCount, EGG::Heap *heap);
    virtual ~dNandThread_c();
    virtual void *run();

    bool cmdExistCheck();
    bool existCheck();

    bool cmdSpaceCheck();
    bool spaceCheck();

    /// @unofficial Name not recovered (unnamed in the symbol map, 0x800CF170).
    /// Shape derived from its body: queues the save command (mState = 4) and
    /// stages the caller's data into l_tmpSave, mirroring cmdExistCheck /
    /// cmdSpaceCheck / cmdLoad / cmdDeleteFile but taking the save payload.
    /// Confirms Batch 2's CMD_SHAPE.md hypothesis independently.
    bool cmdSave(const void *saveData);
    /// @unofficial run() does `if (save() == 2) goto retry;` -- a normalized
    /// bool cannot carry that value. PROPOSED fix: s32, not bool.
    s32 save();
    bool createBanner();
    /// @unofficial createBanner() compares the raw return against 0x72a0 (the
    /// exact byte count written) with no bool normalization at the call site
    /// or in this function's own tail. PROPOSED fix: s32, not bool.
    s32 writeBanner(NANDFileInfo *fileInfo);

    bool cmdLoad();
    bool load();
    bool checkCRC();

    bool cmdDeleteFile();
    bool deleteFile();

    /// @note `long`, not `s32`. s32 is `signed int` here and would mangle as
    /// `Fi`; the symbol is `setNandError__13dNandThread_cFl`.
    void setNandError(long err);
    void *getSaveData();

    static void create(EGG::Heap *heap);

    /// @unofficial Never read or written by any function in the TU; present so
    /// that mMutex lands at 0x50, which the constructor proves directly.
    u8 mPad4C[4];      ///< [0x4C]
    mMutex mMutex;     ///< [0x50] size 0x24
    int mState;        ///< [0x74] The command currently queued or running.
    int mError;        ///< [0x78] The last NAND error code.
    bool mFileExists;  ///< [0x7C]

    static dNandThread_c *m_instance;
};

STATIC_ASSERT(sizeof(EGG::Mutex) == 0x4);
STATIC_ASSERT(sizeof(mMutex) == 0x24);
STATIC_ASSERT(sizeof(dNandThread_c) == 0x80);

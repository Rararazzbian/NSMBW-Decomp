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
    mMutex() {
        OSInitMutex(&mOSMutex);
        OSInitCond(&mOSCond);
    }
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

    int save();
    bool createBanner();
    bool writeBanner(NANDFileInfo *fileInfo);

    bool cmdLoad();
    int load();
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

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
    /// @note OSInitMutex must be called from THIS constructor, not from the
    /// derived one. The target calls it between the base vtable store and the
    /// derived vtable store, and only a call from here lands there; anywhere
    /// else the two stores end up adjacent and MWCC drops the first as dead.
    /// @unofficial An alternative layout gives EGG::Mutex the OSMutex member
    /// outright (sizeof 0x1C) instead of taking a pointer to the derived
    /// class's. Both produce the same object layout; only this one is measured.
    Mutex(OSMutex *mutex) { OSInitMutex(mutex); }
    virtual ~Mutex() {}
};

} // namespace EGG

/// @brief An EGG::Mutex paired with the OS condition variable it is waited on.
/// @unofficial Reconstructed from dNandThread_c's constructor, which stores the
/// vtable at +0x00, passes +0x04 to OSInitMutex and +0x1C to OSInitCond.
class mMutex : public EGG::Mutex {
public:
    /// @note Only OSInitCond belongs here; OSInitMutex is the base's, see above.
    mMutex() : EGG::Mutex(&mOSMutex) { OSInitCond(&mOSCond); }
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

    /// @note `s32`, not `bool`: run() tests `save() == 2`, and createBanner()
    /// compares writeBanner()'s raw return against 0x72a0. Neither is a
    /// truth test, and CFront cannot encode a return type in the symbol.
    s32 save();
    bool createBanner();
    bool writeBanner(NANDFileInfo *fileInfo);

    bool cmdLoad();
    /// @note `s32` for the same reason as save(): run() tests its result
    /// against a value, not for truth.
    s32 load();
    bool checkCRC();

    bool cmdDeleteFile();
    /// @note `void`, not `bool`: declaring it bool costs four instructions the
    /// target does not have. Nothing in run() witnesses this either way, so the
    /// codegen is the only evidence there is.
    void deleteFile();

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

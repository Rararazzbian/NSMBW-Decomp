#pragma once

#include <types.h>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggThread.h>
#include <lib/revolution/NAND.h>
#include <lib/revolution/OS.h>

namespace EGG {

class Mutex {
public:
    Mutex() {}
    virtual ~Mutex() {}
};

} // namespace EGG

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

class dNandThread_c : public EGG::Thread {
public:
    dNandThread_c(int msgCount, EGG::Heap *heap);
    virtual ~dNandThread_c();
    virtual void *run();

    bool cmdExistCheck();
    bool existCheck();

    bool cmdSpaceCheck();
    bool spaceCheck();

    s32 save();
    bool createBanner();
    s32 writeBanner(NANDFileInfo *fileInfo);

    bool cmdLoad();
    s32 load();
    bool checkCRC();

    bool cmdDeleteFile();
    // SHADOW EXPERIMENT (batch4): real header has `bool deleteFile()`. Proven
    // empirically: the `bool` version compiles 4 instructions longer with an
    // extra branch+materialisation the target does not have; the `void`
    // version is byte-exact (27/27 instructions, matching size and order).
    void deleteFile();

    void setNandError(long err);
    void *getSaveData();

    static void create(EGG::Heap *heap);

    u8 mPad4C[4];      ///< [0x4C]
    mMutex mMutex;     ///< [0x50] size 0x24
    int mState;        ///< [0x74]
    int mError;        ///< [0x78]
    bool mFileExists;  ///< [0x7C]

    static dNandThread_c *m_instance;
};

STATIC_ASSERT(sizeof(EGG::Mutex) == 0x4);
STATIC_ASSERT(sizeof(mMutex) == 0x24);
STATIC_ASSERT(sizeof(dNandThread_c) == 0x80);

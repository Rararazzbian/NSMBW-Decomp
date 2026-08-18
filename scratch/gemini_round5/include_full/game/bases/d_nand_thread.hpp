#pragma once

#include <types.h>
#include <revolution/OS.h>
#include <revolution/NAND.h>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggThread.h>

// Forward declarations & primitives

namespace EGG {

/**
 * @brief Thread mutex synchronization primitive.
 * @unofficial
 */
class Mutex {
public:
    Mutex() {}
    virtual ~Mutex() {}
};

} // namespace EGG

/**
 * @brief Game-level OS mutex wrapper.
 * @unofficial
 */
class mMutex : public EGG::Mutex {
public:
    mMutex() {}
    virtual ~mMutex() {}

    OSMutex mOSMutex;       ///< 0x04..0x1B: Embedded OS mutex (size 0x18)
    OSCond mOSCond;         ///< 0x1C..0x23: Condition variable (size 0x08)
};

/**
 * @brief Dedicated background thread for asynchronous NAND flash filesystem operations.
 */
class dNandThread_c : public EGG::Thread {
public:
    enum Status_e {
        STATUS_IDLE = 0,
        STATUS_BUSY = 1,
        STATUS_ERROR = 2
    };

    enum Command_e {
        CMD_NONE = 0,
        CMD_EXIST_CHECK = 1,
        CMD_SPACE_CHECK = 2,
        CMD_LOAD = 3,
        CMD_SAVE = 4,
        CMD_DELETE_FILE = 5
    };

    dNandThread_c(int msgCount, EGG::Heap *heap);
    virtual ~dNandThread_c();

    virtual void *run();

    void cmdExistCheck();
    bool existCheck();

    void cmdSpaceCheck();
    bool spaceCheck();

    bool cmdSave(const void *saveData);
    bool save();

    bool createBanner();
    bool writeBanner(NANDFileInfo *fileInfo);

    void cmdLoad();
    bool load();

    bool checkCRC();

    void cmdDeleteFile();
    bool deleteFile();

    void setNandError(s32 err);
    void *getSaveData();

    static void create(EGG::Heap *heap);

    // Layout
    u8 mPad_4c[0x4];        ///< 0x4C: Padding / unmeasured region
    mMutex mMutex;          ///< 0x50: Synchronization mutex & condition variable (size 0x24)
    int mCommand;           ///< 0x74: Active NAND command ID
    int mStatus;            ///< 0x78: Execution status / error code
    bool mFileExists;       ///< 0x7C: Flag indicating file presence
    u8 mPad_7d[0x3];        ///< 0x7D: 4-byte struct alignment padding

    static dNandThread_c *m_instance;
};

STATIC_ASSERT(sizeof(EGG::Mutex) == 0x4);
STATIC_ASSERT(sizeof(mMutex) == 0x24);
STATIC_ASSERT(sizeof(EGG::Thread) == 0x4C);
STATIC_ASSERT(sizeof(dNandThread_c) == 0x80);
STATIC_ASSERT(offsetof(dNandThread_c, mMutex) == 0x50);
STATIC_ASSERT(offsetof(dNandThread_c, mCommand) == 0x74);
STATIC_ASSERT(offsetof(dNandThread_c, mStatus) == 0x78);
STATIC_ASSERT(offsetof(dNandThread_c, mFileExists) == 0x7C);

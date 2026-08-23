
#include <types.h>

struct PSwData_s {
    u32 mFlags;
    int mTimer[3];
};

struct dBgParameter_c {
    u8 pad[0x84];
    PSwData_s mPSwData;
    static dBgParameter_c *ms_Instance_p;
};

dBgParameter_c *dBgParameter_c::ms_Instance_p;

struct dPSwManager_c {
    virtual ~dPSwManager_c();
    PSwData_s mData;

    void initialize();
    void finalize();
};

void dPSwManager_c::initialize() {
    mData = dBgParameter_c::ms_Instance_p->mPSwData;
}

void dPSwManager_c::finalize() {
    dBgParameter_c::ms_Instance_p->mPSwData = mData;
}

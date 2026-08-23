#pragma once

#include <types.h>

class dPSwManager_c {
public:
    enum SwType_e {
        SW_TYPE_0 = 0,
        SW_TYPE_1 = 1,
        SW_TYPE_2 = 2,
    };

    struct PSwData_s {
        u32 mSwitchFlags;
        int mTimer[3];
    };

    dPSwManager_c();
    virtual ~dPSwManager_c();

    void initialize();
    void execute();
    void ProcMain();
    void finalize();
    u32 checkSwitch(SwType_e type);
    bool checkMove();
    int getTimer(SwType_e type);
    void onSwitch(SwType_e type, int timer);
    void offSwitch(SwType_e type);
    void setTimer(SwType_e type, int timer);

    static dPSwManager_c *ms_instance;

public:
    PSwData_s mData;
};

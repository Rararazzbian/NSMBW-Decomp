#pragma once
#include <game/bases/d_actor_state.hpp>

class daBossDemo_c : public dActorState_c {
public:
    STATE_VIRTUAL_FUNC_DECLARE(daBossDemo_c, Ready);
    STATE_VIRTUAL_FUNC_DECLARE(daBossDemo_c, BattleStDemo);
    STATE_VIRTUAL_FUNC_DECLARE(daBossDemo_c, BattleIn);
    STATE_VIRTUAL_FUNC_DECLARE(daBossDemo_c, BattleEdDemo);

    virtual void abandonRetryAfterOtehonClear();
    virtual void retryAfterOtehonClear();
    virtual void startBGM();
    virtual void stopBGM();
    virtual fBaseID_e getBossID();
    virtual void setBossID(fBaseID_e);
    virtual bool checkBattleStDemo();
    virtual bool checkBattleEdDemo();
    virtual void demoScroll();
    virtual void bossSearch();
    virtual void initialize();
};

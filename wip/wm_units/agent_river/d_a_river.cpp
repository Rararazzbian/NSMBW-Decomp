#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_actor_state.hpp>

// @unofficial The RIVER_* family: nine profiles, one translation unit.
// Each is an empty class deriving directly from dActorState_c -- sizeof
// (0x3D0) and base ctor (__ct__13dActorState_cFv) confirmed IDENTICAL
// across all nine via direct disassembly.

class daRiverBarrel_c : public dActorState_c {
public:
};
ACTOR_PROFILE(RIVER_BARREL, daRiverBarrel_c, 0);

class daRiverCoin_c : public dActorState_c {
public:
};
ACTOR_PROFILE(RIVER_COIN, daRiverCoin_c, 0);

class daRiverItem_c : public dActorState_c {
public:
};
ACTOR_PROFILE(RIVER_ITEM, daRiverItem_c, 0);

class daRiverLift_c : public dActorState_c {
public:
};
ACTOR_PROFILE(RIVER_LIFT, daRiverLift_c, 0);

// @unofficial RIVER_MGR is the one outlier in the family -- its .text
// span is double the other eight (0x120 vs 0xB0), because it carries two
// REAL overrides of its own (confirmed against target: fn_2_12B070,
// 0x24 bytes, `bl deleteRequest__7fBase_cFv; ...; li r3,0x0` -- an
// int-returning, no-arg override matching fBase_c::doDelete()'s
// signature exactly; and fn_2_12B0D0, 0x8 bytes, `li r3,0x1; blr` --
// a second int-returning override, matching fBase_c::preDelete()'s
// signature, unconditionally reporting ready). Both are ordinary
// out-of-line (global) overrides; their surrounding shared weak stubs
// (isSpinLiftUpEnable/vf68/finalUpdate) are unrelated inherited
// defaults that happen to flush into this same gap.
class daRiverMgr_c : public dActorState_c {
public:
    virtual int doDelete();
    virtual int preDelete();
};
ACTOR_PROFILE(RIVER_MGR, daRiverMgr_c, 0);
int daRiverMgr_c::doDelete() {
    deleteRequest();
    return 0;
}
int daRiverMgr_c::preDelete() {
    return 1;
}

class daRiverPaipo_c : public dActorState_c {
public:
    virtual ~daRiverPaipo_c();
};
ACTOR_PROFILE(RIVER_PAIPO, daRiverPaipo_c, 0);
daRiverPaipo_c::~daRiverPaipo_c() {}

class daRiverPakkun_c : public dActorState_c {
public:
    virtual ~daRiverPakkun_c();
};
ACTOR_PROFILE(RIVER_PAKKUN, daRiverPakkun_c, 0);
daRiverPakkun_c::~daRiverPakkun_c() {}

class daRiverPuku_c : public dActorState_c {
public:
    virtual ~daRiverPuku_c();
};
ACTOR_PROFILE(RIVER_PUKU, daRiverPuku_c, 0);
daRiverPuku_c::~daRiverPuku_c() {}

class daRiverStarcoin_c : public dActorState_c {
public:
    virtual ~daRiverStarcoin_c();
};
ACTOR_PROFILE(RIVER_STARCOIN, daRiverStarcoin_c, 0);
daRiverStarcoin_c::~daRiverStarcoin_c() {}

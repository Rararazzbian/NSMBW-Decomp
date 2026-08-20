#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_actor_state.hpp>

// EMPIRICAL PROBE 2 -- does classInit alone (a TU that declares but does NOT
// define create()/execute()/dtor overrides) compile and, more importantly,
// what symbol does the implicit default ctor's vtable patch reference?
class daProbe2_c : public dActorState_c {
public:
    virtual int create();
    virtual int execute();
    virtual ~daProbe2_c();
};

static void *classInit_AC_FLAGON() { return new daProbe2_c(); }
static void *classInit_AC_4SWICHAND() { return new daProbe2_c(); }
static void *classInit_AC_4SWICHOR() { return new daProbe2_c(); }
static void *classInit_AC_RANDSWICH() { return new daProbe2_c(); }
static void *classInit_AC_CHNGESWICH() { return new daProbe2_c(); }
static void *classInit_AC_IFSWICH() { return new daProbe2_c(); }
static void *classInit_AC_RNSWICH() { return new daProbe2_c(); }

fProfile::fActorProfile_c g_profile_AC_FLAGON = { &classInit_AC_FLAGON, fProfile::AC_FLAGON, fProfile::DRAW_ORDER::AC_FLAGON, 0 };
fProfile::fActorProfile_c g_profile_AC_4SWICHAND = { &classInit_AC_4SWICHAND, fProfile::AC_4SWICHAND, fProfile::DRAW_ORDER::AC_4SWICHAND, 0 };
fProfile::fActorProfile_c g_profile_AC_4SWICHOR = { &classInit_AC_4SWICHOR, fProfile::AC_4SWICHOR, fProfile::DRAW_ORDER::AC_4SWICHOR, 0 };
fProfile::fActorProfile_c g_profile_AC_RANDSWICH = { &classInit_AC_RANDSWICH, fProfile::AC_RANDSWICH, fProfile::DRAW_ORDER::AC_RANDSWICH, 0 };
fProfile::fActorProfile_c g_profile_AC_CHNGESWICH = { &classInit_AC_CHNGESWICH, fProfile::AC_CHNGESWICH, fProfile::DRAW_ORDER::AC_CHNGESWICH, 0 };
fProfile::fActorProfile_c g_profile_AC_IFSWICH = { &classInit_AC_IFSWICH, fProfile::AC_IFSWICH, fProfile::DRAW_ORDER::AC_IFSWICH, 0 };
fProfile::fActorProfile_c g_profile_AC_RNSWICH = { &classInit_AC_RNSWICH, fProfile::AC_RNSWICH, fProfile::DRAW_ORDER::AC_RNSWICH, 0 };

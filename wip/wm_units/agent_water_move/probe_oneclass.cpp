#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_actor_state.hpp>

// EMPIRICAL PROBE: does ONE class shared by AC_WATER_MOVE + AC_WATER_MOVE_REGULAR classInit
// produce ONE vtable reference (matching target's single lbl_2_data_421C0)?
class daWmProbe_c : public dActorState_c {
public:
};

static void *classInit_AC_WATER_MOVE() { return new daWmProbe_c(); }
static void *classInit_AC_WATER_MOVE_REGULAR() { return new daWmProbe_c(); }

fProfile::fActorProfile_c g_profile_AC_WATER_MOVE_probe = { &classInit_AC_WATER_MOVE, fProfile::AC_WATER_MOVE, fProfile::DRAW_ORDER::AC_WATER_MOVE, 0 };
fProfile::fActorProfile_c g_profile_AC_WATER_MOVE_REGULAR_probe = { &classInit_AC_WATER_MOVE_REGULAR, fProfile::AC_WATER_MOVE_REGULAR, fProfile::DRAW_ORDER::AC_WATER_MOVE_REGULAR, 0 };

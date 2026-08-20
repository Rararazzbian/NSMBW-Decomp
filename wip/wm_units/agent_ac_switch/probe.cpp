#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_actor_state.hpp>

// EMPIRICAL PROBE -- does NOT resemble the real daFlagObj_c. Minimal trivial
// class, no overrides, to see whether MWCC emits ONE vtable/local object
// when the SAME class name is used across seven ACTOR_PROFILE invocations,
// mirroring the sibling unit's compile-and-compare method for settling
// one-class-vs-many.
class daProbe_c : public dActorState_c {
public:
};

ACTOR_PROFILE(AC_FLAGON, daProbe_c, 0);
ACTOR_PROFILE(AC_4SWICHAND, daProbe_c, 0);
ACTOR_PROFILE(AC_4SWICHOR, daProbe_c, 0);
ACTOR_PROFILE(AC_RANDSWICH, daProbe_c, 0);
ACTOR_PROFILE(AC_CHNGESWICH, daProbe_c, 0);
ACTOR_PROFILE(AC_IFSWICH, daProbe_c, 0);
ACTOR_PROFILE(AC_RNSWICH, daProbe_c, 0);

#include <types.h>
#include <game/bases/d_base_actor.hpp>
#include <game/bases/d_a_player_base.hpp>

u32 g_mPosOffs;
u32 g_szActor;

void probe(dBaseActor_c *a) {
    g_mPosOffs = (u32) & a->mPos - (u32)a;
    g_szActor = sizeof(dBaseActor_c);
}

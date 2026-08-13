#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_actor.hpp>
#include <game/framework/f_manager.hpp>

typedef s8 &(*GetPlrNoFn)(dActor_c *);

static inline GetPlrNoFn get_vfunc_6c(fBase_c *base) {
    return (*(GetPlrNoFn **)((u8 *)base + 0x60))[0x6c / 4];
}

daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    fBase_c *base;
    for (int i = 0; i < 4; i++) {
        base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            if (get_vfunc_6c(base)((dActor_c *)base) == plrNo) {
                return (daYoshi_c *)base;
            }
        }
    }
    return nullptr;
}

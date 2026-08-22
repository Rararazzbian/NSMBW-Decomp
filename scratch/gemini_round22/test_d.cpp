
#include <game/bases/d_enemy.hpp>

// test 1: non-const dDeathInfo_c array
dDeathInfo_c g_deathInfo[4];

// test 2: initialized non-const
dDeathInfo_c g_deathInfoInit[4] = {
    { }, { }, { }, { }
};

int foo() {
    return g_deathInfo[0].mScore + g_deathInfoInit[0].mScore;
}

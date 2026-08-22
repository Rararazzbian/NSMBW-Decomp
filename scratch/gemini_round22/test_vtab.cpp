
#include <game/bases/d_enemy_toride_kokoopa.hpp>

void test(dEnTorideKokoopa_c *kokoopa) {
    kokoopa->mStateMgr.getStateID()->isEqual(dEnTorideKokoopa_c::StateID_FumiHit);
    *kokoopa->mStateMgr.getStateID() == dEnTorideKokoopa_c::StateID_FumiHit;
    *kokoopa->mStateMgr.getStateID() != dEnTorideKokoopa_c::StateID_FumiHit;
}

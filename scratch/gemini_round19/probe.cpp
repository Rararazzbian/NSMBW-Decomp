
#include <game/bases/d_enemy_toride_kokoopa.hpp>

void testA(dEnTorideKokoopa_c *p) {
    dActor_c *blitz;
    if (p->mUnk770 != 0) {
        blitz = (dActor_c*)fManager_c::searchBaseByID((fBaseID_e)p->mUnk770);
    } else {
        blitz = 0;
    }
    p->blitzMove(blitz);
}
void testB(dEnTorideKokoopa_c *p) {
    dActor_c *blitz = 0;
    if (p->mUnk770 != 0) {
        blitz = (dActor_c*)fManager_c::searchBaseByID((fBaseID_e)p->mUnk770);
    }
    p->blitzMove(blitz);
}
void testC(dEnTorideKokoopa_c *p) {
    if (p->mUnk770 != 0) {
        p->blitzMove((dActor_c*)fManager_c::searchBaseByID((fBaseID_e)p->mUnk770));
    } else {
        p->blitzMove(0);
    }
}

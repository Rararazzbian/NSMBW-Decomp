
#include <game/bases/d_enemy_toride_kokoopa.hpp>
static const float l_bounceSpeed[4] = { 0.0f, 1.5f, 2.75f, 4.0f };

void test1(dEnTorideKokoopa_c *p) {
    if (--p->mUnkAA0 > 0) {
        p->mSpeed.y = l_bounceSpeed[p->mUnkAA0];
    } else {
        p->mSpeed.y = 0.0f;
        p->changeState(dEnTorideKokoopa_c::StateID_ShellAtk);
    }
}
void test2(dEnTorideKokoopa_c *p) {
    if (p->mUnkAA0-- > 1) {
        p->mSpeed.y = l_bounceSpeed[p->mUnkAA0];
    } else {
        p->mSpeed.y = 0.0f;
        p->changeState(dEnTorideKokoopa_c::StateID_ShellAtk);
    }
}
void test3(dEnTorideKokoopa_c *p) {
    p->mUnkAA0--;
    if (p->mUnkAA0 > 0) {
        p->mSpeed.y = l_bounceSpeed[p->mUnkAA0];
    } else {
        p->mSpeed.y = 0.0f;
        p->changeState(dEnTorideKokoopa_c::StateID_ShellAtk);
    }
}

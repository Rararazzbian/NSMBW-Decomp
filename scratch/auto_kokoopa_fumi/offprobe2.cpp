#include <stddef.h>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_a_player_base.hpp>

unsigned long a1 = offsetof(daPlBase_c, mNowBgCross1);
unsigned long a2 = offsetof(daPlBase_c, mNowBgCross2);
unsigned long a3 = offsetof(daPlBase_c, mBc);
unsigned long a4 = offsetof(daPlBase_c, mPos);
unsigned long a5 = offsetof(daPlBase_c, mSpeed);

int slotOfGetPlrNo(daPlBase_c *p) { return p->getPlrNo(); }

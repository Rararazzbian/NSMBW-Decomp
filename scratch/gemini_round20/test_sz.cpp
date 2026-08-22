
#include <game/bases/d_enemy.hpp>
#include <stddef.h>
#define P(m) int get_##m() { return offsetof(dEn_c, m); }
P(mFlags)
P(mTimer1)
P(mTimer2)
P(mNoHitPlayer)
P(mTimer3)
P(mCombo)
P(mFumiProc)

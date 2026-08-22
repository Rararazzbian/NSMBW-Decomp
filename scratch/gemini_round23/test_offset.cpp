
#include <game/bases/d_enemy_toride_kokoopa.hpp>
#include <stddef.h>
#define CHECK(m) char check_##m[offsetof(dEnTorideKokoopa_c, m) == 0x360 ? 1 : -1];
CHECK(mFlags)

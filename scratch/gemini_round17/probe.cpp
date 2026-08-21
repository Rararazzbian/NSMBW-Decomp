#include <game/bases/d_enemy_toride_kokoopa.hpp>
#include <cstddef>

char off_speed_max[offsetof(dEnTorideKokoopa_c, mSpeedMax) == 0x110 ? 1 : -1];
char off_speed_max_y[offsetof(dEnTorideKokoopa_c, mSpeedMax.y) == 0x114 ? 1 : -1];

#include <stddef.h>
#include <game/bases/d_info.hpp>

static_assert(sizeof(dCyuukan_c) == 0x34, "dCyuukan_c size probe");
static_assert(sizeof(dInfo_c) == 0xb5c, "dInfo_c size probe");

char dInfoSizeProbe[sizeof(dInfo_c)];

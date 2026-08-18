#include "game/bases/d_info.hpp"

static_assert(sizeof(dInfo_c) == 0xb5c, "size wrong");

// Test that the shadow header is being used (only in shadow, not original)
struct _shadow_tag_used {};

unsigned long probe_before() { return sizeof(((dInfo_c *)0)->pad_before_mEffectStopOverride); }
unsigned long probe_size() { return (unsigned long)sizeof(dInfo_c); }

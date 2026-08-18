#include <game/bases/d_info.hpp>

unsigned long probe_override() { return (unsigned long)&((dInfo_c *)0)->mEffectStopOverride; }
unsigned long probe_size() { return (unsigned long)sizeof(dInfo_c); }

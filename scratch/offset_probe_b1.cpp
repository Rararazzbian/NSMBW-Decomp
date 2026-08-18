#include <game/bases/d_a_player_base.hpp>

unsigned long probe_press() { return (unsigned long)&((daPlBase_c *)0)->mPressAttachPos; }
unsigned long probe_wind_ground() { return (unsigned long)&((daPlBase_c *)0)->mWindGroundTimer; }
unsigned long probe_wind_speed() { return (unsigned long)&((daPlBase_c *)0)->mWindSpeed; }
unsigned long probe_final() { return (unsigned long)&((daPlBase_c *)0)->mFinalAirPushForceX; }
unsigned long probe_1134() { return (unsigned long)&((daPlBase_c *)0)->m_1134; }
unsigned long probe_player_type() { return (unsigned long)&((daPlBase_c *)0)->mPlayerType; }
unsigned long probe_size() { return (unsigned long)sizeof(daPlBase_c); }

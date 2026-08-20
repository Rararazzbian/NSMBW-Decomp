#include <game/bases/d_line_mng.hpp>

template <int N> struct Probe;
template <> struct Probe<0xEC> {};
static Probe<sizeof(dLineMng_c)> check;

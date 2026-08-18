#include "d_py_effect_real.hpp"

int main() {
    return sizeof(dPyEffect_c) == 0x13C && sizeof(dEf::followEffect_c) == 0x114 ? 0 : 1;
}

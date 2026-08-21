
#include <game/bases/d_enemy_toride_kokoopa.hpp>

bool test_ne(const sStateIDIf_c *id, const sStateIDIf_c &other) {
    return *id != other;
}
bool test_call_ne(const sStateIDIf_c *id, const sStateIDIf_c &other) {
    return id->operator!=(other);
}
bool test_is_equal(const sStateIDIf_c *id, const sStateIDIf_c &other) {
    return id->isEqual(other);
}

#include <game/bases/d_tag_processor.hpp>

void TagProcessor_c::getOkCancellDisp(MsgRes_c *bmg) {
    MsgIDSet(bmg, 0, 0x20);
}

void TagProcessor_c::getRedBlock(MsgRes_c *bmg, void *arg) {
    MsgIDSet(bmg, 0, *static_cast<u8 *>(arg) != 0 ? 0x21 : 0x22);
}

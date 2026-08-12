#include <game/bases/d_tag_processor.hpp>

int TagProcessor_c::getOkCancellDisp(MsgRes_c *bmg) {
    return MsgIDSet(bmg, 0, 0x20);
}

int TagProcessor_c::getRedBlock(MsgRes_c *bmg, void *arg) {
    return MsgIDSet(bmg, 0, *static_cast<u8 *>(arg) != 0 ? 0x21 : 0x22);
}

#include <game/bases/d_message.hpp>

MsgRes_c::MsgRes_c(const void *msgArchive, EGG::Heap *) : EGG::MsgRes(msgArchive) {}

MsgRes_c::~MsgRes_c() {}

u16 MsgRes_c::getScale(ulong messageGroup, ulong messageID) {
    return getMsgEntry(messageGroup, messageID)->scale;
}

u8 MsgRes_c::getFont(ulong messageGroup, ulong messageID) {
    return getMsgEntry(messageGroup, messageID)->font;
}

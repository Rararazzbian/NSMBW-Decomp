#pragma once

#include <types.h>

namespace EGG {

class MsgRes {
public:
    /* 0x00 */ /// @brief Decoded archive fields, unrecovered. Size fixed by
               /// the zeroing of 0x0-0x18 and the base-vtable store at
               /// this+0x1C inside __ct__Q23EGG6MsgResFPCv, and by the
               /// 0x20-byte allocation in buildMsgRes__10dMessage_c.
               /// MWCC places the class's own vptr AFTER these bytes, at
               /// 0x1C. @unofficial
    u8 mFields_0x00[0x1C];

    /// @brief One decoded message-table entry.
    /// @details Only the fields read by MsgRes_c::getScale()/getFont() are
    /// identified (u16 at 0x4, u8 at 0x6). The rest is unrecovered. @unofficial
    struct MsgEntry {
        /* 0x00 */ u8 pad_0x00[4];
        /* 0x04 */ u16 scale;
        /* 0x06 */ u8 font;
    };

    /// @param msgArchive Pointer to the loaded BMG archive data.
    MsgRes(const void *msgArchive);
    virtual ~MsgRes();

    /// @brief @unofficial Return type observed as a pointer (call sites index
    /// through r3 immediately after the call).
    MsgEntry *getMsgEntry(ulong messageGroup, ulong messageID);

    wchar_t *getMsg(ulong messageGroup, ulong messageID);

    /// @brief Decodes the control code at @p str .
    static void analyzeTag(u16 tag, const wchar_t *str, u8 *tagLen, ulong *tagInfo, void **arg);
};

} // namespace EGG

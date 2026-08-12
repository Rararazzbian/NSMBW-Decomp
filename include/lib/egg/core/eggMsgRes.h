#pragma once

#include <types.h>

namespace EGG {

class MsgRes {
public:
    wchar_t *getMsg(ulong messageGroup, ulong messageID);

    /// @brief Decodes the control code at @p str .
    static void analyzeTag(u16 tag, const wchar_t *str, u8 *tagLen, ulong *tagInfo, void **arg);
};

} // namespace EGG

#pragma once

#include <types.h>
#include <egg/core/eggHeap.h>
#include <egg/core/eggMsgRes.h>

class MsgRes_c : public EGG::MsgRes {
public:
    u8 getFont(ulong messageGroup, ulong messageID);
    u16 getScale(ulong messageGroup, ulong messageID);
};

class dMessage_c {
public:
    static bool create(EGG::Heap *heap);
    static MsgRes_c *getMesRes();

    /// @brief Gets the text of the given message.
    static const wchar_t *getMsg(ulong messageGroup, ulong messageID);
};

/// @brief Writes @p value into @p dst as a wide string, zero-padded to @p digits .
/// @unofficial Name not recovered; the function is at 0x800CDD60.
extern "C" void fn_800CDD60(int value, wchar_t *dst, int bufLen, int digits, int flag);

/// @brief As fn_800CDD60, but space-padded. @unofficial
/// @unofficial Name not recovered; the function is at 0x800CDEA0.
extern "C" void fn_800CDEA0(int value, wchar_t *dst, int bufLen, int digits, int flag);

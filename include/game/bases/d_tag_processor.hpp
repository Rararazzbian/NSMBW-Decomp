#pragma once

#include <cstdarg>
#include <game/bases/d_message.hpp>
#include <nw4r/ut.h>

class TagProcessor_c : public nw4r::ut::WideTagProcessor {
public:
    TagProcessor_c();
    ~TagProcessor_c();

    u8 mPad[0xc0];
    u8 mFontIndex;

    void MsgIDSet(MsgRes_c *bmg, ulong messageGroup, ulong messageID);

    void getOkCancellDisp(MsgRes_c *bmg);
    void getRedBlock(MsgRes_c *bmg, void *arg);

    void preProcess(
        const wchar_t *text, wchar_t *buf,
        unsigned long bufLen, int *writeLen,
        long param, va_list *vargs,
        MsgRes_c *bmg
    );

    static bool isZeroWidthSpace; ///< @unofficial
};

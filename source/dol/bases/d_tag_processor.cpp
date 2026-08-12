#include <game/bases/d_tag_processor.hpp>

#include <cwchar>
#include <game/bases/d_font_manager.hpp>
#include <game/bases/d_info.hpp>
#include <game/mLib/m_video.hpp>
#include <lib/MSL/internal/mem.h>
#include <nw4r/lyt.h>
#include <nw4r/ut.h>
#include <revolution/GX.h>

// Defined before SetupGXCommon so that it precedes that function's guarded
// static in .sbss, matching the original's 0x8042A3A8 / 0x8042A3A9 order.
bool TagProcessor_c::isZeroWidthSpace;

/// @brief Maps the font index to the tag code that selects it. @unofficial
static const u16 cTagCode[4] = { 10, 11, 12, 14 };

const f32 TagProcessor_c::c_RUBY_SCALE = 0.6f;
const f32 TagProcessor_c::c_RUBY_WRITE_SCALE_Y = 0.75f;

TagProcessor_c::TagProcessor_c() {
    mScissor[0].mPaneSizeX = 0.0f;
    mScissor[0].mPaneSizeY = 0.0f;
    for (ScissorEntry_s *p = &mScissor[1]; p < &mScissor[4]; p++) {
        p->mPaneSizeX = 0.0f;
        p->mPaneSizeY = 0.0f;
    }
}

TagProcessor_c::~TagProcessor_c() {}

void TagProcessor_c::SetupGXCommon() {
    static nw4r::ut::Color fog = 0;

    GXSetFog(GX_FOG_NONE, fog, 0.0f, 0.0f, 0.0f, 0.0f);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumIndStages(0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
}

void TagProcessor_c::SetupGXTevSet() {
    nw4r::ut::Color color0(0, 0, 0, 0);
    nw4r::ut::Color color1(0, 0, 0, 0x80);

    GXSetNumTevStages(2);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColor(GX_TEVREG0, color0);
    GXSetTevColor(GX_TEVREG1, color1);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_A1, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

void TagProcessor_c::SetupVertexFormat() {
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0xf);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
}

void TagProcessor_c::FontChange(nw4r::ut::PrintContext<wchar_t> *ctx) {
    nw4r::ut::TextWriterBase<wchar_t> writer = *ctx->writer;

    nw4r::ut::Font *font = dFontMng_c::getFont(1);
    wchar_t type = ctx->str[0] >> 8;
    f32 size = writer.GetFont()->GetHeight() * writer.GetScaleV();
    u8 ch = ctx->str[0];

    writer.SetFont(*font);
    writer.ResetColorMapping();
    writer.SetFontSize(size);
    writer.SetCursorY(ctx->y);

    if (type == 2) {
        SetupGXCommon();
        SetupGXTevSet();
        SetupVertexFormat();
    } else {
        writer.SetupGX();
    }

    writer.CharWriter::Print(ch);
    ctx->str += 2;
    ctx->writer->SetCursorX(writer.GetCursorX());
    ctx->writer->SetupGX();
}

void TagProcessor_c::ScaleSet(nw4r::ut::PrintContext<wchar_t> *ctx) {
    ulong scale = *ctx->str;
    f32 value = (f32)scale / 100.0f * ctx->writer->GetScaleV();

    ctx->writer->SetScale(value, value);
    ctx->str++;
}

void TagProcessor_c::RuBySet(nw4r::ut::PrintContext<wchar_t> *ctx) {
    nw4r::ut::TextWriterBase<wchar_t> *writer = ctx->writer;
    f32 savedWidthLimit = writer->GetWidthLimit();
    writer->ResetWidthLimit();

    if (!(ctx->flags & nw4r::ut::PrintContext<wchar_t>::FLAGS_CHARSPACE)) {
        writer->MoveCursorX(writer->GetCharSpace());
    }

    const wchar_t *str = ctx->str;
    ulong lengths = *str;
    int rubyLen = (lengths >> 8) & 0xFF;
    u8 mainLen = lengths;
    const wchar_t *rubyStr = str + 1;
    const wchar_t *mainStr = rubyStr + rubyLen;

    f32 x = writer->GetCursorX();
    ulong savedDrawFlag = writer->GetDrawFlag();
    f32 y = writer->GetCursorY();

    nw4r::ut::TextWriterBase<wchar_t> rubyWriter = *writer;
    rubyWriter.SetDrawFlag(0x300);
    rubyWriter.SetCharSpace(0.0f);
    rubyWriter.SetLineSpace(0.0f);
    f32 rubyScale = c_RUBY_SCALE;
    rubyWriter.SetScale(rubyWriter.GetScaleV() * rubyScale, rubyWriter.GetScaleH() * rubyScale);

    f32 mainWidth = writer->CalcStringWidth(mainStr, mainLen);
    f32 diff = mainWidth - rubyWriter.CalcStringWidth(rubyStr, rubyLen);

    if (diff > 0.0f) {
        f32 space = diff / rubyLen;
        rubyWriter.SetCharSpace(space);
        x += space / 2.0f;
    }

    rubyWriter.SetCursor(
        x, y - c_RUBY_WRITE_SCALE_Y * (writer->GetFontAscent() + rubyWriter.GetFontDescent())
    );
    rubyWriter.Print(rubyStr, rubyLen);

    if (diff < 0.0f) {
        writer->MoveCursorX(-diff / 2.0f);
    }

    writer->SetDrawFlag(0x300);
    writer->Print(mainStr, mainLen);
    writer->SetDrawFlag(savedDrawFlag);

    if (diff < 0.0f) {
        writer->MoveCursorX(-diff / 2.0f);
    }

    ctx->str = mainStr + mainLen;
    writer->SetWidthLimit(savedWidthLimit);
}

void TagProcessor_c::setScissorStart(int idx) {
    u32 x, y, w, h;
    GXGetScissor(&x, &y, &w, &h);
    w = mVideo::m_video->mRenderModeObj.fbWidth;
    h = mVideo::m_video->mRenderModeObj.efbHeight;
    mScissor[idx].mSavedScissorX = x;
    mScissor[idx].mSavedScissorY = y;
    mScissor[idx].mSavedScissorWidth = w;
    mScissor[idx].mSavedScissorHeight = h;

    nw4r::lyt::DrawInfo *info = mScissor[idx].mpDrawInfo;
    nw4r::math::MTX34 mtx = mScissor[idx].mpPane->GetGlobalMtx();
    f32 posX = mtx._03 / info->GetLocationAdjustScale().x;
    f32 posY = -1.0f * mtx._13;
    posX *= mScissor[idx].mScreenScaleX;
    posY *= mScissor[idx].mScreenScaleY;
    f32 sizeX = mScissor[idx].mPaneSizeX * mScissor[idx].mScreenScaleX;
    f32 sizeY = mScissor[idx].mPaneSizeY * mScissor[idx].mScreenScaleY;

    w = (u32)(w / 2.0f);
    h = (u32)(h / 2.0f);

    GXSetScissor(
        (u32)((x + w) + (posX - 0.5f * sizeX)), (u32)((y + h) + (posY - 0.5f * sizeY)),
        (u32)sizeX, (u32)sizeY
    );
}

void TagProcessor_c::setScissorEnd(int idx) {
    GXSetCullMode(GX_CULL_NONE);
    GXSetScissor(
        mScissor[idx].mSavedScissorX, mScissor[idx].mSavedScissorY,
        mScissor[idx].mSavedScissorWidth, mScissor[idx].mSavedScissorHeight
    );
}

void TagProcessor_c::setScissorCursorX(nw4r::ut::PrintContext<wchar_t> *ctx) {
    f32 width = 0.0f;
    int i = 1;
    nw4r::ut::TextWriterBase<wchar_t> writer = *ctx->writer;
    f32 scaleX = ctx->writer->GetScaleH();

    u16 code = ctx->str[1];
    if (code == 2) {
        code = ctx->str[2];
        i = (code >> 8) + 2;
    }

    while (code != 0xF) {
        width += 0.5f * writer.GetFont()->GetCharWidth(code);
        i++;
        code = ctx->str[i];
    }

    width *= scaleX;
    width *= -1.0f;
    ctx->writer->SetCursorX(width);
}

void TagProcessor_c::setScissor(nw4r::ut::PrintContext<wchar_t> *ctx) {
    int code = *ctx->str;
    int idx = code / 16;
    if ((code & 0xF) == 2) {
        setScissorEnd(idx);
    } else {
        setScissorStart(idx);
        setScissorCursorX(ctx);
    }
    ctx->str++;
}

TagProcessor_c::Operation TagProcessor_c::Process(u16 tag, nw4r::ut::PrintContext<wchar_t> *ctx) {
    switch (tag) {
        case 0xB:
            FontChange(ctx);
            return OPERATION_NO_CHAR_SPACE;
        case 2:
            RuBySet(ctx);
            return OPERATION_CHAR_SPACE;
        case 1:
            ScaleSet(ctx);
            return OPERATION_DEFAULT;
        case 0xF:
            setScissor(ctx);
            return OPERATION_DEFAULT;
    }
    return nw4r::ut::WideTagProcessor::Process(tag, ctx);
}

void TagProcessor_c::PictureFontCalcRect(nw4r::ut::Rect *rect, nw4r::ut::PrintContext<wchar_t> *ctx) {
    nw4r::ut::Font *font = dFontMng_c::getFont(1);
    f32 charWidth = font->GetCharWidth((u8)*ctx->str);
    f32 fontSize = ctx->writer->GetFont()->GetHeight() * ctx->writer->GetScaleV();

    nw4r::ut::TextWriterBase<wchar_t> pictureWriter = *ctx->writer;
    pictureWriter.SetFont(*font);
    pictureWriter.SetFontSize(fontSize);

    f32 width = charWidth * pictureWriter.GetScaleH();
    if (isZeroWidthSpace) {
        width = 0.0f;
    }
    ctx->writer->MoveCursorX(width);

    f32 x = ctx->writer->GetCursorX();
    rect->left = nw4r::ut::Min(rect->left, x);
    rect->right = nw4r::ut::Max(rect->right, x);

    ctx->str += 2;
}

void TagProcessor_c::ScaleCalcRect(nw4r::ut::PrintContext<wchar_t> *ctx) {
    ulong scale = *ctx->str;
    f32 value = (f32)scale / 100.0f;

    ctx->writer->GetFontHeight();
    ctx->writer->GetFontDescent();

    value *= ctx->writer->GetScaleV();
    ctx->writer->SetScale(value, value);

    ctx->writer->GetFontDescent();
    ctx->str++;
}

void TagProcessor_c::RuByCalcRect(nw4r::ut::Rect *rect, nw4r::ut::PrintContext<wchar_t> *ctx) {
    nw4r::ut::TextWriterBase<wchar_t> *writer = ctx->writer;

    const wchar_t *str = ctx->str;
    ulong lengths = *str;
    int rubyLen = (lengths >> 8) & 0xFF;
    u8 mainLen = lengths;
    const wchar_t *rubyStr = str + 1;
    const wchar_t *mainStr = rubyStr + rubyLen;

    nw4r::ut::TextWriterBase<wchar_t> rubyWriter = *writer;
    rubyWriter.SetDrawFlag(0);
    rubyWriter.SetCharSpace(0.0f);
    f32 rubyScale = c_RUBY_SCALE;
    rubyWriter.SetScale(rubyWriter.GetScaleV() * rubyScale, rubyWriter.GetScaleH() * rubyScale);
    rubyWriter.ResetWidthLimit();

    f32 savedWidthLimit = writer->GetWidthLimit();
    writer->ResetWidthLimit();

    bool addCharSpace = (ctx->flags & nw4r::ut::PrintContext<wchar_t>::FLAGS_CHARSPACE) == 0;

    f32 mainWidth = writer->CalcStringWidth(mainStr, mainLen);
    f32 rubyWidth = rubyWriter.CalcStringWidth(rubyStr, rubyLen);
    f32 width = nw4r::ut::Max(mainWidth, rubyWidth);

    f32 space = addCharSpace ? writer->GetCharSpace() : 0.0f;
    writer->SetWidthLimit(savedWidthLimit);
    width += space;

    writer->MoveCursorX(width);
    rect->right = rect->left + width;
    rect->bottom = rect->top + writer->GetFontHeight();

    ctx->str = mainStr + mainLen;
}

TagProcessor_c::Operation TagProcessor_c::CalcRect(
    nw4r::ut::Rect *rect, u16 tag, nw4r::ut::PrintContext<wchar_t> *ctx
) {
    switch (tag) {
        case 2:
            RuByCalcRect(rect, ctx);
            return OPERATION_CHAR_SPACE;
        case 1:
            ScaleCalcRect(ctx);
            return OPERATION_DEFAULT;
        case 0xB:
            PictureFontCalcRect(rect, ctx);
            return OPERATION_NO_CHAR_SPACE;
        case 0xF:
            return OPERATION_DEFAULT;
    }
    return nw4r::ut::WideTagProcessor::CalcRect(rect, tag, ctx);
}

int TagProcessor_c::PlaceCheck(int value) {
    if (value >= 1000) {
        return 4;
    }
    if (value >= 100) {
        return 3;
    }
    return (value >= 10) + 1;
}

void TagProcessor_c::TextBufClear() {
    for (int i = 0; i < 16; i++) {
        mTextBuf[i] = 0;
    }
}

int TagProcessor_c::MsgIDSet(MsgRes_c *bmg, ulong messageGroup, ulong messageID) {
    const wchar_t *msg = dMessage_c::getMsg(messageGroup, messageID);
    int len = wcslen(msg);

    u8 font;
    if (bmg == nullptr) {
        font = mFontIndex;
    } else {
        font = bmg->getFont(messageGroup, messageID);
    }

    TextBufClear();
    mTextBuf[0] = cTagCode[font];
    memcpy(&mTextBuf[1], msg, len * 2);

    if (dInfo_c::m_instance->m_3da != 0) {
        mTextBuf[1] += 0x200;
    } else {
        mTextBuf[1] += 0x100;
    }

    return len + 2;
}

int TagProcessor_c::getWorldNum(void *arg) {
    int world = dInfo_c::m_instance->mDisplayCourseWorld;
    TextBufClear();

    u8 type = *(u8 *)arg;
    if (type == 0) {
        fn_800CDD60(world, mTextBuf, 16, 1, 0);
    } else if (type == 1) {
        fn_800CDD60(world - 1, mTextBuf, 16, 1, 0);
    } else if (type == 2) {
        fn_800CDD60(world + 1, mTextBuf, 16, 1, 1);
    }

    return wcslen(mTextBuf);
}

int TagProcessor_c::getCourseNum() {
    int courseNum = dInfo_c::m_instance->mDisplayCourseNum;
    fn_800CDD60(courseNum, mTextBuf, 16, PlaceCheck(courseNum), 1);
    return wcslen(mTextBuf);
}

int TagProcessor_c::getOkCancellDisp(MsgRes_c *bmg, void *arg) {
    ulong id;
    u8 kind = *static_cast<u8 *>(arg);
    if (kind == 0) {
        id = 1;
    } else if (kind == 1) {
        id = 3;
    }
    return MsgIDSet(bmg, 0, id);
}

int TagProcessor_c::getTotalCollectionCoin() {
    fn_800CDD60(dInfo_c::m_instance->mTotalCollectionCoin, mTextBuf, 16, 3, 1);
    return wcslen(mTextBuf);
}

int TagProcessor_c::getCrossKeyDisp(MsgRes_c *bmg) {
    return MsgIDSet(bmg, 0, 5);
}

int TagProcessor_c::getCourseSelectIcon(MsgRes_c *bmg, void *arg) {
    ulong messageID;

    switch (*(u8 *)arg) {
        case 0:  messageID = 14; break;
        case 1:  messageID = 15; break;
        case 2:  messageID = 16; break;
        case 3:  messageID = 17; break;
        case 4:  messageID = 18; break;
        case 5:  messageID = 19; break;
        case 6:  messageID = 20; break;
        case 7:  messageID = 21; break;
        case 8:  messageID = 22; break;
        case 9:  messageID = 23; break;
        case 10: messageID = 25; break;
        case 11: messageID = 24; break;
        case 12: messageID = 29; break;
        case 13: messageID = 30; break;
        case 14: messageID = 31; break;
    }

    return MsgIDSet(bmg, 0, messageID);
}

int TagProcessor_c::getSaveFileNumber() {
    int fileNum = dInfo_c::m_instance->mSaveFileNumber;
    fn_800CDD60(fileNum, mTextBuf, 16, PlaceCheck(fileNum), 1);
    return wcslen(mTextBuf);
}

int TagProcessor_c::getCourseSelectButtonFunction(MsgRes_c *bmg, void *arg) {
    ulong messageID;

    u8 type = *(u8 *)arg;
    dInfo_c *info = dInfo_c::m_instance;

    if (type == 0) {
        if (info->mExtensionAttached) {
            messageID = 10;
        } else {
            messageID = 2;
        }
    } else if (type == 1) {
        messageID = 8;
    } else if (type == 2) {
        messageID = 7;
    } else if (type == 3) {
        messageID = 3 + (info->mExtensionAttached != 0);
    }

    return MsgIDSet(bmg, 0, messageID);
}

int TagProcessor_c::setSize(void *arg) {
    TextBufClear();
    mTextBuf[1] = ((u8 *)arg)[1];
    mTextBuf[0] = 1;
    return wcslen(mTextBuf);
}

int TagProcessor_c::setRuBi(void *arg) {
    TextBufClear();

    const u8 *p = (const u8 *)arg;
    u8 code = *p++;
    u32 len = 0;
    while (*p != 0) {
        p++;
        len++;
    }

    mTextBuf[0] = 2;
    mTextBuf[1] = ((len << 7) & 0xff00) | code;
    memcpy(&mTextBuf[2], (const u8 *)arg + 1, len);

    return wcslen(mTextBuf);
}

int TagProcessor_c::getMenuButton(MsgRes_c *bmg, void *arg) {
    ulong id = 0;
    u8 kind = *static_cast<u8 *>(arg);
    if (kind == 0) {
        id = 6;
    } else if (kind == 1) {
        id = 7;
    } else if (kind == 2) {
        id = 8;
    } else if (kind == 3) {
        id = 2;
    } else if (kind == 4) {
        id = 4;
    } else if (kind == 5) {
        id = 0xB;
    }
    return MsgIDSet(bmg, 0, id);
}

int TagProcessor_c::getScissor(void *arg) {
    dInfo_c *info = dInfo_c::m_instance;
    nw4r::lyt::DrawInfo *drawInfo = info->mScissorDrawInfo;
    nw4r::lyt::Pane *pane = info->mScissorPane;
    u16 idx = info->mScissorIndex;

    mScissor[idx].mpPane = pane;
    mScissor[idx].mPaneSizeX = pane->GetSize().width;
    mScissor[idx].mPaneSizeY = pane->GetSize().height;
    mScissor[idx].mpDrawInfo = drawInfo;

    nw4r::ut::Rect rect = drawInfo->GetViewRect();
    f32 scaleX = rect.GetWidth() / drawInfo->GetLocationAdjustScale().x;
    f32 scaleY = rect.GetHeight() / drawInfo->GetLocationAdjustScale().y;

    if (scaleX < 0.0f) {
        scaleX *= -1.0f;
    }
    if (scaleY < 0.0f) {
        scaleY *= -1.0f;
    }

    mScissor[idx].mScreenScaleX = mVideo::m_video->getWidth() / scaleX;
    mScissor[idx].mScreenScaleY = mVideo::m_video->getHeight() / scaleY;

    u16 tagChar = idx << 4;
    tagChar |= (u16)(*(u8 *)arg + 1);

    TextBufClear();
    mTextBuf[0] = 0xF;
    mTextBuf[1] = tagChar;
    return wcslen(mTextBuf);
}

int TagProcessor_c::getEasyPairing(MsgRes_c *bmg, void *arg) {
    return MsgIDSet(bmg, 0, *static_cast<u8 *>(arg) != 0 ? 1 : 3);
}

int TagProcessor_c::getPlayerNum() {
    fn_800CDEA0(dInfo_c::m_instance->mPlayerNum, mTextBuf, 16, 1, 0);
    return wcslen(mTextBuf);
}

int TagProcessor_c::getDebugDisp() {
    dInfo_c *info = dInfo_c::m_instance;

    fn_800CDD60(info->mTextBoxMessageGroup, mTextBuf, 16, 4, 0);
    mTextBuf[4] = L'_';
    fn_800CDD60(info->mTextBoxMessageID, &mTextBuf[5], 12, 4, 0);

    return wcslen(mTextBuf);
}

int TagProcessor_c::getPlayNumber() {
    int playNumber = dInfo_c::m_instance->mPlayNumber;
    TextBufClear();
    fn_800CDEA0(playNumber, mTextBuf, 16, 1, 0);
    return wcslen(mTextBuf);
}

int TagProcessor_c::getOkCancellDisp(MsgRes_c *bmg) {
    return MsgIDSet(bmg, 0, 0x20);
}

int TagProcessor_c::getRedBlock(MsgRes_c *bmg, void *arg) {
    return MsgIDSet(bmg, 0, *static_cast<u8 *>(arg) != 0 ? 0x21 : 0x22);
}

void TagProcessor_c::preProcess(
    const wchar_t *text, wchar_t *buf,
    unsigned long bufLen, int *writeLen,
    long param, va_list vargs,
    MsgRes_c *bmg
) {
    const wchar_t *p = text;
    wchar_t *cursor = buf;
    int count = 0;

    for (;;) {
        wchar_t c = *p;
        if (c == 0) {
            *cursor = 0;
            break;
        }

        if (c == 0x1A) {
            u8 tagLen = 0;
            ulong tagInfo = 0;
            void *arg = nullptr;

            EGG::MsgRes::analyzeTag(c, p + 1, &tagLen, &tagInfo, &arg);

            if (tagInfo != 0) {
                if (param > count) {
                    const wchar_t *sub = va_arg(vargs, const wchar_t *);
                    memcpy(cursor, sub, wcslen(sub) * 2);
                    count++;
                    cursor += wcslen(sub);
                } else {
                    ulong len = 0;
                    u16 idx = tagInfo;

                    if ((tagInfo & 0xFF0000) != 0xFF0000) {
                        switch (idx) {
                            case 0:  len = getWorldNum(arg); break;
                            case 1:
                            case 14:
                            case 15:
                            case 16: len = getCourseNum(); break;
                            case 2:  len = getOkCancellDisp(bmg, arg); break;
                            case 3:  len = getTotalCollectionCoin(); break;
                            case 4:  len = getCrossKeyDisp(bmg); break;
                            case 5:  len = getCourseSelectIcon(bmg, arg); break;
                            case 6:  len = getSaveFileNumber(); break;
                            case 7:  len = getCourseSelectButtonFunction(bmg, arg); break;
                            case 8:  len = getMenuButton(bmg, arg); break;
                            case 9:  len = getScissor(arg); break;
                            case 10: len = getEasyPairing(bmg, arg); break;
                            case 11: len = getPlayerNum(); break;
                            case 13: len = getDebugDisp(); break;
                            case 17: len = getPlayNumber(); break;
                            case 18: len = getOkCancellDisp(bmg); break;
                            case 19: len = getRedBlock(bmg, arg); break;
                        }
                    } else {
                        if (idx == 2) {
                            len = setRuBi(arg);
                        } else if (idx == 1) {
                            len = setSize(arg);
                        }
                    }

                    memcpy(cursor, mTextBuf, len * 2);
                    cursor += len;
                    count++;
                }
            } else {
                memcpy(cursor, p, tagLen + 2);
                cursor += tagLen / 2 + 1;
            }

            p += tagLen / 2 + 1;
        } else {
            *cursor = c;
            p++;
            cursor++;
        }

        if (bufLen <= (ulong)(cursor - buf)) {
            buf[bufLen - 1] = 0;
            break;
        }
    }

    if (writeLen != nullptr) {
        *writeLen = cursor - buf;
    }
}

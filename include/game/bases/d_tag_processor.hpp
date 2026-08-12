#pragma once

#include <cstdarg>
#include <game/bases/d_message.hpp>
#include <nw4r/ut.h>

namespace nw4r {
namespace lyt {
class Pane;
class DrawInfo;
} // namespace lyt
} // namespace nw4r

/// @brief Processes the formatting tags embedded in a message.
class TagProcessor_c : public nw4r::ut::WideTagProcessor {
public:
    /// @brief A saved scissor box and the pane it was derived from. @unofficial
    struct ScissorEntry_s {
        u32 mSavedScissorX;              ///< The scissor box's saved left edge.
        u32 mSavedScissorY;              ///< The scissor box's saved top edge.
        u32 mSavedScissorWidth;          ///< The scissor box's saved width.
        u32 mSavedScissorHeight;         ///< The scissor box's saved height.
        f32 mScreenScaleX;               ///< The horizontal screen scale.
        f32 mScreenScaleY;               ///< The vertical screen scale.
        nw4r::lyt::Pane *mpPane;         ///< The pane the scissor box applies to.
        f32 mPaneSizeX;                  ///< The pane's width.
        f32 mPaneSizeY;                  ///< The pane's height.
        nw4r::lyt::DrawInfo *mpDrawInfo; ///< The pane's draw info.
    };

    TagProcessor_c();
    virtual ~TagProcessor_c();

    void SetupGXCommon();
    void SetupGXTevSet();
    void SetupVertexFormat();

    void FontChange(nw4r::ut::PrintContext<wchar_t> *ctx);
    void ScaleSet(nw4r::ut::PrintContext<wchar_t> *ctx);
    void RuBySet(nw4r::ut::PrintContext<wchar_t> *ctx);

    void setScissorStart(int idx);
    void setScissorEnd(int idx);
    void setScissorCursorX(nw4r::ut::PrintContext<wchar_t> *ctx);
    void setScissor(nw4r::ut::PrintContext<wchar_t> *ctx);

    virtual Operation Process(u16 tag, nw4r::ut::PrintContext<wchar_t> *ctx);

    void PictureFontCalcRect(nw4r::ut::Rect *rect, nw4r::ut::PrintContext<wchar_t> *ctx);
    void ScaleCalcRect(nw4r::ut::PrintContext<wchar_t> *ctx);
    void RuByCalcRect(nw4r::ut::Rect *rect, nw4r::ut::PrintContext<wchar_t> *ctx);
    virtual Operation CalcRect(nw4r::ut::Rect *rect, u16 tag, nw4r::ut::PrintContext<wchar_t> *ctx);

    /// @brief Gets the number of digits needed to display the given value. @unofficial
    int PlaceCheck(int value);

    /// @brief Clears the message text buffer. @unofficial
    void TextBufClear();

    /// @brief Substitutes a message into the text buffer.
    /// @return The number of wide characters written, including the tag header.
    int MsgIDSet(MsgRes_c *bmg, ulong messageGroup, ulong messageID);

    int getWorldNum(void *arg);
    int getCourseNum();
    int getOkCancellDisp(MsgRes_c *bmg);
    int getOkCancellDisp(MsgRes_c *bmg, void *arg);
    int getTotalCollectionCoin();
    int getCrossKeyDisp(MsgRes_c *bmg);
    int getCourseSelectIcon(MsgRes_c *bmg, void *arg);
    int getSaveFileNumber();
    int getCourseSelectButtonFunction(MsgRes_c *bmg, void *arg);
    int setSize(void *arg);
    int setRuBi(void *arg);
    int getMenuButton(MsgRes_c *bmg, void *arg);
    int getScissor(void *arg);
    int getEasyPairing(MsgRes_c *bmg, void *arg);
    int getDebugDisp();
    int getPlayNumber();

    /// @brief Substitutes the number of players. @unofficial
    int getPlayerNum();
    int getRedBlock(MsgRes_c *bmg, void *arg);

    void preProcess(
        const wchar_t *text, wchar_t *buf,
        unsigned long bufLen, int *writeLen,
        long param, va_list vargs,
        MsgRes_c *bmg
    );

    wchar_t mTextBuf[16];       ///< The buffer the substituted text is built in. @unofficial
    ScissorEntry_s mScissor[4]; ///< The scissor box stack. @unofficial
    u8 mFontIndex;              ///< The index of the font in use. @unofficial

    static bool isZeroWidthSpace; ///< @unofficial

    /// @brief The scale of ruby text relative to the main text.
    static const f32 c_RUBY_SCALE;
    /// @brief The factor applied to the line height when offsetting ruby text upwards.
    static const f32 c_RUBY_WRITE_SCALE_Y;
};

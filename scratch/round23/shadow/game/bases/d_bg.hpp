#pragma once
// SHADOW COPY of include/game/bases/d_bg.hpp for the d_bg_actor_mng.cpp draft.
// PROPOSED additions (not yet in the real header):
//   - m_8fe64/m_8fe68/m_8fe6c/m_8fe70: four view-rect floats at the tail of
//     what the real header pads as mPad1[0x8fe70] + m_8fe00. The binary reads
//     them via `addis r5, r5, 0x9; lfs f1, -0x19c(r5)` etc. (offsets
//     0x8FE64/0x8FE68/0x8FE6C/0x8FE70). The real m_8fe00 name is misleading:
//     the field it refers to sits at 0x8FE70.
//   - CheckExistLayer(u8)  -> .text:0x80077980 (dbg `CheckExistLayer__5dBg_cFUc`)
//   - GetMaskedUnitNumber(u16,u16,u8) -> .text:0x80077610
#include <game/mLib/m_vec.hpp>

class dBg_c {
    class dBg_autoScroll_c {
    public:
        dBg_autoScroll_c() {}
        ~dBg_autoScroll_c() {}

        mVec3_c mPos;
        float m_0c;
        float m_10;
        u8 m_14, m_15, m_16, m_17;
        u8 m_18;
        bool m_19;
        bool mActive;
    };

public:
    u8 mPad1[0x8fe64];
    /// @unofficial view-rect floats (real header hides these in mPad1/m_8fe00)
    float m_8fe64;
    float m_8fe68;
    float m_8fe6c;
    float m_8fe70;
    u8 mPad2[0x2c];
    float mLoopOffset;
    u8 mPad3[0x4];
    /// @brief [0x8FEA8] Screen-tracking X used by @p daEnHatenaBalloon_c. @unofficial
    float m_8fea8;
    /// @brief [0x8FEAC] Screen-tracking Y used by @p daEnHatenaBalloon_c. @unofficial
    float m_8feac;
    u8 mPad3b[0x14];
    float mLiquidHeight;
    u8 mPad4[0x144];
    float mDispScale;
    float m_8ffa8;
    float mPrevDispScale;
    u8 mPad5[0x61];
    u8 m_90009;
    u8 mPad6[0x30];
    dBg_autoScroll_c mAutoscrolls[2];
    u8 mPad7[0x1a];
    u8 m_9008e;

    float getLiquidHeight() const { return mLiquidHeight; }

    void setWaterInWave(float x, float y, u8 type);
    float getLeftLimit();
    float getRightLimit();

    float getDispScale() { return mDispScale; }
    float getPrevDispScale() { return mPrevDispScale; }

    /// @unofficial PROPOSED addition. Both symbols are in the full DOL map
    /// (bin/dtk/wiimj2d_symbols.txt):
    ///   CoinGetBitCheck__5dBg_cFUsUsi = .text:0x80077810  size 0x44
    ///   CoinGetBitSet__5dBg_cFUsUsi   = .text:0x800777B0  size 0x60
    /// CFront mangling gives the exact parameter types -- `F Us Us i` is
    /// (unsigned short, unsigned short, int) -- but omits the return type,
    /// this project's own recurring trap. Return types below are read off
    /// the ONLY two call sites that exist anywhere in the codebase today
    /// (both in wip/wm_units/agent_nice_coin/d_a_nice_coin.cpp, not yet
    /// landed):
    ///   - CoinGetBitCheck's result (r3) is tested immediately after the
    ///     call (`cmpwi r3,0x0; beq ...`) and consumed as a condition
    ///     (`create()`: `if (CoinGetBitCheck(...)) { return 2; }`) -- a
    ///     consumed, tested result proves a non-void return; `bool` is the
    ///     natural type for a value used only as a boolean condition.
    ///   - CoinGetBitSet's result (r3) is NOT read at all after the call
    ///     (`executeState_Search()`: the very next instruction loads an
    ///     unrelated field, `lwz r12, 0x394(r31)`) -- consistent with
    ///     `void`.
    bool CoinGetBitCheck(u16 x, u16 negY, int index);
    void CoinGetBitSet(u16 x, u16 negY, int index);

    /// @unofficial PROPOSED addition (binary .text:0x80077980, size 0x98).
    /// Called from dBgActorManager_c::createObjList as
    /// `if (m_bg_p->CheckExistLayer(layer))`.
    bool CheckExistLayer(u8 layer);

    /// @unofficial PROPOSED addition (binary .text:0x80077610, size 0x3C).
    /// Returns a masked unit number for a grid cell; 0xFFFF means "no unit".
    u16 GetMaskedUnitNumber(u16 x, u16 y, u8 layer);

    static dBg_c *m_bg_p;
};

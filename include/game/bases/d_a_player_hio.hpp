#pragma once

#include <game/mLib/m_vec.hpp>

/// @unofficial
struct sPowerChangeSpeedData {
    float mDefaultAccel;
    float mNoInputAccel;
    float mTurnNoInputAccel;
    float mTurnAccel;
    float mVerySlowAccel;
    float mSlowAccel;
    float mRunSlowAccel;
    float mMediumAccel;
    float mFastAccel;
};

/// @unofficial
struct sSpeedData {
    float mLowSpeed, mMediumSpeed, mHighSpeed;
    sPowerChangeSpeedData mPowerChangeNormal;
    sPowerChangeSpeedData mPowerChangeIce;
    sPowerChangeSpeedData mPowerChangeLowSlip;
};

class dAcPy_HIO_Speed_c {
public:
    dAcPy_HIO_Speed_c();
    ~dAcPy_HIO_Speed_c();

    /// @brief Copies the default speed data for player @p idx into this object.
    /// @unofficial
    void init(int idx);

    sSpeedData mDataNormal;
    sSpeedData mDataStar;

    /// @unofficial
    /// @note An array of sSpeedData, NOT of this class: the generated
    /// __sinit for this TU contains no constructor calls at all, only four
    /// straight 0x78-byte block copies from the four sc_player_* tables below
    /// into offsets 0/0x78/0xf0/0x168 of this object. That is what a plain
    /// aggregate initialiser over a non-constant source compiles to; an array
    /// of dAcPy_HIO_Speed_c would instead emit a construct loop and an
    /// __arraydtor. init() indexes it with a 0xF0 stride, which fixes the
    /// outer dimension at 2 and the inner at 2.
    /// @note `const` with a *dynamic* initialiser, so it still lives in .bss
    /// rather than .rodata. The const is load-bearing for init()'s codegen --
    /// see the note there.
    static const sSpeedData sc_playerSpeedDt[2][2];

    /// @unofficial
    /// @note The four shipped default speed records, in .rodata immediately
    /// ahead of scStoopOffset/scYoshiOffset/scCloudOffset. Their real data has
    /// to live in this TU for dPyModel_HIO_c::resetParam to compile: it
    /// addresses those three tables as this object's own @ha/@l pair plus a
    /// compile-time delta, which MWCC can only do when it lays all of them out
    /// itself.
    static const sSpeedData sc_player_mame;
    static const sSpeedData sc_player_mame_star;
    static const sSpeedData sc_player_normal;
    static const sSpeedData sc_player_normal_star;

    /// @unofficial
    static int ms_num_of_instance;
};

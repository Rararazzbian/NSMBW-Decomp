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
    /// @note Constructed as a plain static array, so each element runs the
    /// default constructor above at static-init time -- including element 0,
    /// whose constructor calls init(ms_num_of_instance), i.e. init(0), which
    /// copies sc_playerSpeedDt[0] into itself before that element exists. The
    /// observed effect is harmless (a self-copy of still-zeroed memory) but the
    /// reason the original ships this ordering is not understood; flagged for
    /// follow-up rather than asserted.
    static dAcPy_HIO_Speed_c sc_playerSpeedDt[2];

    /// @unofficial
    static int ms_num_of_instance;
};

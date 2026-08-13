#pragma once

class dGameDisplay_c {
public:
    /// @note Signatures pinned by their mangled symbols:
    /// setPlayNum__14dGameDisplay_cFPi (0x801599C0),
    /// setCoinNum__14dGameDisplay_cFi (0x80159AA0),
    /// setScore__14dGameDisplay_cFi (0x80159DF0),
    /// setCollect__14dGameDisplay_cFv (0x80159C30). @unofficial
    void setPlayNum(int *playNum);
    void setCoinNum(int coinNum);
    void setScore(int score);
    void setCollect();

    static const int c_PLAYNUM_DIGIT;
};

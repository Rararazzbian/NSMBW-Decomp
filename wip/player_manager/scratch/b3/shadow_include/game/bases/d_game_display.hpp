#pragma once

class dGameDisplay_c {
public:
    static const int c_PLAYNUM_DIGIT;

    // SHADOW-ONLY additions, not in the real header. See BATCH3.md.
    void setPlayNum(int *);
    void setCoinNum(int);
    void setScore(int);
    void setCollect();
};

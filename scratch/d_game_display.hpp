#pragma once
#include <types.h>

class dGameDisplay_c {
public:
    void setPlayNum(int *playNum);
    void setCoinNum(int coinNum);
    void setTime(int time);
    void setCollect();
    void setScore(int score);

    static const int c_PLAYNUM_DIGIT;
    static const int c_COINNUM_DIGIT;
    static const int c_TIME_DIGIT;
    static const int c_SCORE_DIGIT;

    static dGameDisplay_c *m_instance;
};

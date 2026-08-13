#pragma once

#include <types.h>
#include <game/mLib/m_vec.hpp>

class dEnemyMng_c {
public:
    void breakdownSE(int, const mVec3_c &);
    void incQuakeComboCount(int);
    void createRevivalBallon(mVec3_c &, int, int);
    void demo_ivy_create(mVec3_c *);

    /// @note The `unsigned long` parameters are load-bearing: they mangle to
    /// `Ul`, giving `multi_item_set__11dEnemyMng_cFP7mVec3_cPUlUliUlScUc`.
    /// Writing `u32` mangles to `Ui` and names a symbol that does not exist --
    /// and the emitted instruction words are identical either way, so only a
    /// callee-symbol-name comparison catches it. @unofficial
    void multi_item_set(mVec3_c *pos, unsigned long *itemNos, unsigned long count,
                        int mode, unsigned long param, s8 playerNo, u8 layer);

    u8 mPad1[0x138];
    int m_138;
    u8 mPad2[0x18];
    int m_154;
    u8 mPad3[0x4];
    int m_15c;
    u8 mPad4[0x4];
    int mWireTurn; ///< Whether a chainlink fence is currently being flipped. @unofficial

    static dEnemyMng_c *m_instance;
};

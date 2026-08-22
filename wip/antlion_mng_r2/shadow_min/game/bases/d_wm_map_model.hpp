#pragma once

#include <types.h>

class dWmMapModel_c {
public:
    /// @unofficial PROPOSED. Mangled setAntlion__13dWmMapModel_cFbib; called from
    /// daWmAntlionMng_c::reviveOnRoute (fn_2_15BC30) and ::rebuildAllModels (fn_2_15BDA0).
    void setAntlion(bool, int, bool);
    /// @unofficial PROPOSED. Mangled endAntlionEffect__13dWmMapModel_cFv; called from
    /// daWmAntlionMng_c::checkAttackSequenceDone (fn_2_15BF80).
    void endAntlionEffect();

private:
    u8 mPad[0xbf8];
};

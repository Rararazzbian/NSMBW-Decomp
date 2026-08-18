#pragma once

#include <types.h>

/// @unofficial SHADOW COPY, proposed addition to the real include/game/bases/d_wm_map_model.hpp.
/// `dWmMapModel_c` is currently an opaque `0xbf8`-byte pad. `daWmAntlionMng_c`
/// (this unit) calls two of its methods directly on `daWmMap_c::m_instance->mCsvData`-adjacent
/// model slots (`&daWmMap_c::m_instance->[...] + 0x1a0`, i.e. `mModels[currIdx]`-shaped access):
///   - `setAntlion__13dWmMapModel_cFbib`   -- (bool, int, bool), called from fn_2_15BC30/fn_2_15BDA0
///   - `endAntlionEffect__13dWmMapModel_cFv` -- (), called from fn_2_15BF80
/// Neither body nor the rest of the class layout was reconstructed this round; only the two
/// mangled names/signatures needed to compile antlion_mng were decoded from the target's own
/// call sites. Do NOT land this as-is -- it is a compile-time stand-in, not a verified layout.
class dWmMapModel_c {
public:
    void setAntlion(bool, int, bool);
    void endAntlionEffect();

    u8 mPad[0xbf8];
};

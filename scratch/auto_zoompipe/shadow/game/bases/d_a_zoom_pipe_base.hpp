#pragma once
#include <game/bases/d_actor.hpp>
#include <types.h>

/// @brief Shared pipe actor behaviour.
/// @details This class has not been decompiled on its own yet; only the members
/// referenced by @ref daZoomPipeBase_c are declared here.
/// @unofficial Reconstructed from the symbol map and cross-references only.
class daObjPipeBase_c {
public:
    void calcDownLength(float len);
    int execute();
};

/// @brief Unnamed helper called from daZoomPipeBase_c::init.
/// @details Unnamed in the symbol map; takes the pipe as an explicit leading
/// argument. The fourth parameter is a float (the caller narrows the
/// integer-to-double conversion with fsubs before the call).
/// @unofficial Reconstructed from the call site only.
extern "C" void fn_80045A10(daObjPipeBase_c *self, unsigned int param,
                            float downLen, float kind, int flag, int id,
                            int mode);

/// @brief Base implementation of a pipe that shrinks or grows while active.
/// @details The packed nibbles of #mParam configure two travel speeds, the step
//  size and a model variant. Every frame the current value eases towards the
/// target installed by #fn_80045A10, toggling between the two speeds whenever
/// it overshoots.
/// @unofficial Reconstructed from the code.
class daZoomPipeBase_c : public daObjPipeBase_c {
public:
    void init(u32 param); ///< Reads the packed parameters and initialises the zoom state.
    int execute(); ///< Advances the zoom animation, then runs the base pipe logic.

private:
    /* 0x000 */ u8 mPad0[0x4];
public:
    /* 0x004 */ u32 mParam; ///< Packed configuration nibbles. See ACTOR_PARAM_LOCAL.
private:
    u8 mPad8[0x5C8 - 0x8];
public:
    /* 0x5C8 */ float mTargetLength; ///< The value the zoom eases towards.
    /* 0x5CC */ float mSpeed[2]; ///< The two alternating zoom speeds.
    /* 0x5D4 */ float mStep; ///< Distance covered per frame while easing.
    /* 0x5D8 */ float mCurrent; ///< The current zoom value.
    /* 0x5DC */ u8 mIdx; ///< Index of the speed currently in effect.
};

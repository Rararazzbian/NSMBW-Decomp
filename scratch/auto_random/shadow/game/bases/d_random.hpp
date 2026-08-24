#pragma once
#include <types.h>

namespace EGG { class CoreController; }

/// @brief Machine-random seed provider.
/// @unofficial The only retail symbol is the static ::calcMachineRandom, which
/// `dGameCom::initRandomSeed` feeds to `setRandomSeed`/`cM::initRnd`. No
/// instance of this class exists anywhere in wiimj2d.dol.
class dRandom_c {
public:
    /// @return a CRC32 over the boot time, the console owner nickname and the
    /// DPD state of all four controller slots.
    static u32 calcMachineRandom();

    /// @unofficial Inferred helper: defined in-class so MWCC (-inline noauto)
    /// inlines it; collects one controller slot's record.
    static void collectOne(float *dst, EGG::CoreController *cc);
};

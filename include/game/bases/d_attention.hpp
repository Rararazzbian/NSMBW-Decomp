#pragma once
#include <game/bases/d_actor.hpp>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_vec.hpp>

/// @brief Tracks the actors eligible to be "attended to" (targeted).
/// @details [.bss instance at 0x803552F0, sizeof 0x58]. `__vt__12dAttention_c`
/// is 0xC, i.e. exactly one virtual slot, so the destructor is the only virtual
/// function. @unofficial
class dAttention_c {
public:
    virtual ~dAttention_c();

    void entry(fBaseID_e id);
    fBase_c *search(mVec3_c pos);
    fBase_c *searchPlayer(const dActor_c *player, mVec3_c pos);

    /// @brief Number of live entries. `reset()` writes 0 here.
    int mEntryCount;
    /// @brief Bounded at 10 -- `entry()` rejects with `cmpwi r0, 10; bge`.
    fBaseID_e mEntryList[10];
    fBaseID_e mActiveList[10];

    static dAttention_c *mspInstance;
};

STATIC_ASSERT(sizeof(dAttention_c) == 0x58);

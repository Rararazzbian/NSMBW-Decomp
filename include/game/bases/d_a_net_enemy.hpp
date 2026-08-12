#pragma once
#include <game/bases/d_enemy.hpp>

/**
 * @brief Base implementation of an enemy that walks on chainlink fences.
 * @details When the fence is flipped, the enemy switches to the other side of it.
 * @ingroup bases
 */
class daNetEnemy_c : public dEn_c {
public:
    // Base class overrides

    virtual int execute();

    STATE_VIRTUAL_FUNC_DECLARE(daNetEnemy_c, NetWait); ///< Waiting while the fence flips.
    STATE_VIRTUAL_FUNC_DECLARE(daNetEnemy_c, NetMove); ///< Moving to the other side of the fence.

    // New virtual functions

    virtual void mdlPlay();
    virtual void calcMdl();

    // Nonvirtuals

    /// @brief Tells every fence enemy that the chainlink fence is being flipped.
    static void setWireTurn(int turn);

    int m_524; ///< @todo Figure out the purpose of this field. @unofficial
};

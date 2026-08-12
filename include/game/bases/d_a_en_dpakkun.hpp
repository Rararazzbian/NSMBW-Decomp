#pragma once
#include <game/bases/d_a_en_dpakkun_base.hpp>

/**
 * @brief A piranha plant that grows out of a pipe.
 * @details The plant waits inside the pipe, rises out of it, snaps for a while
 * and sinks back in.
 * @statetable
 */
class daEnDpakkun_c : public daEnDpakkunBase_c {
public:
    virtual ~daEnDpakkun_c();

    // Base class overrides

    virtual void returnAnm_Ice();

    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkun_c, Wait);      ///< Hiding inside the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkun_c, Appear);    ///< Rising out of the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkun_c, Attack);    ///< Snapping at the players.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDpakkun_c, Disappear); ///< Sinking back into the pipe.

    virtual void initialize();
    virtual void setVanishAnm();

    // New virtual functions

    virtual bool checkAppear(); ///< Checks whether the plant may leave the pipe.

    // Nonvirtuals

    void setAttackAnm(float blendFrame);
    void setMoveSpeed(int reverse);
};

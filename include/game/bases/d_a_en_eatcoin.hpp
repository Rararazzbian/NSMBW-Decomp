#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>

/**
 * @brief The coin spat out by Yoshi after eating a coin-yielding actor.
 * @details The actor immediately awards a coin and its score to the player who
 * ate it, then deletes itself.
 * @ingroup bases
 * @paramtable
 */
class daEnEatCoin_c : public dEn_c {
public:
    virtual ~daEnEatCoin_c();

    // Base class overrides

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual bool setEatGlupDown(dActor_c *eatingActor);

    STATE_VIRTUAL_FUNC_DECLARE(daEnEatCoin_c, EatOut); ///< Being swallowed.

    // Nonvirtuals

    void model_set(); ///< Builds the coin's model.

    dHeapAllocator_c mAllocator; ///< The actor's allocator.
    nw4r::g3d::ResFile mResFile; ///< The actor's model archive.
    m3d::mdl_c mModel; ///< The coin's model.
    mMtx_c mMatrix; ///< The model's transform matrix. @unofficial
    int mCoinType; ///< The coin colour. @unofficial

    ACTOR_PARAM_CONFIG(Type, 0, 4); ///< @unofficial
};

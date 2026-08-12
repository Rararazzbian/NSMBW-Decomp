#pragma once

#include <types.h>
#include <game/bases/d_base.hpp>
#include <game/bases/d_2d/multi.hpp>
#include <game/mLib/m_2d/simple.hpp>
#include <game/mLib/m_vec.hpp>

/// @brief The Wii Remote pointer cursor shown on top of the menus.
/// @details Holds one layout per player and per cursor shape, and fades the active one out
/// while the pointer is off-screen.
/// @ingroup bases
class dAcCursor_c : public dBase_c {
public:
    /// @brief The available cursor layouts.
    /// @unofficial
    enum Lyt_e {
        P1_DEF, ///< Player 1, default shape.
        P1_CAT, ///< Player 1, grabbing shape.
        P2_DEF, ///< Player 2, default shape.
        P2_CAT, ///< Player 2, grabbing shape.
        P3_DEF, ///< Player 3, default shape.
        P3_CAT, ///< Player 3, grabbing shape.
        P4_DEF, ///< Player 4, default shape.
        P4_CAT, ///< Player 4, grabbing shape.
        LYT_COUNT
    };

    virtual ~dAcCursor_c(); ///< @copydoc dBase_c::~dBase_c

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    /// @brief Selects the cursor layout to be drawn.
    /// @param lyt The layout to switch to.
    void setLyt(Lyt_e lyt);

    d2d::ResAccMultLoader_c mResLoader; ///< The resource loader for the cursor archive.
    m2d::Simple_c mLyts[LYT_COUNT]; ///< The cursor layouts.
    m2d::Simple_c *mpCurLyt; ///< The layout currently being drawn.
    int m_608; ///< @unused

    static dAcCursor_c *l_cursor; ///< The active cursor instance.
    static u8 l_alpha; ///< The opacity the cursor is drawn with.
    static mVec2_c l_pos; ///< The position the cursor is drawn at.
};

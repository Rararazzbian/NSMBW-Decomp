#pragma once
#include <game/bases/d_a_rot_objs_base.hpp>

/// @brief Base implementation of a single-tile-collider rotating object. @unofficial
/// @ingroup bases
class daRotBlock_c : public daRotObjsBase_c {
public:
    ~daRotBlock_c();

    /// @brief Sets up the actor's collider and visible area from the given size.
    void rot_block_init(f32 width, f32 height);

    virtual bool scroll_out_check();

    dBg_ctr_c mBgCtr[1]; ///< The actor's tile collider. @unofficial
    obj_bg_data_t mObjBgData[1]; ///< The collider's corner data. @unofficial
};

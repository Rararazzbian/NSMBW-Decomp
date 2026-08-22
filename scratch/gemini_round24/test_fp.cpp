
#include <game/bases/d_enemy_toride_kokoopa.hpp>
extern const s8 l_EnMuki[2];

void test_fn(dEnTorideKokoopa_c *self, mVec2_c speed) {
    
        float rate = self->calcJumpRate();
        f32 muki = (f32)l_EnMuki[self->mDirection];
        f32 sy = speed.y;
        f32 sx = speed.x;
        self->mSpeed.y = sy;
        self->mSpeed.x = (muki * rate) * sx;
    
}

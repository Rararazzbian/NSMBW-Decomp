#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_game_display.hpp>

void daPyMng_c::update() {
    dGameDisplay_c *disp = dScStage_c::getGameDisplay();
    if (disp != nullptr) {
        int *pRest = (int *) ((char *) m_playerID + 0x80);
        int rest[4] = { pRest[0], pRest[1], pRest[2], pRest[3] };
        disp->setPlayNum(rest);
        disp->setCoinNum(getCoinAll());
        disp->setScore(mScore);
        disp->setCollect();
    }
}

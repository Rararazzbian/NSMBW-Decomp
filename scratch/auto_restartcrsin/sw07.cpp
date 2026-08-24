/* sw07: named pointer local BEFORE idx declaration. */
#include <game/bases/d_s_restart_crsin.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_fader.hpp>

STATIC_ASSERT(sizeof(dScRestartCrsin_c::StartGameInfo) == 0x10);

void dScRestartCrsin_c::startTitle(unsigned char param1, bool param2) {
    static const char c_stage[] = "G\0_GER\0_FRA\0_SPA\0_ITA\0_NED\0_CHN";

    if (param1 == 0) {
        m_startGameInfo._0C = 0;
        m_startGameInfo._0D = 39;
        m_startGameInfo._08 = 2;
        m_startGameInfo._07 = 0;
    } else {
        const char *p = c_stage;
        int idx = dScStage_c::m_titleRandomTable[dScStage_c::m_titleCount] * 2;
        m_startGameInfo._0C = p[idx];
        m_startGameInfo._0D = p[idx + 1];
        m_startGameInfo._08 = 3;
        m_startGameInfo._07 = 1;
    }

    m_startGameInfo._0E = m_startGameInfo._0C;
    m_startGameInfo._0F = m_startGameInfo._0D;
    m_startGameInfo._06 = 0;
    m_startGameInfo._05 = 0xFF;

    dFader_c::setFader(dFader_c::FADER_FADE);
    dScCrsin_c::m_isDispOff = true;
    dScene_c::setNextScene(fProfile::RESTART_CRSIN, param2, param2);
}

void dScRestartCrsin_c::reStartPeachCastle() {
    m_startGameInfo._08 = 0;
    m_startGameInfo._07 = 0;
    m_startGameInfo._06 = 0;
    m_startGameInfo._05 = 0xFF;
    m_startGameInfo._0C = 0;
    m_startGameInfo._0D = 40;
    m_startGameInfo._0E = 0;
    m_startGameInfo._0F = 40;

    dFader_c::setFader(dFader_c::FADER_CIRCLE_MIDDLE);
    dScCrsin_c::m_isDispOff = true;
    dScStage_c::m_exitMode = dScStage_c::EXIT_2;
    dScene_c::setNextScene(fProfile::RESTART_CRSIN, 0, 0);
}

/* sw05: c_stage as char[15][2], subscripted [t][0]/[t][1]. Matches symbol
   size 0x1E, explains the *2 stride, and should fold base-first. */
#include <game/bases/d_s_restart_crsin.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_fader.hpp>

void dScRestartCrsin_c::startTitle(unsigned char param1, bool param2) {
    static const char c_stage[15][2] = "G\0_GER\0_FRA\0_SPA\0_ITA\0_NED\0_CH";

    if (param1 == 0) {
        m_startGameInfo._0C = 0;
        m_startGameInfo._0D = 39;
        m_startGameInfo._08 = 2;
        m_startGameInfo._07 = 0;
    } else {
        int t = dScStage_c::m_titleRandomTable[dScStage_c::m_titleCount];
        m_startGameInfo._0C = c_stage[t][0];
        m_startGameInfo._0D = c_stage[t][1];
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

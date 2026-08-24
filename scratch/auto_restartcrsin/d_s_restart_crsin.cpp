/* sw08: (&c_stage[idx])[1] form for the second byte. */
#include <game/bases/d_s_restart_crsin.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_fader.hpp>

STATIC_ASSERT(sizeof(dScRestartCrsin_c::StartGameInfo) == 0x10);

dScRestartCrsin_c::StartGameInfo dScRestartCrsin_c::m_startGameInfo;

void dScRestartCrsin_c::startTitle(unsigned char param1, bool param2) {
    static const u8 c_stage[15][2] = {
        {0x00, 0x03}, {0x00, 0x05}, {0x01, 0x00}, {0x01, 0x03}, {0x02, 0x00},
        {0x02, 0x04}, {0x02, 0x15}, {0x03, 0x00}, {0x03, 0x04}, {0x04, 0x03},
        {0x04, 0x04}, {0x04, 0x14}, {0x05, 0x17}, {0x06, 0x05}, {0x07, 0x06},
    };

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

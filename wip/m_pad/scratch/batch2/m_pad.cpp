#include <game/mLib/m_pad.hpp>

namespace mPad {

// --- data (declared for compile purposes only; another batch owns the real
// definitions and __sinit/ctor/dtor bookkeeping) ---
EGG::CoreController *g_currentCore;
CH_e g_currentCoreID;
EGG::CoreController *g_core[4];

EGG::CoreControllerMgr *g_padMg;
u32 g_PadFrame;
bool g_IsConnected[4];
ulong s_GetWPADInfoInterval;
u32 s_GetWPADInfoCount;
PadAdditionalData_t g_PadAdditionalData[4];

void beginPad() {
    g_PadFrame++;
    g_padMg->beginFrame();

    for (int i = 0; i < 4; i++) {
        PadAdditionalData_t &pad = g_PadAdditionalData[i];
        EGG::CoreController *core = g_padMg->getNthController(i);
        g_core[i] = core;

        if (*((u8 *)core + 0xb1c) & 1) {
            float newX = *(float *)((u8 *)core + 0x6c);
            float newY = *(float *)((u8 *)core + 0x70);
            float dX = newX - pad.mPosX;
            float dY = newY - pad.mPosY;
            pad.mPosX = newX;
            pad.mPosY = newY;
            float ddX = dX - pad.mVelX;
            float ddY = dY - pad.mVelY;
            float unused[6] = { ddX, ddY, dX, dY, newX, newY };
            pad.mAccX = ddX;
            pad.mAccY = ddY;
            pad.mVelX = dX;
            pad.mVelY = dY;

            if (!g_IsConnected[i])
                g_IsConnected[i] = true;
        } else {
            if (g_IsConnected[i]) {
                ((EGG::CoreStatus *)((u8 *)core + 0x18))->init();
                core->sceneReset();
                pad.mPosX = 0.0f;
                pad.mPosY = 0.0f;
                pad.mAccX = 0.0f;
                pad.mAccY = 0.0f;
                pad.mVelX = 0.0f;
                pad.mVelY = 0.0f;
                clearWPADInfo((CH_e)i);
                g_IsConnected[i] = false;
            }
        }

        if (s_GetWPADInfoInterval != 0) {
            if (s_GetWPADInfoInterval == 1 || s_GetWPADInfoCount == (u32)i ||
                (s_GetWPADInfoInterval <= 3 &&
                 (s_GetWPADInfoCount & 1) == ((u32)i & 1))) {
                getWPADInfoAsync((CH_e)i);
            }
        }
    }

    if (s_GetWPADInfoInterval != 0) {
        if (++s_GetWPADInfoCount > s_GetWPADInfoInterval)
            s_GetWPADInfoCount = 0;
    }

    g_currentCore = g_core[g_currentCoreID];
}

} // namespace mPad

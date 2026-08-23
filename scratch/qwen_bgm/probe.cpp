#include <game/bases/d_wm_bgm_sync.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_audio.hpp>

namespace dAudio {
    u8 getBgmBeatTrg();
    u8 getBgmAccentSign();
    int getBgmTempo();
}

bool dWmBgmSync_c::execute() {
    if (m_18 == nullptr) {
        return false;
    }

    m_0c = false;
    if (!m_0e) {
        if (dAudio::getBgmBeatTrg()) {
            if (m_08 > 0) {
                m_08--;
            }
            if (m_08 == 0) {
                m_04++;
            }

            if (m_04 == *m_18) {
                m_0c = true;
                m_04 = 0;
                m_10 = 0.0f;
                m_14 = fn_80102F10();
            } else {
                m_10 += 1.0f;
                m_14 = fn_80102F10() - m_10;
            }
        }

        if (dAudio::getBgmAccentSign()) {
            m_0d = true;
        } else {
            m_0d = false;
        }
    }
    return true;
}

float dWmBgmSync_c::getAnmRate(float frameCount) {
    dAudio::getBgmTempo();
    return frameCount / fn_80102F10();
}

float dWmBgmSync_c::fn_80102F10() {
    int tempo = dAudio::getBgmTempo();
    const s16 *beat = m_18;
    f32 t = (f32)(u32)(tempo & 0xffff);
    return 3600.0f * (f32)*beat / t;
}

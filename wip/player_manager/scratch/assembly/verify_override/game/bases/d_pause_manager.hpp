#pragma once
// VERIFICATION-ONLY SHADOW COPY -- not part of the assembled.cpp deliverable
// and NOT written into include/. PauseManager_c does not exist anywhere in
// the real include tree (grepped, zero hits outside our own target
// disassembly). See wip/player_manager/BATCH3.md and ASSEMBLY.md for the
// report to the integrator: this header needs to be created for real before
// daPyMng_c::update() can compile against the project's actual state.

class PauseManager_c {
public:
    void setPauseEnable(bool);

    static PauseManager_c *m_instance;
};

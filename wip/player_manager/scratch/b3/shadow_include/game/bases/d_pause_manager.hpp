#pragma once
// SHADOW-ONLY, compile-testing stand-in. PauseManager_c does not exist anywhere
// in the real include tree (grepped, zero hits outside our own target
// disassembly). See BATCH3.md for the report to the integrator.

class PauseManager_c {
public:
    void setPauseEnable(bool);

    static PauseManager_c *m_instance;
};

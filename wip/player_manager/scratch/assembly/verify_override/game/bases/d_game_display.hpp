#pragma once
// VERIFICATION-ONLY SHADOW COPY -- not part of the assembled.cpp deliverable
// and NOT written into include/. Real header only has c_PLAYNUM_DIGIT; the
// four methods below are needed by daPyMng_c::update() (mangled names
// confirmed by B3 from the disassembly: setPlayNum__14dGameDisplay_cFPi,
// setCoinNum__14dGameDisplay_cFi, setScore__14dGameDisplay_cFi,
// setCollect__14dGameDisplay_cFv). See BATCH3.md / ASSEMBLY.md.

class dGameDisplay_c {
public:
    static const int c_PLAYNUM_DIGIT;

    // SHADOW-ONLY additions, not in the real header.
    void setPlayNum(int *);
    void setCoinNum(int);
    void setScore(int);
    void setCollect();
};

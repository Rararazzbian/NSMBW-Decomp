// Probe 4: named static const declared in the MIDDLE, between fnA/fnB and a
// third function fnC. If declaration point truly governs, .rodata order
// should be: fnA-pool, fnB-pool, sMid, fnC-pool. If it were pass-based
// (named objects first, then all anonymous pools), sMid would lead instead.
float fnA(float x) {
    float v[3] = {111.0f, 111.1f, 111.2f};
    return x * v[0] + v[1] + v[2];
}

float fnB(float x) {
    float v[3] = {222.0f, 222.1f, 222.2f};
    return x * v[0] + v[1] + v[2];
}

static const unsigned int sMid[3] = {7, 8, 9};

float fnC(float x) {
    float v[3] = {333.0f, 333.1f, 333.2f};
    return x * v[0] + v[1] + v[2];
}

unsigned int useMid(int i) {
    return sMid[i];
}

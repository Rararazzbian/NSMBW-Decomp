// Probe 1: named static const declared LAST, after both functions.
// fnA/fnB each build a local 3-float array from distinctive literals, big
// enough (0xC bytes) to defeat -sdata2 small-data placement and force an
// anonymous per-function literal pool entry into .rodata.
float fnA(float x) {
    float v[3] = {111.0f, 111.1f, 111.2f};
    return x * v[0] + v[1] + v[2];
}

float fnB(float x) {
    float v[3] = {222.0f, 222.1f, 222.2f};
    return x * v[0] + v[1] + v[2];
}

static const unsigned int sTail[3] = {1, 0, 0};

unsigned int useTail(int i) {
    return sTail[i];
}

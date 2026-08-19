// Probe 2: control -- named static const declared FIRST, before both functions.
static const unsigned int sTail[3] = {1, 0, 0};

float fnA(float x) {
    float v[3] = {111.0f, 111.1f, 111.2f};
    return x * v[0] + v[1] + v[2];
}

float fnB(float x) {
    float v[3] = {222.0f, 222.1f, 222.2f};
    return x * v[0] + v[1] + v[2];
}

unsigned int useTail(int i) {
    return sTail[i];
}

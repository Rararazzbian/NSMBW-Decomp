// Probe 3d: the shape the real target actually looks like -- a named struct
// array mixing floats and an integer flag, declared LAST, after both
// functions, referenced so it survives.
struct R {
    float a, b, c;
    unsigned int flag;
};

float fnA(float x) {
    float v[3] = {111.0f, 111.1f, 111.2f};
    return x * v[0] + v[1] + v[2];
}

float fnB(float x) {
    float v[3] = {222.0f, 222.1f, 222.2f};
    return x * v[0] + v[1] + v[2];
}

static const R sRecords[2] = {
    {2160.0f, -30.0f, -478.0f, 0},
    {2160.0f, -30.0f, -478.0f, 1},
};

const R *getRecord(int i) {
    return &sRecords[i];
}

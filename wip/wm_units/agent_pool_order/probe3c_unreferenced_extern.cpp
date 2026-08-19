// Probe 3c: extern const array declared LAST, after both functions, and
// NEVER referenced by anything IN THIS TU. Per AGENT_CONTEXT.md,
// "extern is load-bearing on an unreferenced const array" -- at namespace
// scope a plain const array has internal linkage in C++, so it would be
// stripped as unused; extern forces external linkage so it survives even
// unreferenced within the TU.
float fnA(float x) {
    float v[3] = {111.0f, 111.1f, 111.2f};
    return x * v[0] + v[1] + v[2];
}

float fnB(float x) {
    float v[3] = {222.0f, 222.1f, 222.2f};
    return x * v[0] + v[1] + v[2];
}

extern const unsigned int sTail[3] = {1, 0, 0};

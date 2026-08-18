struct C {
    static int a[4], b[4], c[4], d[4], e[4], f[4], g[4], h[4], i[4], j[4], k[4], l[4];
    static void touch();
};
// definitions present, contiguous, in order -- as in the assembled TU
int C::a[4]; int C::b[4]; int C::c[4]; int C::d[4];
int C::e[4]; int C::f[4]; int C::g[4]; int C::h[4];
int C::i[4]; int C::j[4]; int C::k[4]; int C::l[4];

void C::touch() {
    for (int n = 0; n < 4; n++) {
        if (k[n] != 0) { k[n]--; if (k[n] == 0) { l[n] = 0; } }
    }
}

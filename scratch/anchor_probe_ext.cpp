struct C {
    static int a[4], b[4], c[4], d[4], e[4], f[4], g[4], h[4], i[4], j[4], k[4], l[4];
    static void touch();
};
// NO definitions -- the situation an isolated batch draft is in
void C::touch() {
    for (int n = 0; n < 4; n++) {
        if (k[n] != 0) { k[n]--; if (k[n] == 0) { l[n] = 0; } }
    }
}

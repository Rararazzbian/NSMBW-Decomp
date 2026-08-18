struct C {
    static int pid[4], b[4], c[4], d[4], ent[4], typ[4], mod[4], itm[4];
    static int rest[4], coin[4], qt[4], qef[4];
    static int getCoin();
    static void upd();
};
int C::pid[4]; int C::b[4]; int C::c[4]; int C::d[4];
int C::ent[4]; int C::typ[4]; int C::mod[4]; int C::itm[4];
int C::rest[4]; int C::coin[4]; int C::qt[4]; int C::qef[4];
int C::getCoin() { return 1; }
// several separate loops over different arrays, as update() has
void C::upd() {
    int buf[4];
    for (int j = 0; j < 4; j++) { buf[j] = rest[j]; }
    getCoin();
    for (int i = 0; i < 4; i++) {
        if (qt[i] != 0) { qt[i]--; if (qt[i] == 0) { qef[i] = 0; } }
    }
    for (int i = 0; i < 4; i++) { if (ent[i]) { typ[i] = 0; } }
}

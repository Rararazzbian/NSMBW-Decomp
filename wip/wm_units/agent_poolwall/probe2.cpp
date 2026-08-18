#include "weak_base.hpp"

// Derived_c has its OWN out-of-line dtor -> key function -> STRONG vtable,
// exactly like daWmKinokoBase_c's real shape (getModelName defined out of
// line makes the class's vtable strong, not weak-by-full-inlining).
struct Derived_c : WeakBaseA_c, WeakBaseB_c {
    Derived_c();
    virtual ~Derived_c();
    virtual int own() { return 42; }
};

Derived_c::Derived_c() {}
Derived_c::~Derived_c() {}

Derived_c g_d;

const char *use() {
    WeakBaseB_c *p = &g_d;
    return p->tailFn();
}

int use2() {
    return g_d.own() + g_d.a() + g_d.b();
}

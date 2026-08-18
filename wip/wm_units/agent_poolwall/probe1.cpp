#include "weak_base.hpp"

// Derived_c has its OWN strong vtable (like daWmKinokoBase_c), inherits from
// two weak bases and does NOT override tailFn -- exactly the noko/kickEffect
// shape: tailFn is reachable only via the inherited vtable slot.
struct Derived_c : WeakBaseA_c, WeakBaseB_c {
    virtual ~Derived_c() {}
    virtual int own() { return 42; }
};

Derived_c g_d;

const char *use() {
    // Force tailFn to be ODR-used through the vtable (Derived_c doesn't
    // override it, so this call goes through WeakBaseB_c's slot).
    WeakBaseB_c *p = &g_d;
    return p->tailFn();
}

int use2() {
    return g_d.own() + g_d.a() + g_d.b();
}

#pragma once

// Mimics the shape of dWmObjActor_c / m3d::anmChr_c: a base class whose own
// .cpp is NOT part of this compile, so every derived TU that needs its vtable
// (via an inherited, non-overridden virtual, or via the destructor chain)
// must weakly re-emit it here.
struct WeakBaseA_c {
    virtual ~WeakBaseA_c();
    virtual int a() { return 1; }
};

struct WeakBaseB_c {
    virtual ~WeakBaseB_c();
    virtual int b() { return 2; }
    // An inline virtual whose BODY carries a small const object -- like
    // noko's kickEffect(). Only emitted (weakly) if referenced through the
    // vtable of a class that does not override it.
    virtual const char *tailFn() {
#pragma explicit_zero_data on
        static char trailing[8] = "";
#pragma explicit_zero_data off
        return trailing;
    }
};

#include <game/bases/d_base_actor.hpp>
#include <stddef.h>

template<int N> struct S;

// Base-subobject offset trick: casting a non-null-but-fake pointer through
// the base gives (address + base_offset); subtracting the original address
// yields the base offset, as a compile-time constant via reinterpret_cast in
// a constant-expression-friendly way is not available pre-C++11, so instead
// force the value out through the same "incomplete template" error channel
// used elsewhere, on a real (non-null) pointer difference computed at
// runtime -- but we need a COMPILE-TIME value for the S<N> trick, so instead
// just measure it indirectly via offsetof on a wrapper struct that places a
// known marker member right after the base.

struct Wrap1 : public dBase_c { int marker; };
struct Wrap2 : public cOwnerSetMg_c { int marker; };

template<int N> struct T;
struct Probe {
    T<sizeof(cOwnerSetMg_c)> a;
    T<sizeof(Wrap2)> b;
};

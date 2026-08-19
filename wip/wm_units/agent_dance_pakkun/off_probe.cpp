#include <game/bases/d_base_actor.hpp>
template<int N> struct S;
struct P : public dBaseActor_c {
    S<offsetof(P, mMng)> a;
    S<offsetof(P, mpUnusedHelper)> b;
    S<offsetof(P, mUnusedList)> c;
    S<offsetof(P, mHeap)> d;
    S<sizeof(fBase_c)> e;
    S<sizeof(dBase_c)> f2;
    S<sizeof(dBaseActor_c)> g;
};

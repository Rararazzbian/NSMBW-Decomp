#include <game/bases/d_base_actor.hpp>
#include <stddef.h>

template<int N> struct S;

struct P : public dBaseActor_c {
    S<offsetof(P, mUniqueID)>        f01;
    S<offsetof(P, mParam)>           f02;
    S<offsetof(P, mProfName)>        f03;
    S<offsetof(P, mLifecycleState)>  f04;
    S<offsetof(P, mDeleteRequested)> f05;
    S<offsetof(P, mDeferExecute)>    f06;
    S<offsetof(P, mDeferRetryCreate)>f07;
    S<offsetof(P, mGroupType)>       f08;
    S<offsetof(P, mProcControl)>     f09;
    S<offsetof(P, mMng)>             f10;
    S<offsetof(P, mpUnusedHelper)>   f11;
    S<offsetof(P, mUnusedList)>      f12;
    S<offsetof(P, mHeap)>            f13;
    S<sizeof(fBase_c)>               f14;
    S<offsetof(P, mpKindString)>     f15;
    S<offsetof(P, mpNameString)>     f16;
    S<sizeof(dBase_c)>               f17;
    S<offsetof(P, mMatrix)>          f18;
    S<sizeof(dBaseActor_c)>          f19;
};

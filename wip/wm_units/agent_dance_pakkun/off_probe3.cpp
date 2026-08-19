#include <game/bases/d_base_actor.hpp>
#include <stddef.h>

template<int N> struct S;

struct P : public dBaseActor_c {
    S<offsetof(P, mUniqueID)>        *f01;
    S<offsetof(P, mParam)>           *f02;
    S<offsetof(P, mProfName)>        *f03;
    S<offsetof(P, mLifecycleState)>  *f04;
    S<offsetof(P, mDeleteRequested)> *f05;
    S<offsetof(P, mDeferExecute)>    *f06;
    S<offsetof(P, mDeferRetryCreate)>*f07;
    S<offsetof(P, mGroupType)>       *f08;
    S<offsetof(P, mProcControl)>     *f09;
    S<offsetof(P, mMng)>             *f10;
    S<offsetof(P, mpUnusedHelper)>   *f11;
    S<offsetof(P, mUnusedList)>      *f12;
    S<offsetof(P, mHeap)>            *f13;
    S<offsetof(P, mpKindString)>     *f15;
    S<offsetof(P, mpNameString)>     *f16;
    S<offsetof(P, mMatrix)>          *f18;
};

void use(P *x) {
    *x->f01 = *x->f01;
    *x->f02 = *x->f02;
    *x->f03 = *x->f03;
    *x->f04 = *x->f04;
    *x->f05 = *x->f05;
    *x->f06 = *x->f06;
    *x->f07 = *x->f07;
    *x->f08 = *x->f08;
    *x->f09 = *x->f09;
    *x->f10 = *x->f10;
    *x->f11 = *x->f11;
    *x->f12 = *x->f12;
    *x->f13 = *x->f13;
    *x->f15 = *x->f15;
    *x->f16 = *x->f16;
    *x->f18 = *x->f18;
}

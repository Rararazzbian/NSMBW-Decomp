#include <types.h>
#include <game/bases/d_wm_demo_actor.hpp>
#include <stddef.h>

STATIC_ASSERT(sizeof(dWmDemoActor_c) == 0x184);
STATIC_ASSERT(offsetof(dBaseActor_c, mPos) == 0xac);
STATIC_ASSERT(offsetof(dBaseActor_c, mScale) == 0xdc);

template<int N> struct Probe;

class dWmDemoActor_probe_c : public dWmDemoActor_c {
public:
    virtual int execute() { return 0; }
    Probe<offsetof(dWmDemoActor_probe_c, mHeapAllocator)> *p1;
    Probe<offsetof(dWmDemoActor_probe_c, mModel)> *p2;
    Probe<offsetof(dWmDemoActor_probe_c, mSvMdl)> *p3;
    Probe<offsetof(dWmDemoActor_probe_c, mTargetPos)> *p4;
    Probe<offsetof(dWmDemoActor_probe_c, mScaleCurr)> *p5;
    Probe<offsetof(dWmDemoActor_probe_c, mScaleDelay)> *p6;
};

void useProbes(dWmDemoActor_probe_c *x) {
    *x->p1 = *x->p1;
    *x->p2 = *x->p2;
    *x->p3 = *x->p3;
    *x->p4 = *x->p4;
    *x->p5 = *x->p5;
    *x->p6 = *x->p6;
}

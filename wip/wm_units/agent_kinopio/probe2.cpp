#include <types.h>
#include <game/bases/d_wm_demo_actor.hpp>
#include <stddef.h>

template<int N> struct Probe;

class dBaseActor_probe_c : public dBaseActor_c {
public:
    virtual int execute() { return 0; }
    Probe<offsetof(dBaseActor_probe_c, mLastPos)> *p1;
    Probe<offsetof(dBaseActor_probe_c, mPosDelta)> *p2;
    Probe<offsetof(dBaseActor_probe_c, mCenterOffs)> *p3;
    Probe<offsetof(dBaseActor_probe_c, mSpeed)> *p4;
    Probe<offsetof(dBaseActor_probe_c, mSpeedMax)> *p5;
    Probe<offsetof(dBaseActor_probe_c, mAngle)> *p6;
    Probe<offsetof(dBaseActor_probe_c, mAngle3D)> *p7;
    Probe<offsetof(dBaseActor_probe_c, mSpeedF)> *p8;
    Probe<offsetof(dBaseActor_probe_c, mMaxSpeedF)> *p9;
    Probe<offsetof(dBaseActor_probe_c, mAccelY)> *p10;
    Probe<offsetof(dBaseActor_probe_c, mMaxFallSpeed)> *p11;
    Probe<offsetof(dBaseActor_probe_c, mAccelF)> *p12;
    Probe<offsetof(dBaseActor_probe_c, mActorProperties)> *p13;
    Probe<offsetof(dBaseActor_probe_c, mVisible)> *p14;
    Probe<sizeof(dBaseActor_c)> *p15;
};

class dWmActor_probe_c : public dWmActor_c {
public:
    virtual int execute() { return 0; }
    Probe<offsetof(dWmActor_probe_c, mClipSphere)> *q1;
    Probe<sizeof(dWmActor_c)> *q2;
};

void useProbes(dBaseActor_probe_c *x, dWmActor_probe_c *y) {
    *x->p1=*x->p1; *x->p2=*x->p2; *x->p3=*x->p3; *x->p4=*x->p4; *x->p5=*x->p5;
    *x->p6=*x->p6; *x->p7=*x->p7; *x->p8=*x->p8; *x->p9=*x->p9; *x->p10=*x->p10;
    *x->p11=*x->p11; *x->p12=*x->p12; *x->p13=*x->p13; *x->p14=*x->p14; *x->p15=*x->p15;
    *y->q1=*y->q1; *y->q2=*y->q2;
}

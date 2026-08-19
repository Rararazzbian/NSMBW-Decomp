#include <types.h>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_a_wm_ghost.hpp>
#include <game/bases/d_wm_enemy.hpp>
#include <game/bases/d_base.hpp>
#include <game/bases/d_base_actor.hpp>
#include <stddef.h>

STATIC_ASSERT(sizeof(dWmDemoActor_c) == 0x184);
STATIC_ASSERT(sizeof(dWmObjActor_c) == 0x188);
STATIC_ASSERT(offsetof(daWmGhost_c, mAllocator) == 0x18c);
STATIC_ASSERT(offsetof(daWmGhost_c, mUnk188) == 0x188);
STATIC_ASSERT(sizeof(dWmEnemy_c) > 0x184);

template<int N> struct Probe;

class dWmDemoActor_probe_c : public dWmDemoActor_c {
public:
    Probe<offsetof(dWmDemoActor_probe_c, m_00)> *p1;
    Probe<offsetof(dWmDemoActor_probe_c, mIsCutEnd)> *p2;
    Probe<offsetof(dWmDemoActor_probe_c, mHeapAllocator)> *p3;
    Probe<offsetof(dWmDemoActor_probe_c, mModel)> *p4;
    Probe<offsetof(dWmDemoActor_probe_c, mSvMdl)> *p5;
    Probe<offsetof(dWmDemoActor_probe_c, mTargetPos)> *p6;
    Probe<offsetof(dWmDemoActor_probe_c, mScaleCurr)> *p7;
    Probe<offsetof(dWmDemoActor_probe_c, mScaleDelta)> *p8;
    Probe<offsetof(dWmDemoActor_probe_c, mScaleTarget)> *p9;
    Probe<offsetof(dWmDemoActor_probe_c, mScaleDelay)> *p10;
    Probe<sizeof(dWmDemoActor_probe_c)> *p11;
};

void useProbes(dWmDemoActor_probe_c *x) {
    *x->p1 = *x->p1;
    *x->p2 = *x->p2;
    *x->p3 = *x->p3;
    *x->p4 = *x->p4;
    *x->p5 = *x->p5;
    *x->p6 = *x->p6;
    *x->p7 = *x->p7;
    *x->p8 = *x->p8;
    *x->p9 = *x->p9;
    *x->p10 = *x->p10;
    *x->p11 = *x->p11;
}

class dBase_probe_c : public dBase_c {
public:
    Probe<offsetof(dBase_probe_c, mpKindString)> *q1;
    Probe<offsetof(dBase_probe_c, mpNameString)> *q2;
    Probe<sizeof(dBase_probe_c)> *q3;
};

void useProbes2(dBase_probe_c *x) {
    *x->q1 = *x->q1;
    *x->q2 = *x->q2;
    *x->q3 = *x->q3;
}

class dBaseActor_probe_c : public dBaseActor_c {
public:
    Probe<offsetof(dBaseActor_probe_c, mMatrix)> *r1;
    Probe<offsetof(dBaseActor_probe_c, mPos)> *r2;
    Probe<offsetof(dBaseActor_probe_c, mLastPos)> *r4;
    Probe<offsetof(dBaseActor_probe_c, mPosDelta)> *r5;
    Probe<offsetof(dBaseActor_probe_c, mCenterOffs)> *r6;
    Probe<offsetof(dBaseActor_probe_c, mScale)> *r7;
    Probe<offsetof(dBaseActor_probe_c, mSpeed)> *r8;
    Probe<offsetof(dBaseActor_probe_c, mSpeedMax)> *r9;
    Probe<offsetof(dBaseActor_probe_c, mAngle)> *r10;
    Probe<offsetof(dBaseActor_probe_c, mAngle3D)> *r11;
    Probe<offsetof(dBaseActor_probe_c, mSpeedF)> *r12;
    Probe<offsetof(dBaseActor_probe_c, mActorProperties)> *r13;
    Probe<offsetof(dBaseActor_probe_c, mVisible)> *r14;
    Probe<sizeof(dBaseActor_probe_c)> *r3;
};

void useProbes3(dBaseActor_probe_c *x) {
    *x->r1 = *x->r1;
    *x->r2 = *x->r2;
    *x->r4 = *x->r4;
    *x->r5 = *x->r5;
    *x->r6 = *x->r6;
    *x->r7 = *x->r7;
    *x->r8 = *x->r8;
    *x->r9 = *x->r9;
    *x->r10 = *x->r10;
    *x->r11 = *x->r11;
    *x->r12 = *x->r12;
    *x->r13 = *x->r13;
    *x->r14 = *x->r14;
    *x->r3 = *x->r3;
}

int dummy() { return 0; }


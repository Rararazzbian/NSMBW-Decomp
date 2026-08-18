// VARIANT 3: same as P0 baseline shape, but the loop is manually unrolled
// into 3 straight-line copies (no for-loop, no back-branch) -- mirroring
// dokan_route's ACTUAL COMPILED SHAPE (its ANIM_COUNT==1 loop is fully
// eliminated by the optimizer into straight-line code with no .L_ label and
// no blt at all -- confirmed by disassembling the real landed object,
// wip/wm_units/agent_ghost/dokan_route_compiled.txt). Question: is
// loop-vs-straight-line the actual discriminator for where the outer
// consumer's slot lands?

struct Val { void *p; };

struct Model {
    void create(Val v, void *alloc, int flags, int n);
};

struct Anim {
    void create(Val v, Val w, void *alloc);
};

struct Res {
    Val GetA();
    Val GetB(const char *name);
};

struct Actor {
    Model mModel;
    Anim mAnim[4];
    Res mRes;
    void *mAllocator;

    void createModel();
};

void Actor::createModel() {
    Val v = mRes.GetA();
    mModel.create(v, mAllocator, 1, 1);

    mAnim[0].create(v, mRes.GetB("x"), mAllocator);
    mAnim[1].create(v, mRes.GetB("x"), mAllocator);
    mAnim[2].create(v, mRes.GetB("x"), mAllocator);
}

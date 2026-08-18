// VARIANT 8: P0's shape (outer + REAL loop, trip count 3, single call per
// iteration -- matching koopa_castle/ghost) but with Model::create reached
// through the SAME inline-wrapper-forwarding-nullptr mechanism that fixed
// V7's no-loop case. Question: does the wrapper mechanism still anchor the
// outer temp low when a GENUINE loop (back-branch, not unrolled) follows it?

struct Val { void *p; };

struct Model {
    void create5(Val v, void *alloc, int flags, int n, void *p);
    void create(Val v, void *alloc, int flags, int n) {
        create5(v, alloc, flags, n, 0);
    }
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

    for (int i = 0; i < 3; i++) {
        mAnim[i].create(v, mRes.GetB("x"), mAllocator);
    }
}

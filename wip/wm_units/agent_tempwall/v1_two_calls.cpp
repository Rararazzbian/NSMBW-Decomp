// VARIANT 1: same shape as P0 (baseline), but the loop body has TWO by-value
// calls per iteration instead of one -- mirroring d_a_wm_dokan_route.cpp's
// landed, byte-exact createModel (mChrAnim[i].create(...) THEN
// mSrtAnim[i].create(...) each iteration, both re-consuming the outer resMdl).
// Question: does adding a second per-iteration call change whether the OUTER
// consumer's slot lands low (like dokan_route's real, matching target) or
// high (like our P0 baseline, matching koopa_castle/ghost's WRONG draft)?

struct Val { void *p; };

struct Model {
    void create(Val v, void *alloc, int flags, int n);
};

struct Anim {
    void create(Val v, Val w, void *alloc);
};

struct SrtAnim {
    void create(Val v, Val w, void *alloc, int n);
};

struct Res {
    Val GetA();
    Val GetB(const char *name);
    Val GetC(const char *name);
};

struct Actor {
    Model mModel;
    Anim mAnim[4];
    SrtAnim mSrt[4];
    Res mRes;
    void *mAllocator;

    void createModel();
};

void Actor::createModel() {
    Val v = mRes.GetA();
    mModel.create(v, mAllocator, 1, 1);

    for (int i = 0; i < 3; i++) {
        mAnim[i].create(v, mRes.GetB("x"), mAllocator);
        mSrt[i].create(v, mRes.GetC("x"), mAllocator, 1);
    }
}

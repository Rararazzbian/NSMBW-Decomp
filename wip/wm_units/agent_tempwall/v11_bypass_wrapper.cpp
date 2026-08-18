// VARIANT 11: completes the 2x2 truth table. Outer call BYPASSES its wrapper
// (explicit trailing nullptr into create5 directly); loop call goes THROUGH
// its double wrapper (create -> create2 -> create4). This is the one
// combination not yet directly measured (V8/V10 = wrapper+bypass = correct;
// V9 = wrapper+wrapper = wrong; real ghost-before-fix = bypass+bypass =
// wrong). Predicted: also wrong, completing "only wrapper+bypass is correct."

struct Val { void *p; };

struct Model {
    void create5(Val v, void *alloc, int flags, int n, void *p);
    void create(Val v, void *alloc, int flags, int n) {
        create5(v, alloc, flags, n, 0);
    }
};

struct Anim {
    void create4(Val v, Val w, void *alloc, void *p);
    void create2(Val v, Val w, void *alloc) {
        create4(v, w, alloc, 0);
    }
    void create(Val v, Val w, void *alloc) {
        create2(v, w, alloc);
    }
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
    mModel.create5(v, mAllocator, 1, 1, 0);  // BYPASS outer wrapper

    for (int i = 0; i < 3; i++) {
        mAnim[i].create(v, mRes.GetB("x"), mAllocator);  // through double wrapper
    }
}

// Minimal synthetic reproduction of the koopa_castle / ghost / dokan_route
// createModel shape: an OUTER by-value consumer before a loop, and a LOOP
// body with a by-value consumer that re-uses the outer value plus one fresh
// per-iteration temporary. No game headers -- just enough structure to force
// MWCC to materialise real stack-passed by-value temporaries (the callees are
// declared, not defined, so nothing inlines away).
//
// Real-shape target (koopa_castle, target slots ascending 0x8/0xc/0x10):
//   Val v = res.GetA();          // outer temporary, consumed once
//   model.create(v, alloc, 1, 1);           // OUTER CONSUMER, target: LOWEST slot
//   for (...) {
//       anim[i].create(v, res.GetB(name), alloc);  // LOOP CONSUMER: ResAnmChr-like
//                                                    // temp + v re-copy, target: ASCENDING after outer
//   }

struct Val { void *p; }; // matches real ResMdl/ResAnmChr: ResCommon<T> wraps ONE pointer

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

    for (int i = 0; i < 3; i++) {
        mAnim[i].create(v, mRes.GetB("x"), mAllocator);
    }
}

// VARIANT 10: same class shape as V9 (Anim has the double-wrapper chain
// create -> create2 -> create4, matching real anm_chr.hpp), but the LOOP
// call site now spells the REAL 4-arg function EXPLICITLY with a trailing
// nullptr (bypassing create/create2), exactly matching dokan_route's real
// source: `mChrAnim[i].create(resMdl, resAnmChr, &mAllocator, nullptr);`
// -- as opposed to koopa_castle's draft, which calls the 3-arg wrapper
// form (no nullptr): `mChrAnim[i].create(resMdl, GetResAnmChr(...), &mAllocator);`
// Question: does bypassing the LOOP call's wrapper restore V8's correct
// low-anchor for the outer temporary?

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
    mModel.create(v, mAllocator, 1, 1);

    for (int i = 0; i < 3; i++) {
        mAnim[i].create4(v, mRes.GetB("x"), mAllocator, 0);  // BYPASS wrapper, explicit nullptr
    }
}

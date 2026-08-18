// VARIANT 9: same as V8 (wrapper on outer call + real loop), but now the
// LOOP's per-iteration call ALSO goes through a double inline-wrapper
// indirection, exactly matching the real m3d::anmChr_c::create():
//   create(mdl, anmChr, alloc)              -- 3-arg, inline
//     -> create2(mdl, anmChr, alloc)        -- 3-arg, inline
//       -> create(mdl, anmChr, alloc, null) -- REAL 4-arg out-of-line
// koopa_castle's real source calls the 3-arg form (mChrAnim[i].create(resMdl,
// GetResAnmChr(...), &mAllocator)), which is this exact double-wrapper path.
// Question: does routing the LOOP call through its own wrapper chain (not
// just the outer call) change/break the low-anchor result V8 established?

struct Val { void *p; };

struct Model {
    void create5(Val v, void *alloc, int flags, int n, void *p);
    void create(Val v, void *alloc, int flags, int n) {
        create5(v, alloc, flags, n, 0);
    }
};

struct Anim {
    // the "real" out-of-line function -- 4 params
    void create4(Val v, Val w, void *alloc, void *p);
    // first inline wrapper
    void create2(Val v, Val w, void *alloc) {
        create4(v, w, alloc, 0);
    }
    // second inline wrapper (what the call site actually spells)
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
        mAnim[i].create(v, mRes.GetB("x"), mAllocator);
    }
}

// VARIANT 4: exact structural clone of dokan_route's real compiled shape --
// ONE outer decl+use statement, then EXACTLY TWO statements (each consuming
// the outer value plus one fresh temporary), NO loop wrapper at all (matching
// ANIM_COUNT==1 exactly, not a manual 3x unroll like V3). If this doesn't
// reproduce outer=low either, the wall is not attributable to loop-count or
// statement-count and must be something else (real class shape, or a
// non-source-addressable compiler internal).

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

    mAnim[0].create(v, mRes.GetB("x"), mAllocator);
    mSrt[0].create(v, mRes.GetC("x"), mAllocator, 1);
}

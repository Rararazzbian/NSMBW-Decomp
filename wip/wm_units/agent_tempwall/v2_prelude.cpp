// VARIANT 2: same as V1 (two calls/iteration), but with two unrelated
// statements BEFORE the outer Val temporary is created -- mirroring
// dokan_route's real createModel, which opens with
// mAllocator.createFrmHeap(...); mResFile = ...getRes(...); BEFORE the
// resMdl declaration and mModel.create() call. Question: does having
// "unrelated" prior statements change whether the outer temp lands low?

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

struct Heap { void createFrmHeap(int a, void *b, void *c, int d); void adjustFrmHeap(); };

struct Actor {
    Model mModel;
    Anim mAnim[4];
    SrtAnim mSrt[4];
    Res mRes;
    Res mResFile;
    Heap mAllocator;
    void *mAllocator2;

    void createModel();
};

void Actor::createModel() {
    mAllocator.createFrmHeap(-1, 0, 0, 0x20);
    mResFile = mRes;

    Val v = mResFile.GetA();
    mModel.create(v, mAllocator2, 1, 1);

    for (int i = 0; i < 3; i++) {
        mAnim[i].create(v, mResFile.GetB("x"), mAllocator2);
        mSrt[i].create(v, mResFile.GetC("x"), mAllocator2, 1);
    }
}

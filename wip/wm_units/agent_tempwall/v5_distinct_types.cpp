// VARIANT 5: same shape as V4 (exact dokan_route clone, no loop), but each
// by-value temporary now has a DISTINCT C++ type (ValMdl, ValChr, ValTexSrt)
// instead of all sharing one 'Val' type -- matching the real
// nw4r::g3d::ResMdl / ResAnmChr / ResAnmTexSrt, which are genuinely different
// classes (all single-pointer ResCommon<T> wrappers, but distinct types).
// Question: does type identity of the by-value temporaries change slot
// assignment?

struct ValMdl { void *p; };
struct ValChr { void *p; };
struct ValTexSrt { void *p; };

struct Model {
    void create(ValMdl v, void *alloc, int flags, int n);
};

struct Anim {
    void create(ValMdl v, ValChr w, void *alloc);
};

struct SrtAnim {
    void create(ValMdl v, ValTexSrt w, void *alloc, int n);
};

struct Res {
    ValMdl GetA();
    ValChr GetB(const char *name);
    ValTexSrt GetC(const char *name);
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
    ValMdl v = mRes.GetA();
    mModel.create(v, mAllocator, 1, 1);

    mAnim[0].create(v, mRes.GetB("x"), mAllocator);
    mSrt[0].create(v, mRes.GetC("x"), mAllocator, 1);
}

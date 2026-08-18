// VARIANT 7: same as V4 (exact dokan_route clone, no loop), but Model::create
// is now a REAL 5-param out-of-line-callable member reached through an
// INLINE 4-param wrapper that forwards nullptr as the 5th arg -- exactly
// matching the real m3d::mdl_c::create() shape confirmed from
// dokan_route_compiled.txt (li r8, 0x0 as a 5th argument even though the
// SOURCE only spells 4 args). Question: is the wrapper indirection what
// anchors the outer temp low?

struct ValMdl { void *p; };
struct ValChr { void *p; };
struct ValTexSrt { void *p; };

struct Model {
    // the "real" out-of-line function -- 5 params, matches
    // create__Q23m3d5mdl_cFQ34nw4r3g3d6ResMdlP12mAllocator_cUliPUl
    void create5(ValMdl v, void *alloc, int flags, int n, void *p);
    // inline wrapper -- what dokan_route's source actually spells
    void create(ValMdl v, void *alloc, int flags, int n) {
        create5(v, alloc, flags, n, 0);
    }
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

// VARIANT 6: same as V4 (exact dokan_route clone, no loop), but with extra
// named-local pointers cached before use, matching dokan_route's REAL
// register pressure (it saves r25-r31, 7 GPRs -- our earlier variants only
// saved 3). dokan_route's real createModel caches &mChrAnim[i], &mSrtAnim[i],
// and pointers into two function-local static arrays (resAnmNames,
// playModes) into callee-saved registers BEFORE the outer mModel.create()
// call. Question: does higher register pressure change whether the outer
// temp lands low or high?

struct ValMdl { void *p; };
struct ValChr { void *p; };
struct ValTexSrt { void *p; };

struct Model {
    void create(ValMdl v, void *alloc, int flags, int n);
};

struct Anim {
    void create(ValMdl v, ValChr w, void *alloc);
    void setRate(float f);
    void setFrame(float f);
};

struct SrtAnim {
    void create(ValMdl v, ValTexSrt w, void *alloc, int n);
    void setPlayMode(int m, long l);
    void setRate(float f, long l);
    void setFrame(float f, long l);
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

static const char *resNames[1] = { "x" };
static const int playModes[1] = { 0 };

void Actor::createModel() {
    ValMdl v = mRes.GetA();
    mModel.create(v, mAllocator, 1, 1);

    const char *name = resNames[0];
    int mode = playModes[0];
    Anim *chr = &mAnim[0];
    SrtAnim *srt = &mSrt[0];

    chr->create(v, mRes.GetB(name), mAllocator);
    chr->setRate(0.0f);
    chr->setFrame(0.0f);

    srt->create(v, mRes.GetC(name), mAllocator, 1);
    srt->setPlayMode(mode, 0);
    srt->setRate(0.0f, 0);
    srt->setFrame(0.0f, 0);
}

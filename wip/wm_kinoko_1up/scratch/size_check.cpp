#include <types.h>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/mdl.hpp>

class daWmKinokoBase_c : public dWmObjActor_c {
public:
    enum ANIM_e { ANIM_0, ANIM_1, ANIM_COUNT };

    daWmKinokoBase_c();
    virtual ~daWmKinokoBase_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);
    virtual void vf7C();
    virtual void vf80();
    virtual void vf84();
    virtual const char *getModelName();

    void createModel();
    void calcModel();
    void mode_exec();

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    m3d::anmChr_c mChrAnim[ANIM_COUNT];
    m3d::anmChrBlend_c mChrBlend;
    u32 mUnk284;
    const char *const *mAnimResNames;
    const char *mModelResName;
};

class daWmKinoko1up_c : public daWmKinokoBase_c {
public:
    daWmKinoko1up_c();
    virtual ~daWmKinoko1up_c();

    virtual void vf7C();
    virtual void vf80();
    virtual void vf84();
    virtual const char *getModelName();

    u32 mFlag;
};

template <bool B> struct STATIC_CHECK;
template <> struct STATIC_CHECK<true> {};

STATIC_CHECK<sizeof(dWmObjActor_c) == 0x188> check_base;
STATIC_CHECK<sizeof(daWmKinokoBase_c) == 0x290> check_kinokobase;
STATIC_CHECK<sizeof(daWmKinoko1up_c) == 0x294> check_1up;


#include <types.h>
#include <game/bases/d_base_actor.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/mdl.hpp>

class daWmKinokoBase_c : public dWmDemoActor_c {
public:
    enum ANIM_e {
        ANIM_0,
        ANIM_1,
        ANIM_COUNT
    };

    daWmKinokoBase_c();
    virtual ~daWmKinokoBase_c();

    virtual int draw();
    virtual int execute();
    virtual int doDelete();
    virtual int doDraw();
    virtual void processCutsceneCommand(int cmd, bool isFirstFrame);
    virtual void vf7C();
    virtual void vf80();
    virtual void vf84();
    virtual const char *getModelName();

    void createModel();
    void calcModel();
    void mode_exec();

    int mResNodeIdx;
    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    m3d::anmChr_c mChrAnim[ANIM_COUNT];
    m3d::anmChrBlend_c mChrBlend;
    u32 m_284;
    u32 m_288;
    u32 m_28c;
    u8 pad[0x24];
};

daWmKinokoBase_c::daWmKinokoBase_c() {}
daWmKinokoBase_c::~daWmKinokoBase_c() {}
int daWmKinokoBase_c::draw() { return 1; }
int daWmKinokoBase_c::execute() { return 1; }
int daWmKinokoBase_c::doDelete() { return 1; }
int daWmKinokoBase_c::doDraw() { return 1; }
void daWmKinokoBase_c::processCutsceneCommand(int cmd, bool isFirstFrame) {}
void daWmKinokoBase_c::vf7C() {}
void daWmKinokoBase_c::vf80() {}
void daWmKinokoBase_c::vf84() {}
const char *daWmKinokoBase_c::getModelName() { return 0; }

#include <game/bases/d_wm_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>

class daWmGrid_c : public dWmActor_c {
public:
    daWmGrid_c();
    virtual ~daWmGrid_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();
    virtual bool processCutsceneCommand(int, bool);

    dHeapAllocator_c mAllocator;
    m3d::smdl_c mModel;
    u32 mPad;
};

char check_size[sizeof(daWmGrid_c) == 0x164 ? 1 : -1];
char check_dWmActor[sizeof(dWmActor_c) == 0x138 ? 1 : -1];

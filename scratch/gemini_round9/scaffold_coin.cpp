#include <types.h>
#include <game/bases/d_a_en_blockmain.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>

class dPanelObjList_c {
public:
    dPanelObjList_c();
    ~dPanelObjList_c();
    u32 m_00;
    u32 m_04;
    u16 m_08;
    u8 m_0a;
    u8 m_0b;
    f32 m_0c;
    f32 m_10;
    f32 m_14;
    f32 m_18;
    u16 m_1c;
    u8 m_1e;
    u8 m_1f;
};

class daEnCoinMain_c : public daEnBlockMain_c {
public:
    void init();
    void set_coin_objbg_center(int);
    void set_coin_objbg(int);
    void NormalCullSizeSet();
    void coll_foot_set(int);
    void set_bgcheck(int);
    void bgin_bgcheck_set(int);
    void sand_effect_set();
    virtual void beginFunsui();
    virtual void endFunsui();
    void bound_non_collision_check();
    virtual void setWaterSpeed();
    void model_set(int);
    void fn_800279F0();
    void angle_add();
    void FootBgInCheck();
    void drop_bgcheck(u8, u8, f32);
    void bg_insert_death_set();
    void flash_move();
    void base_speed_set();
    void draw_coin(int, f32);
    virtual ~daEnCoinMain_c();

    dPanelObjList_c mPanelObjList; // 0x698
    u8 pad_6B8[0x728 - 0x6B8];
    u32 m_728;
    u32 m_72c;
    u32 m_730;
    u32 m_734;
    u32 m_738;
    u32 m_73c;
    u32 m_740;
    u16 m_744;
    u16 m_746;
    u16 m_748;
    u8 m_74a;
    u8 pad_74b;
    dHeapAllocator_c mAllocator; // 0x74C
    m3d::bmdl_c *mpBmdl; // 0x768
    m3d::mdl_c mMdl1; // 0x76C
    m3d::mdl_c mMdl2; // 0x7AC
    m3d::mdl_c mMdl3; // 0x7EC
    m3d::anmChr_c mAnmChr1; // 0x82C
    m3d::anmChr_c mAnmChr2; // 0x864
    m3d::anmTexSrt_c mTexSrt; // 0x89C

    static const f32 l_bound_yspd[4];
    static const f32 smc_DRAW_OFFSET_Y;
    static const f32 smc_OFFSET_Y;
};

const f32 daEnCoinMain_c::l_bound_yspd[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
const f32 daEnCoinMain_c::smc_DRAW_OFFSET_Y = 0.0f;
const f32 daEnCoinMain_c::smc_OFFSET_Y = 0.0f;

void daEnCoinMain_c::init() {}
void daEnCoinMain_c::set_coin_objbg_center(int) {}
void daEnCoinMain_c::set_coin_objbg(int) {}
void daEnCoinMain_c::NormalCullSizeSet() {}
void daEnCoinMain_c::coll_foot_set(int) {}
void daEnCoinMain_c::set_bgcheck(int) {}
void daEnCoinMain_c::bgin_bgcheck_set(int) {}
void daEnCoinMain_c::sand_effect_set() {}
void daEnCoinMain_c::beginFunsui() {}
void daEnCoinMain_c::endFunsui() {}
void daEnCoinMain_c::bound_non_collision_check() {}
void daEnCoinMain_c::setWaterSpeed() {}
void daEnCoinMain_c::model_set(int) {}
void daEnCoinMain_c::fn_800279F0() {}
void daEnCoinMain_c::angle_add() {}
void daEnCoinMain_c::FootBgInCheck() {}
void daEnCoinMain_c::drop_bgcheck(u8, u8, f32) {}
void daEnCoinMain_c::bg_insert_death_set() {}
void daEnCoinMain_c::flash_move() {}
void daEnCoinMain_c::base_speed_set() {}
void daEnCoinMain_c::draw_coin(int, f32) {}
daEnCoinMain_c::~daEnCoinMain_c() {}

STATIC_ASSERT(sizeof(daEnCoinMain_c) == 0x8C8);
STATIC_ASSERT(offsetof(daEnCoinMain_c, mPanelObjList) == 0x698);
STATIC_ASSERT(offsetof(daEnCoinMain_c, mAllocator) == 0x74C);
STATIC_ASSERT(offsetof(daEnCoinMain_c, mpBmdl) == 0x768);
STATIC_ASSERT(offsetof(daEnCoinMain_c, mMdl1) == 0x76C);
STATIC_ASSERT(offsetof(daEnCoinMain_c, mMdl2) == 0x7AC);
STATIC_ASSERT(offsetof(daEnCoinMain_c, mMdl3) == 0x7EC);
STATIC_ASSERT(offsetof(daEnCoinMain_c, mAnmChr1) == 0x82C);
STATIC_ASSERT(offsetof(daEnCoinMain_c, mAnmChr2) == 0x864);
STATIC_ASSERT(offsetof(daEnCoinMain_c, mTexSrt) == 0x89C);

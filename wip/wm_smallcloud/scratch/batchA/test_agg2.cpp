#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_a_wm_smallcloud.hpp>

struct LocalData_t {
    u8 pad[0x88];
    const char *resMdlNames[4];
    char fmtStr[8];
    char pathStr[16];
};

static const LocalData_t sData = {
    {0},
    { "CS_W7_MoveCloud01", "CS_W7_MoveCloud02", "CS_W7_MoveCloud03", "CS_W6aCloud" },
    "CS_W%d",
    "g3d/model.brres"
};

void daWmSmallCloud_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    char arcName[8];
    sprintf(arcName, sData.fmtStr, dScWMap_c::m_WorldNo + 1);
    mResFile = dResMng_c::m_instance->getRes(arcName, sData.pathStr);

    const char *const *resMdlNames = sData.resMdlNames;
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl(resMdlNames[ACTOR_PARAM(CourseNo)]);
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    static bool sInit = false;
    static const char *resAnmNames[4];
    if (!sInit) {
        resAnmNames[0] = resMdlNames[0];
        resAnmNames[1] = resMdlNames[1];
        resAnmNames[2] = resMdlNames[2];
        resAnmNames[3] = resMdlNames[3];
        sInit = true;
    }

    static const m3d::playMode_e playModes[daWmSmallCloud_c::ANIM_COUNT] = {
        m3d::FORWARD_LOOP
    };

    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(resAnmNames[ACTOR_PARAM(CourseNo)]);
    mChrAnim[CS_W7_SmallCloud].create(resMdl, resAnmChr, &mAllocator, nullptr);
    mChrAnim[CS_W7_SmallCloud].mPlayMode = playModes[CS_W7_SmallCloud];
    mChrAnim[CS_W7_SmallCloud].setRate(0.0f);
    mChrAnim[CS_W7_SmallCloud].setFrame(0.0f);
    mModel.setAnm(mChrAnim[CS_W7_SmallCloud]);

    dWmActor_c::setSoftLight_Map(mModel);
    mAllocator.adjustFrmHeap();
}

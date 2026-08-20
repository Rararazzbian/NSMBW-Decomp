#pragma once
// SHADOW COPY of include/game/bases/d_game_com.hpp -- adds ONE declaration,
// dGameCom::rndF(float), that the real header is missing.
//
// PROOF: fn_2_152150 (AC_WATER_MOVE/AC_WATER_MOVE_REGULAR's create()) calls
// `bl rndF__8dGameComFf` four times (target_auto_00_00151E90_text.txt, the
// four `bl rndF__8dGameComFf` at 0x1521E8/0x1521F4/0x152200/0x15220C). dtk's
// own full-DOL symbol map already knows this exact mangled name:
//   bin/dtk/wiimj2d_symbols.txt:4783: rndF__8dGameComFf = .text:0x800B2F10
// -- a real, already-existing DOL function, not something this unit invents.
// `rndF__8dGameComFf` demangles to `dGameCom::rndF(float)`. The real
// `include/game/bases/d_game_com.hpp` declares `rnd()` and `rndInt(size_t)`
// in this exact namespace but not `rndF` -- confirmed by grep, and NOT the
// same function as the already-landed `cM::rndF` in
// `source/dol/cLib/c_math.cpp` (different namespace, different mangled name:
// `rndF__2cMFf`). No landed unit under source/ has hit this gap yet (grepped
// first, per the coordinator's own rule).
#include <types.h>
#include <nw4r/lyt.h>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/sLib/s_RangeData.hpp>
#include <game/bases/d_lyttextBox.hpp>

namespace dGameCom {

    void initRandomSeed();
    u32 getRandomSeed();
    int rndInt(size_t max);
    float rnd();
    float rndF(float max); ///< @unofficial NOT in the real header -- see shadow file comment.

    enum GAME_STOP_e {
        GAME_STOP_PAUSE = BIT_FLAG(0),
        GAME_STOP_WARNING = BIT_FLAG(1),
        GAME_STOP_OTASUKE_PAUSE = BIT_FLAG(2),
        GAME_STOP_HOME_MENU = BIT_FLAG(3),
        GAME_STOP_ANY = -1,
    };

    bool isGameStop(ulong flag);
    void clearGameStop();
    void setGameStop();

    u8 GetLanguageHBM();

    void SetSoftLight_Player(m3d::bmdl_c&, int);
    void SetSoftLight_Map(m3d::bmdl_c&, int);
    void SetSoftLight_Boss(m3d::bmdl_c&, int);
    void SetSoftLight_Enemy(m3d::bmdl_c&, int);
    void SetSoftLight_MapObj(m3d::bmdl_c&, int);
    void SetSoftLight_Item(m3d::bmdl_c&, int);

    void showFukidashi(int playerId, int fukidashiAction);
    void hideFukidashiTemporarily(int playerId, int fukidashiAction, int param3);
    void hideFukidashiForLevel(int playerId, int fukidashiAction, int param3);
    void hideFukidashiForSession(int playerId, int fukidashiAction);
    void FUN_800b37b0(int playerId, int fukidashiAction);

    void CreateSmallScore(const mVec3_c &, int, int, bool);
    u8 GetAspectRatio();
    bool PlayerEnterCheck(int);
    void Player1upColor(LytTextBox_c *, int);
    void getGlbPosToLyt(mVec3_c &);
    bool someCheck(mVec3_c *a, sRangeDataF *b);

    bool checkRectangleOverlap(mVec3_c *, mVec3_c *, mVec3_c *, mVec3_c *, float);

    void SelectCursorSetup();
    void SelectCursorSetup(nw4r::lyt::Pane *pane, int index, bool useSpecialDraw);

    void WindowPaneColorSet(nw4r::lyt::Window *, int);
    float getDispCenterX();
    float getDispCenterY();

    void DispSizeScale(nw4r::math::VEC2 &scale);

    void LayoutDispNumber(const int &value, const int &fillLeft, LytTextBox_c *textBox, bool fillWidth);

    bool isNowCourseClear();

    void initGame();
    void AreaLanguageFolder(const char *, char *);
}

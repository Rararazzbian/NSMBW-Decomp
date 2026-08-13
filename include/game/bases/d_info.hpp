#pragma once
#include <types.h>

namespace nw4r {
namespace lyt {
class Pane;
class DrawInfo;
} // namespace lyt
} // namespace nw4r
#include <game/bases/d_cyuukan.hpp>
#include <game/mLib/m_vec.hpp>
#include <constants/game_constants.h>

class dInfo_c {
public:
    /// @unofficial
    enum GameMode_e {
        GAME_MODE_NORMAL,
        GAME_MODE_SUPER_GUIDE,
        GAME_MODE_TITLE,
        GAME_MODE_TITLE_REPLAY,
        GAME_MODE_HINT_MOVIE
    };

    struct StartGameInfo_s {
        u32 mReplayDuration;
        u8 mMovieType;
        u8 mEntrance;
        u8 mArea;
        bool mIsReplay;
        GameMode_e mGameMode;
        u8 mWorld1;
        u8 mLevel1;
        u8 mWorld2;
        u8 mLevel2;
    };

    /// @unofficial
    /// @todo Fill out this enum.
    enum GAME_FLAG_e {
        GAME_FLAG_DISABLE_ACTOR_CREATE = BIT_FLAG(0), ///< Disables map actor creation.
        GAME_FLAG_MULTIPLAYER_MODE = BIT_FLAG(4), ///< Whether the game is in a multiplayer mode.
        GAME_FLAG_IS_FREE_MODE = BIT_FLAG(5), ///< Whether the game is in Free Mode.
        GAME_FLAG_IS_COIN_BATTLE = BIT_FLAG(6), ///< Whether the game is in Coin Battle mode.
        GAME_FLAG_AUTO_SKIP = BIT_FLAG(19), ///< Whether to automatically skip the Wii strap and controller information screens.
    };

    struct enemy_s {
        int mSubworld;
        int mPathIndex;
        PATH_DIRECTION_e mWalkDirection;
        bool m_0c;
    };

    dInfo_c();

    void GetMapEnemyInfo(int, int, enemy_s &);
    void SetMapEnemyInfo(int, int, int, int);
    void FUN_800bbc40(int, int, int);

    u8 getCourse() const { return m_startGameInfo.mLevel1; }
    u8 getWorld() const { return m_startGameInfo.mWorld1; }
    dCyuukan_c *getCyuukan() { return &mCyuukan; }

    static dInfo_c *getInstance() { return m_instance; }

    char pad1[0x8];
    dCyuukan_c mCyuukan;
    int mCurrentCourseWorld;
    int mCurrentCourseNo;
    int mCurrentCourseNode;
    char pad2[0xc];
    int m_54;
    u8 pad3[0x8];
    int m_60;
    /// @brief [0x64] and [0x68]. Were hidden inside `pad4[0x8]`; both are plain
    /// `stw`s of small int/bool-like values from daPyDemoMng_c::executeGoalCastle.
    /// The pad is now fully accounted for, with nothing left over. @unofficial
    int m_64;
    int m_68;
    bool m_6c;
    u8 pad5[0x2c];
    int m_9c;
    u8 pad6[0x2e4];
    int mCharIDs[4];
    bool mIsWorldSelect; ///< Whether the World Select Menu is being displayed.
    u8 pad7[0x1e];
    bool mClearCyuukan; ///< Clear the checkpoint data if this is @p true. [Used for the backdoor entrance of 7-C]
    int mDisplayCourseWorld;
    int mDisplayCourseNum;
    int mTotalCollectionCoin;              ///< 0x3bc The total collection coin count. @unofficial
    int mSaveFileNumber;                   ///< 0x3c0 The save file number. @unofficial
    int mPlayerNum;                        ///< 0x3c4 The number of players. @unofficial
    int mScissorIndex;                     ///< 0x3c8 The scissor stack index. @unofficial
    int mPlayNumber;                       ///< 0x3cc The play count. @unofficial
    int mTextBoxMessageGroup;
    int mTextBoxMessageID;
    u8 pad9[0x1];
    bool mExtensionAttached;
    u8 m_3da;
    u8 pad10[0x1];
    nw4r::lyt::Pane *mScissorPane;         ///< 0x3dc The pane the scissor box applies to. @unofficial
    nw4r::lyt::DrawInfo *mScissorDrawInfo; ///< 0x3e0 That pane's draw info. @unofficial
    int mCourseSelectPageNum;
    int mCourseSelectIndexInPage;
    u8 pad11[0x712];
    bool mFukidashiActionPerformed[4][0x16];
    u32 pad12;

    static dInfo_c *m_instance;
    static unsigned int mGameFlag; ///< See GAME_FLAG_e
    static StartGameInfo_s m_startGameInfo;
};

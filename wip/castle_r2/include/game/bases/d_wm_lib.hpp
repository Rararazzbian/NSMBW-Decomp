#pragma once
#include <constants/game_constants.h>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_3d/bmdl.hpp>
#include <game/bases/d_wm_csv_data.hpp>

// Shadow copy of the real include/game/bases/d_wm_lib.hpp, plus four proposed additions (see
// below) used by d_a_wm_castle.cpp. Everything else is an unmodified copy of the real header.
//
// The KoopaShipStopConfig_t / sc_KoopaShipStopConfig / GetKoopaShipStopPos experiment that used
// to live at the bottom of this file (five shapes tried against fn_2_15FAA0 and the
// .data/.rodata/.bss shortfall -- see this task's earlier reports) has MOVED to
// d_a_wm_castle.cpp itself. The target's fn_2_15F8F0/fn_2_15F950/fn_2_15FAA0 all carry GLOBAL
// (not WEAK) ELF symbol binding, checked directly against the object file's .symtab -- and
// inline functions are ALWAYS weak (that is the entire point of the keyword: it lets identical
// per-TU copies coexist and be linker-folded). A GLOBAL symbol is defined EXACTLY ONCE in the
// whole program, so whatever those three functions read cannot live in a shared, included
// header; it has to be castle-local. See this task's report for the binding evidence.
namespace dWmLib {
    enum Direction3D_e {
        DIR3D_UP,
        DIR3D_DOWN,
        DIR3D_FRONT,
        DIR3D_BACK,
        DIR3D_LEFT,
        DIR3D_RIGHT
    };

    /// @unofficial
    enum CourseType_e {
        COURSE_TYPE_NORMAL,
        COURSE_TYPE_GHOST,
        COURSE_TYPE_TOWER,
        COURSE_TYPE_CASTLE,
        COURSE_TYPE_KINOKO,
        COURSE_TYPE_JUNCTION,
        COURSE_TYPE_CANNON,
        COURSE_TYPE_STAGE_37,
        COURSE_TYPE_KOOPA_SHIP,
        COURSE_TYPE_KINOKO_START,
        COURSE_TYPE_PEACH_CASTLE,
        COURSE_TYPE_INVALID
    };

    /// @unofficial
    enum PointType_e {
        POINT_TYPE_INTERSECTION,
        POINT_TYPE_PATH,
        POINT_TYPE_START_NODE,
        POINT_TYPE_REGULAR_COURSE,
        POINT_TYPE_OTHER
    };

    struct ForceInCourseList_t {
        int mNodeWorld;
        const char *mNodeName;
        int mWorld;
        int mLevel;
        int mEntrance;
        const char *mLevelNode;
        mVec3_c mNodePos;
    };

    int GetCourseTypeFromCourseNo(int courseNo);
    int GetCourseNoFromPointName(const char *pointName);
    void GetStartPointNameFromRouteName(const char *routeName, char *dst);
    void GetEndPointNameFromRouteName(const char *routeName, char *dst);
    bool isKoopaShipAnchor();
    u8 getStartPointKinokoHouseKindNum();
    bool isStartPointKinokoHouseStar();
    bool isStartPointKinokoHouseRed();

    int GetOpenStatus(int world, int course);
    int GetClearStatus(int world, int course);
    int GetCurrentPlayResultStatus();
    int GetCurrentPlayResultStatus(int world, int course, int pathNode);

    bool IsCourseOmoteClear(int world, int course);
    bool IsCourseUraClear(int world, int course);
    bool IsCourseOtasukeClear(int world, int course);
    bool IsCourseClear(int world, int course);
    bool IsCourseFirstOmoteClear(int world, int course, int pathNode);
    bool IsCourseFirstUraClear(int world, int course, int pathNode);
    bool IsCourseFailed(int world, int course, int pathNode);
    bool IsCourseFirstClear(int world, int course);
    bool IsCourseOtasukeClearSimple(int world, int course);
    bool IsCourseOmoteClearSimple(int world, int course);
    bool IsCourseUraClearSimple(int world, int course);
    bool IsCourseUraOtasukeClearSimple(int world, int course);

    int getPointDir(const mVec3_c &v1, const mVec3_c &v2);
    int getEnemyRevivalCount(int, int);

    nw4r::math::VEC3 GetModelNodePos(const m3d::bmdl_c *model, int nodeId);

    // Proposed additions used by d_a_wm_castle.cpp -- NOT part of the real header today.
    nw4r::math::VEC3 GetModelNodePos(const m3d::bmdl_c *model, const char *nodeName);
    bool hasKoopaShipStop();
    bool isKoopaShipOnCurrentWorld();
    bool isSpecialWorld();

    static ForceInCourseList_t sc_ForceList[] = {
        {WORLD_7, "F7C0", WORLD_7, dCsvData_c::c_CASTLE_ID, 4, "W7C0", mVec3_c(2160.0f, -30.0f, -478.0f)}
    };

    static int c_StartPointKinokoHouseID = dCsvData_c::c_START_ID;
};

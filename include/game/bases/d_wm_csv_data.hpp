#pragma once
#include <types.h>

/// @brief World map route data, parsed from the world's CSV file.
class dCsvData_c {
public:
    /// @brief Information about a single world map point. @unofficial
    struct PointData_t {
        char mPointName[5];                 ///< The name of this point.
        char mOpenPointName[4][5];          ///< Names of the points this point opens up.
        char mOpenRouteName[16][10];        ///< Names of the routes this point opens up.
        char mOpenPointNameSecret[4][5];    ///< Names of the points this point opens up (secret exit).
        char mOpenRouteNameSecret[16][10];  ///< Names of the routes this point opens up (secret exit).
        int mOpenPointNum;                  ///< The amount of points this point opens up.
        int mOpenRouteNum;                  ///< The amount of routes this point opens up.
        int mOpenRouteNumSecret;            ///< The amount of routes this point opens up (secret exit).
        int mOpenPointNumSecret;            ///< The amount of points this point opens up (secret exit).
        char mAnimeRouteName[2][10];        ///< Names of the routes animated by this point.
        char mAnimeRouteNameSecret[2][10];  ///< Names of the routes animated by this point (secret exit).
        int mAnimeRouteNum;                 ///< The amount of routes animated by this point.
        int mAnimeRouteNumSecret;           ///< The amount of routes animated by this point (secret exit).
        u32 mAction;                        ///< The actions triggered by this point.
        u8 mPointParam;                     ///< The point's parameter.
        u32 mRouteFlag;                     ///< The point's route flags.
        u8 mPointFlag;                      ///< The point's flags.
    };

    /// @brief Information about a single world map sub route. @unofficial
    struct SubRouteData_t {
        char mPointName[10];      ///< The names of the points the sub route starts and ends at.
        int mRouteIdx;            ///< The index of the route this sub route belongs to.
        int mPointIdx;            ///< The index of the sub route's first point.
        u8 mFlag;                 ///< The sub route's flags.
    };

    /// @brief Information about a single world map route. @unofficial
    struct RouteData_t {
        /// @brief Frees the route's child point name buffer.
        void deleteChildPointName();

        char mPointName[10];      ///< The names of the points the route starts and ends at.
        char *mChildPointName;    ///< The names of the points making up the route.
        int mPointNum;            ///< The amount of points making up the route.
        int mCost;                ///< The route's traversal cost.
    };

    virtual ~dCsvData_c();

    void initialize(int worldNo, int fileNo);
    void RouteInfoInit();
    void ReadCsvData();
    const char *GetPointName(int) const;

    int mWorldNo;                        ///< The world this data belongs to.
    int mFileNo;                         ///< The index of the CSV file this data was read from.
    int mPointNum;                       ///< The amount of points read.
    int mSubRouteNum;                    ///< The amount of sub routes read.
    int mRouteNum;                       ///< The amount of routes read.
    PointData_t mPointData[192];         ///< The point data.
    SubRouteData_t mSubRouteData[160];   ///< The sub route data.
    RouteData_t mRouteData[64];          ///< The route data.

public:
    static const int c_COURSE_ID;
    static const int c_GHOST_ID;
    static const int c_TOWER_ID;
    static const int c_CASTLE_ID;
    static const int c_KINOKO_ID;
    static const int c_ENEMY_ID;
    static const int c_CANON_ID;
    static const int c_TRSHIP_ID;
    static const int c_AIRSHIP_ID;
    static const int c_START_ID;
    static const int c_PEACH_ID;
};

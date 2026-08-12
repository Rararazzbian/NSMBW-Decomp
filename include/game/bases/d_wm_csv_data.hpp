#pragma once
#include <types.h>

namespace m3d { class mdl_c; }
namespace nw4r { namespace g3d { class ResNode; } }

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
        /// @brief Allocates and clears the route's child point name buffer. @unofficial
        void newChildPointName(int num);

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

    /// @brief Reads the point's name field.
    void ReadPointName(char *csv, int &pos);

    /// @brief Reads the point's type field, which sets the action and route flags.
    void ReadPointType(char *csv, int &pos);

    /// @brief Reads the names of the points this point opens up.
    /// @param normal Whether to read into the normal fields, rather than the secret exit ones.
    void ReadOpenPointName(char *csv, int &pos, bool normal);

    /// @brief Reads the names of the routes this point opens up.
    /// @param normal Whether to read into the normal fields, rather than the secret exit ones.
    void ReadOpenRouteName(char *csv, int &pos, bool normal);

    /// @brief Reads the point's flag data field.
    /// @param normal Whether to read into the normal fields, rather than the secret exit ones.
    void ReadFlagData(char *csv, int &pos, bool normal);

    /// @brief Reads the name of the route animated by this point.
    void ReadAnimeRouteName(char *csv, int &pos);

    /// @brief Reads the sub route's action field.
    void ReadAction(char *csv, int &pos);

    /// @brief Reads the sub route's flag field.
    void ReadRouteFlag(char *csv, int &pos);

    /// @brief Whether the given position is at a CRLF line terminator.
    bool isLineEnd(char *csv, int pos);

    const char *GetPointName(int) const;

    /// @brief Gets the action of the sub route with the given point name pair.
    int GetActionLabel(const char *pointName);

    /// @brief Gets the index of the point with the given name.
    int GetIndexFromPointName(const char *pointName);

    /// @brief Records the world map's key points from the given model. @unofficial
    void addKeyPoint(const m3d::mdl_c &mdl);

    /// @brief Builds the route information from the given model. @unofficial
    void SetRouteInfo(const m3d::mdl_c &mdl);

    /// @brief Gets the name of a point the given point opens up.
    const char *GetOpenPointName(bool, int, int) const;

    /// @brief Gets the amount of points the given point opens up.
    int GetOpenPointNum(bool, int) const;

    /// @brief Gets the name of a route the given point opens up.
    const char *GetOpenRouteName(bool, int, int) const;

    /// @brief Gets the amount of routes the given point opens up.
    int GetOpenRouteNum(bool, int) const;

    /// @brief Gets the given point's actions, masked by @p action . @unofficial
    u32 GetAction(int, u32);

    /// @brief Gets the given point's route flags, masked by @p flag . @unofficial
    u32 GetRouteFlag(int, u32);

    /// @brief Gets the given point's parameter.
    u8 GetPointParam(int);

    /// @brief Gets the given route's point name pair.
    char *GetRouteName(int);

    /// @brief Gets the name of a point making up the given route.
    char *GetChildPointName(int, int);

    /// @brief Gets the given sub route's point name pair.
    char *GetSubRouteName(int);

    /// @brief Gets the index of the sub route connecting the two given points.
    int GetSubRouteIdx(const char *pointName1, const char *pointName2);

    /// @brief Gets the amount of points making up the given route.
    int GetPointNum(int routeNo);

    /// @brief Gets the given sub route's flags.
    u8 GetSubRouteFlag(int subRouteNo);

    /// @brief Gets the amount of routes animated by the given point.
    int GetRouteAnimNum(bool normal, int pointNo);

    /// @brief Gets the name of a route animated by the given point.
    const char *GetRouteAnimName(bool normal, int pointNo, int animNo);

    /// @brief Recursively walks the sub routes to record the chain of points from
    ///        @p startName to @p endName into @p route . @unofficial
    bool SearchChildPointName(const char *startName, const char *endName, RouteData_t *route,
                              int maxDepth, bool reset);

    /// @brief Recursively walks the sub routes to find the cheapest chain of points
    ///        from @p fromName to @p toName , recording its cost. @unofficial
    bool SearchRouteCost(const char *fromName, const char *toName, RouteData_t *route,
                         int maxDepth, bool isFirst);

    /// @brief Fills the route's child point names from the model node's child chain.
    void appendChildFromModel(const nw4r::g3d::ResNode &node, int routeNo);

    /// @brief Reverses the order of the route's child point names. @unofficial
    void reverseChildPointName(RouteData_t *route);

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

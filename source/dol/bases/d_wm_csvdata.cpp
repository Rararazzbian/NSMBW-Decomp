#include <game/bases/d_wm_csv_data.hpp>

/// @brief Reads a single comma-separated field out of a CSV line.
/// @param dst The buffer the field is copied into.
/// @param src The position in the CSV line to read from.
/// @return The length of the field.
int read(char *dst, const char *src, int len) {
    int i = 0;

    if (*src == '"') {
        src++;
        if (*src == '"') {
            return 0;
        }
    }

    if (*src == ',') {
        return 0;
    }

    while (true) {
        if (*src == ',' || *src == '"' || (*src == '\r' && src[1] == '\n')) {
            dst[i] = '\0';
            break;
        }

        dst[i] = *src;
        i++;
        src++;
    }

    return i;
}

dCsvData_c::~dCsvData_c() {
    for (int i = 0; i < 64; i++) {
        if (mRouteData[i].mChildPointName != nullptr) {
            mRouteData[i].deleteChildPointName();
        }
    }
}

void dCsvData_c::initialize(int worldNo, int fileNo) {
    mWorldNo = worldNo;
    mFileNo = fileNo;

    RouteInfoInit();
    ReadCsvData();
}

void dCsvData_c::RouteInfoInit() {
    mPointNum = 0;
    mSubRouteNum = 0;
    mRouteNum = 0;

    for (int i = 0; i < 192; i++) {
        for (int j = 0; j < 5; j++) {
            mPointData[i].mPointName[j] = '\0';
        }

        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                mPointData[i].mOpenPointName[j][k] = '\0';
                mPointData[i].mOpenPointNameSecret[j][k] = '\0';
            }
        }

        /// [The loop bound is the route count, not the size of the arrays below (16 entries each),
        /// so this writes past the end of both. Harmless, as the overrun lands on entries that are
        /// zeroed by a later iteration or by the loops below.]
        for (int j = 0; j < 64; j++) {
            for (int k = 0; k < 10; k++) {
                mPointData[i].mOpenRouteName[j][k] = '\0';
                mPointData[i].mOpenRouteNameSecret[j][k] = '\0';
            }
        }

        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 10; k++) {
                mPointData[i].mAnimeRouteName[j][k] = '\0';
                mPointData[i].mAnimeRouteNameSecret[j][k] = '\0';
            }
        }

        mPointData[i].mAnimeRouteNum = 0;
        mPointData[i].mAnimeRouteNumSecret = 0;
        mPointData[i].mAction = 0;
        mPointData[i].mPointParam = 0;
        mPointData[i].mRouteFlag = 0;
        mPointData[i].mPointFlag = 0;
    }

    for (int i = 0; i < 160; i++) {
        for (int j = 0; j < 10; j += 5) {
            for (int k = 0; k < 5; k++) {
                mSubRouteData[i].mPointName[j + k] = '\0';
            }
        }

        mSubRouteData[i].mRouteIdx = -1;
        mSubRouteData[i].mPointIdx = 0;
        mSubRouteData[i].mFlag = 0;
    }

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 10; j += 5) {
            for (int k = 0; k < 5; k++) {
                mRouteData[i].mPointName[j + k] = '\0';
            }
        }

        mRouteData[i].mChildPointName = nullptr;
        mRouteData[i].mCost = 99999;
        mRouteData[i].mPointNum = 0;
    }
}

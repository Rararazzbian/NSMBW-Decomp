#include <game/bases/d_wm_csv_data.hpp>

#include <cstdio>
#include <cstdlib>
#include <game/bases/d_res_mng.hpp>
#include <string.h>

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

static const char *scArcName = "RouteInfo";

void dCsvData_c::ReadCsvData() {
    dResMng_c *resMng = dResMng_c::m_instance;
    char path[0x20];
    unsigned long size;
    int pos;
    char *routeCsv;

    snprintf(path, 0x20, "W%X/pointW%X.csv", mWorldNo + 1, mWorldNo + 1);
    char *csv = (char *)resMng->getResSilently(scArcName, path, &size).ptr();

    if (csv == nullptr) {
        snprintf(path, 0x20, "W%X/pointW%Xa.csv", mWorldNo + 1, mWorldNo + 1);
        path[10] += (char)mFileNo;
        csv = (char *)resMng->getRes(scArcName, path, &size).ptr();
    }

    pos = 0;
    do {
        while (csv[pos] != ',') {
            pos++;
        }
        pos++;

        ReadPointName(csv, pos);
        if (isLineEnd(csv, pos)) goto nextPoint;
        pos++;
        if (isLineEnd(csv, pos)) goto nextPoint;

        ReadPointType(csv, pos);
        if (isLineEnd(csv, pos)) goto nextPoint;
        if (isLineEnd(csv, pos)) goto nextPoint;

        ReadOpenPointName(csv, pos, true);
        if (isLineEnd(csv, pos)) goto nextPoint;
        pos++;
        if (isLineEnd(csv, pos)) goto nextPoint;

        ReadOpenRouteName(csv, pos, true);
        if (isLineEnd(csv, pos)) goto nextPoint;
        pos++;
        if (isLineEnd(csv, pos)) goto nextPoint;

        ReadFlagData(csv, pos, true);
        if (isLineEnd(csv, pos)) goto nextPoint;
        if (isLineEnd(csv, pos)) goto nextPoint;

        ReadOpenPointName(csv, pos, false);
        if (isLineEnd(csv, pos)) goto nextPoint;
        pos++;
        if (isLineEnd(csv, pos)) goto nextPoint;

        ReadOpenRouteName(csv, pos, false);
        if (isLineEnd(csv, pos)) goto nextPoint;
        pos++;
        if (isLineEnd(csv, pos)) goto nextPoint;

        ReadFlagData(csv, pos, false);

    nextPoint:
        mPointNum++;
        pos += 2;
    } while (pos != size);

    snprintf(path, 0x20, "W%X/routeW%X.csv", mWorldNo + 1, mWorldNo + 1);
    void *res = resMng->getResSilently(scArcName, path, &size).ptr();

    if (res == nullptr) {
        snprintf(path, 0x20, "W%X/routeW%Xa.csv", mWorldNo + 1, mWorldNo + 1);
        path[10] += (char)mFileNo;
        res = resMng->getRes(scArcName, path, &size).ptr();
    }

    pos = 0;
    routeCsv = (char *)res;
    do {
        ReadAnimeRouteName(routeCsv, pos);
        pos++;
        ReadAction(routeCsv, pos);
        pos++;
        ReadRouteFlag(routeCsv, pos);

        if (mSubRouteData[mSubRouteNum].mPointName[0] != '0') {
            mSubRouteNum++;
        }

        pos += 2;
    } while (pos != size);
}

void dCsvData_c::ReadPointName(char *csv, int &pos) {
    pos += read(mPointData[mPointNum].mPointName, &csv[pos], 5);
}

void dCsvData_c::ReadPointType(char *str, int &pos) {
    GetPointName(mPointNum);

    if (str[pos] == '"' && str[pos + 1] == '"') {
        pos += 3;
        return;
    }

    if (str[pos] == ',') {
        pos++;
        return;
    }

    int quoted = 0;
    if (str[pos] == '"') {
        quoted = 1;
        pos++;
    }

    char buf[32];

    while (true) {
        int i = 0;

        while (true) {
            if (str[pos] == ',' || str[pos] == '"' || isLineEnd(str, pos)) {
                buf[i] = '\0';
                break;
            }

            buf[i] = str[pos];
            i++;
            pos++;
        }

        if (strcmp(buf, "ura") == 0) {
            mPointData[mPointNum].mAction |= 0x1;
        } else if (strcmp(buf, "stop") == 0) {
            mPointData[mPointNum].mAction |= 0x2;
        } else if (strcmp(buf, "link1") == 0) {
            mPointData[mPointNum].mAction |= 0x4;
        } else if (strcmp(buf, "link2") == 0) {
            mPointData[mPointNum].mAction |= 0x8;
        } else if (strcmp(buf, "link3") == 0) {
            mPointData[mPointNum].mAction |= 0x10;
        } else if (strcmp(buf, "link4") == 0) {
            mPointData[mPointNum].mAction |= 0x20;
        } else if (strcmp(buf, "link5") == 0) {
            mPointData[mPointNum].mAction |= 0x40;
        } else if (strcmp(buf, "scroll") == 0) {
            mPointData[mPointNum].mAction |= 0x80;
        } else if (strcmp(buf, "scale") == 0) {
            mPointData[mPointNum].mAction |= 0x100;
        } else if (strcmp(buf, "dokan") == 0) {
            mPointData[mPointNum].mAction |= 0x400;
        } else if (strcmp(buf, "switch") == 0) {
            mPointData[mPointNum].mAction |= 0x800;
        } else if (strcmp(buf, "crossroad") == 0) {
            mPointData[mPointNum].mAction |= 0x40000;
        } else if (strcmp(buf, "focus") == 0) {
            mPointData[mPointNum].mAction |= 0x80000;
        } else if (strncmp(buf, "anchor", 6) == 0) {
            int len = strlen(buf);

            if (len == 6) {
                mPointData[mPointNum].mAction |= 0x1000;
                mPointData[mPointNum].mAction |= 0x2000;
            } else if (len == 7 || len == 8) {
                if (buf[6] == 'x') {
                    mPointData[mPointNum].mAction |= 0x1000;
                } else if (buf[6] == 'y') {
                    mPointData[mPointNum].mAction |= 0x2000;
                } else if ('1' <= buf[6] && buf[6] <= '9') {
                    mPointData[mPointNum].mPointParam = atoi(&buf[6]);
                }

                if ('1' <= buf[7] && buf[7] <= '9') {
                    mPointData[mPointNum].mPointParam = atoi(&buf[7]);
                }
            }
        } else if (strcmp(buf, "tilt") == 0) {
            mPointData[mPointNum].mAction |= 0x4000;
        } else if (strcmp(buf, "demo1") == 0) {
            mPointData[mPointNum].mAction |= 0x8000;
        } else if (strcmp(buf, "demo2") == 0) {
            mPointData[mPointNum].mAction |= 0x10000;
        } else if (strcmp(buf, "demo3") == 0) {
            mPointData[mPointNum].mAction |= 0x20000;
        } else if (strcmp(buf, "demo4") == 0) {
            mPointData[mPointNum].mAction |= 0x2000000;
        } else if (strcmp(buf, "demo5") == 0) {
            mPointData[mPointNum].mAction |= 0x4000000;
        } else if (strcmp(buf, "demo6") == 0) {
            mPointData[mPointNum].mAction |= 0x8000000;
        } else if (strcmp(buf, "demo7") == 0) {
            mPointData[mPointNum].mAction |= 0x10000000;
        } else if (strcmp(buf, "camstop") == 0) {
            mPointData[mPointNum].mAction |= 0x20000000;
        } else if (strcmp(buf, "noshift") == 0) {
            mPointData[mPointNum].mAction |= 0x40000000;
        } else if (strncmp(buf, "board", 5) == 0) {
            mPointData[mPointNum].mAction |= 0x100000;
        } else if (strcmp(buf, "demostop") == 0) {
            mPointData[mPointNum].mAction |= 0x200000;
        } else if (strcmp(buf, "scrollY") == 0) {
            mPointData[mPointNum].mAction |= 0x400000;
        } else if (strcmp(buf, "sand") == 0) {
            mPointData[mPointNum].mAction |= 0x800000;
        } else if (strcmp(buf, "ice") == 0) {
            mPointData[mPointNum].mAction |= 0x1000000;
        } else if (strcmp(buf, "scrollA") == 0) {
            mPointData[mPointNum].mAction |= 0x80000000;
        }

        if (strcmp(buf, "kuribo1") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x2;
        } else if (strcmp(buf, "kuribo2") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x4;
        } else if (strcmp(buf, "Puku1") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x8;
        } else if (strcmp(buf, "Puku2") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x10;
        } else if (strcmp(buf, "Pak1") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x20;
        } else if (strcmp(buf, "Pak2") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x40;
        } else if (strcmp(buf, "Pak3") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x80;
        } else if (strcmp(buf, "Hbros1") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x100;
        } else if (strcmp(buf, "trap1") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x400;
        } else if (strcmp(buf, "trap2") == 0) {
            mPointData[mPointNum].mRouteFlag |= 0x800;
        }

        if (str[pos] == '"') {
            pos++;
            break;
        }

        if (quoted == 0 && str[pos] == ',') {
            break;
        }

        if (isLineEnd(str, pos)) {
            break;
        }

        pos++;
    }

    if (!isLineEnd(str, pos)) {
        pos++;
    }
}

void dCsvData_c::ReadOpenPointName(char *line, int &pos, bool normal) {
    int num = 0;

    if (line[pos] == '"' && line[pos + 1] == '"') {
        pos += 2;
    } else if (line[pos] == '"') {
        pos++;

        do {
            for (int i = 0; i < 4; i++) {
                if (line[pos] != ',') {
                    if (normal) {
                        mPointData[mPointNum].mOpenPointName[num][i] = line[pos];
                    } else {
                        mPointData[mPointNum].mOpenPointNameSecret[num][i] = line[pos];
                    }
                    pos++;
                }
            }

            num++;
        } while (line[pos++] != '"');

    } else if (line[pos] != ',') {
        do {
            for (int i = 0; i < 4; i++) {
                if (line[pos] != ',') {
                    if (normal) {
                        mPointData[mPointNum].mOpenPointName[num][i] = line[pos];
                    } else {
                        mPointData[mPointNum].mOpenPointNameSecret[num][i] = line[pos];
                    }
                    pos++;
                }
            }

            num++;

            while (line[pos] != ',') {
                if (isLineEnd(line, pos)) {
                    break;
                }
                pos++;
            }

            if (line[pos] == ',') {
                break;
            }
        } while (!isLineEnd(line, pos));
    }

    if (normal) {
        mPointData[mPointNum].mOpenPointNum = num;
    } else {
        mPointData[mPointNum].mOpenPointNumSecret = num;
    }
}

void dCsvData_c::ReadOpenRouteName(char *line, int &pos, bool normal) {
    int num = 0;

    if (line[pos] == '"' && line[pos + 1] == '"') {
        pos += 2;
    } else if (line[pos] == '"') {
        pos++;

        do {
            for (int i = 0; i < 9; i++) {
                if (line[pos] != ',' && line[pos] != '"') {
                    if (line[pos] != ' ') {
                        if (normal) {
                            mPointData[mPointNum].mOpenRouteName[num][i] = line[pos];
                        } else {
                            mPointData[mPointNum].mOpenRouteNameSecret[num][i] = line[pos];
                        }
                    }
                    pos++;
                }
            }

            num++;
        } while (line[pos++] != '"');

    } else if (line[pos] != ',') {
        do {
            for (int i = 0; i < 9; i++) {
                if (line[pos] != ',' && line[pos] != '"') {
                    if (line[pos] != ' ') {
                        if (normal) {
                            mPointData[mPointNum].mOpenRouteName[num][i] = line[pos];
                        } else {
                            mPointData[mPointNum].mOpenRouteNameSecret[num][i] = line[pos];
                        }
                    }
                    pos++;
                }
            }

            num++;

            while (line[pos] != ',') {
                if (isLineEnd(line, pos)) {
                    break;
                }
                pos++;
            }

            if (line[pos] == ',') {
                break;
            }
        } while (!isLineEnd(line, pos));
    }

    if (normal) {
        mPointData[mPointNum].mOpenRouteNum = num;
    } else {
        mPointData[mPointNum].mOpenRouteNumSecret = num;
    }
}

void dCsvData_c::ReadFlagData(char *csv, int &offset, bool secret) {
    int j;
    int k;
    bool quote = false;

    if (csv[offset] == '"') {
        offset++;
        quote = true;
    }

    for (j = 0; ; j += 10) {
        k = 0;

        while (true) {
            if (csv[offset] == ',' || csv[offset] == '"' ||
                (csv[offset] == '\r' && csv[offset + 1] == '\n')) {
                if (secret) {
                    mPointData[mPointNum].mAnimeRouteName[0][j + k] = '\0';
                } else {
                    mPointData[mPointNum].mAnimeRouteNameSecret[0][j + k] = '\0';
                }

                if (k > 0) {
                    if (secret) {
                        mPointData[mPointNum].mAnimeRouteNum++;
                    } else {
                        mPointData[mPointNum].mAnimeRouteNumSecret++;
                    }
                }

                break;
            }

            if (secret) {
                mPointData[mPointNum].mAnimeRouteName[0][j + k] = csv[offset];
            } else {
                mPointData[mPointNum].mAnimeRouteNameSecret[0][j + k] = csv[offset];
            }

            k++;
            offset++;
        }

        if (csv[offset] == '"') {
            offset++;
            return;
        }

        if ((!quote && csv[offset] == ',') || (csv[offset] == '\r' && csv[offset + 1] == '\n')) {
            if (k == 0 && csv[offset] == ',') {
                offset++;
            }
            return;
        }

        offset++;
    }
}

void dCsvData_c::ReadAnimeRouteName(char *csv, int &offset) {
    char name[24];
    char *dst = mSubRouteData[mSubRouteNum].mPointName;

    int len = read(name, &csv[offset], 10);
    bool found = false;

    for (int i = 0; i < mSubRouteNum - 1; i++) {
        if (strncmp(mSubRouteData[i].mPointName, name, 10) == 0) {
            found = true;
        }
    }

    if (!found) {
        strncpy(dst, name, 10);
    }

    offset += len;
}

void dCsvData_c::ReadAction(char *csv, int &offset) {
    static const int c_ACTION_NAME_LEN = 20;

    static const char *l_actionName[] = {
        "道", "砂", "木", "ジャンプ", "はしご",
        "ツタ", "坂", "氷坂", "スイッチブロック",
        "流砂", "雪", "氷", "雲", "水", "はしご右",
        "はしご左", "はしご岩", "はしご縄", "土",
        nullptr
    };

    static const u32 l_actionType[] = {
        2, 15, 26, 4, 3, 6, 7, 8, 14, 16, 17, 18, 19, 20, 22, 21, 24, 25, 23
    };

    char name[24];

    offset += read(name, &csv[offset], c_ACTION_NAME_LEN);

    if (mSubRouteData[mSubRouteNum].mPointName[0] != '0') {
        for (int i = 0; l_actionName[i] != nullptr; i++) {
            if (strcmp(l_actionName[i], name) == 0) {
                mSubRouteData[mSubRouteNum].mRouteIdx = l_actionType[i];
                break;
            }
        }
    }
}

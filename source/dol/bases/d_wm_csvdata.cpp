#include <game/bases/d_wm_csv_data.hpp>

#include <cstdio>
#include <cstdlib>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <lib/nw4r/g3d/res/g3d_resnode.h>
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
    enum { c_ACTION_NAME_LEN = 20 };

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

void dCsvData_c::ReadRouteFlag(char *csv, int &pos) {
    if (csv[pos] == '\r' && csv[pos + 1] == '\n') {
        return;
    }

    int quoted = 0;
    if (csv[pos] == '"') {
        quoted = 1;
        pos++;
    }

    char buf[32];

    while (true) {
        int i = 0;

        while (true) {
            if (csv[pos] == ',' || csv[pos] == '"' || (csv[pos] == '\r' && csv[pos + 1] == '\n')) {
                buf[i] = '\0';
                break;
            }

            buf[i] = csv[pos];
            i++;
            pos++;
        }

        if (mSubRouteData[mSubRouteNum].mPointName[0] != '0') {
            if (strcmp(buf, "A") == 0) {
                mSubRouteData[mSubRouteNum].mFlag |= 0x1;
            } else if (strcmp(buf, "B") == 0) {
                mSubRouteData[mSubRouteNum].mFlag |= 0x2;
            } else if (strcmp(buf, "C") == 0) {
                mSubRouteData[mSubRouteNum].mFlag |= 0x4;
            }
        }

        if (csv[pos] == '"') {
            pos++;
            break;
        }

        if (quoted == 0 && csv[pos] == ',') {
            break;
        }

        if (csv[pos] == '\r' && csv[pos + 1] == '\n') {
            break;
        }

        pos++;
    }
}

int dCsvData_c::GetActionLabel(const char *name) {
    for (int i = 0; i < mSubRouteNum; i++) {
        if (strcmp(mSubRouteData[i].mPointName, name) == 0) {
            return mSubRouteData[i].mRouteIdx;
        }
    }

    return -1;
}

int dCsvData_c::GetIndexFromPointName(const char *name) {
    for (int i = 0; i < mPointNum; i++) {
        if (strcmp(mPointData[i].mPointName, name) == 0) {
            return i;
        }
    }

    return -1;
}

void dCsvData_c::addKeyPoint(const m3d::mdl_c &mdl) {
    nw4r::g3d::ResMdl resMdl = mdl.getResMdl();

    for (int i = 0; i < resMdl.GetResNodeNumEntries(); i++) {
        const char *name = resMdl.GetResNode(i).GetName();

        if (name[0] == 'K') {
            strncpy(mPointData[mPointNum].mPointName, name, 5);
            mPointNum++;
        }
    }
}

void dCsvData_c::SetRouteInfo(const m3d::mdl_c &mdl) {
    char startName[5];
    char endName[5];
    nw4r::g3d::ResMdl resMdl = mdl.getResMdl();

    for (int i = 0; i < resMdl.GetResNodeNumEntries(); i++) {
        nw4r::g3d::ResNode node = resMdl.GetResNode(i);
        const char *name = node.GetName();

        if (strlen(name) == 9 && name[0] == 'R' && name[4] >= '0' && name[4] <= '9' &&
            name[8] >= '0' && name[8] <= '9') {

            strncpy(mRouteData[mRouteNum].mPointName, name, 10);
            dWmLib::GetStartPointNameFromRouteName(mRouteData[mRouteNum].mPointName, startName);
            dWmLib::GetEndPointNameFromRouteName(mRouteData[mRouteNum].mPointName, endName);

            mRouteData[mRouteNum].newChildPointName(20);
            SearchRouteCost(startName, endName, &mRouteData[mRouteNum], 20, true);
            SearchChildPointName(startName, endName, &mRouteData[mRouteNum], 20, true);

            mRouteNum++;
            reverseChildPointName(&mRouteData[mRouteNum - 1]);
            appendChildFromModel(node, mRouteNum - 1);
        }
    }
}

const char *dCsvData_c::GetPointName(int idx) const {
    return mPointData[idx].mPointName;
}

const char *dCsvData_c::GetOpenPointName(bool normal, int idx, int no) const {
    if (normal) {
        return mPointData[idx].mOpenPointName[no];
    } else {
        return mPointData[idx].mOpenPointNameSecret[no];
    }
}

int dCsvData_c::GetOpenPointNum(bool normal, int idx) const {
    if (normal) {
        return mPointData[idx].mOpenPointNum;
    } else {
        return mPointData[idx].mOpenPointNumSecret;
    }
}

const char *dCsvData_c::GetOpenRouteName(bool normal, int idx, int no) const {
    if (normal) {
        return mPointData[idx].mOpenRouteName[no];
    } else {
        return mPointData[idx].mOpenRouteNameSecret[no];
    }
}

int dCsvData_c::GetOpenRouteNum(bool normal, int idx) const {
    if (normal) {
        return mPointData[idx].mOpenRouteNum;
    } else {
        return mPointData[idx].mOpenRouteNumSecret;
    }
}

u32 dCsvData_c::GetAction(int idx, u32 action) {
    return mPointData[idx].mAction & action;
}

u32 dCsvData_c::GetRouteFlag(int idx, u32 flag) {
    return mPointData[idx].mRouteFlag & flag;
}

u8 dCsvData_c::GetPointParam(int idx) {
    return mPointData[idx].mPointParam;
}

char *dCsvData_c::GetRouteName(int idx) {
    return mRouteData[idx].mPointName;
}

char *dCsvData_c::GetChildPointName(int idx, int no) {
    return &mRouteData[idx].mChildPointName[no * 5];
}

char *dCsvData_c::GetSubRouteName(int idx) {
    return mSubRouteData[idx].mPointName;
}

int dCsvData_c::GetSubRouteIdx(const char *pointName1, const char *pointName2) {
    for (int i = 0; i < mSubRouteNum; i++) {
        const char *name1 = GetSubRouteName(i) + 1;
        const char *name2 = GetSubRouteName(i) + 5;

        if ((strncmp(name1, pointName1, 4) == 0 && strncmp(name2, pointName2, 4) == 0) ||
            (strncmp(name2, pointName1, 4) == 0 && strncmp(name1, pointName2, 4) == 0)) {
            return i;
        }
    }

    return -1;
}

int dCsvData_c::GetPointNum(int routeNo) {
    return mRouteData[routeNo].mPointNum;
}

u8 dCsvData_c::GetSubRouteFlag(int subRouteNo) {
    return mSubRouteData[subRouteNo].mFlag;
}

int dCsvData_c::GetRouteAnimNum(bool normal, int pointNo) {
    if (normal) {
        return mPointData[pointNo].mAnimeRouteNum;
    }

    return mPointData[pointNo].mAnimeRouteNumSecret;
}

const char *dCsvData_c::GetRouteAnimName(bool normal, int pointNo, int animNo) {
    if (normal) {
        return mPointData[pointNo].mAnimeRouteName[animNo];
    }

    return mPointData[pointNo].mAnimeRouteNameSecret[animNo];
}

void dCsvData_c::RouteData_t::newChildPointName(int num) {
    mChildPointName = new char[num * 5];

    for (int i = 0; i < num; i++) {
        memset(&mChildPointName[i * 5], 0, 5);
    }

    mPointNum = 0;
    mCost = 99999;
}

void dCsvData_c::RouteData_t::deleteChildPointName() {
    delete[] mChildPointName;
}

bool dCsvData_c::SearchChildPointName(const char *startName, const char *endName,
                                      RouteData_t *route, int maxDepth, bool reset) {
    static int depth;

    char startBuf[5];
    char endBuf[5];
    bool found = false;

    if (reset) {
        depth = 0;
    }

    depth++;

    if (depth >= maxDepth) {
        depth--;
        return false;
    }

    for (int i = 0; i < mSubRouteNum; i++) {
        dWmLib::GetStartPointNameFromRouteName(mSubRouteData[i].mPointName, startBuf);

        if (strncmp(startBuf, startName, 4) == 0) {
            dWmLib::GetEndPointNameFromRouteName(mSubRouteData[i].mPointName, endBuf);

            if (strncmp(endBuf, endName, 4) == 0) {
                if (route->mCost == depth) {
                    found = true;
                }
                break;
            }

            if (SearchChildPointName(endBuf, endName, route, maxDepth, false)) {
                strncpy(&route->mChildPointName[route->mPointNum * 5], endBuf, 4);
                (&route->mChildPointName[route->mPointNum * 5])[5] = '0';
                found = true;
                route->mPointNum++;
                break;
            }
        }
    }

    depth--;
    return found;
}

static int l_searchCount;

bool dCsvData_c::SearchRouteCost(const char *fromName, const char *toName, RouteData_t *route,
                                 int maxDepth, bool isFirst) {
    char startName[5];
    char endName[5];

    if (isFirst) {
        l_searchCount = 0;
    }

    bool found = false;

    l_searchCount++;
    if (l_searchCount >= maxDepth) {
        l_searchCount--;
        return false;
    }

    for (int i = 0; i < mSubRouteNum; i++) {
        dWmLib::GetStartPointNameFromRouteName(mSubRouteData[i].mPointName, startName);
        if (strncmp(startName, fromName, 4) == 0) {
            dWmLib::GetEndPointNameFromRouteName(mSubRouteData[i].mPointName, endName);
            if (strncmp(endName, toName, 4) == 0) {
                if (route->mCost > l_searchCount) {
                    route->mCost = l_searchCount;
                }
                found = true;
                break;
            }

            SearchRouteCost(endName, toName, route, maxDepth, false);
        }
    }

    l_searchCount--;
    return found;
}

void dCsvData_c::appendChildFromModel(const nw4r::g3d::ResNode &node, int routeNo) {
    nw4r::g3d::ResNode child = node.GetChildNode();
    int num = 0;

    while (child.IsValid()) {
        num++;
        child = child.GetChildNode();
    }

    if (num >= mRouteData[routeNo].mPointNum) {
        nw4r::g3d::ResNode work((void *)node.ptr());
        child = work.GetChildNode();
        mRouteData[routeNo].mPointNum = 0;

        while (child.IsValid()) {
            strncpy(&mRouteData[routeNo].mChildPointName[mRouteData[routeNo].mPointNum * 5],
                    child.GetName(), 5);
            mRouteData[routeNo].mPointNum++;
            child = child.GetChildNode();
        }
    }
}

static char l_swapName[5];

void dCsvData_c::reverseChildPointName(RouteData_t *route) {
    int num = route->mPointNum;

    if (num > 1) {
        for (int i = 0; i < num / 2; i++) {
            strncpy(l_swapName, &route->mChildPointName[i * 5], 5);
            strncpy(&route->mChildPointName[i * 5], &route->mChildPointName[(num - i - 1) * 5], 5);
            strncpy(&route->mChildPointName[(num - i - 1) * 5], l_swapName, 5);
        }
    }
}

bool dCsvData_c::isLineEnd(char *csv, int pos) {
    return csv[pos] == '\r' && csv[pos + 1] == '\n';
}

const int dCsvData_c::c_COURSE_ID = 0;
const int dCsvData_c::c_GHOST_ID = 20;
const int dCsvData_c::c_TOWER_ID = 21;
const int dCsvData_c::c_CASTLE_ID = 23;
const int dCsvData_c::c_KINOKO_ID = 25;
const int dCsvData_c::c_ENEMY_ID = 32;
const int dCsvData_c::c_CANON_ID = 35;
const int dCsvData_c::c_TRSHIP_ID = 36;
const int dCsvData_c::c_AIRSHIP_ID = 37;
const int dCsvData_c::c_START_ID = 38;
const int dCsvData_c::c_PEACH_ID = 40;

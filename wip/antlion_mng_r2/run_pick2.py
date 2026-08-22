import sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import score, parse

SRC = open(os.path.join(HERE, 'd_a_wm_antlion_mng.cpp'), encoding='utf-8').read()
START = SRC.index('bool daWmAntlionMng_c::pickRevivedIndices(')
END = SRC.index('/// @unofficial fn_2_15BC30', START)
OLD = SRC[START:END]
assert len(OLD) > 500

HEAD = """bool daWmAntlionMng_c::pickRevivedIndices(int *out, int count, int worldIndex, bool excludeCurrent) {
    int candidates[9];
    for (int k = 0; k < 9; k++) {
        candidates[k] = -1;
    }

    daWmMap_c *map = daWmMap_c::m_instance;
    int foundCount = 0;
    for (int i = 0; i < 0xc0; i++) {
        if (worldIndex == 0) {
            if (map->mCsvData[map->currIdx].GetRouteFlag(i, 0x400)) {
                candidates[foundCount] = i;
                foundCount++;
            }
        } else if (worldIndex == 1) {
            if (map->mCsvData[map->currIdx].GetRouteFlag(i, 0x800)) {
                candidates[foundCount] = i;
                foundCount++;
            }
        }
    }

"""

def mk(mid, phase3):
    return HEAD + mid + phase3 + """
    return true;
}

"""

MID0 = """    int playerPoint;
    int numPicked = 0;
    if (foundCount < count) {
        return false;
    }

    playerPoint = daWmPlayer_c::ms_instance->m_22c;
    do {
        int randIdx = dGameCom::getRandom(foundCount);
        if (candidates[randIdx] > 0 && (!excludeCurrent || playerPoint != candidates[randIdx])) {
            out[numPicked] = candidates[randIdx];
            numPicked++;
            candidates[randIdx] = -1;
        }
    } while (numPicked != count);
"""
MID1 = """    int numPicked = 0;
    if (foundCount < count) {
        return false;
    }

    int playerPoint = daWmPlayer_c::ms_instance->m_22c;
    do {
        int randIdx = dGameCom::getRandom(foundCount);
        if (candidates[randIdx] > 0 && (!excludeCurrent || playerPoint != candidates[randIdx])) {
            out[numPicked] = candidates[randIdx];
            numPicked++;
            candidates[randIdx] = -1;
        }
    } while (numPicked != count);
"""
MID2 = """    int playerPoint;
    int numPicked = 0;
    if (foundCount < count) {
        return false;
    }

    int *outPtr = out;
    playerPoint = daWmPlayer_c::ms_instance->m_22c;
    do {
        int randIdx = dGameCom::getRandom(foundCount);
        if (candidates[randIdx] > 0 && (!excludeCurrent || playerPoint != candidates[randIdx])) {
            outPtr[numPicked] = candidates[randIdx];
            numPicked++;
            candidates[randIdx] = -1;
        }
    } while (numPicked != count);
"""

P3A = """
    map = daWmMap_c::m_instance;
    for (int j = 0; j < 2; j++) {
        if (dWmLib::getEnemyRevivalCount(*(int *)((char *)map + 0x3388), worldIndex * 2 + j) > 0) {
            out[j] = -1;
        }
    }
"""
P3B = """
    int j;
    daWmMap_c &map2 = *daWmMap_c::m_instance;
    for (j = 0; j < 2; j++) {
        if (dWmLib::getEnemyRevivalCount(*(int *)((char *)&map2 + 0x3388), worldIndex * 2 + j) > 0) {
            out[j] = -1;
        }
    }
"""
P3C = """
    int j;
    daWmMap_c *map2 = daWmMap_c::m_instance;
    for (j = 0; j < 2; j++) {
        if (dWmLib::getEnemyRevivalCount(*(int *)((char *)map2 + 0x3388), worldIndex * 2 + j) > 0) {
            out[j] = -1;
        }
    }
"""

V = {}
V['Q0_ctrl'] = mk(MID0, P3A)
V['Q1_p3ref'] = mk(MID0, P3B)
V['Q2_p3ptr_jfirst'] = mk(MID0, P3C)
V['Q3_ppinline'] = mk(MID1, P3A)
V['Q4_outptr'] = mk(MID2, P3A)
V['Q5_best'] = mk(MID1, P3B)

results = []
for tag, b in V.items():
    out, txt = score(SRC.replace(OLD, b), tag)
    if out is None:
        print('%-18s %s' % (tag, txt)); continue
    n = parse(out, '0x0015ba70')
    print('%-18s pick=%-3d totalMATCH=%d' % (tag, n, out.count('MATCH')))
    results.append((n, tag))
results.sort()
print('\nBEST:', results[:4])

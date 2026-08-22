import sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import score, parse

SRC = open(os.path.join(HERE, 'd_a_wm_antlion_mng.cpp'), encoding='utf-8').read()
START = SRC.index('bool daWmAntlionMng_c::pickRevivedIndices(')
END = SRC.index('/// @unofficial fn_2_15BC30', START)
OLD = SRC[START:END]
assert len(OLD) > 500

def mk(mapdecl, mapuse, cand_local, cond, phase3map, phase3use, pp_decl='    int playerPoint;\n',
       pp_assign='    playerPoint = daWmPlayer_c::ms_instance->m_22c;\n', pp_use='playerPoint'):
    candline = '        int cand = candidates[randIdx];\n' if cand_local else ''
    C = 'cand' if cand_local else 'candidates[randIdx]'
    return """bool daWmAntlionMng_c::pickRevivedIndices(int *out, int count, int worldIndex, bool excludeCurrent) {
    int candidates[9];
    for (int k = 0; k < 9; k++) {
        candidates[k] = -1;
    }

""" + mapdecl + """    int foundCount = 0;
    for (int i = 0; i < 0xc0; i++) {
        if (worldIndex == 0) {
            if (""" + mapuse + """mCsvData[""" + mapuse + """currIdx].GetRouteFlag(i, 0x400)) {
                candidates[foundCount] = i;
                foundCount++;
            }
        } else if (worldIndex == 1) {
            if (""" + mapuse + """mCsvData[""" + mapuse + """currIdx].GetRouteFlag(i, 0x800)) {
                candidates[foundCount] = i;
                foundCount++;
            }
        }
    }

""" + pp_decl + """    int numPicked = 0;
    if (foundCount < count) {
        return false;
    }

""" + pp_assign + """    do {
        int randIdx = dGameCom::getRandom(foundCount);
""" + candline + """        if (""" + cond.replace('CAND', C).replace('PP', pp_use) + """) {
            out[numPicked] = """ + C + """;
            numPicked++;
            candidates[randIdx] = -1;
        }
    } while (numPicked != count);

""" + phase3map + """    for (int j = 0; j < 2; j++) {
        if (dWmLib::getEnemyRevivalCount(*(int *)((char *)""" + phase3use + """ + 0x3388), worldIndex * 2 + j) > 0) {
            out[j] = -1;
        }
    }

    return true;
}

"""

MAPP = '    daWmMap_c *map = daWmMap_c::m_instance;\n'
MAPR = '    daWmMap_c &map = *daWmMap_c::m_instance;\n'
COND = 'CAND > 0 && (!excludeCurrent || PP != CAND)'

V = {}
V['P0_ctrl'] = mk(MAPP, 'map->', True, COND, '    map = daWmMap_c::m_instance;\n', 'map')
V['P1_ref'] = mk(MAPR, 'map.', True, COND,
                 '    daWmMap_c &map2 = *daWmMap_c::m_instance;\n', '&map2')
V['P2_nocand'] = mk(MAPP, 'map->', False, COND, '    map = daWmMap_c::m_instance;\n', 'map')
V['P3_ref_nocand'] = mk(MAPR, 'map.', False, COND,
                        '    daWmMap_c &map2 = *daWmMap_c::m_instance;\n', '&map2')
V['P4_nocand_ptr2'] = mk(MAPP, 'map->', False, COND,
                         '    daWmMap_c *map2 = daWmMap_c::m_instance;\n', 'map2')
V['P5_ref_ptr2'] = mk(MAPR, 'map.', True, COND,
                      '    daWmMap_c *map2 = daWmMap_c::m_instance;\n', 'map2')

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

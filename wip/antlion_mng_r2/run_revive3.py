import sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import score, parse

SRC = open(os.path.join(HERE, 'd_a_wm_antlion_mng.cpp'), encoding='utf-8').read()
START = SRC.index('void daWmAntlionMng_c::reviveOnRoute() {')
END = SRC.index('/// @unofficial fn_2_15BDA0', START)
OLD = SRC[START:END]
assert len(OLD) > 500

PRE = """    int accum;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;
    int route;
    static bool sToggle = true;
    const int worldIndexTable[2] = {0, 1};
    const int unofficialTable85F8[4] = {0, 1, 0, 1};
    (void)unofficialTable85F8;
"""

def mk(loophead, tail_inc, posdecl, posstmt, idxexpr='i + accum'):
    return """void daWmAntlionMng_c::reviveOnRoute() {
""" + PRE + """
""" + loophead + """
        int picked[2];
        if (!pickRevivedIndices(picked, 2, worldIndexTable[route], sToggle)) {
            continue;
        }
        sToggle = false;

""" + posdecl + """        dBase_c *antlion = dBase_c::searchBaseByProfName(0x28e, nullptr);
        for (int i = 0; i < 2; i++) {
            if (picked[i] < 0) {
                continue;
            }

            const char *pointName = map.mCsvData[map.currIdx].GetPointName(picked[i]);
            int world = pointName[3] - '0';
            info->SetMapEnemyInfo(*(int *)((char *)&map + 0x3388), """ + idxexpr + """, map.currIdx, picked[i]);
            map.mModels[map.currIdx].setAntlion(true, world, true);

""" + posstmt + """            dWmSeManager_c::m_pInstance->playSound(0x58, pos, 1);
        }
""" + tail_inc + """    }
}

"""

LH = '    for (route = 0, accum = 0; route < 2; route++, accum += 2) {'
LH2 = '    for (route = 0, accum = 0; route < 2; route++) {'
POSL = '        mVec3_c pos;\n'
ASSIGN = '            pos = map.GetPos(picked[i]);\n'
TWO = '            mVec3_c t = map.GetPos(picked[i]);\n            pos = t;\n'
INIT = '            mVec3_c pos = map.GetPos(picked[i]);\n'

V = {}
V['X0_ctrl'] = mk(LH, '', POSL, ASSIGN)
V['X1_two_named'] = mk(LH, '', POSL, TWO)
V['X2_copyinit'] = mk(LH, '', '', INIT)
V['X3_tail_accum'] = mk(LH2, '        accum += 2;\n', POSL, ASSIGN)
V['X4_accum_i'] = mk(LH, '', POSL, ASSIGN, 'accum + i')
V['X5_two_named_tail'] = mk(LH2, '        accum += 2;\n', POSL, TWO)

results = []
for tag, b in V.items():
    out, txt = score(SRC.replace(OLD, b), tag)
    if out is None:
        print('%-18s %s' % (tag, txt)); continue
    n = parse(out, '0x0015bc30')
    print('%-18s revive=%-3d totalMATCH=%d' % (tag, n, out.count('MATCH')))
    results.append((n, tag))
results.sort()
print('\nBEST:', results[:4])

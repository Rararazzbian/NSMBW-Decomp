import sys, os, re
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import score, parse

SRC = open(os.path.join(HERE, 'd_a_wm_antlion_mng.cpp'), encoding='utf-8').read()
START = SRC.index('void daWmAntlionMng_c::reviveOnRoute() {')
END = SRC.index('/// @unofficial fn_2_15BDA0', START)
OLD = SRC[START:END]
assert len(OLD) > 500

STATICS = """    static bool sToggle = true;
    const int worldIndexTable[2] = {0, 1};
    const int unofficialTable85F8[4] = {0, 1, 0, 1};
    (void)unofficialTable85F8;
"""

def mk(pre, pickdecl, posdecl_loop, posdecl_inner, posstmt, extra_loop=''):
    return """void daWmAntlionMng_c::reviveOnRoute() {
""" + pre + """
    for (route = 0, accum = 0; route < 2; route++, accum += 2) {
""" + pickdecl + """        if (!pickRevivedIndices(picked, 2, worldIndexTable[route], sToggle)) {
            continue;
        }
        sToggle = false;

""" + posdecl_loop + """        dBase_c *antlion = dBase_c::searchBaseByProfName(0x28e, nullptr);
        for (int i = 0; i < 2; i++) {
            if (picked[i] < 0) {
                continue;
            }

""" + posdecl_inner + """            const char *pointName = map.mCsvData[map.currIdx].GetPointName(picked[i]);
            int world = pointName[3] - '0';
            info->SetMapEnemyInfo(*(int *)((char *)&map + 0x3388), i + accum, map.currIdx, picked[i]);
            map.mModels[map.currIdx].setAntlion(true, world, true);

""" + posstmt + """            dWmSeManager_c::m_pInstance->playSound(0x58, pos, 1);
        }
    }
}

"""

ACC = '    int accum;\n'
MAPR = '    daWmMap_c &map = *daWmMap_c::m_instance;\n'
INFO = '    dInfo_c *info = dInfo_c::m_instance;\n'
ROUTE = '    int route;\n'
PICK_LOOP = '        int picked[2];\n'
PICK_TOP = '    int picked[2];\n'
POS_LOOP = '        mVec3_c pos;\n'
POS_TOP = '    mVec3_c pos;\n'
POS_INNER = '            mVec3_c pos;\n'
ASSIGN = '            pos = map.GetPos(picked[i]);\n'
BASE_PRE = ACC + MAPR + INFO + ROUTE + STATICS

V = {}
V['W0_ctrl'] = mk(BASE_PRE, PICK_LOOP, POS_LOOP, '', ASSIGN)
V['W1_picktop'] = mk(BASE_PRE + PICK_TOP, '', POS_LOOP, '', ASSIGN)
V['W2_picktop_postop'] = mk(BASE_PRE + PICK_TOP + POS_TOP, '', '', '', ASSIGN)
V['W3_postop'] = mk(BASE_PRE + POS_TOP, PICK_LOOP, '', '', ASSIGN)
V['W4_posinner'] = mk(BASE_PRE, PICK_LOOP, '', POS_INNER, ASSIGN)
V['W5_acc_route_first'] = mk(ACC + ROUTE + MAPR + INFO + STATICS, PICK_LOOP, POS_LOOP, '', ASSIGN)
V['W6_pos_before_pick'] = mk(BASE_PRE, '        mVec3_c pos;\n        int picked[2];\n', '', '', ASSIGN)
V['W7_pos_after_antlion'] = mk(BASE_PRE, PICK_LOOP,
                               '        dBase_c *antlion0 = 0; (void)antlion0;\n        mVec3_c pos;\n', '', ASSIGN)

results = []
for tag, b in V.items():
    out, txt = score(SRC.replace(OLD, b), tag)
    if out is None:
        print('%-22s %s' % (tag, txt)); continue
    n = parse(out, '0x0015bc30')
    print('%-22s revive=%-3d totalMATCH=%d' % (tag, n, out.count('MATCH')))
    results.append((n, tag))
results.sort()
print('\nBEST:', results[:4])

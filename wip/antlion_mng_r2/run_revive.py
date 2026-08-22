import sys, os, re
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import BASE, score, parse

SRC = open(os.path.join(HERE, 'd_a_wm_antlion_mng.cpp'), encoding='utf-8').read()
START = SRC.index('void daWmAntlionMng_c::reviveOnRoute() {')
END = SRC.index('/// @unofficial fn_2_15BDA0', START)
OLD = SRC[START:END]
assert END > START and len(OLD) > 500, (START, END, len(OLD))

DECL_STATICS = """    static bool sToggle = true;
    const int worldIndexTable[2] = {0, 1};
    const int unofficialTable85F8[4] = {0, 1, 0, 1};
    (void)unofficialTable85F8;
"""

def body(pre, loophead, mapref, posdecl, posstmt):
    d = '&map' if mapref else 'map'
    m = 'map.' if mapref else 'map->'
    return ("""void daWmAntlionMng_c::reviveOnRoute() {
""" + pre + """
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

            const char *pointName = """ + m + """mCsvData[""" + m + """currIdx].GetPointName(picked[i]);
            int world = pointName[3] - '0';
            info->SetMapEnemyInfo(*(int *)((char *)""" + d + """ + 0x3388), i + accum, """ + m + """currIdx, picked[i]);
            """ + m + """mModels[""" + m + """currIdx].setAntlion(true, world, true);

""" + posstmt.replace('MAP', m) + """            dWmSeManager_c::m_pInstance->playSound(0x58, pos, 1);
        }
    }
}

""")

MAPP = '    daWmMap_c *map = daWmMap_c::m_instance;\n'
MAPR = '    daWmMap_c &map = *daWmMap_c::m_instance;\n'
INFO = '    dInfo_c *info = dInfo_c::m_instance;\n'
ACC = '    int accum;\n'
ROUTE = '    int route;\n'

POS_OUT = '        mVec3_c pos;\n'
POS_STMT_ASSIGN = '            pos = MAPGetPos(picked[i]);\n'
POS_IN_INIT = ''
POS_STMT_INIT = '            mVec3_c pos = MAPGetPos(picked[i]);\n'

LH_COMMA = '    for (route = 0, accum = 0; route < 2; route++, accum += 2) {'
LH_DECL = '    for (int route = 0, accum = 0; route < 2; route++, accum += 2) {'

V = {}
# baseline control
V['V0_ctrl'] = OLD
# accum declared first, route declared, map pointer
V['V1_accum1st_ptr'] = body(ACC + MAPP + INFO + ROUTE + DECL_STATICS, LH_COMMA, False, POS_OUT, POS_STMT_ASSIGN)
V['V2_accum1st_ref'] = body(ACC + MAPR + INFO + ROUTE + DECL_STATICS, LH_COMMA, True, POS_OUT, POS_STMT_ASSIGN)
# table declared first
V['V3_tbl1st_ptr'] = body(DECL_STATICS + ACC + MAPP + INFO + ROUTE, LH_COMMA, False, POS_OUT, POS_STMT_ASSIGN)
V['V4_tbl1st_ref'] = body(DECL_STATICS + ACC + MAPR + INFO + ROUTE, LH_COMMA, True, POS_OUT, POS_STMT_ASSIGN)
V['V5_tbl1st_accum_info_map'] = body(DECL_STATICS + ACC + INFO + MAPP + ROUTE, LH_COMMA, False, POS_OUT, POS_STMT_ASSIGN)
V['V6_infomapswap'] = body(ACC + INFO + MAPP + ROUTE + DECL_STATICS, LH_COMMA, False, POS_OUT, POS_STMT_ASSIGN)
V['V7_accum1st_ptr_posin'] = body(ACC + MAPP + INFO + ROUTE + DECL_STATICS, LH_COMMA, False, POS_IN_INIT, POS_STMT_INIT)

results = []
for tag, b in V.items():
    out, txt = score(SRC.replace(OLD, b), tag)
    if out is None:
        print('%-24s %s' % (tag, txt)); continue
    n = parse(out, '0x0015bc30')
    print('%-24s revive=%-3d totalMATCH=%d' % (tag, n, out.count('MATCH')))
    results.append((n, tag))
results.sort()
print('\nBEST:', results[:4])

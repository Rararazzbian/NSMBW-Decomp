import sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import BASE, REBUILD_BODY, score, parse

INNER = """        for (int sub = 0; sub < 2; sub++) {
            int idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)&map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                const char *pointName = map.mCsvData[map.currIdx].GetPointName(enemy.mPathIndex);
                int world = pointName[3] - '0';
                daWmMap_c::m_instance->mModels[daWmMap_c::m_instance->currIdx].setAntlion(param0, world, param1);
            }
        }
"""

V = {}
V['T0_refonly'] = """void daWmAntlionMng_c::rebuildAllModels(bool param0, bool param1) {
    daWmMap_c &map = *daWmMap_c::m_instance;
    int base = 0;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
""" + INNER + """        base += 2;
    }
}"""
V['T1_base_first'] = """void daWmAntlionMng_c::rebuildAllModels(bool param0, bool param1) {
    int base = 0;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
""" + INNER + """        base += 2;
    }
}"""
V['T2_S4shape'] = """void daWmAntlionMng_c::rebuildAllModels(bool param0, bool param1) {
    int base;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;
    int slot;

    for (slot = 0, base = 0; slot < 2; slot++) {
""" + INNER + """        base += 2;
    }
}"""
V['T3_S2shape'] = """void daWmAntlionMng_c::rebuildAllModels(bool param0, bool param1) {
    int base;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;
    int slot = 0;
    base = 0;

    for (; slot < 2; slot++) {
""" + INNER + """        base += 2;
    }
}"""
V['T4_mapfirst_S4'] = """void daWmAntlionMng_c::rebuildAllModels(bool param0, bool param1) {
    daWmMap_c &map = *daWmMap_c::m_instance;
    int base;
    dInfo_c *info = dInfo_c::m_instance;
    int slot;

    for (slot = 0, base = 0; slot < 2; slot++) {
""" + INNER + """        base += 2;
    }
}"""

results = []
for tag, body in V.items():
    out, txt = score(BASE.replace(REBUILD_BODY, body), tag)
    if out is None:
        print('%-18s %s' % (tag, txt)); continue
    n = parse(out, '0x0015bda0')
    print('%-18s rebuild=%-3d totalMATCH=%d' % (tag, n, out.count('MATCH')))
    results.append((n, tag))
results.sort()
print('\nBEST:', results[:3])

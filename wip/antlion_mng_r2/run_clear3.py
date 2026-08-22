import sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import BASE, CLEAR_BODY, score, parse

TAIL = """
    for (int slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info.GetMapEnemyInfo(*(int *)((char *)&map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info.SetMapEnemyInfo(*(int *)((char *)&map + 0x3388), idx, map.currIdx, -1);
            }
        }
        base += 2;
    }
}"""
TAILP = TAIL.replace('info.', 'info->')

V = {}
V['R1_base_idx'] = """void daWmAntlionMng_c::clearAllModels() {
    int base = 0;
    int idx;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;
""" + TAILP
V['R2_base_idx_map1st'] = """void daWmAntlionMng_c::clearAllModels() {
    daWmMap_c &map = *daWmMap_c::m_instance;
    int base = 0;
    int idx;
    dInfo_c *info = dInfo_c::m_instance;
""" + TAILP
V['R3_bothref'] = """void daWmAntlionMng_c::clearAllModels() {
    int base = 0;
    int idx;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c &info = *dInfo_c::m_instance;
""" + TAIL
V['R4_base_map_idx'] = """void daWmAntlionMng_c::clearAllModels() {
    int base = 0;
    daWmMap_c &map = *daWmMap_c::m_instance;
    int idx;
    dInfo_c *info = dInfo_c::m_instance;
""" + TAILP
V['R5_base_info_idx'] = """void daWmAntlionMng_c::clearAllModels() {
    int base = 0;
    dInfo_c *info = dInfo_c::m_instance;
    int idx;
    daWmMap_c &map = *daWmMap_c::m_instance;
""" + TAILP
V['R6_noidx'] = """void daWmAntlionMng_c::clearAllModels() {
    int base = 0;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)&map + 0x3388), sub + base, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)&map + 0x3388), sub + base, map.currIdx, -1);
            }
        }
        base += 2;
    }
}"""

results = []
for tag, body in V.items():
    out, txt = score(BASE.replace(CLEAR_BODY, body), tag)
    if out is None:
        print('%-20s %s' % (tag, txt)); continue
    n = parse(out, '0x0015be80')
    print('%-20s clear=%-3d totalMATCH=%d' % (tag, n, out.count('MATCH')))
    results.append((n, tag))
results.sort()
print('\nBEST:', results[:3])

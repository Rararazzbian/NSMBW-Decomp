import sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import BASE, CLEAR_BODY, score, parse

INNER = """        for (int sub = 0; sub < 2; sub++) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)&map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)&map + 0x3388), idx, map.currIdx, -1);
            }
        }
"""

V = {}
# S1: base declared first (uninitialised), slot=0 written BEFORE base=0, while-loop
V['S1_while_slotfirst'] = """void daWmAntlionMng_c::clearAllModels() {
    int base;
    int idx;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;
    int slot = 0;
    base = 0;

    while (slot < 2) {
""" + INNER + """        slot++;
        base += 2;
    }
}"""
# S2: same idea but keep a for with an empty init
V['S2_for_emptyinit'] = """void daWmAntlionMng_c::clearAllModels() {
    int base;
    int idx;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;
    int slot = 0;
    base = 0;

    for (; slot < 2; slot++) {
""" + INNER + """        base += 2;
    }
}"""
# S3: base declared first, then assigned after the for-init via comma list order
V['S3_forcomma_slot_base'] = """void daWmAntlionMng_c::clearAllModels() {
    int base;
    int idx;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = (base = 0); slot < 2; slot++) {
""" + INNER + """        base += 2;
    }
}"""
# S4: base declared first, slot declared before base's assignment, plain for
V['S4_slotdecl_then_base'] = """void daWmAntlionMng_c::clearAllModels() {
    int base;
    int idx;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;
    int slot;

    for (slot = 0, base = 0; slot < 2; slot++) {
""" + INNER + """        base += 2;
    }
}"""
# S5: control -- R1 shape with base assigned in a separate statement (should stay 2)
V['S5_ctrl_split'] = """void daWmAntlionMng_c::clearAllModels() {
    int base;
    int idx;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;
    base = 0;

    for (int slot = 0; slot < 2; slot++) {
""" + INNER + """        base += 2;
    }
}"""

results = []
for tag, body in V.items():
    out, txt = score(BASE.replace(CLEAR_BODY, body), tag)
    if out is None:
        print('%-24s %s' % (tag, txt)); continue
    n = parse(out, '0x0015be80')
    print('%-24s clear=%-3d totalMATCH=%d' % (tag, n, out.count('MATCH')))
    results.append((n, tag))
results.sort()
print('\nBEST:', results[:3])

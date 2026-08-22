import sys, os, re
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import BASE, CLEAR_BODY, score, parse

V = {}

V['A_noidx'] = """void daWmAntlionMng_c::clearAllModels() {
    int base = 0;
    daWmMap_c *map = daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)map + 0x3388), sub + base, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)map + 0x3388), sub + base, map->currIdx, -1);
            }
        }
        base += 2;
    }
}"""

V['B_ref'] = """void daWmAntlionMng_c::clearAllModels() {
    int idx;
    int base = 0;
    daWmMap_c &map = *daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)&map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)&map + 0x3388), idx, map.currIdx, -1);
            }
        }
        base += 2;
    }
}"""

V['C_inforef'] = """void daWmAntlionMng_c::clearAllModels() {
    int idx;
    int base = 0;
    daWmMap_c *map = daWmMap_c::m_instance;
    dInfo_c &info = *dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info.GetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info.SetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, map->currIdx, -1);
            }
        }
        base += 2;
    }
}"""

V['D_slotfirst'] = """void daWmAntlionMng_c::clearAllModels() {
    int slot;
    int idx;
    int base = 0;
    daWmMap_c *map = daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;

    for (slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, map->currIdx, -1);
            }
        }
        base += 2;
    }
}"""

V['E_incin_for'] = """void daWmAntlionMng_c::clearAllModels() {
    int idx;
    int base = 0;
    daWmMap_c *map = daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++, base += 2) {
        for (int sub = 0; sub < 2; sub++) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, map->currIdx, -1);
            }
        }
    }
}"""

V['F_while'] = """void daWmAntlionMng_c::clearAllModels() {
    int idx;
    int base = 0;
    daWmMap_c *map = daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;
    int slot = 0;

    while (slot < 2) {
        int sub = 0;
        while (sub < 2) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, map->currIdx, -1);
            }
            sub++;
        }
        slot++;
        base += 2;
    }
}"""

V['G_mapinner'] = """void daWmAntlionMng_c::clearAllModels() {
    int idx;
    int base = 0;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
        daWmMap_c *map = daWmMap_c::m_instance;
        for (int sub = 0; sub < 2; sub++) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, map->currIdx, -1);
            }
        }
        base += 2;
    }
}"""

V['H_mapconst'] = """void daWmAntlionMng_c::clearAllModels() {
    int idx;
    int base = 0;
    daWmMap_c *const map = daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, map->currIdx, -1);
            }
        }
        base += 2;
    }
}"""

results = []
for tag, body in V.items():
    out, txt = score(BASE.replace(CLEAR_BODY, body), tag)
    if out is None:
        print('%-14s %s' % (tag, txt))
        continue
    n = parse(out, '0x0015be80')
    tot = out.count('MATCH')
    results.append((n, tag))
    print('%-14s clear=%-3d totalMATCH=%d' % (tag, n, tot))
results.sort()
print('\nBEST:', results[:4])

import sys, os, itertools, re
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import BASE, CLEAR_BODY, score, parse

# Four preamble declaration slots, permuted.  `base` may also live in the for-init.
DECLS = {
    'i': '    int idx;',
    'b': '    int base = 0;',
    'm': '    daWmMap_c *map = daWmMap_c::m_instance;',
    'n': '    dInfo_c *info = dInfo_c::m_instance;',
}

TAIL = """
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
for perm in itertools.permutations('ibmn'):
    body = 'void daWmAntlionMng_c::clearAllModels() {\n'
    body += '\n'.join(DECLS[c] for c in perm) + '\n' + TAIL
    tag = 'c_' + ''.join(perm)
    out, txt = score(BASE.replace(CLEAR_BODY, body), tag)
    if out is None:
        print(tag, txt)
        continue
    n = parse(out, '0x0015be80')
    tot = out.count('MATCH')
    results.append((n, tag, tot))
    print('%-8s clear=%-3d  totalMATCH=%d' % (tag, n, tot))

results.sort()
print('\nBEST:', results[:5])

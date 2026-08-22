import sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sweep import score, parse

SRC = open(os.path.join(HERE, 'd_a_wm_antlion_mng.cpp'), encoding='utf-8').read()
A = SRC.index('    playerPoint = daWmPlayer_c::ms_instance->m_22c;')
B = SRC.index('    } while (numPicked != count);', A)
OLD = SRC[A:B]
assert len(OLD) > 100

HEAD = "    playerPoint = daWmPlayer_c::ms_instance->m_22c;\n    do {\n        int randIdx = dGameCom::getRandom(foundCount);\n"
STORE = """            out[numPicked] = candidates[randIdx];
            numPicked++;
            candidates[randIdx] = -1;
"""

V = {}
V['R0_ctrl'] = OLD
V['R1_nested'] = HEAD + """        if (candidates[randIdx] > 0) {
            if (!excludeCurrent || playerPoint != candidates[randIdx]) {
""" + STORE + """            }
        }
"""
V['R2_demorgan'] = HEAD + """        if (candidates[randIdx] > 0 && !(excludeCurrent && playerPoint == candidates[randIdx])) {
""" + STORE + """        }
"""
V['R3_guards'] = HEAD + """        if (candidates[randIdx] > 0) {
            if (excludeCurrent && playerPoint == candidates[randIdx]) {
                continue;
            }
""" + STORE + """        }
"""
V['R4_skipflag'] = HEAD + """        bool skip = false;
        if (excludeCurrent && playerPoint == candidates[randIdx]) {
            skip = true;
        }
        if (candidates[randIdx] > 0 && !skip) {
""" + STORE + """        }
"""
V['R5_nested3'] = HEAD + """        if (candidates[randIdx] > 0) {
            if (excludeCurrent) {
                if (playerPoint == candidates[randIdx]) {
                    continue;
                }
            }
""" + STORE + """        }
"""
V['R6_or_excl_twice'] = HEAD + """        if (candidates[randIdx] > 0 && (!excludeCurrent || playerPoint != candidates[randIdx]) &&
            (!excludeCurrent || playerPoint != candidates[randIdx])) {
""" + STORE + """        }
"""

results = []
for tag, b in V.items():
    out, txt = score(SRC.replace(OLD, b), tag)
    if out is None:
        print('%-20s %s' % (tag, txt)); continue
    n = parse(out, '0x0015ba70')
    print('%-20s pick=%-3d totalMATCH=%d' % (tag, n, out.count('MATCH')))
    results.append((n, tag))
results.sort()
print('\nBEST:', results[:4])

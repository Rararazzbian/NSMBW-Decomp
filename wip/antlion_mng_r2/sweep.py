"""Sweep source variants of clearAllModels / rebuildAllModels and score them.

Usage: python sweep.py <which>     (which = clear | rebuild)
Each variant is a full replacement body for the function; the rest of the file
is untouched, so the score of every other function is a control.
"""
import sys, os, re, subprocess, itertools

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

BASE = open(os.path.join(HERE, 'd_a_wm_antlion_mng.cpp'), encoding='utf-8').read()

CLEAR_BODY = """void daWmAntlionMng_c::clearAllModels() {
    int idx;
    int base = 0;
    daWmMap_c *map = daWmMap_c::m_instance;
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

REBUILD_BODY = """void daWmAntlionMng_c::rebuildAllModels(bool param0, bool param1) {
    daWmMap_c *map = daWmMap_c::m_instance;
    int base = 0;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            int idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                const char *pointName = map->mCsvData[map->currIdx].GetPointName(enemy.mPathIndex);
                int world = pointName[3] - '0';
                daWmMap_c::m_instance->mModels[daWmMap_c::m_instance->currIdx].setAntlion(param0, world, param1);
            }
        }
        base += 2;
    }
}"""


TARGET_FN = {'clear': 'fn_2_15BE80', 'rebuild': 'fn_2_15BDA0'}
DRAFT_FN = {'clear': 'clearAllModels__16daWmAntlionMng_cFv',
            'rebuild': 'rebuildAllModels__16daWmAntlionMng_cFbb'}
ANCHOR = {'clear': CLEAR_BODY, 'rebuild': REBUILD_BODY}

OBJS = [os.path.join(ROOT, 'bin/dtkspl/d_basesNP/obj/auto_00_0015B564_text.o'),
        os.path.join(ROOT, 'bin/dtkspl/d_basesNP/obj/auto_fn_2_15C150_text.o'),
        os.path.join(ROOT, 'bin/dtkspl/d_basesNP/obj/auto_00_0015C1D4_text.o')]


def score(src_text, tag):
    src = os.path.join(HERE, 'sw_%s.cpp' % tag)
    obj = os.path.join(HERE, 'sw_%s.o' % tag)
    txt = os.path.join(HERE, 'sw_%s.txt' % tag)
    open(src, 'w', encoding='utf-8', newline='\n').write(src_text)
    ok, log = H.compile_draft(src, obj, extra_inc=[os.path.join(HERE, 'shadow_include')],
                              module='d_basesNP')
    if not ok:
        return None, 'COMPILE FAILED: ' + log[-400:]
    ok2, log2 = H.disasm(obj, txt)
    if not ok2:
        return None, 'DISASM FAILED'
    out = subprocess.run([sys.executable, os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py'),
                          txt, '0x15b590', '0x15c200'] + OBJS,
                         capture_output=True, text=True).stdout
    return out, txt


def parse(out, addr):
    for line in out.splitlines():
        if line.startswith(addr):
            if 'MATCH' in line:
                return 0
            m = re.search(r'(\d+) differing', line)
            if m:
                return int(m.group(1))
    return -1

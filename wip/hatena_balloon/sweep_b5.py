"""Sweep source shapes for all_bgcheck's inner-loop vector construction.

The GPR colouring already matches; the residue is a 4-way permutation of the
FP temporaries. Instruction ORDER is already identical, so the lever has to be
something that changes the allocator's numbering without changing the schedule.
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SCR = os.path.dirname(HERE)
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402

TARGET = os.path.join(SCR, 'dis', 'hb_a.txt')
BASE = os.path.join(SCR, 'hb-b5.cpp')
INC = os.path.join(HERE, 'inc')
FN = 'all_bgcheck__19daEnHatenaBalloon_cFRUc'

HEAD = open(BASE, encoding='utf-8').read().split(
    'u8 daEnHatenaBalloon_c::all_bgcheck')[0]

# ---------------------------------------------------------------- variants
# Each variant is the complete all_bgcheck definition.

PROLOGUE = '''u8 daEnHatenaBalloon_c::all_bgcheck(u8 &floorFlags) {
%(decls)s    if (goalpole_check()) {
        floorFlags = 0xFF;
        return 0xF;
    }

    if (m_81f != -1) {
        dAcPy_c *player = daPyMng_c::getPlayer(m_81f);
        if (player != nullptr) {
            mBc.mLayer = player->mLayer;
        }
    }

    ret = 0;
    row = s_someCheckData;
    for (i = 0; i < 4; i++) {
        flag = row->mFlag;
        ofs = &row->mOffsetX;
        hit = 0;
        for (j = 0; j < 2; j++) {
%(build)s            if (!(ret & flag)) {
                hit |= pointBgCheck(%(arg)s, 1, 1, hit);
                if (hit != 0) {
                    ret |= (u8) row->mFlag;
                }
            }
            ofs += 2;
        }
        row++;
    }

    floorFlags = floor_check();

    %(tail)s
    return ret;
}
'''

TAIL_DEFAULT = '''float waterOut = 0.0f;
    dAcPy_c *player = daPyMng_c::getPlayer(m_81f);
    if (player != nullptr) {
        int type = dBc_c::checkWater(mPos.x, mPos.y, player->mLayer, &waterOut);
        if (type >= dBc_c::WATER_CHECK_YOGAN && type <= dBc_c::WATER_CHECK_POISON) {
            ret = 1;
        }
    }'''

BASE_DECLS = '''    u32 flag;
    const checkData_s *row;
    const float *ofs;
    u8 ret;
    u32 i;
    u32 hit;
    u32 j;

'''

I = ' ' * 12

VARIANTS = []


def add(label, build, arg='pt', decls=BASE_DECLS, tail=TAIL_DEFAULT):
    VARIANTS.append((label, PROLOGUE % dict(decls=decls, build=build,
                                            arg=arg, tail=tail)))


# 1. baseline -- what the predecessor left
add('baseline ctor(x+o0, y+o1, z)',
    I + 'mVec3_c pt(mPos.x + ofs[0], mPos.y + ofs[1], mPos.z);\n')

# 2. member-wise assignment into a default-constructed vector
add('memberwise pt.x/pt.y/pt.z',
    I + 'mVec3_c pt;\n' +
    I + 'pt.x = mPos.x + ofs[0];\n' +
    I + 'pt.y = mPos.y + ofs[1];\n' +
    I + 'pt.z = mPos.z;\n')

# 3. hoist the two mPos components into locals first
add('hoist mPos.x/.y into locals',
    I + 'float px = mPos.x;\n' +
    I + 'float py = mPos.y;\n' +
    I + 'mVec3_c pt(px + ofs[0], py + ofs[1], mPos.z);\n')

# 4. hoist the two offsets into locals first
add('hoist ofs[0]/ofs[1] into locals',
    I + 'float o0 = ofs[0];\n' +
    I + 'float o1 = ofs[1];\n' +
    I + 'mVec3_c pt(mPos.x + o0, mPos.y + o1, mPos.z);\n')

# 5. hoist the sums into locals
add('hoist sums into locals',
    I + 'float sx = mPos.x + ofs[0];\n' +
    I + 'float sy = mPos.y + ofs[1];\n' +
    I + 'mVec3_c pt(sx, sy, mPos.z);\n')

# 6. copy-construct from mPos then add in place
add('copy mPos then += offsets',
    I + 'mVec3_c pt(mPos);\n' +
    I + 'pt.x += ofs[0];\n' +
    I + 'pt.y += ofs[1];\n')

# 7. temporary passed straight into the call
add('temporary as call argument',
    '',
    arg='mVec3_c(mPos.x + ofs[0], mPos.y + ofs[1], mPos.z)')

# 8. vector declared at the top of the function body (the documented lever)
add('pt declared at top of body',
    I + 'pt.x = mPos.x + ofs[0];\n' +
    I + 'pt.y = mPos.y + ofs[1];\n' +
    I + 'pt.z = mPos.z;\n',
    decls=BASE_DECLS.replace('    u32 j;\n', '    u32 j;\n    mVec3_c pt;\n'))

# 9. float scratch locals declared at top, assigned in the loop
add('float scratch declared at top',
    I + 'px = mPos.x + ofs[0];\n' +
    I + 'py = mPos.y + ofs[1];\n' +
    I + 'mVec3_c pt(px, py, mPos.z);\n',
    decls=BASE_DECLS.replace('    u32 j;\n',
                             '    u32 j;\n    float px;\n    float py;\n'))

# 10. waterOut declared at the top of the body
add('waterOut declared at top',
    I + 'mVec3_c pt(mPos.x + ofs[0], mPos.y + ofs[1], mPos.z);\n',
    decls=BASE_DECLS.replace('    u32 j;\n', '    u32 j;\n    float waterOut;\n'),
    tail='''waterOut = 0.0f;
    dAcPy_c *player = daPyMng_c::getPlayer(m_81f);
    if (player != nullptr) {
        int type = dBc_c::checkWater(mPos.x, mPos.y, player->mLayer, &waterOut);
        if (type >= dBc_c::WATER_CHECK_YOGAN && type <= dBc_c::WATER_CHECK_POISON) {
            ret = 1;
        }
    }''')

# 11. explicit copy-initialisation from a temporary
add('copy-init from temporary',
    I + 'mVec3_c pt = mVec3_c(mPos.x + ofs[0], mPos.y + ofs[1], mPos.z);\n')

# 12. z first, then the two sums, member-wise
add('memberwise z first',
    I + 'mVec3_c pt;\n' +
    I + 'pt.z = mPos.z;\n' +
    I + 'pt.x = mPos.x + ofs[0];\n' +
    I + 'pt.y = mPos.y + ofs[1];\n')

# 13. build via mVec2_c then the (mVec2_c, z) constructor
add('mVec2_c then (v2, z) ctor',
    I + 'mVec2_c p2(mPos.x + ofs[0], mPos.y + ofs[1]);\n' +
    I + 'mVec3_c pt(p2, mPos.z);\n')

# 14. set() on a default-constructed vector -- mVec3_c inherits set from EGG
add('assign from constructed temporary',
    I + 'mVec3_c pt;\n' +
    I + 'pt = mVec3_c(mPos.x + ofs[0], mPos.y + ofs[1], mPos.z);\n')

# 15. offsets read through named struct members rather than a float pointer
add('offsets via ofs[0]/ofs[1] reversed decl order',
    I + 'float o1 = ofs[1];\n' +
    I + 'float o0 = ofs[0];\n' +
    I + 'mVec3_c pt(mPos.x + o0, mPos.y + o1, mPos.z);\n')

# 16. mPos hoisted as a whole vector
add('hoist mPos as a vector',
    I + 'mVec3_c base = mPos;\n' +
    I + 'mVec3_c pt(base.x + ofs[0], base.y + ofs[1], base.z);\n')


# ---------------------------------------------------------------- driver

def insn(path, name):
    n, inside = 0, False
    for line in open(path, encoding='utf-8', errors='replace'):
        s = line.strip()
        m = re.match(r'^\.fn\s+"?(.+?)"?\s*,', s)
        if m:
            inside = (H.norm_name(m.group(1)) == H.norm_name(name))
            continue
        if s.startswith('.endfn'):
            inside = False
            continue
        if inside and re.match(r'^/\*.*?\*/\s*\S', s):
            n += 1
    return n


def main():
    only = sys.argv[1:] and sys.argv[1]
    results = []
    for idx, (label, body) in enumerate(VARIANTS):
        if only and only not in label and only != str(idx + 1):
            continue
        src = os.path.join(HERE, 'v%02d.cpp' % (idx + 1))
        obj = os.path.join(HERE, 'v%02d.o' % (idx + 1))
        txt = os.path.join(HERE, 'v%02d.txt' % (idx + 1))
        open(src, 'w', encoding='utf-8', newline='\n').write(HEAD + body)
        ok, log = H.compile_draft(src, obj, extra_inc=[INC])
        if not ok:
            results.append((999, label, 'COMPILE FAIL: ' +
                            log.strip().splitlines()[-1][:90] if log.strip() else 'COMPILE FAIL'))
            continue
        ok, log = H.disasm(obj, txt)
        if not ok:
            results.append((999, label, 'DISASM FAIL'))
            continue
        n = insn(txt, FN)
        if n * 4 != 0x164:
            results.append((998, label, 'SIZE %d insn (want 89)' % n))
            continue
        good, rep = H.diff_fn(TARGET, txt, FN)
        if good:
            results.append((0, label, 'MATCH'))
        else:
            diffs = len([l for l in rep.splitlines() if 'want:' in l])
            results.append((diffs, label, 'diff %d lines' % diffs))
    for n, label, msg in sorted(results):
        print('%-44s %s' % (label, msg))
    return 0


if __name__ == '__main__':
    sys.exit(main())

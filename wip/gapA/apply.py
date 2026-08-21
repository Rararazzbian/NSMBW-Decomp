"""Apply the Gap A shape to every `newBase` site in d_line_mng.cpp.

    mVec2_c newBase(mUnitBasePos.x + 16.0f, mUnitBasePos.y);
->  mVec2_c newBase = mUnitBasePos;
    newBase.x += 16.0f;

Copy-then-adjust reloads both fields fresh, which is what retail does; the
constructor form lets -O4 reuse the sum computed for the `>=` test one line
above. Proven on executeState_Left30Left: 97 -> 99 insns, gap region exact.

Usage:  python apply.py <in.cpp> <out.cpp>
"""
import re, sys

X = re.compile(r'^(\s*)mVec2_c newBase\(mUnitBasePos\.x ([+-]) 16\.0f, mUnitBasePos\.y\);$', re.M)
Y = re.compile(r'^(\s*)mVec2_c newBase\(mUnitBasePos\.x, mUnitBasePos\.y ([+-]) 16\.0f\);$', re.M)


def sub(field):
    def f(m):
        pad, op = m.group(1), m.group(2)
        return ('%smVec2_c newBase = mUnitBasePos;\n'
                '%snewBase.%s %s= 16.0f;' % (pad, pad, field, op))
    return f


src = open(sys.argv[1], encoding='utf-8').read()
src, nx = X.subn(sub('x'), src)
src, ny = Y.subn(sub('y'), src)
open(sys.argv[2], 'w', encoding='utf-8').write(src)
print('rewrote %d x-sites, %d y-sites' % (nx, ny))

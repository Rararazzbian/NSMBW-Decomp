import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sweep import run

OFF = '    const Vec3Pod_t &offset = sc_KoopaShipStopConfig[0].mOffset;\n'
V = {}

# explicit 3-arg construction as the return expression -- the idiom used by the two
# matched functions that DO have the target's deferred 4,0,8 store tail.
V['I1_ctor_inline'] = OFF + """\
    return mVec3_c(mPos.x + offset.x, mPos.y + offset.y, mPos.z + offset.z);
"""
V['I2_ctor_noref'] = """\
    return mVec3_c(mPos.x + sc_KoopaShipStopConfig[0].mOffset.x,
                   mPos.y + sc_KoopaShipStopConfig[0].mOffset.y,
                   mPos.z + sc_KoopaShipStopConfig[0].mOffset.z);
"""
V['I3_ctor_named'] = OFF + """\
    mVec3_c result(mPos.x + offset.x, mPos.y + offset.y, mPos.z + offset.z);
    return result;
"""
# staged locals, decouple-declaration-order lever, then ctor
V['I4_zyx_locals_ctor'] = OFF + """\
    float z = mPos.z + offset.z;
    float y = mPos.y + offset.y;
    float x = mPos.x + offset.x;
    return mVec3_c(x, y, z);
"""
# set() route
V['I5_set'] = OFF + """\
    float z = mPos.z + offset.z;
    float y = mPos.y + offset.y;
    float x = mPos.x + offset.x;
    mVec3_c result;
    result.set(x, y, z);
    return result;
"""
# the three field-store orders not yet re-measured here, on the named-local shape
for tag, order in (('J_xyz', 'xyz'), ('J_xzy', 'xzy'), ('J_yzx', 'yzx'),
                   ('J_zxy', 'zxy'), ('J_zyx', 'zyx')):
    body = OFF + "    float z = mPos.z + offset.z;\n    float y = mPos.y + offset.y;\n    float x = mPos.x + offset.x;\n    mVec3_c result;\n"
    for c in order:
        body += "    result.%s = %s;\n" % (c, c)
    body += "    return result;\n"
    V[tag] = body

# offset read through a pointer-to-element (different base-register pressure)
V['K_elem_ptr'] = """\
    const KoopaShipStopConfig_t *cfg = &sc_KoopaShipStopConfig[0];
    return mVec3_c(mPos.x + cfg->mOffset.x, mPos.y + cfg->mOffset.y, mPos.z + cfg->mOffset.z);
"""

for k in sorted(V):
    run(k, V[k])

import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sweep import run

OFF = '    const Vec3Pod_t &offset = sc_KoopaShipStopConfig[0].mOffset;\n'

V = {}

# --- baseline reproduction through the splicer itself (must give 6) -------------
V['A_baseline'] = OFF + """\
    float z = mPos.z + offset.z;
    float y = mPos.y + offset.y;
    float x = mPos.x + offset.x;
    mVec3_c result;
    result.y = y;
    result.x = x;
    result.z = z;
    return result;
"""

# --- lever 13 / "named temporaries in the target's READ order" ------------------
# target load order: mPos.z, mPos.y, offset.z, offset.y, mPos.x, offset.x
V['B_leaves_readorder'] = OFF + """\
    f32 mz = mPos.z;
    f32 my = mPos.y;
    f32 oz = offset.z;
    f32 oy = offset.y;
    f32 mx = mPos.x;
    f32 ox = offset.x;
    float z = mz + oz;
    float y = my + oy;
    float x = mx + ox;
    mVec3_c result;
    result.y = y;
    result.x = x;
    result.z = z;
    return result;
"""

# --- only x's member hoisted (minimal def-point on the outlier) ----------------
V['C_hoist_mx_only'] = OFF + """\
    float z = mPos.z + offset.z;
    float y = mPos.y + offset.y;
    f32 mx = mPos.x;
    float x = mx + offset.x;
    mVec3_c result;
    result.y = y;
    result.x = x;
    result.z = z;
    return result;
"""

# --- only x's offset hoisted --------------------------------------------------
V['D_hoist_ox_only'] = OFF + """\
    float z = mPos.z + offset.z;
    float y = mPos.y + offset.y;
    f32 ox = offset.x;
    float x = mPos.x + ox;
    mVec3_c result;
    result.y = y;
    result.x = x;
    result.z = z;
    return result;
"""

# --- both x leaves hoisted, member first --------------------------------------
V['E_hoist_x_pair'] = OFF + """\
    float z = mPos.z + offset.z;
    float y = mPos.y + offset.y;
    f32 mx = mPos.x;
    f32 ox = offset.x;
    float x = mx + ox;
    mVec3_c result;
    result.y = y;
    result.x = x;
    result.z = z;
    return result;
"""

# --- mVec3_c::operator+ route (never tried; adds a temporary + RVO chain) ------
V['F_operator_plus'] = """\
    return mPos + reinterpret_cast<const mVec3_c &>(sc_KoopaShipStopConfig[0].mOffset);
"""

# --- lever 11 compound assignment on the RESULT members -----------------------
V['G_result_compound'] = OFF + """\
    mVec3_c result;
    result.z = mPos.z;
    result.z += offset.z;
    result.y = mPos.y;
    result.y += offset.y;
    result.x = mPos.x;
    result.x += offset.x;
    return result;
"""

# --- lever 10 aggregate copy then compound assignment -------------------------
V['H_aggcopy_compound'] = OFF + """\
    mVec3_c result = mPos;
    result.z += offset.z;
    result.y += offset.y;
    result.x += offset.x;
    return result;
"""

for k in sorted(V):
    run(k, V[k])

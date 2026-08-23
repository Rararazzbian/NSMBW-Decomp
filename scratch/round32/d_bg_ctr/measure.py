import os
import sys
sys.path.insert(0, os.path.dirname(__file__))
import build

items = [
    ('calc_baseline.cpp', 'calc_baseline', 'calc__9dBg_ctr_cFv'),
    ('calc_decl_f29.cpp', 'calc_decl_f29', 'calc__9dBg_ctr_cFv'),
    ('dokan_lifetimes.cpp', 'dokan_lifetimes', 'addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c'),
    ('dokan_target_shape.cpp', 'dokan_target_shape', 'addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c'),
    ('target_math_dokan.cpp', 'target_math_dokan', 'addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c'),
    ('revise_order.cpp', 'revise_order', 'revisePos__9dBg_ctr_cFv'),
    ('fn809_liveness.cpp', 'fn809_liveness', 'fn_80080900__9dBg_ctr_cFP7mVec3_cPsi'),
    ('filter_dc_first.cpp', 'filter_dc_first', 'fn_80080E40__9dBg_ctr_cFP5dBc_cUcUc'),
]
for source, label, function in items:
    try:
        result = build.build(source, label, function)
        print('%s: %s' % (label, result[:4]))
    except Exception as exc:
        print('%s: ERROR %s' % (label, exc))

import os
import sys
sys.path.insert(0, os.path.dirname(__file__))
import build

root = os.path.dirname(__file__)
base = open(os.path.join(root, 't2_baseline.cpp'), encoding='utf-8').read()

variants = {
    't2_baseline': (base, 'fn_80080670__9dBg_ctr_cFP7mVec3_cf'),
    't2_local_vec': (base.replace('        Vec diff;\n', '        Vec diff;\n        Vec localDiff;\n').replace('        diff.x = cx - pos->x;\n', '        localDiff.x = cx - pos->x;\n        diff.x = localDiff.x;\n').replace('        diff.y = cy - pos->y;\n', '        localDiff.y = cy - pos->y;\n        diff.y = localDiff.y;\n'), 'fn_80080670__9dBg_ctr_cFP7mVec3_cf'),
    't5_baseline': (base, 'fn_8007FFA0__FP9dBg_ctr_cP8dActor_cP7mVec3_ci'),
    't5_stores': (base.replace('        stack_x += f2;\n        *(f32 *)((u8 *)actor + 0xDC) = vec->x;', '        stack_x += f2;\n        *(f32 *)((u8 *)actor + 0xDC) = vec->x;\n        *(f32 *)((u8 *)actor + 0x10) = stack_x;\n        *(f32 *)((u8 *)actor + 0x14) = stack_y;').replace('    pos->y += stack_y;', '    *(f32 *)((u8 *)actor + 0x10) = stack_x;\n    *(f32 *)((u8 *)actor + 0x14) = stack_y;\n    pos->y += stack_y;'), 'fn_8007FFA0__FP9dBg_ctr_cP8dActor_cP7mVec3_ci'),
}
for label, (text, fn) in variants.items():
    src = os.path.join(root, label + '.cpp')
    with open(src, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write(text)
    try:
        print(label, build.build(src, label, fn)[:4])
    except Exception as exc:
        print(label, 'ERROR', exc)

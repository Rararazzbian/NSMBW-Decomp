import os, sys
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

target = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round23\d_bg_ctr\target.txt'

funcs = ['fn_80080900', 'fn_80080670', 'calc__9dBg_ctr_cFv', 'fn_80080E40',
         'fn_8007FFA0', 'addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c',
         'revisePos__9dBg_ctr_cFv', 'fn_80080880']

for f in funcs:
    body = harness.extract(target, f)
    if body:
        print('=== %s (%d instructions) ===' % (f, len(body)))
        for l in body:
            print(l)
        print()
    else:
        print('=== %s NOT FOUND ===' % f)
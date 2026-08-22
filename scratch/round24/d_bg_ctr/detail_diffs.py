import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round22', 'd_bg_ctr')
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target.txt')

for name in ['set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c',
             'setLinkNetPlayer__9dBg_ctr_cFP5dBc_c',
             'setLinkWallSlidPlayer__9dBg_ctr_cFP5dBc_c',
             'update__9dBg_ctr_cFv',
             'CheckRevSideSpeed__9dBg_ctr_cFP8dActor_cP8dActor_cUc']:
    want = harness.extract(TARGET, name)
    got = harness.extract(DIS, name)
    print('=== %s ===' % name)
    for i in range(max(len(want), len(got))):
        a = want[i] if i < len(want) else '<none>'
        b = got[i] if i < len(got) else '<none>'
        if a != b:
            print('%3d | want: %-48s got: %s' % (i, a, b))
    print()

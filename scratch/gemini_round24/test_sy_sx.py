import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.run_jump_batch1 import test_jump_tail

test_jump_tail("Test sy, sx after rate, before muki",
"""    float rate = calcJumpRate();
    float sy = speed.y;
    float sx = speed.x;
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = sy;
    mSpeed.x = (muki * rate) * sx;""")

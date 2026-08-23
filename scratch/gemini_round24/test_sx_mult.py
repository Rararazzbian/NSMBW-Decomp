import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.run_jump_batch1 import test_jump_tail

test_jump_tail("Test speed.x * ((float)l_EnMuki[mDirection] * rate)",
"""    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = speed.x * ((float)l_EnMuki[mDirection] * rate);""")

test_jump_tail("Test sx * ((float)l_EnMuki[mDirection] * rate)",
"""    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = sx * ((float)l_EnMuki[mDirection] * rate);""")

test_jump_tail("Test speed.x * (float)l_EnMuki[mDirection] * rate",
"""    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = speed.x * (float)l_EnMuki[mDirection] * rate;""")

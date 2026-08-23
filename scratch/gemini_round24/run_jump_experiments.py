import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.run_jump_batch1 import test_jump_tail

candidates = [
    ("E1: sx before muki",
     """    float rate = calcJumpRate();
    float sx = speed.x;
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    mSpeed.x = (muki * rate) * sx;"""),

    ("E2: sx before rate",
     """    float sx = speed.x;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    mSpeed.x = (muki * rate) * sx;"""),

    ("E3: sy, sx before rate",
     """    float sy = speed.y;
    float sx = speed.x;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = sy;
    mSpeed.x = (muki * rate) * sx;"""),

    ("E4: sy before rate, sx after rate",
     """    float sy = speed.y;
    float rate = calcJumpRate();
    float sx = speed.x;
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = sy;
    mSpeed.x = (muki * rate) * sx;"""),

    ("E5: mSpeed.y before rate",
     """    mSpeed.y = speed.y;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;"""),

    ("E6: mSpeed.y before rate, no sx local",
     """    mSpeed.y = speed.y;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * speed.x;"""),
]

for name, code in candidates:
    test_jump_tail(name, code)

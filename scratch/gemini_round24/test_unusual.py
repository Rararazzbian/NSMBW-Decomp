import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.run_jump_batch1 import test_jump_tail

unusual = [
    ("U1: comma operator (mSpeed.y=speed.y, speed.x)",
     """    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    float sx = (mSpeed.y = speed.y, speed.x);
    mSpeed.x = (muki * rate) * sx;"""),

    ("U2: inline mSpeed.y in mSpeed.x",
     """    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * (mSpeed.y = speed.y, speed.x);"""),

    ("U3: comma operator before muki",
     """    float rate = calcJumpRate();
    float sx = (mSpeed.y = speed.y, speed.x);
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * sx;"""),

    ("U4: speed.x read into temp during mSpeed.y store",
     """    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    f32 sx = speed.x;
    mSpeed.x = muki * (rate * sx);"""),

    ("U5: double cast for int-to-float",
     """    float rate = calcJumpRate();
    double muki = (double)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;"""),

    ("U6: mSpeed.y = speed.y; mSpeed.x = ((float)l_EnMuki[mDirection] * calcJumpRate()) * speed.x",
     """    mSpeed.y = speed.y;
    mSpeed.x = ((float)l_EnMuki[mDirection] * calcJumpRate()) * speed.x;"""),
]

for name, code in unusual:
    test_jump_tail(name, code)

import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.run_jump_batch1 import test_jump_tail

inlines = [
    ("I1: inline calcJumpRate in (muki*rate)*sx",
     """    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * calcJumpRate()) * sx;"""),

    ("I2: inline calcJumpRate in (muki*rate)*speed.x",
     """    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    mSpeed.x = (muki * calcJumpRate()) * speed.x;"""),

    ("I3: mSpeed.y=speed.y before muki, inline calcJumpRate",
     """    mSpeed.y = speed.y;
    float muki = (float)l_EnMuki[mDirection];
    float sx = speed.x;
    mSpeed.x = (muki * calcJumpRate()) * sx;"""),

    ("I4: mSpeed.y=speed.y, inline calcJumpRate in ((float)l_EnMuki[mDirection]*calcJumpRate())*speed.x",
     """    mSpeed.y = speed.y;
    mSpeed.x = ((float)l_EnMuki[mDirection] * calcJumpRate()) * speed.x;"""),

    ("I5: sx=speed.x, mSpeed.y=speed.y, inline calcJumpRate in (l_EnMuki[mDirection]*calcJumpRate())*sx",
     """    float sx = speed.x;
    mSpeed.y = speed.y;
    mSpeed.x = ((float)l_EnMuki[mDirection] * calcJumpRate()) * sx;"""),

    ("I6: inline calcJumpRate: mSpeed.x=(speed.x*calcJumpRate())*l_EnMuki[mDirection]",
     """    mSpeed.y = speed.y;
    mSpeed.x = (speed.x * calcJumpRate()) * (float)l_EnMuki[mDirection];"""),
]

for name, code in inlines:
    test_jump_tail(name, code)

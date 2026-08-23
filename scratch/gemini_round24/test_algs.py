import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.run_jump_batch1 import test_jump_tail

algs = [
    ("A1: mSpeed.y=speed.y; mSpeed.x=(speed.x*rate)*(float)l_EnMuki[mDirection]",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = (speed.x * rate) * (float)l_EnMuki[mDirection];"""),

    ("A2: mSpeed.y=speed.y; mSpeed.x=(rate*speed.x)*(float)l_EnMuki[mDirection]",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = (rate * speed.x) * (float)l_EnMuki[mDirection];"""),

    ("A3: mSpeed.y=speed.y; mSpeed.x=rate*(speed.x*(float)l_EnMuki[mDirection])",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = rate * (speed.x * (float)l_EnMuki[mDirection]);"""),

    ("A4: mSpeed.y=speed.y; mSpeed.x=rate*((float)l_EnMuki[mDirection]*speed.x)",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = rate * ((float)l_EnMuki[mDirection] * speed.x);"""),

    ("A5: mSpeed.y=speed.y; mSpeed.x=(float)l_EnMuki[mDirection]*(speed.x*rate)",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = (float)l_EnMuki[mDirection] * (speed.x * rate);"""),

    ("A6: mSpeed.y=speed.y; mSpeed.x=(float)l_EnMuki[mDirection]*(rate*speed.x)",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = (float)l_EnMuki[mDirection] * (rate * speed.x);"""),

    ("A7: mSpeed.y=speed.y; mSpeed.x=((float)l_EnMuki[mDirection]*rate)*speed.x",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = ((float)l_EnMuki[mDirection] * rate) * speed.x;"""),

    ("A8: mSpeed.y=speed.y; mSpeed.x=((float)l_EnMuki[mDirection]*speed.x)*rate",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    mSpeed.x = ((float)l_EnMuki[mDirection] * speed.x) * rate;"""),

    ("A9: mSpeed.y=speed.y; float sx=speed.x; mSpeed.x=(sx*rate)*(float)l_EnMuki[mDirection]",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (sx * rate) * (float)l_EnMuki[mDirection];"""),

    ("A10: mSpeed.y=speed.y; float sx=speed.x; mSpeed.x=(rate*sx)*(float)l_EnMuki[mDirection]",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (rate * sx) * (float)l_EnMuki[mDirection];"""),

    ("A11: mSpeed.y=speed.y; float sx=speed.x; mSpeed.x=(float)l_EnMuki[mDirection]*(rate*sx)",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (float)l_EnMuki[mDirection] * (rate * sx);"""),

    ("A12: mSpeed.y=speed.y; float sx=speed.x; mSpeed.x=((float)l_EnMuki[mDirection]*rate)*sx",
     """    float rate = calcJumpRate();
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = ((float)l_EnMuki[mDirection] * rate) * sx;"""),
]

for name, code in algs:
    test_jump_tail(name, code)

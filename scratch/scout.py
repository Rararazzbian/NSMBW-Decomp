import os
import sys
import json

sys.path.insert(0, os.path.abspath('.'))

with open('bin/dtk/dtk_splits_wiimj2d.txt', 'r', encoding='utf-8') as f:
    splits_text = f.read()

print("dtk_splits_wiimj2d.txt total lines:", len(splits_text.splitlines()))
print("First 40 lines:")
for line in splits_text.splitlines()[:40]:
    print(line)

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
syms_file = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'

symbols_to_check = [
    'getCourseIn__10dScStage_cFv',
    '__dt__Q23EGG8Vector3fFv',
    'isItemKinopio__7dAcPy_cFv',
    '__dt__7mVec2_cFv',
    '__dt__Q23EGG8Vector2fFv',
    'executeLastPlayer__10daPlBase_cFv',
    'executeLastAll__10daPlBase_cFv',
    'getPlrNo__8dActor_cFv',
    '__sinit_\d_a_player_manager_cpp'
]

lines = syms_file.read_text().splitlines()

for sym in symbols_to_check:
    print(f"\nChecking: {sym}")
    found = False
    for line in lines:
        if line.startswith(sym + ' ') or line.startswith(sym + '='):
            print(f"  {line}")
            found = True
    if not found:
        print("  NOT FOUND in wiimj2d_symbols.txt")

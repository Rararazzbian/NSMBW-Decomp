import sys
sys.path.insert(0, r'..\..\tools\auto_decomp')
import harness

cases = [
    ('calcMdl__21dIggyWanKusariPiece_cFv', 'auto_03_800BAB04_text.txt'),
    ('draw__21dIggyWanKusariPiece_cFv', 'auto_03_800BAB04_text.txt'),
    ('collapseMove__21dIggyWanKusariPiece_cFv', 'auto_03_800BAB04_text.txt'),
    ('make_kusari__16dIggyWanKusari_cFv', 'auto_03_800B9098_text.txt'),
    ('initializeState_Collapse__16dIggyWanKusari_cFv', 'auto_03_800B9098_text.txt'),
    ('executeState_Ready__16dIggyWanKusari_cFv', 'auto_03_800B9098_text.txt'),
]
for n, tf in cases:
    r = harness.diff_fn(tf, 'd_iggy_wan_kusari.txt', n)
    print('=' * 70)
    print(n, '| matched=', r[0])
    print(r[1][:1700])

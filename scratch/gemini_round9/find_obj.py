import os
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
dtkspl = ROOT / 'bin' / 'dtkspl'

for p in dtkspl.rglob('*800272F0*'):
    print(p)

for p in dtkspl.rglob('*auto_*'):
    if '80027' in p.name or '80026' in p.name or '80028' in p.name:
        print(p)

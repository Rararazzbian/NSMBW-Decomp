from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
dtkspl = ROOT / 'bin' / 'dtkspl' / 'obj'

for p in dtkspl.rglob('*80028*'):
    print(p)

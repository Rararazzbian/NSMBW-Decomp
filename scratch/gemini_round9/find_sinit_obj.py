import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
dtkspl = ROOT / 'bin' / 'dtkspl' / 'obj'

for p in dtkspl.rglob('*.o'):
    if 'auto_' in p.name and 'text' in p.name:
        parts = p.name.split('_')
        if len(parts) >= 3:
            try:
                addr = int(parts[2], 16)
                if addr <= 0x80028150 < addr + 0x1000:
                    print(f"Contains 0x80028150: {p}")
            except:
                pass

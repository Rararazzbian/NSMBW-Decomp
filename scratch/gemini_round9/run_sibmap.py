import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SIBMAP = ROOT / 'tools' / 'sibmap.py'

cmd = [
    sys.executable, str(SIBMAP), 'map',
    '--lo', '0x800272F0',
    '--hi', '0x800281C0',
    '--out', str(ROOT / 'scratch' / 'gemini_round9')
]

p = subprocess.run(cmd, cwd=str(ROOT), capture_output=True, text=True)
print("Return code:", p.returncode)
print("STDERR:", p.stderr)
print("STDOUT (first 2000 chars):", p.stdout[:2000])

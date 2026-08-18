import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sibmap_json_path = ROOT / 'scratch' / 'gemini_round9' / 'sibmap.json'

data = json.loads(sibmap_json_path.read_text())
print("Keys of item 0:", data[0].keys())
print("item 0 top:", data[0].get('top'))

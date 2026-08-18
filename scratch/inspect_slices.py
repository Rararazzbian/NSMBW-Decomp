import json

with open('slices/wiimj2d.json', 'r') as f:
    slices = json.load(f)

for s in slices['slices']:
    name = s.get('name', s.get('source', 'unknown'))
    origin = s.get('origin', '')
    secs = s.get('sections', {})
    text_range = secs.get('.text', [])
    print(f"{str(name):50} origin: {str(origin):10} .text: {text_range}")

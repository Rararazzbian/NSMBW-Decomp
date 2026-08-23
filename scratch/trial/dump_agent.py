import io
import json
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, errors='replace')
p = (r'C:\Users\Razz\.qwen\projects\c--users-razz-documents-projects-nsmbw-decomp'
     r'\subagents\4b8a4317-1ad7-4744-affa-4484a8b5a177'
     r'\agent-general-purpose-99ec9a50-b7bb-4838-9a07-f4bb91eef7e6.jsonl')
lines = [l for l in open(p, encoding='utf-8', errors='replace') if l.strip()]
best = ''
for l in lines:
    d = json.loads(l)
    m = d.get('message') or {}
    c = m.get('parts') or []
    txt = ' '.join(x.get('text', '') for x in c if isinstance(x, dict) and 'text' in x)
    if len(txt) > len(best):
        best = txt
print(best)

import re

def get_bytes(path, fname):
    fn_start = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
    fn_end = re.compile(r'^\.endfn\b')
    insn_word = re.compile(r'^/\*\s*\S+\s+\S+\s+((?:[0-9A-Fa-f]{2}\s+){3}[0-9A-Fa-f]{2})\s*\*/')
    body = None
    out = []
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = fn_start.match(s)
            if m:
                body = [] if m.group(1) == fname else None
                continue
            if fn_end.match(s):
                if body is not None:
                    return body
                continue
            if body is not None:
                mw = insn_word.match(s)
                if mw:
                    body.append(mw.group(1).replace(' ', ''))
    return body

t = get_bytes('wip/agent_line_mng/work/target.txt', 'executeState_Left30Left__10dLineMng_cFv')
d = get_bytes('wip/fix_bigtwo/_tally/d.txt', 'executeState_Left30Left__10dLineMng_cFv')
print('target words:', len(t) if t else None)
print('draft words:', len(d) if d else None)
print('RAW BYTES EQUAL:', t == d)
if t and d:
    for i in range(max(len(t), len(d))):
        a = t[i] if i < len(t) else None
        b = d[i] if i < len(d) else None
        if a != b:
            print(i, a, b)

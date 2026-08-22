import re

hdr = open('wip/kokoopa_verify5/include/game/bases/d_enemy_toride_kokoopa.hpp', encoding='utf-8').read()
cpp = open('wip/kokoopa_verify5/d_enemy_toride_kokoopa.cpp', encoding='utf-8').read()

def get_class_body(text, classname):
    m = re.search(r'class\s+' + re.escape(classname) + r'\s*(?::[^\{]*)?\{', text)
    if not m:
        return None
    start = m.end() - 1
    depth = 0
    for i in range(start, len(text)):
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                return text[start+1:i]
    return None

def declared_methods(classname):
    body = get_class_body(hdr, classname)
    state_names = re.findall(r'STATE_VIRTUAL_FUNC_DECLARE\(\s*' + re.escape(classname) + r'\s*,\s*(\w+)\s*\)', body)
    state_fns = []
    for n in state_names:
        state_fns += [f'initializeState_{n}', f'executeState_{n}', f'finalizeState_{n}']

    plain_fns = []
    inline_defined = set()
    for line in body.splitlines():
        l = line.strip()
        if not l or l.startswith('//') or 'STATE_VIRTUAL_FUNC_DECLARE' in l:
            continue
        m = re.match(r'(?:virtual\s+)?(?:static\s+)?[\w:<>,\*&~ ]+?\s[\*&]*(~?\w+)\s*\([^;{]*\)\s*(?:const)?\s*(=\s*0)?\s*([;{])', l)
        if m:
            name = m.group(1)
            plain_fns.append(name)
            if m.group(3) == '{':
                inline_defined.add(name)
    return state_fns, plain_fns, inline_defined

for cls in ['KokoopaSpFumiCheck_c', 'dEnTorideKokoopa_c']:
    state_fns, plain_fns, inline_defined = declared_methods(cls)
    all_fns = state_fns + plain_fns
    print(f"=== {cls}: {len(all_fns)} declared methods ({len(inline_defined)} inline-defined) ===")
    missing = []
    for fn in all_fns:
        if fn in inline_defined:
            continue
        pat = re.escape(cls) + r'::' + re.escape(fn) + r'\s*\('
        if not re.search(pat, cpp):
            missing.append(fn)
    print(f"Missing definitions ({len(missing)}):")
    for m in missing:
        print(" -", m)

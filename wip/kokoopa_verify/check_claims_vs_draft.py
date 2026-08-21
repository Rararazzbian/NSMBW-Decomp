import re, sys
sys.path.insert(0, 'tools/auto_decomp')
import harness

text = open('GEMINI_RESPONSE.md', encoding='utf-8').read()
start = text.find('## 4. Per-Function Table')
end = text.find('## 5.')
table = text[start:end]
rows = re.findall(r'^\|\s*(\d+)\s*\|\s*`(0x[0-9A-Fa-f.]+)`\s*\|\s*`([^`]+)`\s*\|\s*MATCH\s*\|', table, re.M)

# expand the two grouped rows using the known state lists from the header
sStateID_states = ['Jump_St','Jump','BigJump_St','BigJump','LandOn','AttackReady','AttackBegin',
                    'AttackSearch','Attack','AttackEnd','FumiHit','FireHit','SlideHit','StarHit',
                    'QuakeHit','ShellHit','ShellAtk_St','ShellAtk','ShellOut','DieFumi_St',
                    'DemoAwake','DemoAwake_Wait','DemoIkaku','DemoIkaku_Wait','DemoEscape_St']
dEnBoss_states = ['DemoWait','DieShell','DieFire']

claimed = []
for size, addr, name in rows:
    if 'sStateID_c> (25' in name or name.startswith('baseID_<StateName><sStateID_c>'):
        for s in sStateID_states:
            claimed.append('baseID_%s<10sStateID_c>__Fv_RC12sStateIDIf_c' % s)
    elif 'dEnBoss_c> (3' in name or name.startswith('baseID_<StateName><dEnBoss_c>'):
        for s in dEnBoss_states:
            claimed.append('baseID_%s<9dEnBoss_c>__Fv_RC12sStateIDIf_c' % s)
    else:
        claimed.append(name)

print('total claimed match names (expanded):', len(claimed))

draft_fns = set(harness.list_functions('wip/kokoopa_verify/draft.txt'))
print('draft function count:', len(draft_fns))

missing_from_draft = [c for c in claimed if not any(harness.norm_name(c) == harness.norm_name(d) or c in d or d in c for d in draft_fns)]
print('\nclaimed-as-MATCH but draft never emits (norm-name lookup):')
for m in missing_from_draft:
    print(' ', m)

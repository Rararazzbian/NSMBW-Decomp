import re
import os

with open('scratch/gemini_pswmgr/auto_03_800D80A8_text.txt', 'r', encoding='utf-8') as f:
    text = f.read()

fns = [
    '__ct__13dPSwManager_cFv',
    '__dt__13dPSwManager_cFv',
    'initialize__13dPSwManager_cFv',
    'execute__13dPSwManager_cFv',
    'ProcMain__13dPSwManager_cFv',
    'finalize__13dPSwManager_cFv',
    'checkSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e',
    'checkMove__13dPSwManager_cFv',
    'getTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_e',
    'onSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_ei',
    'offSwitch__13dPSwManager_cFQ213dPSwManager_c8SwType_e',
    'setTimer__13dPSwManager_cFQ213dPSwManager_c8SwType_ei',
]

target_out = []
for fn in fns:
    pattern = re.compile(rf'(?:# [^\n]*\n)?\.fn \"?{re.escape(fn)}\"?,\s*global.*?\n\.endfn\b', re.DOTALL)
    m = pattern.search(text)
    if m:
        target_out.append(m.group(0))
    else:
        print('MISSING:', fn)

with open('scratch/gemini_pswmgr/target.txt', 'w', encoding='utf-8') as f:
    f.write('\n\n'.join(target_out) + '\n')

print(f'Wrote target.txt with {len(target_out)} functions')

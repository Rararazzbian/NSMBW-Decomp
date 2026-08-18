import json

with open('scratch/d_enemiesNP_tus.json') as f:
    tus = json.load(f)

small_tus = [t for t in tus if t['code'] < 4500 and t['fns'] >= 4]
small_tus.sort(key=lambda t: t['code'])

print(f'Total small TUs (< 4500 B): {len(small_tus)}')
for t in small_tus:
    p = ', '.join(t['profs']) if t['profs'] else 'NO_PROFILE'
    print(f"TU {t['tu_idx']:3d} (.ctors {t['ctor_off']}): .text {t['start']}-{t['end']} ({t['code']} B code, {t['span']} B span, {t['fns']} fns) -> {p}")

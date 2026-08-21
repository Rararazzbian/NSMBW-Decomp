with open('scratch/gemini_round14/vtable_kokoopa.txt', 'r', encoding='utf-8') as f:
    kokoopa_lines = [l.strip() for l in f]

new_slots = kokoopa_lines[226:]
print(f"Total new virtual slots in dEnTorideKokoopa_c: {len(new_slots)}")

for idx, line in enumerate(new_slots):
    print(f"[slot {226+idx:3d} / new {idx:3d}] {line}")


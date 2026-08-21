with open('scratch/gemini_round14/vtable_dEnBoss.txt', 'r', encoding='utf-8') as f:
    boss_lines = [l.strip() for l in f]

with open('scratch/gemini_round14/vtable_kokoopa.txt', 'r', encoding='utf-8') as f:
    kokoopa_lines = [l.strip() for l in f]

print("=== First 30 slots ===")
for i in range(min(30, len(boss_lines))):
    b = boss_lines[i]
    k = kokoopa_lines[i] if i < len(kokoopa_lines) else "NONE"
    print(f"Boss:    {b}")
    print(f"Kokoopa: {k}")
    print("-" * 50)

print("\n=== Slots 200 to 235 ===")
for i in range(200, min(240, len(kokoopa_lines))):
    b = boss_lines[i] if i < len(boss_lines) else "--- END OF BOSS ---"
    k = kokoopa_lines[i]
    print(f"[{i:3d}] Boss:    {b}")
    print(f"      Kokoopa: {k}")

print("\n=== Slots 350 to 375 of Kokoopa ===")
for i in range(350, len(kokoopa_lines)):
    print(f"Kokoopa: {kokoopa_lines[i]}")


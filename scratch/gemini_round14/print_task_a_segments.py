with open('scratch/gemini_round14/task_a_funcs.txt', 'r', encoding='utf-8') as f:
    lines = [l.strip() for l in f]

for l in lines[:30]:
    print(l)
print("...")
for l in lines[115:140]:
    print(l)
print("...")
for l in lines[210:245]:
    print(l)
print("...")
for l in lines[355:380]:
    print(l)
print("...")
for l in lines[410:435]:
    print(l)
print("...")
for l in lines[-20:]:
    print(l)

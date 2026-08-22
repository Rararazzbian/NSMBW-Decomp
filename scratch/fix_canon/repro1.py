import sys, os
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness

target_lines = [
    'lis r31, "@49614_80359100"@ha',
    'addi r31, r31, "@49614_80359100"@l',
]
draft_lines = [
    'lis r29, ...bss.0@ha',
]

print("POOL_SYM pattern:", harness.POOL_SYM.pattern)
print()
for l in target_lines:
    print(repr(l), '->', repr(harness.POOL_SYM.sub(lambda m: 'MATCH[%s]' % m.group(0), l)))
for l in draft_lines:
    print(repr(l), '->', repr(harness.POOL_SYM.sub(lambda m: 'MATCH[%s]' % m.group(0), l)))

print()
print("canonicalise(target):", harness.canonicalise(target_lines))
print("canonicalise(draft joined with matching structure):")
draft_lines2 = [
    'lis r29, ...bss.0@ha',
    'addi r29, r29, ...bss.0@l',
]
print(harness.canonicalise(draft_lines2))

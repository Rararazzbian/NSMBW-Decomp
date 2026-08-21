import sys, os, re, hashlib
sys.path.insert(0, os.path.join('tools','auto_decomp'))
import harness

OVERRIDE = os.path.join('wip','vec3pod','variants','v1_no_copyctor')

def module_for(path):
    p = path.replace(chr(92), '/')
    if p.startswith('source/dol/'):
        return 'wiimj2d'
    if p.startswith('source/d_basesNP/'):
        return 'd_basesNP'
    if p.startswith('source/d_enemiesNP/'):
        return 'd_enemiesNP'
    if p.startswith('source/d_profileNP/'):
        return 'd_profileNP'
    if p.startswith('source/d_en_bossNP/'):
        return 'd_en_bossNP'
    return None

def compile_variant(src, tag, extra_inc):
    base = os.path.splitext(os.path.basename(src))[0]
    obj = os.path.join('wip','vec3pod','out', f'{base}_{tag}.o')
    dis = os.path.join('wip','vec3pod','out', f'{base}_{tag}_disasm.txt')
    mod = module_for(src)
    ok, log = harness.compile_draft(src, obj, extra_inc=extra_inc, module=mod)
    if not ok:
        return None, log
    ok2, log2 = harness.disasm(obj, dis)
    if not ok2:
        return None, log2
    return dis, None

def main():
    files = [l.strip() for l in open(sys.argv[1]) if l.strip()]
    results = []
    for src in files:
        mod = module_for(src)
        if mod is None:
            print(f'SKIP (no module mapping): {src}')
            continue
        dis_a, err_a = compile_variant(src, 'orig', [])
        if err_a:
            print(f'FAIL(orig) {src}: {err_a[-400:]}')
            continue
        dis_b, err_b = compile_variant(src, 'mod', [OVERRIDE])
        if err_b:
            print(f'FAIL(mod)  {src}: {err_b[-400:]}')
            continue
        with open(dis_a, encoding='utf-8', errors='replace') as fh:
            ta = harness.canonicalise(fh.readlines())
        with open(dis_b, encoding='utf-8', errors='replace') as fh:
            tb = harness.canonicalise(fh.readlines())
        if ta == tb:
            print(f'SAME       {src}')
        else:
            print(f'DIFF       {src}')
            results.append(src)
    print()
    print('Files with differences:', results)

if __name__ == '__main__':
    main()

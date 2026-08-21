import sys, os
sys.path.insert(0, os.path.join('tools','auto_decomp'))
import harness

def main():
    diff_files = [l.strip() for l in sys.stdin if l.strip()]
    total_changed = 0
    for src in diff_files:
        base = os.path.splitext(os.path.basename(src))[0]
        dis_a = os.path.join('wip','vec3pod','out', f'{base}_orig_disasm.txt')
        dis_b = os.path.join('wip','vec3pod','out', f'{base}_mod_disasm.txt')
        if not (os.path.exists(dis_a) and os.path.exists(dis_b)):
            print(f'MISSING disasm for {src}')
            continue
        names = harness.list_functions(dis_a)
        names_b = harness.list_functions(dis_b)
        changed = []
        for n in names:
            ba = harness.extract(dis_a, n)
            bb = harness.extract(dis_b, n)
            if ba != bb:
                changed.append(n)
        total_changed += len(changed)
        print(f'{src}: {len(changed)} function(s) changed: {changed}')
    print()
    print('TOTAL functions changed across all DIFF files:', total_changed)

if __name__ == '__main__':
    main()

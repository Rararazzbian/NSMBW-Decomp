import sys
sys.path.insert(0, 'tools/auto_decomp')
import harness

def check(name):
    obj = 'wip/line_mng_r4/smallsix/work/draft.o'
    harness.compile_draft('wip/line_mng_r4/smallsix/draft.cpp', obj, extra_inc=('wip/line_mng_r4/smallsix/shadow_include',))
    harness.disasm(obj, 'wip/line_mng_r4/smallsix/work/draft.txt')
    ok, msg = harness.diff_fn('wip/line_mng_shared/target.txt', 'wip/line_mng_r4/smallsix/work/draft.txt', name)
    print(name, '->', ok)
    print(msg)

if __name__ == '__main__':
    for n in sys.argv[1:]:
        check(n)

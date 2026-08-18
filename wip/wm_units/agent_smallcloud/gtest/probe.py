"""Quick iteration harness: compile a candidate d_a_wm_smallcloud.cpp variant,
dump .rodata bytes + symbol table, and run verify_anon + check_sections.
Usage: python probe.py <variant.cpp> [--quiet]
"""
import os
import sys
import struct as st

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H
sys.path.insert(0, os.path.join(ROOT, 'wip', 'wm_units'))
import check_sections as C

HERE = os.path.dirname(os.path.abspath(__file__))


def dump_rodata(path):
    sections, symbols = C.read_elf(path)
    if '.rodata' not in sections or not sections['.rodata']['size']:
        print('  (no .rodata)')
        return
    blob = open(path, 'rb').read()
    shoff, = st.unpack_from('>I', blob, 0x20)
    shentsize, shnum, shstrndx = st.unpack_from('>HHH', blob, 0x2E)
    idx = sections['.rodata']['index']
    for i in range(shnum):
        base = shoff + i * shentsize
        name, _typ, _flags, addr, off, size, link, _info, _align, entsize = \
            st.unpack_from('>10I', blob, base)
        if i == idx:
            data = blob[off:off + size]
            for j in range(0, len(data), 4):
                print('  +0x%02x  %s' % (j, data[j:j + 4].hex()))
    print('  symbols:')
    for sec, value, size, name, weak in sorted(x for x in symbols if x[0] == '.rodata' and x[2]):
        print('    %#06x %#4x %-6s %s' % (value, size, 'WEAK' if weak else 'strong', name))


def main():
    src = sys.argv[1]
    obj = os.path.join(HERE, 'probe.o')
    txt = os.path.join(HERE, 'probe.txt')
    extra_inc = [os.path.join(HERE, '..', 'include'), os.path.join(HERE, '..', 'shadow_include')]
    ok, log = H.compile_draft(src, obj, extra_inc=extra_inc, module='d_basesNP')
    if not ok:
        print('COMPILE FAIL')
        print(log[-4000:])
        return 1
    print('compile OK')
    H.disasm(obj, txt)

    sys.path.insert(0, os.path.join(ROOT, 'wip', 'wm_units'))
    import importlib
    import verify_anon as V
    importlib.reload(V)
    sys.argv = ['verify_anon.py', txt, '0x1797e0', '0x179ff0',
                os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_001797B4_text.o'),
                os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_fn_2_179F40_text.o'),
                os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_00179FC4_text.o')]
    V.main()

    print('\n--- .rodata bytes ---')
    dump_rodata(obj)

    print('\n--- section check ---')
    sections, symbols = C.read_elf(obj)
    claim = {'.text': '0x1797e0-0x179ff0', '.ctors': '0x430-0x434', '.rodata': '0x8fa8-0x8fd8',
             '.data': '0x47258-0x47450', '.bss': '0x102a0-0x102c8'}
    for name, rng in claim.items():
        lo, hi = (int(x, 16) for x in rng.split('-'))
        want = hi - lo
        got = sections.get(name, {}).get('size', 0)
        print('  %-8s claim %#x  object %#x  %s' % (name, want, got, 'ok' if want == got else 'DIFF'))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

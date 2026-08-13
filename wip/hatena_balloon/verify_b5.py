"""Full verification for batch 5, including the checks .text diffing cannot do.

COPY FOR DURABILITY -- this ran from a session scratch directory laid out as
    <scratch>/hb-b5.cpp          the draft
    <scratch>/dis/hb_a.txt       dtk elf disasm of
                                 bin/dtkspl/obj/auto_03_801102B0_text.o
    <scratch>/hb5/verify.py      this file
    <scratch>/hb5/inc/           include overlay, see below
To re-run, recreate that layout (regenerate dis/hb_a.txt with
`bin/dtk-windows-x86_64.exe elf disasm <obj> <out>`) and put this back in hb5/.

The include overlay holds two headers that differ from the committed ones and
that the draft needs to compile: d_a_en_hatena_balloon.hpp with goalpole_check
/ floor_check / all_bgcheck returning bool / u8 / u8 instead of void, and
d_actor_manager.hpp with mGoalPoleX at +0x44 and floorEntryBufferCheck. Neither
change alters a mangled name.

harness.canonicalise() numbers pool references by first appearance, so it proves
the PATTERN of literal references matches, not the VALUES -- a lone 0.0f and a
lone 8.0f compare equal. This reads the actual float out of the draft object's
.sdata2 and compares it to the DOL word at the address the target references.

Run with --corrupt-text or --corrupt-lit to confirm the checks are not vacuous.
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SCR = os.path.dirname(HERE)
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402

TARGET = os.path.join(SCR, 'dis', 'hb_a.txt')
SRC = os.path.join(SCR, 'hb-b5.cpp')
OBJ = os.path.join(HERE, 'verify.o')
TXT = os.path.join(HERE, 'verify.txt')
INC = os.path.join(HERE, 'inc')
SYMS = os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')
DOL = os.path.join(ROOT, 'original', 'wiimj2d.dol')

NAMES = [
    'pointBgCheck__19daEnHatenaBalloon_cFRC7mVec3_cUlUlUl',
    'goalpole_check__19daEnHatenaBalloon_cFv',
    'floor_check__19daEnHatenaBalloon_cFv',
    'all_bgcheck__19daEnHatenaBalloon_cFRUc',
]
RODATA_SYM = 's_someCheckData__19daEnHatenaBalloon_c'

FAIL = []


def bad(msg):
    FAIL.append(msg)
    print('  FAIL: %s' % msg)


# ---------------------------------------------------------------- symbol map

def symbol_map():
    """name -> (section, address, size), read from dtk's map, not hardcoded."""
    out = {}
    pat = re.compile(r'^(\S+)\s*=\s*\.(\w+):(0x[0-9A-Fa-f]+);.*?size:(0x[0-9A-Fa-f]+)')
    for line in open(SYMS, encoding='utf-8', errors='replace'):
        m = pat.match(line.strip())
        if m:
            out[m.group(1)] = (m.group(2), int(m.group(3), 16), int(m.group(4), 16))
    return out


# ---------------------------------------------------------------- ELF reading

def elf(path):
    d = open(path, 'rb').read()
    assert d[:4] == b'\x7fELF'
    e_shoff, = struct.unpack('>I', d[0x20:0x24])
    e_shentsize, e_shnum, e_shstrndx = struct.unpack('>HHH', d[0x2E:0x34])
    secs = []
    for i in range(e_shnum):
        o = e_shoff + i * e_shentsize
        f = struct.unpack('>10I', d[o:o + 40])
        secs.append(dict(name_off=f[0], type=f[1], off=f[4], size=f[5],
                         link=f[6], idx=i))
    stro = secs[e_shstrndx]['off']
    for s in secs:
        end = d.index(b'\0', stro + s['name_off'])
        s['name'] = d[stro + s['name_off']:end].decode()
    return d, secs


def symbols(path):
    """name -> (section name, file offset, size)."""
    d, secs = elf(path)
    out = {}
    for s in secs:
        if s['type'] != 2:
            continue
        strtab = secs[s['link']]
        for i in range(s['size'] // 16):
            o = s['off'] + i * 16
            st_name, st_value, st_size, _, _, st_shndx = \
                struct.unpack('>IIIBBH', d[o:o + 16])
            if st_shndx >= len(secs):
                continue
            end = d.index(b'\0', strtab['off'] + st_name)
            name = d[strtab['off'] + st_name:end].decode()
            sec = secs[st_shndx]
            out[name] = (sec['name'], sec['off'] + st_value, st_size)
    return d, out


def dol_bytes(addr, size):
    d = open(DOL, 'rb').read()
    offs = struct.unpack('>18I', d[0:0x48])
    addrs = struct.unpack('>18I', d[0x48:0x90])
    sizes = struct.unpack('>18I', d[0x90:0xD8])
    for i in range(18):
        if sizes[i] and addrs[i] <= addr < addrs[i] + sizes[i]:
            o = offs[i] + (addr - addrs[i])
            return d[o:o + size]
    raise SystemExit('addr 0x%08X not in DOL' % addr)


# ---------------------------------------------------------------- disasm parsing

FNRE = re.compile(r'^\.fn\s+"?(.+?)"?\s*,')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')
SDA = re.compile(r'"?([@\w.$<>:]+)"?@sda21')


def body(path, name):
    out, inside = [], False
    for line in open(path, encoding='utf-8', errors='replace'):
        s = line.strip()
        m = FNRE.match(s)
        if m:
            inside = (H.norm_name(m.group(1)) == H.norm_name(name))
            continue
        if s.startswith('.endfn'):
            inside = False
            continue
        if inside:
            m = INSN.match(s)
            if m:
                out.append(m.group(1))
    return out


def fn_order(path):
    out = []
    for line in open(path, encoding='utf-8', errors='replace'):
        m = FNRE.match(line.strip())
        if m and not m.group(1).startswith('gap_'):
            out.append(H.norm_name(m.group(1)))
    return out


def target_literals(name):
    """Ordered @sda21 references of the TARGET, resolved to comparable values.

    A compiler-pool literal is named `@<n>_<ADDR>`, so its address is in the
    name and its value comes from the DOL. An ordinary named symbol
    (`m_instance__11dActorMng_c`) is NOT a literal -- compare it by name, or a
    wrong data reference would be scored against a nonsense float.
    """
    vals = []
    for t in body(TARGET, name):
        m = SDA.search(t)
        if not m:
            continue
        sym = m.group(1)
        if not sym.startswith('@'):
            vals.append(('sym', sym))
            continue
        a = re.search(r'_([0-9A-Fa-f]{8})$', sym)
        if not a:
            bad('target pool symbol %r carries no address' % sym)
            vals.append(('lit', None))
            continue
        addr = int(a.group(1), 16)
        vals.append(('lit', struct.unpack('>f', dol_bytes(addr, 4))[0]))
    return vals


def draft_literals(name, syms, data):
    vals = []
    for t in body(TXT, name):
        m = SDA.search(t)
        if not m:
            continue
        sym = m.group(1)
        if not sym.startswith('@'):
            vals.append(('sym', sym))
            continue
        if sym not in syms:
            bad('draft pool symbol %r not in the object symbol table' % sym)
            vals.append(('lit', None))
            continue
        sec, off, _ = syms[sym]
        if sec != '.sdata2':
            bad('draft literal %r sits in %s, expected .sdata2' % (sym, sec))
        vals.append(('lit', struct.unpack('>f', data[off:off + 4])[0]))
    return vals


# ---------------------------------------------------------------- main

def main():
    corrupt_text = '--corrupt-text' in sys.argv
    corrupt_lit = '--corrupt-lit' in sys.argv
    corrupt_tab = '--corrupt-tab' in sys.argv

    src = open(SRC, encoding='utf-8').read()
    if corrupt_text:
        # Must survive the optimiser AND keep the instruction count, so that
        # check 1 still passes and check 2 is the only thing that can catch it.
        # (An earlier attempt added `+ 0.0f`, which mwcc folded away -- the
        # control passed and proved nothing.)
        src = src.replace('pointBgCheck(pt, 1, 1, hit)',
                          'pointBgCheck(pt, 1, 2, hit)')
    if corrupt_lit:
        # one literal only: floor_check's 29.0f -> 28.0f
        src = src.replace('29.0f + mPos.y', '28.0f + mPos.y')
    if corrupt_tab:
        src = src.replace('{ 3.0f, 5.0f, -3.0f, 5.0f, 2 }',
                          '{ 3.0f, 5.5f, -3.0f, 5.0f, 2 }')
    use = SRC
    if corrupt_text or corrupt_lit or corrupt_tab:
        use = os.path.join(HERE, 'corrupt.cpp')
        open(use, 'w', encoding='utf-8', newline='\n').write(src)
        print('*** NEGATIVE CONTROL: %s ***\n' % ' '.join(
            a for a in sys.argv[1:] if a.startswith('--')))

    ok, log = H.compile_draft(use, OBJ, extra_inc=[INC])
    if not ok:
        print('COMPILE FAILED\n' + log[-3000:])
        return 1
    if log.strip():
        print('compiler said: %s\n' % log.strip()[-300:])
    ok, log = H.disasm(OBJ, TXT)
    if not ok:
        print('DISASM FAILED\n' + log[-2000:])
        return 1

    smap = symbol_map()
    data, syms = symbols(OBJ)

    # -------------------------------------------------- 1. size assertion
    print('1. instruction count x 4 == symbol-map size')
    for n in NAMES:
        sec, addr, size = smap[n]
        tn, dn = len(body(TARGET, n)), len(body(TXT, n))
        if tn * 4 != size:
            bad('%s: target %d insn x4 != map size 0x%X' % (n, tn, size))
        elif dn != tn:
            bad('%s: draft %d insn vs target %d' % (n, dn, tn))
        else:
            print('  ok  %-34s 0x%08X  %3d insn = 0x%X'
                  % (n.split('__')[0], addr, tn, size))

    # -------------------------------------------------- 2. text diff
    print('\n2. .text word/branch comparison')
    for n in NAMES:
        good, rep = H.diff_fn(TARGET, TXT, n)
        if good:
            print('  ok  %-34s MATCH' % n.split('__')[0])
        else:
            bad('%s .text differs\n%s' % (n.split('__')[0],
                                          '\n'.join(rep.splitlines()[:12])))

    # -------------------------------------------------- 3. callee symbols
    print('\n3. callee symbol names (dtk zeroes relocations; words cannot see these)')
    for n in NAMES:
        tc = [re.match(r'bl\s+"?([^"\s]+)', t).group(1)
              for t in body(TARGET, n) if re.match(r'bl\s', t)]
        dc = [re.match(r'bl\s+"?([^"\s]+)', t).group(1)
              for t in body(TXT, n) if re.match(r'bl\s', t)]
        if tc == dc:
            print('  ok  %-34s %d calls identical' % (n.split('__')[0], len(tc)))
        else:
            bad('%s callee mismatch\n    target %s\n    draft  %s'
                % (n.split('__')[0], tc, dc))

    # -------------------------------------------------- 4. literal VALUES
    print('\n4. .sdata2 literal values (the check canonicalise() cannot make)')
    for n in NAMES:
        tv = target_literals(n)
        dv = draft_literals(n, syms, data)
        if len(tv) != len(dv):
            bad('%s: %d literal refs in target, %d in draft'
                % (n.split('__')[0], len(tv), len(dv)))
            continue
        if tv == dv:
            lits = ['%g' % v for k, v in tv if k == 'lit']
            print('  ok  %-34s %d literal refs %s | %d named data refs'
                  % (n.split('__')[0], len(lits),
                     '[' + ', '.join(lits) + ']' if lits else '(none)',
                     len([1 for k, _ in tv if k == 'sym'])))
        else:
            bad('%s .sdata2 references differ\n    target %s\n    draft  %s'
                % (n.split('__')[0], tv, dv))

    # -------------------------------------------------- 5. rodata table bytes
    print('\n5. s_someCheckData bytes vs original/wiimj2d.dol')
    sec, addr, size = smap[RODATA_SYM]
    if RODATA_SYM not in syms:
        bad('%s not emitted' % RODATA_SYM)
    else:
        dsec, off, dsize = syms[RODATA_SYM]
        got = data[off:off + dsize]
        want = dol_bytes(addr, size)
        if dsec != '.rodata':
            bad('%s landed in %s, expected .rodata' % (RODATA_SYM, dsec))
        if dsize != size:
            bad('%s size 0x%X, map says 0x%X' % (RODATA_SYM, dsize, size))
        elif got != want:
            bad('%s BYTES DIFFER' % RODATA_SYM)
            for i in range(0, size, 4):
                if want[i:i + 4] != got[i:i + 4]:
                    print('    +0x%02X want %s (%g)  got %s (%g)'
                          % (i, want[i:i + 4].hex(),
                             struct.unpack('>f', want[i:i + 4])[0],
                             got[i:i + 4].hex(),
                             struct.unpack('>f', got[i:i + 4])[0]))
        else:
            print('  ok  0x%08X size 0x%X in %s, byte-exact' % (addr, size, dsec))

    # -------------------------------------------------- 6. emitted order
    print('\n6. emitted .text order vs target address order')
    want = [n for n in sorted(NAMES, key=lambda x: smap[x][1])]
    got = [n for n in fn_order(TXT) if n in NAMES]
    if want == got:
        print('  ok  %s' % ' -> '.join(n.split('__')[0] for n in got))
    else:
        bad('order differs\n    want %s\n    got  %s' % (want, got))
    extra = [n for n in fn_order(TXT) if n not in NAMES]
    if extra:
        print('  note: object also emits %s' % extra)

    print('\n%s' % ('ALL CHECKS PASSED' if not FAIL
                    else '%d CHECK(S) FAILED' % len(FAIL)))
    return 1 if FAIL else 0


if __name__ == '__main__':
    sys.exit(main())

"""The gate. Nothing reaches source/ unless the real build proves it byte-exact.

The harness only ever writes to tools/auto_decomp/work/. This is the ONLY script
that touches the project, and it refuses to leave anything behind that does not
pass the same check a human would run:

    ninja  &&  python progress.py --verify-bin   ->  5/5 binaries hash-identical

If any part fails, every file it touched is restored and it exits non-zero. A
model cannot talk its way past this: nothing here reads the model's opinion, only
the MD5 of the produced binaries against the original game.

    python tools/auto_decomp/land.py --unit dol/bases/d_a_foo.cpp \
        --cpp work/dol_bases_d_a_foo/MATCHED.cpp \
        --hpp work/dol_bases_d_a_foo/d_a_foo.hpp \
        --slice '{".text": "0x1000-0x2000", ".ctors": "0x40-0x44"}'

Deliberately NOT automated further: it does not commit, and it does not push.
A human (or a stronger model) reviews the diff and writes the commit message.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def sh(*args, **kw):
    return subprocess.run(list(args), cwd=ROOT, capture_output=True, text=True, **kw)


def git_clean():
    return not sh('git', 'status', '--porcelain').stdout.strip()


def restore(paths, created):
    for p in created:
        if os.path.exists(p):
            os.remove(p)
    if paths:
        sh('git', 'checkout', '--', *paths)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--unit', required=True, help='slice source path, e.g. dol/bases/d_a_foo.cpp')
    ap.add_argument('--cpp', required=True, help='candidate .cpp to install')
    ap.add_argument('--hpp', help='candidate header to install')
    ap.add_argument('--hpp-dest', help='header destination (default include/game/bases/<name>)')
    ap.add_argument('--slice', help='JSON object of memoryRanges for slices/wiimj2d.json')
    ap.add_argument('--syms', nargs='*', default=[], help='name=0xADDR lines to append')
    args = ap.parse_args()

    if not git_clean():
        sys.exit('REFUSING: working tree is dirty. Commit or stash first so a '
                 'failed landing can be reverted cleanly.')

    cpp_dest = os.path.join(ROOT, 'source', args.unit.replace('/', os.sep))
    touched, created = [], []

    # --- stage ---------------------------------------------------------------
    os.makedirs(os.path.dirname(cpp_dest), exist_ok=True)
    (touched if os.path.exists(cpp_dest) else created).append(
        os.path.relpath(cpp_dest, ROOT) if os.path.exists(cpp_dest) else cpp_dest)
    shutil.copyfile(args.cpp, cpp_dest)

    if args.hpp:
        dest = args.hpp_dest or os.path.join(
            ROOT, 'include', 'game', 'bases', os.path.basename(args.hpp))
        os.makedirs(os.path.dirname(dest), exist_ok=True)
        (touched if os.path.exists(dest) else created).append(
            os.path.relpath(dest, ROOT) if os.path.exists(dest) else dest)
        shutil.copyfile(args.hpp, dest)

    slices_path = os.path.join(ROOT, 'slices', 'wiimj2d.json')
    syms_path = os.path.join(ROOT, 'syms.txt')

    if args.slice:
        raw = open(slices_path, encoding='utf-8', newline='').read()
        doc = json.loads(raw)
        if any(s.get('source') == args.unit for s in doc['slices']):
            restore(touched, created)
            sys.exit('REFUSING: %s already has a slice entry.' % args.unit)
        ranges = json.loads(args.slice)
        start = int(ranges['.text'].split('-')[0], 16)
        after = next((s['source'] for s in doc['slices']
                      if s['memoryRanges'].get('.text')
                      and int(s['memoryRanges']['.text'].split('-')[0], 16) > start), None)
        if not after:
            restore(touched, created)
            sys.exit('REFUSING: could not place the slice in .text order.')
        nl = '\r\n' if '\r\n' in raw else '\n'
        ind = ' ' * 8
        blob = (ind + '{' + nl + ind + '    "source": "%s",' % args.unit + nl +
                ind + '    "memoryRanges": {' + nl +
                (',' + nl).join(ind + '        "%s": "%s"' % kv for kv in ranges.items()) + nl +
                ind + '    }' + nl + ind + '},' + nl)
        i = raw.index('"source": "%s"' % after)
        j = raw.rindex('{', 0, i)
        raw = raw[:j - len(ind)] + blob + raw[j - len(ind):]
        json.loads(raw)  # must still parse
        open(slices_path, 'w', encoding='utf-8', newline='').write(raw)
        touched.append('slices/wiimj2d.json')

    if args.syms:
        with open(syms_path, 'a', encoding='utf-8', newline='') as fh:
            for line in args.syms:
                fh.write(line.rstrip() + '\n')
        touched.append('syms.txt')

    # --- prove it ------------------------------------------------------------
    print('building...')
    if sh('python', 'configure.py').returncode != 0:
        restore(touched, created)
        sys.exit('REJECTED: configure.py failed. Nothing was kept.')

    build = sh('ninja')
    if build.returncode != 0:
        restore(touched, created)
        tail = (build.stdout + build.stderr).strip().splitlines()[-12:]
        sys.exit('REJECTED: build failed. Nothing was kept.\n' + '\n'.join(tail))

    print('verifying against the original binaries...')
    verify = sh('python', 'progress.py', '--verify-bin')
    if verify.returncode != 0 or 'Failed' in verify.stdout:
        restore(touched, created)
        bad = [l for l in verify.stdout.splitlines() if 'Failed' in l]
        sys.exit('REJECTED: binaries do not match the original. Nothing was kept.\n' +
                 '\n'.join(bad[:6]))

    summary = sh('python', 'progress.py', '--progress-summary').stdout
    total = next((l for l in summary.splitlines() if l.startswith('Total:')), '')
    print('\nACCEPTED -- all five binaries are byte-identical to the original.')
    print(total.strip())
    print('\nStaged but NOT committed. Review with `git diff` and commit yourself.')


if __name__ == '__main__':
    main()

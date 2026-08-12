"""Build a self-contained HTML progress map of the decompilation.

Renders the same information decomp.dev shows for this project, but from the
local working tree, so it reflects uncommitted and unpushed work.

    python tools/progress_page/make_progress_page.py [-o OUT]

Reads:
  objdiff.json              the unit list (regenerate with prepare_objdiff.py)
  slices/wiimj2d.json       to find nonMatching slices
  progress.py               for the authoritative byte counts

Writes a single HTML file with no external requests, safe to open locally or
publish anywhere.

NOTE on `nonMatching`: objdiff reports those units as complete because a source
file exists for them, but progress.py does not count them, because they do not
reproduce the original bytes. This script trusts progress.py -- a nonMatching
unit is shown amber, never green. Keep that precedence if you edit it, or the
page will claim more than the build can prove.
"""
import argparse
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HERE = os.path.dirname(os.path.abspath(__file__))


def git(*args, default=''):
    try:
        out = subprocess.run(['git'] + list(args), cwd=ROOT,
                             capture_output=True, text=True, check=True)
        return out.stdout.strip()
    except Exception:
        return default


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('-o', '--out', default=os.path.join(ROOT, 'bin', 'progress.html'),
                    help='output HTML path (default: bin/progress.html)')
    args = ap.parse_args()

    objdiff = os.path.join(ROOT, 'objdiff.json')
    if not os.path.exists(objdiff):
        sys.exit('objdiff.json not found -- run `python prepare_objdiff.py` first.')

    with open(objdiff, encoding='utf-8') as fh:
        od = json.load(fh)
    with open(os.path.join(ROOT, 'slices', 'wiimj2d.json'), encoding='utf-8') as fh:
        sl = json.load(fh)

    nonmatching = {s['source'] for s in sl['slices'] if s.get('nonMatching')}

    units = []
    for u in od['units']:
        meta = u.get('metadata', {})
        code, data = u.get('code_size') or 0, u.get('data_size') or 0
        if code + data == 0:
            continue
        name = u['name']
        if name in nonmatching or name + '.cpp' in nonmatching:
            state = 'wip'                      # progress.py does not count these
        elif meta.get('complete'):
            state = 'done'
        else:
            state = 'todo'
        units.append({
            'n': name, 'c': code, 'd': data, 's': state,
            'g': (meta.get('progress_categories') or ['dol'])[0],
            'a': 1 if meta.get('auto_generated') else 0,
        })
    units.sort(key=lambda x: -(x['c'] + x['d']))

    summary = subprocess.run([sys.executable, 'progress.py', '--progress-summary'],
                             cwd=ROOT, capture_output=True, text=True).stdout
    totals = {}
    for m in re.finditer(r'(\S+): Decompiled (\d+)/(\d+) code bytes \(([\d.]+)%\)', summary):
        totals[m.group(1)] = {'done': int(m.group(2)), 'total': int(m.group(3)),
                              'pct': float(m.group(4))}
    grand = re.search(r'Total: Decompiled (\d+)/(\d+) code bytes \(([\d.]+)%\)', summary)
    if not grand:
        sys.exit('could not parse progress.py output -- did the build succeed?')

    payload = {
        'units': units,
        'totals': totals,
        'grand': {'done': int(grand.group(1)), 'total': int(grand.group(2)),
                  'pct': float(grand.group(3))},
        'counts': {k: sum(1 for u in units if u['s'] == k) for k in ('done', 'wip', 'todo')},
        'commit': git('log', '-1', '--format=%h', default='working tree'),
        'subject': git('log', '-1', '--format=%s', default=''),
        'when': git('log', '-1', '--format=%cI', default=''),
    }

    with open(os.path.join(HERE, 'template.html'), encoding='utf-8') as fh:
        tpl = fh.read()
    html = (tpl
            .replace('__DATA__', json.dumps(payload, separators=(',', ':')))
            .replace('__BRANCH__', git('rev-parse', '--abbrev-ref', 'HEAD', default='?')))
    assert '__DATA__' not in html and '__BRANCH__' not in html

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    with open(args.out, 'w', encoding='utf-8') as fh:
        fh.write(html)

    print('%s  (%.1f KB)' % (args.out, os.path.getsize(args.out) / 1024))
    print('%.3f%%  %d exact / %d nearly / %d not started'
          % (payload['grand']['pct'], payload['counts']['done'],
             payload['counts']['wip'], payload['counts']['todo']))


if __name__ == '__main__':
    main()

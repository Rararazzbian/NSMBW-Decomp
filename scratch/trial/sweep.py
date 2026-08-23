"""Sweep source shapes for dPosShake_c::move() and report fndiff counts."""
import io
import subprocess
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, errors='replace')

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\trial'

HEADER_ACCESORS = True  # shadow header currently exposes getOffset()/getK()

INIT = '''#include <game/bases/d_pos_shake.hpp>

void dPosShake_c::init(f32 k, f32 damp, f32 limit, f32 offset, f32 vel, f32 snap) {
    mK = k;
    mDamp = damp;
    mLimit = limit;
    mOffset = offset;
    mVel = vel;
    mSnap = snap;
}
'''

TAIL = '''
void dPosShake_c::startShake(f32 amt) {
    mVel += amt;
}
'''

BODY_CLAMP = '''
    if (vel >= mLimit) {
        vel = mLimit;
    } else if (vel <= -mLimit) {
        vel = -mLimit;
    }
'''

BODY_PULL = '''
    if (vel < 0.0f) {
        vel += mDamp;
        if (vel >= 0.0f) {
            vel = 0.0f;
        }
    } else {
        vel -= mDamp;
        if (vel <= 0.0f) {
            vel = 0.0f;
        }
    }
'''

BODY_BOTTOM = '''
    if (vel >= -mSnap && vel <= mSnap) {
        if (x < 0.0f) {
            x += mSnap;
            if (x >= 0.0f) {
                x = 0.0f;
                vel = 0.0f;
            }
        } else {
            x -= mSnap;
            if (x <= 0.0f) {
                x = 0.0f;
                vel = 0.0f;
            }
        }
    } else {
        x += vel;
    }

    mOffset = x;
    mVel = vel;
}
'''


def x_src(form):
    if form == 'init':
        return ['    f32 x = mOffset;', None]
    if form == 'assign':
        return ['    f32 x', '    x = mOffset;']
    if form == 'acc':
        return ['    f32 x = getOffset()', None]  # caller appends ';'
    if form == 'acc_assign':
        return ['    f32 x', '    x = getOffset();']
    raise ValueError(form)


def make_body(vx, vk, vv, sub, mtemp):
    pre, post = [], []
    xs, xa = x_src(vx)
    pre.append(xs + (';' if vx == 'acc' else ''))
    if xa:
        post.append(xa)
    if vk == 'init':
        pre.append('    f32 k = mK;')
    elif vk == 'assign':
        pre.append('    f32 k;')
        post.append('    k = mK;')
    elif vk == 'bare':
        pass
    if vv == 'init':
        pre.append('    f32 vel = mVel;')
    elif vv == 'assign':
        pre.append('    f32 vel;')
        post.append('    vel = mVel;')
    mul = 'x * %s' % ('k' if vk != 'bare' else 'mK')
    if mtemp:
        post.append('    f32 t = %s;' % mul)
        sub_stmt = '    vel -= t;'
    else:
        sub_stmt = '    vel -= %s;' % mul if sub == 'compound' else '    vel = vel - %s;' % mul
    lines = []
    lines += pre
    lines += post[:len(post)] if not mtemp else []
    # order: declarations/assignments, optional multemp, subtract
    seq = []
    seq += [l for l in pre]
    seq += [l for l in post if l.strip().startswith(('x =', 'k =', 'vel ='))]
    if mtemp:
        seq.append('    f32 t = %s;' % mul)
    seq.append(sub_stmt)
    return '\n'.join(seq) + '\n' + BODY_PULL + BODY_CLAMP + BODY_BOTTOM


VARIANTS = {
    's01': ('assign', 'init', 'init', 'compound', False),
    's03': ('acc', 'init', 'init', 'compound', False),
    's04': ('acc_assign', 'init', 'init', 'compound', False),
    's06': ('acc', 'bare', 'init', 'compound', False),
    's08': ('init', 'init', 'init', 'explicit', False),
    's09': ('acc', 'init', 'init', 'compound', True),
    's07': ('init', 'init', 'init', 'compound', True),
}


def main():
    sys.path.insert(0, BASE)
    import build
    results = {}
    for name, spec in VARIANTS.items():
        src = INIT + 'void dPosShake_c::move() {\n' + make_body(*spec) + TAIL
        path = '%s\\%s.cpp' % (BASE, name)
        with open(path, 'w', newline='\n') as fh:
            fh.write(src)
        try:
            shape = build.build('%s.cpp' % name, name, 'move__11dPosShake_cFv')
            words = shape[0]
        except RuntimeError as e:
            results[name] = ('COMPILE FAIL', str(e)[-300:])
            continue
        p = subprocess.run(
            [sys.executable, r'tools\auto_decomp\fndiff.py',
             'scratch/trial/target.txt', 'scratch/trial/%s.txt' % name,
             'move__11dPosShake_cFv'],
            capture_output=True, text=True, cwd=r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp')
        diffs = '???'
        for line in p.stdout.splitlines():
            if line.startswith('  DIFFS'):
                diffs = line.strip()
        results[name] = ('%d words' % words, diffs)
    for name in sorted(results):
        print(name, results[name])


if __name__ == '__main__':
    main()

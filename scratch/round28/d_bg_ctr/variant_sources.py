import os, re, sys
ROOT=r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'; BASE=os.path.join(ROOT,'scratch','round28','d_bg_ctr')
sys.path.insert(0,os.path.join(ROOT,'tools','auto_decomp')); import harness
base=open(os.path.join(BASE,'d_bg_ctr.cpp'),encoding='utf-8').read()

def build(label, s):
 p=os.path.join(BASE,label+'.cpp'); o=os.path.join(BASE,label+'.o'); d=os.path.join(BASE,label+'.txt')
 open(p,'w',encoding='utf-8',newline='\n').write(s)
 ok,log=harness.compile_draft(p,o,extra_inc=(os.path.join(BASE,'shadow'),)); print(label,'COMPILE',ok); print(log or '')
 if not ok:return
 ok,log=harness.disasm(o,d); print(label,'DISASM',ok,log or '')
 for n in ['calc__9dBg_ctr_cFv','revisePos__9dBg_ctr_cFv','addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c','fn_80080900','fn_80080E40']:
  w=harness.extract(os.path.join(BASE,'target.txt'),n); g=harness.extract(d,n)
  if g is None:
   for line in open(d,encoding='utf-8',errors='replace'):
    if line.startswith('.fn '+n+'__'): g=harness.extract(d,line.split()[1].rstrip(','));break
  print(n,len(w or []),len(g or []),'MATCH' if w==g else 'DIFFER')

# exact target operation order for revisePos
s=base.replace('''    f32 f0 = rawF32(this, 0x9C);
    f32 f1 = rawF32(actor, 0xB4);
    f32 f3 = rawF32(actor, 0xB0);
    f32 f4 = f1 - f0;
    f32 f2 = rawF32(this, 0x98);
    f1 = rawF32(actor, 0xAC);
    f0 = rawF32(this, 0x94);
    f2 = f3 - f2;
    f1 = f1 - f0;''','''    f32 oldY = rawF32(this, 0x9C);
    f32 actorY = rawF32(actor, 0xB4);
    f32 actorZ = rawF32(actor, 0xB0);
    f32 f4 = actorY - oldY;
    f32 oldZ = rawF32(this, 0x98);
    f32 actorX = rawF32(actor, 0xAC);
    f32 oldX = rawF32(this, 0x94);
    f32 f2 = actorZ - oldZ;
    f32 f1 = actorX - oldX;''')
build('target_order_revise',s)

# target uses fractional-index trig and retains the corrected values in stack locals
s=base.replace('''    f32 dx = out->x - mPos.x;
    f32 dy = out->y - mPos.y;

    f32 len = EGG::Math<f32>::sqrt(dx * dx + dy * dy);

    s16 angle = (s16)(nw4r::math::Atan2Idx(dy, dx) + m_c4);

    f32 cos = nw4r::math::CosIdx(angle);
    f32 sin = nw4r::math::SinIdx(angle);

    f32 corrected_dx = len * cos - dx;
    f32 corrected_dy = len * sin - dy;

    out->x = dScStage_c::getLoopPosX(mPos.x + len * nw4r::math::CosIdx(angle));
    out->y = mPos.y + len * nw4r::math::SinIdx(angle);''','''    f32 dy = out->y - mPos.y;
    f32 dx = out->x - mPos.x;
    f32 len = EGG::Math<f32>::sqrt(dx * dx + dy * dy);
    s16 angle = (s16)(nw4r::math::Atan2Idx(dy, dx) + m_c4);
    f32 scale = 0.00390625f;
    f32 cos = nw4r::math::CosFIdx((f32)angle * scale);
    f32 sin = nw4r::math::SinFIdx((f32)angle * scale);
    f32 corrected_y = len * sin - dy;
    f32 corrected_x = len * cos - dx;
    out->x = dScStage_c::getLoopPosX(mPos.x + len * nw4r::math::CosFIdx((f32)angle * scale));
    out->y = mPos.y + len * nw4r::math::SinFIdx((f32)angle * scale);''')
build('target_math_dokan',s)

# add target-style locals to force the measured stack liveness
s=base.replace('''    nw4r::math::SEGMENT3 edgeSeg;
    f32 distSqResult;
    f32 *retDistSq = &distSqResult;
    int found = 0;''','''    nw4r::math::SEGMENT3 edgeSeg;
    mVec3_c edgeStart;
    mVec3_c edgeEnd;
    mVec3_c hitResult;
    f32 distSqResult;
    f32 *retDistSq = &distSqResult;
    int found = 0;''')
build('target_liveness_809',s)

s=base.replace('''    if (m_d4 != 0) {
        return false;
    }

    if (!(*(const u8 *)((const u8 *)bc + 0xE5) & m_dd)) {''','''    if (mEntryFlag != 0) {
        if (mpActor == nullptr) {
            return false;
        }
    } else {
        return false;
    }
    if (m_d4 != 0) {
        return false;
    }

    if (!(*(const u8 *)((const u8 *)bc + 0xE5) & m_dd)) {''')
build('target_filter_gate',s)

s=base.replace('''    if (mMode == 1) {
        f32 cx = f1 + mPos.x;
        f32 cy = mPos.y;

        Vec diff;
        diff.x = cx - pos->x;
        diff.y = cy - pos->y;
        diff.z = -pos->z;

        f32 mag = PSVECMag(&diff);
        if (mag <= mRadius) {
            return true;
        }
        return false;
    }''','''    if (mMode == 1) {
        Vec diff;
        diff.x = f1 + mPos.x - pos->x;
        diff.y = mPos.y - pos->y;
        diff.z = -pos->z;
        if (PSVECMag(&diff) <= mRadius) {
            return true;
        }
        return false;
    }''')
build('target_shape_80670',s)

import os,re,sys
ROOT=r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'; BASE=os.path.join(ROOT,'scratch','round28','d_bg_ctr')
sys.path.insert(0,os.path.join(ROOT,'tools','auto_decomp')); import harness

def funcs(path):
 out={}; lines=open(path,encoding='utf-8',errors='replace').read().splitlines(); cur=None; body=[]
 for x in lines:
  m=re.match(r'^\.fn\s+"?(.+?)"?\s*,',x)
  if m: cur=m.group(1).strip('"'); body=[]
  if cur: body.append(x)
  if cur and x.startswith('.endfn'):
   out[cur]=body;cur=None
 return out
T=funcs(os.path.join(BASE,'target.txt')); D=funcs(os.path.join(BASE,'draft_disasm.txt'))
for name in ['calc__9dBg_ctr_cFv','revisePos__9dBg_ctr_cFv','addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c','fn_80080900','fn_80080E40']:
 t=T.get(name,[]); ds=[(k,v) for k,v in D.items() if k==name or k.startswith(name+'__')]; d=ds[0][1] if ds else []
 print(name,'target',sum('/* ' in x and '*/' in x for x in t),'draft',sum('/* ' in x and '*/' in x for x in d))
 for label, b in [('T',t),('D',d)]:
  pro=' '.join(b[:20]); print(label, re.findall(r'stwu r1, -0x([0-9A-Fa-f]+)',pro)[:1], re.findall(r'_savegpr_([0-9]+)',pro), re.findall(r'stfd (f[0-9]+)',pro))

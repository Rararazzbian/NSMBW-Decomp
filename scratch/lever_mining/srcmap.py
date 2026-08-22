"""Map a disassembled function to its source body text."""
import re, os

BS = chr(92)

def strip_comments(text):
    out=[]; i=0; n=len(text)
    while i<n:
        c=text[i]
        if c=='/' and i+1<n and text[i+1]=='/':
            j=text.find('\n',i)
            if j<0: j=n
            out.append(' '*(j-i)); i=j
        elif c=='/' and i+1<n and text[i+1]=='*':
            j=text.find('*/',i+2)
            j = n if j<0 else j+2
            out.append(''.join(ch if ch=='\n' else ' ' for ch in text[i:j])); i=j
        elif c=='"' or c=="'":
            q=c; j=i+1
            while j<n:
                if text[j]==BS: j+=2; continue
                if text[j]==q: j+=1; break
                j+=1
            out.append(' '*(j-i)); i=j
        else:
            out.append(c); i+=1
    return ''.join(out)

_cache={}
def src_clean(path):
    if path not in _cache:
        t=open(path,'r',encoding='utf-8',errors='replace').read()
        _cache[path]=(t, strip_comments(t))
    return _cache[path]

_bodies={}
def bodies(path):
    """Return list of (qualified_name, start_line, end_line, clean_body, raw_body)
       for every function definition in the .cpp."""
    if path in _bodies: return _bodies[path]
    raw, clean = src_clean(path)
    res=[]
    pat=re.compile(r'([A-Za-z_][A-Za-z0-9_]*\s*::\s*~?[A-Za-z_][A-Za-z0-9_]*|\b[A-Za-z_][A-Za-z0-9_]*)\s*\(')
    for m in pat.finditer(clean):
        i=m.end()-1; depth=0
        while i<len(clean):
            if clean[i]=='(': depth+=1
            elif clean[i]==')':
                depth-=1
                if depth==0: break
            i+=1
        j=i+1
        while j<len(clean) and clean[j] in ' \t\r\n': j+=1
        while clean[j:j+5]=='const':
            j+=5
            while j<len(clean) and clean[j] in ' \t\r\n': j+=1
        if j>=len(clean) or clean[j]!='{': continue
        k=j; depth=0
        while k<len(clean):
            if clean[k]=='{': depth+=1
            elif clean[k]=='}':
                depth-=1
                if depth==0: break
            k+=1
        name=re.sub(r'\s+','',m.group(1))
        sl=raw.count('\n',0,m.start())+1
        el=raw.count('\n',0,k)+1
        res.append((name, sl, el, clean[j:k+1], raw[m.start():k+1]))
    _bodies[path]=res
    return res

def find_body(path, demangled):
    """demangled like 'daWmGrid_c::execute()' or 'd2d::init()'"""
    if not demangled: return None
    d=demangled.split('(')[0].strip()
    d=re.sub(r'^[A-Za-z_][A-Za-z0-9_:* ]*\s+(?=[A-Za-z_])','',d)
    d=re.sub(r'\s+','',d)
    bs=bodies(path)
    cands=[b for b in bs if b[0]==d]
    if len(cands)==1: return cands[0]
    if not cands:
        tail=d.split('::')[-1]
        cands=[b for b in bs if b[0].split('::')[-1]==tail]
        if len(cands)==1: return cands[0]
    return None

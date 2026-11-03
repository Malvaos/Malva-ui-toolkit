#!/usr/bin/env python3
"""Generates docs/COMPONENTS.md and docs/TOKENS.md from the .slint sources (doc comments are `///`).
   python3 tools/make-docs.py"""
import re,glob,os
R=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..')
def esc(s): return s.replace('|','\\|')
# ---------------- components ----------------
out=['# Component reference\n','_Generated from the `///` comments in `ui/components/*.slint` by `tools/make-docs.py`. Import everything from `ui/malva.slint`._\n']
order=['label','button','icon-button','toggle','checkbox','radio','slider','text-field','card','layout','progress','badge','picture','scroll-view','tabs','sidebar','list-row','window-frame','menu','dialog','feedback','charts','widgets','dock','tone','focus-ring']
files=['ui/components/%s.slint'%n for n in order]+['ui/icons.slint']
for f in files:
    p=os.path.join(R,f)
    if not os.path.exists(p): continue
    lines=open(p).read().split('\n'); doc=[]; comp=None; props=[]
    def flush():
        global comp,props
        if comp:
            out.append('\n### `%s`\n'%comp[0])
            if comp[1]: out.append(comp[1]+'\n')
            if props:
                out.append('| Property / callback | Type | Default | Notes |\n|---|---|---|---|')
                for r in props: out.append('| '+' | '.join(esc(x) for x in r)+' |')
        comp=None; props=[]
    pend=[]
    for ln in lines:
        t=ln.strip()
        if t.startswith('///'): pend.append(t[3:].strip()); continue
        m=re.match(r'export component (\w+) inherits (\w+)',t)
        if m:
            flush(); comp=(m.group(1),' '.join(pend)); pend=[]; continue
        if comp:
            m=re.match(r'(in-out|in|out) property <([^>]+)> ([\w-]+)(?:: (.*?))?;',t)
            if m and m.group(1)!='out' or (m and m.group(1)=='out' and not t.startswith('private')):
                if m: props.append(('`%s`'%m.group(3),m.group(2),('`%s`'%m.group(4)) if m.group(4) else '',' '.join(pend)+(' (output)' if m.group(1)=='out' else ''))); pend=[]; continue
            m=re.match(r'callback ([\w-]+)\((.*?)\);',t)
            if m: props.append(('`%s(%s)`'%(m.group(1),m.group(2)),'callback','',' '.join(pend))); pend=[]; continue
        if not t.startswith('//'): 
            if t: pend=[]
    flush()
open(os.path.join(R,'docs','COMPONENTS.md'),'w').write('\n'.join(out)+'\n')
# ---------------- tokens ----------------
t=open(os.path.join(R,'ui','tokens.slint')).read().split('\n'); o=['# Token reference\n','_Generated from `ui/tokens.slint`. Every component reads only from the `Theme` global._\n','| Token | Type | Value |\n|---|---|---|']
for ln in t:
    s=ln.strip()
    m=re.match(r'// ---- (.*?) -+',s)
    if m: o.append('| **%s** | | |'%m.group(1).strip()); continue
    m=re.match(r'(in-out|out) property <([^>]+)> ([\w-]+): (.*?);?$',s)
    if m: o.append('| `Theme.%s` | %s | `%s` |'%(m.group(3),m.group(2),esc(m.group(4).rstrip(';'))))
open(os.path.join(R,'docs','TOKENS.md'),'w').write('\n'.join(o)+'\n'); print('docs written')

#!/usr/bin/env python3
"""Generates ui/icons/*.svg (24x24 single-colour strokes) and ui/icons.slint (the Icons global + Icon component).
Edit ICONS and re-run:  python3 tools/make-icons.py"""
import os
ROOT=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','ui')
C=lambda cx,cy,r:f'<circle cx="{cx}" cy="{cy}" r="{r}"/>'
R=lambda x,y,w,h,rx=2:f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}"/>'
P=lambda d:f'<path d="{d}"/>'
GEAR="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"
ICONS={
 'check':P('M5 12.5l4.5 4.5L19 7.5'),'close':P('M6 6l12 12M18 6L6 18'),'plus':P('M12 5v14M5 12h14'),'minus':P('M5 12h14'),
 'chevron-right':P('M9 6l6 6-6 6'),'chevron-left':P('M15 6l-6 6 6 6'),'chevron-up':P('M6 15l6-6 6 6'),'chevron-down':P('M6 9l6 6 6-6'),
 'arrow-left':P('M19 12H5M11 6l-6 6 6 6'),'arrow-right':P('M5 12h14M13 6l6 6-6 6'),'arrow-up':P('M12 19V5M6 11l6-6 6 6'),'arrow-down':P('M12 5v14M6 13l6 6 6-6'),
 'search':C(11,11,6.5)+P('M16 16l4.5 4.5'),'info':C(12,12,9)+P('M12 11v5M12 8h.01'),'alert-circle':C(12,12,9)+P('M12 7.5v5M12 16h.01'),
 'alert-triangle':P('M10.3 4.5L2.6 18a2 2 0 0 0 1.7 3h15.4a2 2 0 0 0 1.7-3L13.7 4.5a2 2 0 0 0-3.4 0zM12 10v4M12 17h.01'),
 'check-circle':C(12,12,9)+P('M8.5 12.5l2.5 2.5 4.5-5'),'x-circle':C(12,12,9)+P('M9 9l6 6M15 9l-6 6'),
 'home':P('M4 11l8-7 8 7v8a1 1 0 0 1-1 1h-4v-6h-6v6H5a1 1 0 0 1-1-1z'),
 'folder':P('M3 7a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z'),
 'file':P('M6 3h8l4 4v14H6zM14 3v4h4'),'image':R(3,4,18,16)+C(9,10,1.5)+P('M21 16l-5-5-8 8'),
 'music':P('M9 18V6l11-2v12M9 18a3 3 0 1 1-3-3 3 3 0 0 1 3 3zM20 16a3 3 0 1 1-3-3 3 3 0 0 1 3 3z'),
 'video':R(3,6,13,12)+P('M16 10l5-3v10l-5-3'),'archive':P('M4 5h16v4H4zM5 9v10h14V9M10 13h4'),
 'trash':P('M4 7h16M9 7V5h6v2M6 7l1 13h10l1-13M10 11v6M14 11v6'),
 'download':P('M12 4v11M7 11l5 5 5-5M5 20h14'),'upload':P('M12 16V5M7 9l5-5 5 5M5 20h14'),
 'refresh':P('M20 12a8 8 0 1 1-2.5-5.8M20 4v4h-4'),'settings':C(12,12,3)+P(GEAR),
 'sliders':P('M21 4h-7M10 4H3M21 12h-9M8 12H3M21 20h-5M12 20H3M14 2v4M8 10v4M16 18v4'),
 'user':P('M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2')+C(12,7,4),
 'users':P('M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2M23 21v-2a4 4 0 0 0-3-3.9M16 3.1a4 4 0 0 1 0 7.8')+C(9,7,4),
 'lock':R(5,11,14,9)+P('M8 11V8a4 4 0 0 1 8 0v3'),'unlock':R(5,11,14,9)+P('M8 11V8a4 4 0 0 1 7.5-2'),
 'eye':P('M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12z')+C(12,12,3),
 'eye-off':P('M3 3l18 18M10.6 6.1A9.6 9.6 0 0 1 12 6c6.5 0 10 6 10 6a17 17 0 0 1-3.2 3.9M6.6 7.6A16 16 0 0 0 2 12s3.5 7 10 7a9.7 9.7 0 0 0 4-.9M9.9 9.9a3 3 0 0 0 4.2 4.2'),
 'bell':P('M6 16v-5a6 6 0 0 1 12 0v5l2 2H4zM10 21h4'),'calendar':R(3,5,18,16)+P('M3 10h18M8 3v4M16 3v4'),
 'clock':C(12,12,9)+P('M12 7v5l3 2'),
 'cpu':R(6,6,12,12)+R(9,9,6,6,1)+P('M9 2v3M15 2v3M9 19v3M15 19v3M2 9h3M2 15h3M19 9h3M19 15h3'),
 'hard-drive':R(3,12,18,7)+P('M5 12l2.5-6h9L19 12M7 15.5h.01M11 15.5h.01'),
 'wifi':P('M2 9a15 15 0 0 1 20 0M5.5 12.5a10 10 0 0 1 13 0M9 16a5 5 0 0 1 6 0M12 19.5h.01'),
 'bluetooth':P('M7 7l10 10-5 4V3l5 4L7 17'),'volume':P('M4 9h4l5-4v14l-5-4H4zM16.5 9a4 4 0 0 1 0 6M19 6.5a8 8 0 0 1 0 11'),
 'battery':R(2,7,17,10)+P('M22 11v2'),
 'sun':C(12,12,4)+P('M12 2v2M12 20v2M4.9 4.9l1.4 1.4M17.7 17.7l1.4 1.4M2 12h2M20 12h2M4.9 19.1l1.4-1.4M17.7 6.3l1.4-1.4'),
 'moon':P('M20 14.5A8.5 8.5 0 1 1 9.5 4a7 7 0 0 0 10.5 10.5z'),
 'star':P('M12 3l2.7 5.6 6.1.9-4.4 4.3 1 6.1L12 17l-5.4 2.9 1-6.1L3.2 9.5l6.1-.9z'),
 'heart':P('M12 20s-8-5-8-11a4.5 4.5 0 0 1 8-2.5A4.5 4.5 0 0 1 20 9c0 6-8 11-8 11z'),
 'copy':R(9,9,11,11)+P('M5 15V6a2 2 0 0 1 2-2h9'),'edit':P('M4 20l1-4L16.5 4.5a2.1 2.1 0 0 1 3 3L8 19zM14.5 6.5l3 3'),
 'filter':P('M4 5h16l-6 8v6l-4-2v-4z'),'menu':P('M4 7h16M4 12h16M4 17h16'),
 'more':P('M5 12h.01M12 12h.01M19 12h.01'),'grid':R(4,4,6,6,1.5)+R(14,4,6,6,1.5)+R(4,14,6,6,1.5)+R(14,14,6,6,1.5),
 'list':P('M9 6h11M9 12h11M9 18h11M4 6h.01M4 12h.01M4 18h.01'),'play':P('M7 5l12 7-12 7z'),'pause':P('M8 5v14M16 5v14'),
 'terminal':P('M4 6l6 6-6 6M12 18h8'),'mail':R(3,5,18,14)+P('M3 7l9 6 9-6'),
 'link':P('M10 14a4 4 0 0 0 5.7 0l3-3a4 4 0 0 0-5.7-5.7l-1 1M14 10a4 4 0 0 0-5.7 0l-3 3a4 4 0 0 0 5.7 5.7l1-1'),
 'share':P('M12 4v11M8 8l4-4 4 4M5 13v6h14v-6'),'pin':P('M12 17v5M8 4h8l-1 6 3 3v2H6v-2l3-3z'),
 'power':P('M12 3v9M6.5 6.5a8 8 0 1 0 11 0'),'window':R(3,5,18,14)+P('M3 9h18'),
 'windows':R(3,8,13,11)+P('M8 8V6a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v7a2 2 0 0 1-2 2h-3'),
 'globe':C(12,12,9)+P('M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18'),'zap':P('M13 3L5 13h6l-1 8 8-10h-6z'),
 'activity':P('M3 12h4l3-8 4 16 3-8h4'),'leaf':P('M5 19c0-9 5-14 15-15 0 9-5 15-13 15M5 19l8-8'),
 'external':P('M14 4h6v6M20 4l-9 9M18 14v5H5V6h5'),
 'command':P('M15 6v12a3 3 0 1 0 3-3H6a3 3 0 1 0 3 3V6a3 3 0 1 0-3 3h12a3 3 0 1 0-3-3'),
 'tag':P('M3 12V4h8l9 9-8 8zM7.5 8.5h.01'),'bookmark':P('M6 4h12v17l-6-4-6 4z'),
 'shield':P('M12 3l8 3v6c0 5-3.5 8-8 9-4.5-1-8-4-8-9V6z'),'sort':P('M7 4v16M3 16l4 4 4-4M17 20V4M13 8l4-4 4 4'),
 'eject':P('M5 15h14L12 5zM5 19h14'),'cloud':P('M7 18a4 4 0 0 1-.5-8A6 6 0 0 1 18 9.5 4 4 0 0 1 17 18z'),
 'thermometer':P('M10 14V5a2 2 0 0 1 4 0v9a4 4 0 1 1-4 0z'),
 'calculator':R(5,3,14,18)+P('M8 7h8M8 12h.01M12 12h.01M16 12h.01M8 16h.01M12 16h.01M16 16h.01'),
 'notes':P('M6 3h12v18H6zM9 8h6M9 12h6M9 16h3'),'maximize':R(5,5,14,14,2),'minimize':P('M6 12h12'),
 'drive':R(3,12,18,7)+P('M7 15.5h.01'),'database':'<ellipse cx="12" cy="6" rx="7" ry="3"/>'+P('M5 6v12c0 1.7 3 3 7 3s7-1.3 7-3V6M5 12c0 1.7 3 3 7 3s7-1.3 7-3'),
 'rocket':P('M5 15c-1 1-1.5 4-1.5 4s3-.5 4-1.5M12 15l-3-3a22 22 0 0 1 2-4 12 12 0 0 1 9-4c0 2.7-1 6.5-4 9a22 22 0 0 1-4 2zM9 12H4s.5-3 2-4c1.6-1 5 0 5 0M12 15v5s3-.5 4-2c1-1.6 0-5 0-5'),
}
os.makedirs(os.path.join(ROOT,'icons'),exist_ok=True)
for n,b in ICONS.items():
    open(os.path.join(ROOT,'icons',n+'.svg'),'w').write(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="#000" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round">{b}</svg>\n')
names=sorted(ICONS)
s='// GENERATED by tools/make-icons.py - do not edit. One-colour 24x24 icons; Icon tints them with Theme colours.\nimport { Theme } from "tokens.slint";\n\nexport global Icons {\n'
for n in names: s+=f'    out property <image> {n}: @image-url("icons/{n}.svg");\n'
s+='    /// Look an icon up by name. Unknown names give the info icon.\n    pure public function get(name: string) -> image {\n'
for n in names: s+=f'        if (name == "{n}") {{ return self.{n}; }}\n'
s+='        return self.info;\n    }\n}\n\n'
s+='''/// A one-colour icon, tinted with `tint` (default: the theme's ink). `name` is any icon of the `Icons` global.
export component Icon inherits Image {
    in property <string> name: "info";
    in property <color> tint: Theme.ink;
    in property <length> size: 20px;
    width: size;
    height: size;
    colorize: tint;
    image-fit: contain;
    source: Icons.get(name);
}
'''
open(os.path.join(ROOT,'icons.slint'),'w').write(s); print(len(names),'icons')

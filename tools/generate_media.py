#!/usr/bin/env python3
"""Generate README media from the actual framebuffer. Requires gcc and Pillow."""
from pathlib import Path
import subprocess, os
from PIL import Image, ImageDraw, ImageFont
ROOT=Path(__file__).resolve().parents[1]
os.chdir(ROOT)
OUT=ROOT/'docs/images';OUT.mkdir(parents=True,exist_ok=True)
BUILD=ROOT/'build';BUILD.mkdir(exist_ok=True)
exe=BUILD/('render-snapshot.exe' if os.name=='nt' else 'render-snapshot')
subprocess.run(['gcc','-std=c99','-O2','src/game.c','src/personality.c','tools/render_snapshot.c','-o',str(exe)],check=True)
def render(scene,seconds=0):
 p=BUILD/'frame.ppm'
 subprocess.run([str(exe),scene,str(p),str(seconds)],check=True)
 return Image.open(p).convert('RGB')
for scene in ['home','stars','pause','sleep']:
 render(scene).resize((720,720),Image.Resampling.NEAREST).save(OUT/(scene+'.png'))
frames=[render('home',i*.16).resize((480,480),Image.Resampling.NEAREST) for i in range(12)]
frames[0].save(OUT/'kishmish.gif',save_all=True,append_images=frames[1:],duration=160,loop=0,optimize=False)
# Editorial banner; the right-hand panel is an unmodified framebuffer screenshot.
BG='#e0edb3';MID='#a9bf70';DARK='#203c36';SUB='#52734b'
canvas=Image.new('RGB',(1280,560),BG);d=ImageDraw.Draw(canvas)
d.rectangle((0,0,1279,559),outline=DARK,width=8)
def font(size,bold=False):
 candidates=[Path('C:/Windows/Fonts/'+('consolab.ttf' if bold else 'consola.ttf')),
 Path('/usr/share/fonts/truetype/dejavu/'+('DejaVuSansMono-Bold.ttf' if bold else 'DejaVuSansMono.ttf'))]
 for p in candidates:
  if p.exists(): return ImageFont.truetype(str(p),size)
 return ImageFont.load_default(size=size)
d.text((55,51),'A TINY CAT. A TINY CONSOLE.',font=font(20,True),fill=SUB)
d.text((49,105),'KISHMISH',font=font(82,True),fill=DARK)
d.text((49,188),'NANO',font=font(82,True),fill=DARK)
d.rectangle((57,300,602,303),fill=SUB)
d.text((55,329),'Feed. Play. Nap. Get weird.',font=font(25),fill=DARK)
d.text((55,383),'240 x 240 / FOUR COLORS / 8-BIT MEOWS',font=font(18,True),fill=SUB)
d.text((55,470),'ANBERNIC RG NANO  +  WINDOWS PREVIEW',font=font(17),fill=DARK)
canvas.paste(render('home').resize((480,480),Image.Resampling.NEAREST),(744,40))
canvas.save(OUT/'banner.png')
print('Generated banner, four screenshots, and animated framebuffer preview.')

# Phase-two contact sheet and an animation montage (not an accelerated idle scheduler).
scenes=[('home','HELLO KISHMISH'),('happy','HAPPY HEADPATS'),('munch','SNACK TIME'),('groom','GROOMING PAW'),('loaf','LITTLE LOAF'),('curious','CURIOUS CAT'),('gremlin','RARE GREMLIN'),('secret','SECRET HOMAGE')]
sheet=Image.new('RGB',(4*480,2*528),BG)
draw=ImageDraw.Draw(sheet)
for i,(scene,label) in enumerate(scenes):
 x=(i%4)*480;y=(i//4)*528
 sheet.paste(render(scene,.14).resize((480,480),Image.Resampling.NEAREST),(x,y+36))
 draw.text((x+18,y+6),label,font=font(20,True),fill=DARK)
sheet.save(OUT/'personality.png')
montage=[]
for scene in ['home','blink','ear','tail','groom','loaf','happy','munch','curious','gremlin','secret']:
 count=3 if scene=='blink' else 12
 for i in range(count):montage.append(render(scene,i*.14).resize((480,480),Image.Resampling.NEAREST))
montage[0].save(OUT/'personality.gif',save_all=True,append_images=montage[1:],duration=140,loop=0,optimize=False)
print('Generated phase-two expressions and animation montage.')

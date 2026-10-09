#!/usr/bin/env python3
"""Regenerate the original code-drawn icon and synthesized 8-bit WAV assets.
Requires Pillow. Run from any directory; outputs are relative to this repository.
"""
from pathlib import Path
from PIL import Image, ImageDraw
root = r = Path(__file__).resolve().parents[1]
(root/'docs/images').mkdir(parents=True, exist_ok=True)
pal=['#e0edb3','#a9bf70','#52734b','#203c36']
im=Image.new('RGB',(32,32),pal[0]); d=ImageDraw.Draw(im)
def box(r,c): d.rectangle(r,fill=pal[c])
box((0,0,31,31),3);box((1,1,30,30),0);box((2,2,29,29),1)
d.polygon([(5,14),(5,4),(13,11),(19,11),(26,4),(26,14)],fill=pal[3])
d.polygon([(7,11),(7,7),(11,11)],fill=pal[0]);d.polygon([(21,11),(24,7),(24,11)],fill=pal[0])
box((5,13,26,24),3);box((7,11,24,26),3);box((7,13,24,24),0);box((9,12,22,25),0)
d.polygon([(12,12),(19,12),(23,16),(23,22),(19,25),(12,25),(8,22),(8,16)],fill=pal[2])
for x in [9,19]:
 box((x,16,x+4,20),3);box((x+1,17,x+3,19),0);box((x+3,17,x+3,20),3)
box((12,22,19,24),0);box((14,21,17,22),3);box((15,23,16,24),3);box((13,25,14,25),3);box((17,25,18,25),3)
for r in [(2,20,7,20),(3,23,7,23),(24,20,29,20),(24,23,28,23)]:box(r,3)
im.save(root/'package/kishmish.png'); im.resize((256,256),Image.Resampling.NEAREST).save(root/'docs/images/icon.png')

r = root
# Generate three short, low-volume square/triangle meows with rising "me" and falling "ow".
import math, wave, struct
(r/'package/sounds').mkdir(exist_ok=True)
for name,base,duration in [('snack',620,.34),('pet',760,.46),('play',880,.30)]:
 samples=[];phase=0
 for n in range(int(22050*duration)):
  t=n/22050;u=t/duration
  hz=base*(.65+u*3 if u<.18 else 1.19-(u-.18)*.95)
  hz*=1+.025*math.sin(t*2*math.pi*24)
  phase=(phase+hz/22050)%1
  pulse=1 if phase<.25+.2*u else -1
  tri=1-4*abs(phase-.5)
  env=min(1,u/.035)*min(1,(1-u)/.18)
  sample=int((pulse*.65+tri*.35)*env*5800)
  sample=round(sample/128)*128
  samples.append(struct.pack('<h',sample))
 for parent in [r/'package/sounds']:
  with wave.open(str(parent/(name+'.wav')),'wb') as f:f.setparams((1,2,22050,0,'NONE','not compressed'));f.writeframes(b''.join(samples))

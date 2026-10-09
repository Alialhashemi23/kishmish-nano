#!/usr/bin/env python3
"""An original 8-bit 'bu-re-nyuuu' vocal homage, synthesized entirely from math.
No samples, character recordings, or voice models are used.
"""
from pathlib import Path
import math
import struct
import wave

RATE = 22050
root = Path(__file__).resolve().parents[1]
segments = [
    # duration, starting/ending pitch, vowel formants
    (.16, 190, 240, (350, 850, 2200)),
    (.19, 240, 310, (500, 1800, 2500)),
    (.12, 310, 400, (350, 1700, 2300)),
    (.56, 400, 170, (330, 850, 2200)),
]
samples = []
phase = 0.0
for duration, start, end, formants in segments:
    count = int(RATE * duration)
    for i in range(count):
        u = i / count
        hz = (start + (end-start)*u) * (1+.025*math.sin(i/RATE*2*math.pi*23))
        phase = (phase + hz/RATE) % 1
        voice = 0.0
        weight_sum = 0.0
        for harmonic in range(1, 19):
            frequency = hz*harmonic
            weight = .12/harmonic + sum(
                strength * math.exp(-((frequency-formant)/170)**2)
                for formant, strength in zip(formants, (1.0, .65, .25)))
            voice += weight * math.sin(2*math.pi*phase*harmonic)
            weight_sum += weight
        envelope = min(1.0, u/.06, (1-u)/.08)
        value = round((voice/weight_sum)*envelope*9500/128)*128
        samples.append(struct.pack('<h', value))
    samples.extend([b'\0\0'] * int(RATE*.012))
output = root/'package/sounds/secret.wav'
output.parent.mkdir(parents=True, exist_ok=True)
with wave.open(str(output), 'wb') as wav:
    wav.setparams((1, 2, RATE, 0, 'NONE', 'not compressed'))
    wav.writeframes(b''.join(samples))

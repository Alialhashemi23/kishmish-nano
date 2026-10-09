"""Validate all four packaged PCM clips without audio hardware or third-party libraries."""
from pathlib import Path
import struct
import wave
root=Path(__file__).resolve().parents[1]
for name in ["snack","pet","play","secret"]:
    with wave.open(str(root/"package/sounds"/(name+".wav"))) as clip:
        assert (clip.getnchannels(),clip.getsampwidth(),clip.getframerate())==(1,2,22050)
        duration=clip.getnframes()/clip.getframerate()
        assert .2<duration<1.5
        samples=struct.unpack("<"+"h"*clip.getnframes(),clip.readframes(clip.getnframes()))
        assert 1000<max(abs(x) for x in samples)<16000
        assert max(samples)>0 and min(samples)<0
print("PASS: four mono PCM clips, duration, signal, and headroom")

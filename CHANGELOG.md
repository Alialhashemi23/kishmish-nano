# Changelog

## 0.2.0-rc.1 — Phase-two playtest

- Five idle behaviors: slow blink, ear twitch, tail flick, grooming paw, and loaf.
- Rare curious/gremlin faces and immediate happy/munch care reactions.
- Needs- and chaos-aware ambient dialogue, occasional neko-speak, and reaction cooldowns.
- Saved affection/return memory without adding a care meter; existing MVP saves load.
- Awake-care-screen secret: Left, Right, Left, Right, B, A, with input timeout and playback cooldown.
- Original math-synthesized vocal homage; no sampled character audio or voice model.
- Separate directional inputs preserve ordinary menu controls while isolating the secret.
- Personality, audio-asset, render isolation, and real SDL input/audio smoke tests.

Status: desktop/emulated checks pass. Hardware playtest pending.

## 0.1.0 â€” 2026-10-09

First repository-ready MVP.

- Code-drawn Siamese kitten and four-color 240Ã—240 room.
- Feeding, affection, sleep, needs, chaos-dependent dialogue, and persistent saves.
- Twinkling-star minigame with independent selection and five-point sprites.
- Three synthesized 8-bit meows; optional mute.
- Start pause menu with resume, manual save, and save-and-quit.
- Stock Anbernic launcher metadata and bundled ARM runtime compatibility.
- Windows preview, portable regression tests, screenshot tools, and CI.

Hardware note: basic launch and play were confirmed on a stock RG Nano. The latest audio and pause refinements still need a full on-device verification pass.

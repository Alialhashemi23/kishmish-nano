# Phase-two Nano playtest

Build: **v0.2.0-rc.1**. Automated tests pass; these checks require the physical Nano.

Before updating, copy `Anbernic/.kishmish-nano/pet.sav` off the mounted card as a backup. Replace only `Applications/kishmish-nano.opk`, then eject USB. The existing MVP save should load normally.

- [ ] Boot from GMenu2X; confirm your existing food, joy, rest and sleep state remain plausible.
- [ ] Feed: see a brief munch and hear the normal snack meow. Pet: see happy eyes and a small heart.
- [ ] Stay awake on the care screen for about a minute: watch for blinks, ear/tail movements, grooming or a loaf. These are occasional, not continuous. Rare faces may take several minutes.
- [ ] Wait for a few thoughts. Neko-speak should appear occasionally; a care reaction should not be overwritten immediately.
- [ ] Navigate during an animation. Inputs should respond immediately and the care menu should remain stable.
- [ ] Enter Left, Right, Left, Right, B, A. Confirm the original vocal homage and goofy face. The final A must not feed Kishmish or open Play.
- [ ] Repeat the complete secret immediately: no sound replay and no accidental care action. It becomes available again after twenty seconds of unpaused play.
- [ ] Start → Sound Off → resume, then try normal care and the secret. Both should be silent.
- [ ] Play the star game. Its cursor must stay independent of the care menu. Secret inputs should not activate there.
- [ ] Start → Save and Quit → reopen. Confirm the save survived. Continued care builds warmer greetings over time without adding a meter.
- [ ] Sleep, quit and reopen; confirm sleep persists. The secret should not activate while asleep.

Use **Start**, not the hardware power button, for the game's pause menu. Full system suspend/resume remains unsupported.

For failures, note the exact input, sound setting and firmware. Mount USB and retain `Anbernic/.kishmish-nano/boot.log` with the report. The device clock controls offline progression; an incorrect date is not corrected automatically.

## Rollback

The stable v0.1.0 release remains available. You can replace the OPK with it; original save fields are still readable. Restore your backup if you want the exact pre-test state. Saving from v0.1.0 drops phase-two affection/visit memory.

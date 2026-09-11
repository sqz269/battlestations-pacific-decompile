# Voice playback service integration

This batch connects the previously reconstructed voice-line constructor and
fade initializer to the recovered subtitle, slot-start and fade-update bodies.
It continues the warning/voice/order work in `WARNING_VOICE_ORDER_INTEGRATION.md`.
The work was isolated on `agent/orch4-20260910`, with disjoint leased workers;
the existing BSP Ghidra project and program remain the analysis source.

## Connected paths

* `005BABB0` calls `display_voice_subtitles_005b8510` for nonempty joined text
  and `start_voice_clip_005b9050` for the selected playable clip. Its bank retain
  supplies the latter's consumed by-value argument. The former host-call scope
  guard is removed because the recovered callee releases that argument itself.
  The constructor still consumes its separate incoming bank argument.
  It reloads the global manager at `005BAE1C/21` after subtitle work and at
  `005BAE85/8B` after polling/retaining. Independent review caught and corrected
  the earlier constructor projection's reuse of a manager argument here.
* `005B9760` calls `update_voice_sound_fade_005b8c30` with the recovered minimum
  delta after storing the fade fields and callback. The update invokes the
  existing scalar step, native string copy/destruction, thread-safe mission Lua
  dispatch and sound-manager gain-application bodies.
* `005B9050` constructs a temporary slot through `00702CC0`, assigns through
  `005B82C0`, destroys through `005B7FC0`, polls through `007027B0`, logs, and
  consumes its bank argument. The sound path uses the canonical sound owner,
  configuration, level entries and intrusive ownership.
* `005B91E0` advances an active line after its slot becomes idle, skips negative
  sound IDs and starts the next clip with a null bank. It reloads the global
  manager before polling and starting. See `VOICE_LINE_ADVANCE.md`.

The three former whole-routine callbacks are replaced by typed accessors for
the actual services required inside those routines. This does not provide a
running game's global owner or fabricate GUI/audio objects.

## Evidence corrections and shared state

`VoiceClip.word_08` is the indexed resource selection within record `+20`,
not an uninterpreted payload. Record `+14` selects the alternate audio path;
`+18` is its owned native string. The slot owns another native string at `+0C`.
Only slot zero is established. Native start still computes the requested
index without rejecting `-1`; a required host mapping must preserve the address
behavior or fault rather than silently select slot zero.

Subtitle layout uses the existing canonical GUI projections and the existing
voice queue. Its manager views retain the native manager identity between
reload points while aliasing fields that callbacks can replace. Native code
reloads the global manager separately for the template, text branch, queue,
duration, initial layout value and dirty flag. `+EC` text length is a signed
integer input to x87 and counts UTF-16 code units.

Fade completion requires ordered equality and a nonempty callback. Logging
precedes the callback copy; the original is cleared before Lua dispatch.
After temporary string destruction, the code rereads manager `+D4`. If the
current sound owner's `+6C` differs, it stores that value, reloads the sound
singleton and reapplies the owner's existing `+4C` gain. This preserves the
native separation between the fade multiplier and gain.

## Review and validation boundary

Address-level ABIs, evidence, floating-point domains, ownership preconditions
and unresolved calls are in `VOICE_SLOT_START.md`, `VOICE_SUBTITLES.md` and
`VOICE_FADE_UPDATE.md`. These are typed C++ reconstructions, not native object
layouts or drop-in binary entry points. Real sound factories, alternate audio,
some GUI dispatch and identity adapters, and game-global service bindings remain
required. No game playback, audible output or rendered subtitle result is
claimed here.

Slot construction requires a fresh, empty native-string projection. It does
not reconstruct a destroyed slot whose string header still points to freed
storage. Slot timestamp assignment uses ordinary C++; signaling-NaN quieting,
payload and x87 status equivalence of the native `005B8365/69` copy are excluded.
The entry-array projection requires stable structure during intrusive releases
and nonthrowing, callback-free retain operations.

The integrated MSVC Win32 Release build passed both existing CTests. The two
ignored slot-ownership and fade-reentry fixtures also passed after relinking
against the integrated library. Those fixtures exercise required service
contracts; they do not execute native FMOD or render the subtitle sequence.
The advance routine received an independent complete-assembly review.
No permanent tests or targets were added.

The locked Ghidra flow repair cleared the false no-return override at
`00A7C14A`, disassembled the 19-byte gap `00A7C14F..00A7C161`, and saved the
existing project. The repaired function has no remaining call gaps.

Sixteen new or updated scoped names/comments were applied under the Ghidra
write lock, saved, read back and verified against their evidence. Existing
comments were preserved and all sixteen exports refreshed. The build against
concurrent main changes passed at commit `19b9892`; subsequent report changes
do not change code. Exact validation and annotation records are in
`reports/voice_services_integration.json`.

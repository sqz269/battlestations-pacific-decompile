# Voice manager scope: packet cc15-voice, parked (2026-10-28)

The cc15-voice packet asked for the voice path to be wired the way the image does it, behind
`kMessageVoicePlaybackCc15Bound`. It covered 00702CC0 and 005B9050, plus what they need of
005B71D0, 00974070 and 00A81860. The scope check showed the packet is the whole deferred
subsystem `cc15_host_voice_manager` (config/parallel_work.json), so cc16 parked it (decision D).
No code was written. cc9 agreed to parking on 2026-10-28.

Callers come from the index (`bsp.py callers`) and Ghidra xrefs on bsp.gpr
/battlestationspacific.exe. The names are provisional ledger hypotheses.

## 1. Caller chains

The voice manager is the HudNarrativeScreen object at `[[00E198C4]+A4h]`, vtable 00CF0ED8.

- **005B9050 VoiceManager_StartClip** has three callers. All of them are inside that manager.
  - Scripted dialog: 005B94D0 VoicePanel_BeginRecord (call 005B953E) -> 005B93D0
    AdmitScheduledRecord (005B940A) -> 005B9300 StartScheduledRecord -> 005B9050.
    005B9420 PollScheduledPlayback (005B9458) also reaches 005B9300. It is called from
    005B94D0 (005B970D) and from 005BBF10 UpdateTimedSequence (005BC587).
  - Warning apply: 00987590 WarningManager_Update -> 00974070 ApplyWarning, at 0098773D.
    00974070 calls 005BBDC0 PlayPositionalLine (009740CB) or 005BBC10 PlayLine (009740E9);
    005BBDC0 itself calls 005BBC10 (005BBEDF).
    005BBC10 -> 005BABB0 VoiceLine_Construct (005BBD59) -> 005B9050. On the way 005BABB0 builds
    the subtitle widgets with 005B8510, and 005BBC10 calls 005A2650
    HudMapScreen_SetSpeakingUnit, which is HUD.
  - Multi-clip advance: 005BC640 VoiceManager_Update -> 005B91E0 AdvancePlayback (005BC6AB)
    -> 005B9050.
- **00702CC0 VoiceSlot_ConstructAndStart** is called by 005B9050 and by 005B8810
  AmbientMessage_PlaySound. 005B8810 has no references at all, so it is unreachable. 00702CC0
  starts the stream with 00A78CE0 StreamedDialogManager_PlayDialogStream (00702EB0) and sets the
  start timer with 00A798C0 Sound_SetStartTimer (00702FAC).
- **005BC920 HudNarrativeScreen_Update** has one caller: vslot 8 of vtable 00CF0ED8 (data at
  00CF0EF8). It is the only caller of 005BBF10 (005BC9C5), 005BC640 (005BC9D4) and 005B8C30
  UpdateSoundFade (005BCA0B). 005B8C30 dispatches mission Lua when a fade completes. All of
  this per-frame update is HUD-layer code, which cc9 owns in the host.
- 005B71D0 VoiceManager_CanPlay is called from 009763E0 and 00987590. It polls the slots through
  007027B0. The host keeps the slots idle, so this answer stays not evaluated
  (docs/CC15_WARNING_APPLY_GAP.md).

## 2. Clip end: 007027B0 VoiceSlot_PollState

The image has no separate clip-duration value; a slot's clip ends when 007027B0 sees one of
these:
- 00A77730 StreamedDialogManager_IsDialogPlaying reports that the stream has stopped, which is
  live FMOD playback state;
- or the fallback timeout fires, when now - start > float(duration + 2) on the start time that
  00A798C0 recorded.

The slot then retires its sound and the reference, and pops the speech duck (00A7D120). The
end condition follows real audio playback. The host steps mission frames at a fixed 0.05 s
(`--mission-frame-seconds`), not in real time, so if clip ends came from FMOD state, ON runs
would not reproduce each other.

## 3. Existing emulation

`src/game_hosts_script_orders.cpp:7222-7389` (cc9) already models the scripted-dialog clip end:
- `dialog_message_timing` reads the record's `voice` and the last subtitle key end from
  `scripts/datatables/messagemaps/<map>.lua`. It takes the clip length from
  `sound/messages/authentic/streamed_dialogs/<voice>.fsb`: sample count over default frequency,
  from the first FSB4 sample header (+50h, +64h).
- `dialog_step_00452740`, `dialog_advance_004527f0` and `dialog_tick` count those down on the
  script think step, which models 005BBF10 steps 3 and 7. They run the dialog callbacks through
  `mission_lua_call_named_00887e50`.

No voice is played. This deterministic projection is what drives the dialog, Lua and callback
timing today.

## 4. Revival options

- **B: deterministic FSB-length projection (preferred).** Host the manager and the slots, but
  have 007027B0's "is playing" answer come from the FSB length on the mission clock instead of
  from 00A77730. Record this as a deviation from the image. The order and timing would match
  the existing emulation, and ON runs would stay reproducible.
- **A: real-audio play mode.** Play the streams through GameSoundDialogRuntime (already hosted
  for the title sound) and use the image's real end condition. This is faithful to the image,
  but ON runs are not deterministic, so it cannot pass the pair checks. Use it only as an
  opt-in play mode.

Either way the work is several packets:
- a voice-manager object at `[[00E198C4]+A4h]`;
- the subtitle GUI (005B8510);
- the slots (00702CC0, 007027B0) and their FMOD services;
- the update hook from 005BC920;
- the dialog-sequencer hook;
- the warning apply in `game_hosts_mission_frame.cpp`, which is empty now, together with
  005A2650.

## cc9 agreement

cc9 agreed to parking on 2026-10-28. If B is revived, cc9 allows a cc15 lane to make both cc9
hooks itself, with these conditions:
- the hooks are the HUD update from 005BC920 and the dialog sequencer in
  game_hosts_script_orders.cpp;
- each goes in under a one-commit lease, behind an OFF switch;
- cc9 reviews them before any flip.

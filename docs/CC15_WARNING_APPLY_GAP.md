# Warning apply side: the host voice-manager gap (cc15, 2026-10-18)

The host's warning manager reports, queues and activates warnings behind `kWarningMessageBankCc15Bound`
(OFF), in `src/game_hosts_mission_frame.cpp`. It never applies them. This page records why, and
names the packet that closes the gap: `cc15_host_voice_manager` in `config/parallel_work.json`.

## The gap

- `005B71D0 voice_ready` polls voice slots (`007027B0`). The host has no voice slots, so it always
  answers false. An accepted warning stays Queued until it expires (5.0 s pending, 15.0 s applied).
- `00974070 apply` calls `005BBDC0`, then `005BBC10 PlayLine`. PlayLine calls:
  - `005A2650 BSP_HudMapScreen_SetSpeakingUnit` (`005A22D0` FindUnitIcon, `00AAB4C0`
    GuiWidget_CloneSubtree). This is a HUD consequence. cc9 has OK'd it behind the same switch,
    provided it writes only the HUD speaking-unit state.
  - `005BABB0 VoiceLine_Construct`, which builds the subtitle widgets (`005B8510`) and calls
    `005B9050 StartClip`. StartClip calls `00702CC0 VoiceSlot_ConstructAndStart`, which is FMOD sound.
  - `005B7790 VoiceLineQueue_Append`, a leaf.
- `005BC640 VoiceManager_Update` consumes that queue (`005B91E0` AdvancePlayback, `005B7290`
  AttachedVoice_Update and the GUI setters). The host has no voice playback manager, and the
  reconstructions in `voice_playback.cpp` have no host caller.

## Mission-Lua reach

None of these paths reaches `00887E50` (mission Lua calls). The only voice-side `00887E50` is in
`005BBF10 VoiceManager_UpdateTimedSequence`, whose only caller is `005BC920
HudNarrativeScreen_Update`, the scripted narrative panels, not the warning queue.

The method was callee walks at depth 2-3 over the ledger graph, by cc15-wB. The voice-slot
internals were not walked exhaustively.

## Random-number coupling

Activation draws from the shared stream 1 in the image (`00979D0C`/`00979D11`, then `00BD2FC0`).
The host uses a warning-local generator instead (cc9 option ii). See `docs/RANDOM_STREAMS.md`.

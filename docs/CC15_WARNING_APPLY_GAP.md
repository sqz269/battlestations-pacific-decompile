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

The method, by cc15-wB, was a transitive walk over the index's direct-call edges. It went to
depth 30 from the apply, voice and activate roots, and reached 1199 functions, none of them
`00887E50`. As a positive control, the same walk from `005BC920` does reach `00887E50` through
`005B9800`. Caveat: the walk does not follow virtual dispatch (vtable calls), so a path through
a virtual call is not ruled out.

## What 005B71D0 answers with no active slot

This is read from `src/voice_playback.cpp` and is not wired. `voice_can_play_005b71d0` returns
false when panel `+34h` or `+24h` is set or the manager's `+74h` pending record is non-null.
Otherwise it polls slot `+08h` once per clip. `poll_voice_slot_007027b0` on an idle slot (state 0)
makes no host call and returns 0. So an idle manager answers `attached_count_6c == 0`, which is
true for a manager with nothing attached.

The host has no manager, so it cannot say whether the slot is idle. `voice_ready` therefore stays
not evaluated and false, rather than assuming an idle manager.

## Random-number coupling

Activation draws from the shared stream 1 in the image (`00979D0C`/`00979D11`, then `00BD2FC0`).
The host uses a warning-local generator instead (cc9 option ii). See `docs/RANDOM_STREAMS.md`.

## The MessageManager (game+21DCh), not hosted

Step 3a's activates reach two reads of the MessageManager:
- `00976B00` resolves the clip's record through `00705E00`;
- `0097D650` runs its `0097B5E0` step, which tests candidate keys against the record map.

The host keeps no MessageManager, so both are counted as not evaluated in the
`summary mission warning activate` line (`records_not_evaluated`, `sound_keys_not_evaluated`), and
`clip.record` stays null. Hosting it is packet `cc15_host_message_manager`. It needs the 30-seam
`MessageManagerCalls` (include/bsp/engine_seg11_35_66_cc13.hpp), plus:
- `Scripts/datatables/Messages.lua` through `load_messages_007065e0`;
- the per-mission `name#side` maps (`00706760` load, `00706FC0` unload, the `+64h` string set);
- the `00A83FD0` sound banks that the records hold references to.

## The director walk over the pending list

`update_mission_events_00987590` (src/world_entities.cpp:236) walks `world.mission_events`. Nothing
writes that queue. It is not the `+E0h` pending list that `009763E0` fills (`host.warning.pending`).

In the image they are one list. Each frame the director:
1. retires at most one expired warning (`+10h <= now - vtable[7]`) and returns;
2. otherwise picks the highest-priority ready warning through `005B71D0`;
3. applies it with `00974070` at `0098773D`.

In the host, only the report walk inside `009763E0` prunes the pending list. This gap belongs
with `cc15_host_voice_manager`, because it only matters once something can apply. The queue
model is cc9's.

## Closing note: packet cc15-wB4

All steps are behind `kWarningMessageBankCc15Bound` (OFF), and every pair has identical gameplay.

| step | commit | what it reaches |
| --- | --- | --- |
| (a) Warnings.lua bank | 500ec1a11 | `009870A0`'s table load; `0096DBB0` message_id_known |
| (b) kill report | 37b3af305 | `009813A0`'s kill block `00981E07..00982096`, `009744C0`, `009763E0` |
| torpedo report | b78b54752 | `00977690`'s `00974150` "torpedo" report. It is unreachable in the image as well: brain `+3F2h` has no producer. |
| hit warning | 6c5c87431 (main 4ef070cd9) | `00988510`'s default block `009895D8..00989992`, `00974C70`, `009763E0` |
| 3a activate and variant | this commit | vtable[3] `0097F970`/`0097F490`/`0097FA90`, `00979B50` (the SUBSTITUTION draw), the `0097D650` gates, `00976B00` clips |

Step 3b (`005B71D0`, `00974070`, `005A2650`, `005B7790`) is deferred to `cc15_host_voice_manager`.


The step table above is flipped ON as one switch by cc9-lua69 (2026-10-18): USN01 `usn01_l65_u1_g` (26000 frames,
reference env) on main 011d651dc, OFF `cc9-lua52\local\l69r_w_u01_main_ref.log`, ON `l69r_w_u01_wb_ref.log` (export
`local\l69_wb`). pair_diff exit 1, gameplay identical (37 death rows, 87-row unit table, plane death modes identical).
ON only: the Warnings.lua table (122 messages), 445 hit reports and 5 kill reports through 009763E0. Step 3b stays with
`cc15_host_voice_manager`.

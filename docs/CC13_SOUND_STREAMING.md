# Segment 74 sound, streaming and helper routines (cc13-audio1)

Addresses: 00A7A440-00A9AC40 (partition segment 74). Worker cc13-audio1, branch `agent/cc13-audio1`.
Source: `src/sound_streaming_cc13.cpp`, `include/bsp/sound_streaming_cc13.hpp`.
Call-site report: `reports/cc13_audio1_calls.json` (64 rows, `tools/verify_report_calls.py` clean).
Functions defined in Ghidra: `reports/cc13_audio1_defined.json`.

All names are hypotheses, not recovered symbols. Status vocabulary: *named* (ledger + Ghidra
name and comment only), *reconstructed* (C++ written), *build-tested* (Win32 build and the two
existing CTests pass). Nothing here is fixture-tested, ABI-compatible or game-validated.

## What the segment holds

The partition keywords say "sound", but the band is four unrelated areas:

| band | content | prior coverage |
|---|---|---|
| 00A7A440-00A8A820 | sound manager, classes, channels, events, samples, streams | heavy (docs/SOUND_*.md, src/sound_*.cpp) |
| 00A8A820-00A90000 | directional shadow owner, shadow jobs, shadow camera fitting | owner/jobs done before; submit and single-view paths reconstructed here, fitters named |
| 00A90000-00A98000 | input backend, actions, bindings, gamepad force, text input | heavy; remaining raw are STL instances |
| 00A98000-00A9AC40 | input backend tails | heavy |

Before this packet the sound band's raw functions were almost all small container helpers; the
handful with behaviour are reconstructed below. Existing reconstructions reused, not duplicated:
`sound_manager_levels` (00A7A3F0, 00A7ABF0, 00A7ACF0), `sound_class_ownership` (00A7C2C0,
append fragment 00A80EBA), `sound_configuration` (00A7AE00, 00A7AE80), `sound_dialog_logical`
(00A865F0), `sound_stream_runtime` (stream layout and states).

## Reconstructed routines

| address | C++ | coverage | evidence |
|---|---|---|---|
| 00A7AC70 | `set_sound_class_duck_levels_00a7ac70` | complete | listing 00A7AC70-00A7ACE8; identical walk to 00A7ABF0 but on descriptor +14h |
| 00A7D120 | `push_pop_sound_speech_duck_00a7d120` | complete | listing 00A7D16C-00A7D1F3 (mask), 00CE3800 = 0.5f |
| 00A7F890 | `set_sound_class_paused_00a7f890` | complete | listing 00A7F890-00A7F8D9 |
| 00A7B710 | `set_sound_listener_environment_00a7b710` | complete | listing 00A7B710-00A7B748 |
| 00A7D9D0 | `add_sound_class_00a7d9d0` | complete normal path; EH cleanup is the parameter release | listing 00A7D9D0-00A7DA38 |
| 00A85C40 | `set_sound_stream_paused_00a85c40` | complete | listing 00A85C40-00A85C57 |
| 00A86710 | `select_sound_stream_row_00a86710` | complete | listing 00A86710-00A8679C (x87) |
| 00A869B0 | `draw_sound_stream_debug_00a869b0` | complete; text formatted with CRT snprintf | listing 00A869B0-00A86B31 |
| 00A82AE0 | `copy_sound_sample_descriptor_00a82ae0` | complete | listing 00A82AE0-00A82B6D |
| 00A83120 | `apply_sound_sample_settings_00a83120`, `format_sound_sample_definition_00a83120` | complete normal path; by-value cleanup left to caller | listing 00A83120-00A83449 |
| 00A7BE50, 00A7BE60, 00A7BEA0, 00A7BED0 | `sound_instance_default_*` | complete | vtable D5ABB8/D5ABF8/D5AD58/D5B480 slots |
| 00A89210 | `sound_event_is_completed_00a89210` | complete | listing 00A89210-00A89244 |
| 00A89600 | `update_sound_event_00a89600` | complete | listing 00A89600-00A896CE |
| 00A8AC90 | `project_shadow_point_00a8ac90` | complete | listing 00A8AC90-00A8AD1E (x87) |
| 00A8B860 | `intersect_lines_2d_00a8b860` | complete; aliasing order kept | listing 00A8B860-00A8B925 mapped per x87 instruction |
| 00A8E860 | `render_shadow_ground_ortho_view_00a8e860` | complete normal path; settings object passed explicitly | listing 00A8E860-00A8EA15 |
| 00A8E180 | `render_shadow_single_view_00a8e180` | complete | listing 00A8E180-00A8E2D9 |
| 00A8EA20 | `submit_shadow_cascades_00a8ea20` | complete normal path; allocation-failure path not modelled | listing 00A8EA20-00A8ECA0 |
| 00A92050 | `any_input_binding_forces_unit_scale_00a92050` | complete | decompile + listing |
| 00A92910 | `input_actions_share_binding_00a92910` | complete | decompile 00A92910-00A929A4 |

### Class ducking (00A7AC70, 00A7D120)

The class descriptor (20h, `SoundClassDescriptor`) field +14h was recorded as
"secondary_level_14, meaning beyond default 1 unknown". It is a duck factor:

- `BSP_SoundInstance_GetMixedVolume` (00A7AAA0) multiplies class +14h with class +10h.
- 00A7AC70 is the only writer found in the band; it is the +14h twin of `SetClassLevels`
  00A7ABF0 (mask of array ordinals, FUCOMIP equality removes a bit, residual bits go to the dirty
  walker 00A7A3F0).
- 00A7D120 keeps a nesting counter at manager +16Ch. The first push applies 0.5 and the last pop
  1.0 to mask `0xFFFF - bit(GUITestSpeech) - bit(Warnings) - bit(GUIMusic)`. The listing really
  subtracts (`OR ESI,-1; SHL ESI,CL; SUB ESI,EDX; SUB ESI,EDX; ADD ESI,0FFFFh`), so a missing class
  (index -1, shift 31) or two names on one ordinal produce a mask that is not a clean bit clear.
  ECX is always `[00F8BBD8]` at the five call sites (voice slot start 00702D88, poll 00702876,
  stop 00702703, HUD map exit 005A4238, 005B349C). The voice-playback hosts already call it as
  `set_sound_flag_00a7d120`: it ducks everything but speech, warnings and GUI music while a voice
  line plays.

### Class pause and listener environment (00A7F890, 00A7B710)

Descriptor +0Ch (`flags_0c`, "meaning unknown") is the class pause byte: the event update
(00A896AD) and the debug dump OR it into the paused state. 00A7F890 is the setter the cinematic
HUD hide path drives for 3DEffect/InGameGUI/Warnings/Sonar (docs/GAME_SIMULATION_GATE.md).

00A7B710 is the in-mission audio environment switch (docs/IN_MISSION_INTERFACE_RUNTIME.md:
Cockpit / Air / Underwater). It works on the Lua "listener" table at manager+A4 that
`SoundConfigurationState` already projects; +10Ch/+110h are its current/previous ordinals.

### Streams (00A85C40, 00A86710, 00A869B0)

Stream states (`00E12F00`): 0 STOP, 1 STARTING, 2 PLAY, 3 OVER. Speaker layouts (`00E12F10`):
0 undef, 1 mono, 2 stereo, 3 5.1. 00A85C40 is the pause setter used by 00A77760 (called from
cinematic mode and the engine-movie stop). 00A86710 cross-fades the dialog-table rows: the named
row gets target 1, the rest 0, and every row's rate becomes |current - target| / 3 (a three-second
fade at the advance routine 00A85B50). The x87 sequence is FSUB then FDIV by the double 3.0 at
00D7A2B0, one FSTP to float, then FCOMIP against 0: the C++ uses double arithmetic, which matches
at MSVC's 53-bit x87 precision.

00A869B0 is a debug overlay. Its sink `004B47E0` is a trivial body in this executable, so retail
draws nothing; the only side effects are the getPosition call and the returned y.

### Sample settings apply (00A82AE0, 00A83120)

00A83120 is the counterpart of the `.def` parser in `src/sound_sample.cpp`: it takes a descriptor
by value (48h bytes, RET 48h), copies it to sample+08h, pushes min/max distance, frequency/volume
variation and the rolloff/decompress mode bits to the FMOD sound, and when the sample was loaded
from a `.def` file writes the settings back through the VFS (`[0109CEEC]` slot +4, mode 35h; file
slot +5Ch writes the builder, slot +4(1) deletes). Its only caller chain is 0093FE90 <- 0093FF10,
a sound editing path in segment 61.

Open: the writer emits `Decompress <0|1>` while the parser treats `Decompress` as a bare keyword,
so a written-back file would present the number as an unknown token to the parser (which the
parser comment says repeats forever). Not run.

### Instance and event virtuals

The base sound-instance vtable D5ABB8 had six slots with no Ghidra function. Slot map shared by
the channel (D5ABF8), retained channel (D5AD58) and FMOD event (D5B480) vtables:

| slot | base | channel | event |
|---|---|---|---|
| +08 | pure | 00A7A570 RequestStop | 00A89170 Stop |
| +0C | pure | 00A799B0 IsCompleted | **00A89210** IsCompleted |
| +10 | pure | 00A7A660 RefreshEnded | **00A89C50** no-op |
| +14 | pure | 00A7A630 IsNonvirtual | 00A89C40 |
| +18 | **00A7BE50** null | 00A79A30 GetHandle | 00A88B90 |
| +1C | **00A7BE60** 1.0 | 00A7A710 GetAudibility | 00A89D00 |
| +20/+24 | pure | progress / virtual-state | 00A89C60 / 00A89C70 |
| +28 | **00A7BEA0** zero record | inherited | 00A896E0 FindParameter |
| +2C | **00A7BE70** no-op (RET 8) | inherited | 00A89B80 SetParameter |
| +30 | pure | 00A7AF10 Update | **00A89600** Update |
| +34 | **00A7BE80** no-op (RET 18h) | inherited | inherited |
| +38 | **00A7BED0** zero vec3 | inherited | inherited |
| +3C | 00A7BE90 NoOpConfigure | inherited | inherited |

Slots +34/+38 are never overridden in these four vtables; their meaning (six-dword setter,
three-float getter) is open.

### Shadow cascades (00A8EA20, 00A8AC90)

`BSP_ShadowMap_UpdateViewFromCamera` (00A8F3B0) reaches the cascade path through 00A8EA20, the
step docs/DIRECTIONAL_SHADOW_OWNER_NEXT.md left as the render-ready boundary. 00A8EA20 fixes the
four cascade far distances at owner +390h..+39Ch to 35, 100, 300 and 1000 (00CE4D90, 00CE3D08,
00CE3AE8, 00CE3804) and +4ECh to 1.5, so the cascades are [5,35], [35,100], [100,300],
[300,1000] with the first near 5.0 (00CE3850). Each is fitted by 00A8BD20 (owner, near, far,
index). Per cascade camera (owner +20h + 4i) it then refreshes the render camera (+30h) world
matrix unless camera flags +5Ch bit 1 is set, gives the cascade camera the clip plane
{0, -1, 0, 2} when the render camera height +124h is below -5.0 (00CFBC84; an underwater
camera) and clears it otherwise, builds a render command (00B1F1F0: owner +34h, cascade camera,
render camera, target +4F0h + 4i) with metadata {0, 62h, 1Eh} and diagnostic name `SHBU`,
appends it to the shadow queue (00B1EB80, immediate 0) and submits it as a shadow frame job
(00A8E090). The frame-job pool is flushed after the fourth cascade.

The two single-view paths share one body. Gated on the settings object at `[00F8BBF0]` (bytes
+0Ch and +0Dh), they retain the caller's camera in owner +30h, build the viewport matrix for
viewport +504h (00A8AAA0) and set it on the shadow camera +20h, fit the view (00A8AEB0), form the
shader matrix +144h = (+44h) x viewport matrix and overwrite its third column with +4ACh..+4B8h,
queue one render command for target +500h with metadata {0, 62h, 1Eh}, and release +30h. The
ground-ortho variant 00A8E860 (from `BSP_InterfaceView_DrawGroundOrtho`) also calls
`[00F8C210]` 00AD7A30(camera, 1.0) and queues 00B1F4D0 named `SHBU`; 00A8E180 (from 00B3C960)
queues 00B1F5E0 with its first argument. 00AD7A30, 00B1F4D0 and 00B1F5E0 are unnamed host calls.

Not reconstructed, named only: the cascade fitter 00A8BD20 (2007h bytes of x87), the single-view
fitter 00A8AEB0, and the unreferenced 00A8ECB0 (an older fit-and-submit path that uses 00A8AC90) and
00A8B930 (the only caller of the reconstructed line intersection 00A8B860). These are the next
shadow packet; they need listing-level x87 work.

00A8B860 keeps two native details: 1/det is rounded to float and stored to both output words
before the inputs are reread, and the y term rereads its inputs and `out[1]` after `out[0]` is
written, so an output that aliases an input gives the native result. The cross terms
`p0.x*p1.y - p0.y*p1.x` and `q0.x*q1.y - q1.x*q0.y` are the only other float spills.

### Input binding queries (00A92050, 00A92910)

00A92050 is the sibling of the reconstructed 00A92090 (`input_binding_poll`): it reports whether
any binding of an action sets `force_unit_scale` (+14h). 00A92910 is the control-binding screen's
conflict test: two actions conflict when any pair of their bindings shares device class and
input code, neither -1.

Named in the same band: `BSP_InputManager_GetContextLevel` 00A92290 (with setter 00A933F0,
docs/FRONTEND_SCREEN_SETS.md), `BSP_InputBackend_AnyDeviceHasInput` 00A90F90 and
`BSP_InputBackend_ReadEvent` 00A90530 (both already host methods in `frontend_menus`),
`BSP_TextOwner_HandleKeyFallback` 00A96750 (docs/TEXT_INPUT_FALLBACK.md),
`BSP_InputAction_Update_Provisional` 00A92A20, `BSP_GamepadForce_ClearAllChannels` 00A95CB0,
input-manager singleton base destructors 00A92180/00A92230, and
`BSP_InputAction_RemoveMatchingBinding` 00A93690. The latter has a native quirk: when no binding
matches, the end iterator is not the last element, so the last binding is copied into the slot
past the end and then destroyed, i.e. a miss removes the last binding.

About 60 raw functions remain in 00A90000-00A9AC40. They are `std::vector`/`std::deque`/
`std::_Tree` instances for the input backend device groups, gamepad force request queues and the
text owner event queue (allocate-with-throw at 00A907A0, 00A94680, 00A96260, 00A974D0; insert
bodies 00A91260, 00A97CB0; erase/clear/destroy families around 00A94730, 00A95330, 00A96550,
00A96FB0). They were not named individually.

## Named-only routines

117 ledger names were added in total (85 in the sound band, 32 in the shadow and input bands). The
first 85 were added (`config/names/00a70000.jsonl`, `00a80000.jsonl`) and applied to
Ghidra with evidence comments. Beyond the reconstructed rows above they are container and STL
template instances, recorded so the band stops showing them as unknown:

- pointer arrays with three growth policies: effect pointers (minimum 8, +4), DSP pointers
  (minimum 16, +16), sample pointers (doubling): 00A7A850, 00A7A930, 00A7B2D0, 00A7B7C0,
  00A7A8B0, 00A7B310, 00A7B390, 00A7BAA0, 00A7BA80, 00A7BF20;
- configured group (18h) and type (14h) record arrays: 00A7D880, 00A7F110, 00A7F160, 00A7FAF0,
  00A7FF30, 00A7FB50, 00A7FBD0, 00A7FEB0, 00A81460, 00A7F0D0;
- class table: 00A7C240 (append+retain; the body behind `SoundClassOwnership::append_retained`),
  thunks 00A7D990/00A7D9A0, destroy 00A7D9B0/00A7F2D0;
- ref-pointer slots (same body as 00440A30): 00A7B960, 00A7B990, 00A7BB40, 00A7BB70, and the
  return-self variants 00A7B4B0, 00A7B4E0, 00A82880; owned-pointer resets 00A81C20, 00A81CF0,
  00A87CB0;
- `std::_Tree` instances (node 1Ch, key native string at +0C, isnil +19): 00A81960 Max,
  00A81980 Min, 00A819C0/00A822E0 rotations, 00A81D10 node allocation, iterator steps 00A82360,
  00A82810, 00A823C0, 00A82870, and 00A88370..00A88620 init/erase/clear/destroy;
- resource cache records (2Ch): 00A84280, 00A84380, 00A84D20; dialog channel append 00A86FB0;
  string temporaries 00A82E20, 00A82E40, 00A82FF0, 00A83040, 00A83090;
- query lock destructors 00A894B0 (base, vtable D5B46C) and 00A89B10;
- 00A7EA30, a debug dump of all live entries that formats " cSoundObject: ... play%:" and throws
  the strings away (no sink call), and 00A818D0, the resource +08h test next to
  `HasFmodSound` (field meaning open).

"No reference found" in a ledger entry means: no rel32 call/jump in .text and no absolute dword
anywhere in the file image (scan by `local/a1_gaps.py`).

## Functions defined in Ghidra

26 bodies that Ghidra had never defined, all starting after INT3 padding at a 16-byte boundary,
ends from a linear sweep that stops at the last RET past every local branch target:

- referenced from vtables: 00A7BE50, 00A7BE60, 00A7BE70, 00A7BE80, 00A7BEA0, 00A7BED0 (sound
  instance), 00A89210, 00A89600, 00A89C50 (event), 00A894B0 (D5B46C), 00A8A9B0, 00A8A9C0,
  00A8DD60 (shadow vtables D5B55C/D5B560/D5B568), 00A90EC0, 00A92230, 00A97220, 00A97B80 (input
  vtables);
- referenced by rel32: 00A7C240 (from the 00A7D990 thunk and 00A7DA06), 00A93690 (00A93874);
- unreferenced: 00A7BA80 (absolute hit 00529B80), 00A90840 (hits 00A75F46, 00A76DFA) and
  00A93900 (hit 009B2719) were selected by absolute-dword matches that turned out to be
  instruction bytes (`MOV EAX,[EAX+8]; TEST EAX,100h` reads `40 08 A9 00`), and 00A7D990,
  00A7D9A0, 00A7D9B0, 00A7D9D0 were defined because this packet names them. All are coherent
  bodies after INT3 padding.

The band still has about 450 INT3-aligned starts with no Ghidra function and no reference of
either kind; they were left undefined.

## Status and handoff (2026-10-12)

Totals for this packet: 23 reconstructed routines (24 C++ entry points; 00A83120 has a formatter
and an apply entry), 117 ledger names applied to Ghidra with evidence comments, 26 bodies newly
defined in Ghidra. Everything is build-tested only (Win32 build plus the two existing CTests);
nothing is fixture-tested, ABI-compatible or game-validated. None of the new entry points is wired
into a host or the game executable yet.

Open items, in suggested order:

1. Shadow fitters: 00A8BD20 (cascade, 2007h bytes), 00A8AEB0 (single view), 00A8ECB0 and
   00A8B930 (unreferenced). Heavy x87; use the ESP-anchored listing sweep. The submit/single-view
   reconstructions here already define their host contracts (`ShadowCascadeSubmitHost::fit_cascade`,
   `ShadowSingleViewHost::fit_single_view`).
2. Unnamed host calls the shadow paths use: 00AD7A30 (on `[00F8C210]`), 00B1F4D0, 00B1F5E0.
3. Input band: about 60 STL instances (vector/deque/tree for backend device groups, gamepad force
   queues, text-owner events) remain raw; `BSP_InputAction_Update_Provisional` 00A92A20 looks like
   an out-of-line copy of logic inlined in 00A92C40 and has no caller.
4. Meaning of sound-instance vtable slots +34 (six-dword setter) and +38 (three-float getter),
   and of the sample resource field +08h tested by 00A818D0.
5. The `.def` round trip: 00A83120 writes `Decompress <n>` but the parser takes `Decompress` as a
   bare keyword; check whether a written file re-parses.
6. Wiring: the voice-playback hosts already call 00A7D120 as `set_sound_flag_00a7d120`, the
   in-mission interface calls 00A7B710 as `set_audio_environment`, and the simulation gate drives
   00A7F890; they could now bind to the reconstructions here instead of recording the call.

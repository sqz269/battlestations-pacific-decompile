# The in-mission interface manager's runtime (packet `cc_hud_updates`)

Addresses: 0068C1F0, 0068A990, 0068CC70, 0068B630, 0068BC60, 0068C0B0, 0068B3F0, 00647300,
0068AA40, 0068AB80, 0068B470, 0068A160, 0068A1F0, 00E198C4, 004E5252, 004DAB53, 004DAB82,
004DAB94, 00644220, 00645060, 004B4B00, 00611750, 00673130, 00612DA0, 00A79880, 00A7B710,
00CF1430, 00D7A24C, 00D7A308, 00D7A348, 00CE9B1C, 00CE5484, 00CF7A90.

`docs/IN_MISSION_INTERFACE_MANAGER.md` established what the manager at 00E198C4 **is**: a 108h-byte
front-end manager that owns 42 HUD screens and turns an interface id into a level-1 screen set. This
packet reads the three halves of its life that the earlier one left as "not analysed here": the
construction and Init pair, the **per-frame update 0068C1F0**, and the teardown the mission exit
runs. Everything below is a hypothesis reconstructed from the listing; no symbol was recovered from
the image. `include/bsp/in_mission_interface_runtime.hpp` and `src/in_mission_interface_runtime.cpp`
carry the reconstruction; they build on `include/bsp/ingame_interface.hpp` and redeclare nothing
from it.

## Where the per-frame update sits

`0068C1F0` is the manager's per-frame tick, and it is **not** reached from the fixed step. It is
step 17 of `BSP_Game_OnMove`, the in-mission frame:

```
004e524c  ECX = [00e198c4]
          if (ECX != 0 && [ECX + 3Ch] != 0)     ; interface_manager_present
004e5252      0068c1f0(ECX)                     ; this routine
004e5259  004c40f0(game)                        ; the front-end screen pump
```

`docs/MISSION_STATE_FRAME.md` rows 16-18. The manager's tick therefore runs **before** the pump that
drives the HUD pages' own update virtuals, and before the request drain. The per-frame interface
chain of one in-mission frame is:

| Frame step | Call site | Callee | What it does for the HUD |
| --- | --- | --- | --- |
| 17 | 004E5252 | 0068C1F0 | this routine: spectator resolution, the interface actions, the level-3 overlays, the ambience and the audio environment. It **enqueues** interface changes through 004CC460; it never applies one |
| 18 / 38 / 47 | 004E5259, 004E53B6, 004E5469 | 004C40F0 | pumps the registered front-end screens, which is what calls the HUD pages' update virtuals (`docs/HUD_CENTRAL_UPDATES.md`: 00649860 slot 44h, 006435D0 slot 4Dh, 005C0F20 slot 35h) through 004F71F0 and 004F8830 |
| 45 / 48 | 004E5442, 004E5477 | 006840F0 | the drain: compares the manager's `+04h/+1Ch` against `+20h/+38h` and, when they differ, calls its virtual `+10h`, which is `ApplyPendingInterface` 0068ACA0 |
| 50 | 004E549D | 004D8620 | the GUI manager update and page loading |

So the "pending-interface apply" the packet asks about is not inside 0068C1F0 at all: the update
pushes requests and the drain three steps later applies them. Nothing in 0068C1F0 calls 0068ACA0,
directly or through the vtable. The only per-frame contact with the screens is 00673130 on the
camera screen and 00612DA0 on the strategic-map screen.

The fixed step's only contact with this object is its tail gate (`docs/FIXED_STEP_FANOUT.md`,
00875FD1..00875FF5): it tests `00E198C4 != 0`, `[00E198C4 + 54h] != 0` and that sub-object's byte
`+5h`, and then calls 00A317F0, which is a bare `RET` in this build. None of the sixteen per-step
calls is the HUD's.

## 1. Construction and Init

### 0068A990, the constructor

`__thiscall(this) -> this in EAX`, `RET` at 0068A9BE. Calls the base 00684E10 at 0068A993, stores vtable
00CF7A60 at 0068A99A, and then clears exactly five fields, in this order: `+100h` (0068A9A0),
`+104h` (0068A9A6), `+ECh` (0068A9AC), `+54h` (0068A9B2) and `+FDh` (0068A9B5). Reconstructed
as `construct_in_game_interface_0068a990` plus the rule
`in_game_interface_constructor_clears(offset)`. `+54h` is the only screen pointer cleared, which is
what lets the fixed step's tail gate test it before Init has run.

### 0068CC70, Init

`__thiscall(this)`, `RET` at 0068D759, vtable slot `+04h`. The body is wrapped in an SEH frame
(handler 00C7E093) whose state slot at `[ESP+20h]` counts up as the screens are constructed, so a
throw unwinds the ones already built; that unwind path is not modelled.

| # | Host method | Call site | Native | Contract, from the callee's body |
| --- | --- | --- | --- | --- |
| 1 | `load_texture_atlas` | 0068CCD1 | 00AF0060 | `[00F8C26C]->00AF0060(&str)` with the pooled string `interface/Textures/game.ats` built at 0068CC9F by 0041DD40 (1Bh bytes) and 00BF7680 from the literal at 00CF7A74. The install has the `_dxt1` and `_dxt5_*` variants, so the literal is a stem |
| 2 | `publish_manager_global` | 0068CCFE | - | `00E198C4 = this`, the second write of the global; the first is 004E0480 in `BSP_Game_LoadMissionScene` |
| 3 | `base_init` | 0068CD04 | 00683A90 | the shared base Init; a bare `RET` in this build |
| 4 | `allocate_screen` | 0068CD0E (first of 41) | 00BF681B | `operator new(size)` for 41 of the 42 screens; the sizes are `kInGameHudScreens[i].size_bytes` |
| 4b | `allocate_screen` | 0068D47F | 00BF55BE | the exception: registry slot 29h at `+CCh` is allocated here and then zeroed with `memset(p, 0, F0h)` at 0068D48D (00BF79F0) before its constructor |
| 5 | `construct_screen` | 0068CD28 (first) | the slot's `constructor` | skipped when the allocation returned null |
| 6 | `register_screen` | 0068CD3F (first of 42) | `vtable+10h` | `BSP_FrontEndScreen_Register` 004F71D0: asks the screen's virtual `+00h` for a registry slot and stores it at `00E18B60 + slot*4`. **Not** guarded by the allocation: the native code reloads the stored pointer and calls through it either way |
| 7 | `push_interface_request` | 0068D73A | 004CC460 | `this->004CC460(20h, nullptr)`: the first interface of the mission is INTF_SCENE3D with no payload |
| 8 | `hud_root_set_field_1c` | 0068D743 | 00644220 | `[this+40h]->00644220(0)`. The callee's whole body is `this->[1Ch] = argument` |

The 42 screens, their offsets, registry slots, constructors, vtables and sizes are the table in
`docs/IN_MISSION_INTERFACE_MANAGER.md` and `kInGameHudScreens`; this packet did not re-derive them.

## 2. The per-frame update, 0068C1F0

`__thiscall(this = 00E198C4)`, no stack arguments, `RET` at 0068CC68, body 0068C1F0..0068CC68.
Ghidra has a function here. The pseudocode is unusable in three places (see **Corrections**), so
every ordering, argument count and register below comes from `bsp.py ghidra disasm`.

The body is six blocks. Only the last two run on every frame; the input block is skipped entirely
while the pending interface is one of the four camera modes.

### Block A, the spectator resolution (0068C1FE..0068C3E7)

Runs only when `004BCA50(game)` returns 4, 5, 6 or 7 (`in_mission_spectator_mode`), the same set
`docs/GAME_SIMULATION_GATE.md` reads.

| Host method | Call site | Native | Receiver and contract |
| --- | --- | --- | --- |
| `effective_game_mode` | 0068C1FE | 004BCA50 | `ECX = game` |
| `camera_screen_spectating` | 0068C23D | 006529E0 | `ECX = [this+BCh]`, the registry-slot 4Eh camera screen |
| `camera_screen_flag_09` | 0068C24C | 00652A20 | same receiver |
| `camera_screen_set_flag_08` | 0068C262 | 00652A50 | same receiver, argument 0 |
| `toggle_tactical_overlay` | 0068C26B | 0068C0B0 | `ECX = this`, argument 0 |
| `unit_is_kind_of` | 0068C29C, 0068C2FD, 0068C38E | `vtable+5Ch` | the unit's `IsKindOf(classId)`; ids 0Fh (plane) and 1Ch (command building) |
| `unit_player_slot` | 0068C2EB | 009FFD20 | `ECX = unit` |
| `pilot_detach` | 0068C32C | 007EE4E0 | `ECX = unit->[9D4h]`, the pilot object |
| `camera_screen_set_flag_08` | 0068C339 | 00652A50 | argument 0 |
| `toggle_tactical_overlay` | 0068C342 | 0068C0B0 | argument 0 |
| `camera_screen_spectating` | 0068C354 | 006529E0 | |
| `unit_bind_player` | 0068C36C | 00927CC0 | `ECX = unit`, argument `game+18ECh` |
| `camera_screen_set_flag_08` | 0068C379 | 00652A50 | argument 1 |
| `exit_free_camera_overlay` | 0068C380 | 0068B3F0 | `ECX = this` |
| `hud_root_set_spectated_unit` | 0068C3AF | 00647300 | **`ECX = [00E198C4]+40h`**, the HUD root screen, `RET 4` |
| `camera_screen_flag_08` | 0068C3BB | 00652A30 | |
| `camera_screen_flag_09` | 0068C3D1 | 00652A20 | |
| `camera_screen_set_flag_08` | 0068C3E2 | 00652A50 | argument 0 |

The walk is over `[[game+19CCh]+58h]`, advancing through node `+4h` with the unit at node `+8h`. It
breaks on the first match of either arm:

1. a plane (`IsKindOf(0Fh)`) with a pilot (`unit+9D4h`), whose `+54h` equals the local slot record's
   `+28h`, and whose `[pilot+3D0h]->[900h]` is 4 or 5: detach the pilot and drop out of the overlay.
2. a live unit (`+5Dh` clear) whose `009FFD20()` equals `game+18ECh`, that is not a command
   building, and whose `+188h` is 9 or the local slot: this is the candidate. When the camera screen
   is spectating, bind the player to it, raise the camera flag, collapse the free-camera overlay and
   hand the unit (or its pilot, for a plane) to the HUD root.

`00647300`, the HUD root's spectate setter, is `__thiscall(hudRoot, unit)`, `RET 4`: it tests the
unit with `00645060(unit, game+18ECh, 1)` (a nine-clause predicate over the unit's flags and
`IsKindOf` results), hands it to `hudRoot->00645600(unit)`, and then either pushes **INTF_LIMBO**
(when there is no controlled unit and the manager's `+04h/+1Ch` already match `+20h/+38h`) or calls
`hudRoot->00647040()`.

### Block B, the free-camera toggle (0068C3E7..0068C462)

`if ([00E1AE80] != 0 || WasPressedThisFrame(5Ah))`, then the latch is cleared.

| Host method | Call site | Native | Notes |
| --- | --- | --- | --- |
| `input_action_pressed(5Ah)` | 0068C3F8 | 004C43C0 | skipped when the latch is already set |
| `hud_root_set_field_1c` | 0068C419 | 00644220 | `[this+40h]->00644220(0)` |
| `push_interface_request(29h)` | 0068C424 | 004CC460 | INTF_FREECAMERA, no payload |
| `input_action_pressed(5Ah)` | 0068C431 | 004C43C0 | a **second, independent** edge read |
| `screen_2b_set_mode` | 0068C442 / 0068C44B | 0053D290 | `ECX = [this+90h]`, the slot 2Bh screen; 0 when the action was pressed, 1 when the arm was reached through the latch |
| `push_interface_request(20h)` | 0068C45D | 004CC460 | the other arm, when `[this+90h]+4h` is already set: back to INTF_SCENE3D carrying `00E188D8` |

### Block C, spectate-next (0068C462..0068C5E4)

Guarded by the local slot record's byte `+19h`. It is suppressed when the camera screen is
spectating, or when the level-3 vector holds exactly one entry and that entry is 39h or 3Bh. The two
id tests are two independent reads of `*00E18D1C` combined with byte ORs, not a short circuit.

Then a five-term OR (mode in {8,0,1,2,3}, a controlled unit exists, the camera screen is spectating,
`[this+A8h]->0054D3C0()` is non-zero, or the camera screen's `006529C0()` byte is set) admits the
level-3 collapse.

| Host method | Call site | Native |
| --- | --- | --- |
| `camera_screen_spectating` | 0068C484, 0068C51D | 006529E0 |
| `screen_34_query` | 0068C52C | 0054D3C0 (`ECX = [this+A8h]`) |
| `camera_screen_flag_66` | 0068C541 | 006529C0 (`ECX = [00E198C4]+BCh`) |
| `set_level3_screen_set` | 0068C587 | 004F8670 |
| `set_level3_input_contexts` | 0068C595 | 004D8B70 |
| `query_scene_unit` | 0068C59D | 004B4B00 |
| `push_interface_request(20h)` | 0068C5DF | 004CC460 |

The tail 0068C5A2..0068C5DF is the body of **0068AA40 inlined**. The rule, reconstructed as
`scene_request_accepts_unit_0068aa40`, is: the unit differs from the pending payload, is non-null,
has `+5Ch` set and `+5Dh`, `+60h`, `+5Eh` clear, and the pending interface id is not one of
{29h, 2Bh, 2Ch, 2Dh}. `004B4B00` itself returns the controlled unit 00E188D8 when it is
`IsKindOf(5)`, that unit's `+3D0h` when it is `IsKindOf(18h)`, and null otherwise.

### Block D, the two movie cameras (0068C5E4..0068C61A)

`WasPressedThisFrame(89h)` at 0068C5EF runs `0068A160` at 0068C5FA (`ECX = this`), which either
calls 005CD0F0 with the controlled unit or, when `[this+98h]+4h` is set, calls 005CD160 and
re-spectates through 00647300. `WasPressedThisFrame(8Ah)` at 0068C60A runs `0068A1F0` at 0068C615,
which `docs/INTERFACE_RUNTIME_TAIL.md` already reads as the entry into **INTF_MOVIECAMERANEW** (2Ch)
through the slot 37h screen at `+9Ch`.

### Block E, the input block (0068C61A..0068CAA9)

Entered only when `this->[20h]` (the pending interface id) is **not** 2Bh, 2Ch, 2Dh or 29h; the four
camera modes jump straight to the audio tail at 0068CAD9. `in_game_interface_is_camera_mode` in
`bsp/ingame_interface.hpp` is the same predicate.

| Host method | Call site | Native | Contract |
| --- | --- | --- | --- |
| `input_action_pressed(C1h)` | 0068C64C | 004C43C0 | the back-out action |
| `modifier_key_allows_back_out` | 0068C67A | `[00CE2350]` (GetKeyState) | `[00F88A30] != 0 \|\| GetKeyState(A4h) >= 0`; A4h is VK_RMENU, so the arm runs while right ALT is up |
| `back_out_one_overlay_level` | 0068C6B1 | 0068B470 | `ECX = this`. The callee collapses level 3, calls `[this+4Ch]->0051E8E0()` and then either `0068AC30()` or a level-2 collapse (004F85D0, 004D8AE0, 0068AA40) depending on `[this+54h]+4h` |
| `screen_3b_idle` | 0068C6BB | 005FB080 | `ECX = [this+64h]`, the slot 3Bh screen. The `else` of the whole C1h test |
| `input_action_pressed(D6h)` | 0068C6CB | 004C43C0 | |
| `collapse_overlays(1, 1)` | 0068C6E7 | 0068AB80 | after clearing `[this+5Ch]+4h` at 0068C6E3 |
| `input_manager_update(0.0f)` | 0068C6F9 | 00A92C40 | receiver from 004BEC00 at 0068C6F2 |
| `input_action_pressed(FBh)` | 0068C709 | 004C43C0 | sampled into `[ESP+12h]` |
| `device_a_flag_0b` | 0068C712 | 004BEC00 | `[[input+4h]+2F3Ch]+0Bh`, held in BL |
| `device_a_flag_10` | 0068C723 | 004BEC00 | `[[input+4h]+2F3Ch]+10h`, into `[ESP+10h]` |
| `input_action_pressed(FCh)` | 0068C744 | 004C43C0 | into `[ESP+13h]` |
| `device_b_flag_0b` | 0068C74D | 004BEC00 | `[[input+4h]+2F6Ch]+0Bh`, into `[ESP+0Fh]` |
| `device_b_flag_10` | 0068C763 | 004BEC00 | `[[input+4h]+2F6Ch]+10h`, into `[ESP+11h]` |
| `camera_screen_frame_step` | 0068C77F | 00673130 | `ECX = [this+BCh]`. SEH frame 00C7C220; its body runs only while its `+9h` and `+8h` are clear, `0066FFA0()` is true and the map screen's `+4h` is clear |
| `map_screen_frame_step` | 0068C78A | 00612DA0 | `ECX = [this+C0h]`, the slot 4Fh screen. Runs while its `+4h` is clear and `[this+B0h]+121h` is clear; branches on the camera screen's byte `+5h` and drives a widget's virtual `+34h` either way |
| `support_request_pending` | 0068C7A1, 0068C923 | 008ED9C0 | `ECX = [00F88C30]`, walking that manager's per-player list at `+20h + slot*0Ch` |
| `set_level3_screen_set` | 0068C7DE, 0068C932, 0068CA0A, 0068CA86 | 004F8670 | |
| `set_level3_input_contexts` | 0068C7EC, 0068C94F, 0068C967, 0068CA17, 0068CA94 | 004D8B70 | |
| `push_scene_request_for_unit` | 0068C7FC, 0068CA27, 0068CAA4 | 0068AA40 | preceded by 004B4B00 at 0068C7F4, 0068CA1F, 0068CA9C |
| `controlled_unit_overlay_argument` | 0068C81B | 00927880 | `ECX = 00E188D8`; virtual `+114h` then virtual `+18h` on the result |
| `toggle_tactical_overlay` | 0068C823, 0068C890, 0068C89F | 0068C0B0 | `ECX = this` |
| `camera_screen_flag_09` | 0068C846 | 00652A20 | |
| `pause_screen_child_query` | 0068C863 | `vtable+38h` | on `[[00E198C4+A8h]+20h]` |
| `input_action_pressed(108h)` | 0068C8C2 | 004C43C0 | |
| `camera_screen_spectating` | 0068C9A6 | 006529E0 | |
| `exit_free_camera_overlay` | 0068C9BB | 0068B3F0 | after `[this+BCh]+0Ah = 1` at 0068C9B5 |
| `map_screen_busy` | 0068C9CE, 0068CA49 | 00611750 | `ECX = [this+C0h]`; the body is `return [this+1F9h] \|\| [this+13Fh]` |
| `input_action_pressed(4Bh)` | 0068CA34 | 004C43C0 | |
| `camera_screen_flag_66` | 0068CABD | 006529C0 | |
| `camera_screen_set_flag_66` | 0068CAD4 | 006529B0 | argument 0 |

Six input bits are sampled **before** any of them is tested, so the two device reads happen every
frame regardless of which arm wins. The chain that follows has four arms and the first match wins:

1. `device_a_flag_0b && !map_screen_wanted`: `0068C0B0(controlled unit ? 00927880() : 0)`.
2. `device_b_flag_0b && !map_screen_wanted && !camera_screen_flag_09 && !(pause child query &&
   pause +8h)`: `0068C0B0(controlled unit ? [this+CCh]+4Ch : 0)`.
3. `(device_a_flag_10 || device_b_flag_10 || WasPressed(108h)) && !slot-3Bh blocks && !(pause +5h &&
   pause +8h)`: when `00E0C978` is set, the map is closed and a support request is pending, it
   **installs** level 3 instead of collapsing it; otherwise it collapses.
4. `FBh || FCh`: when the camera screen is spectating, raise its `+0Ah` and call 0068B3F0; otherwise
   collapse level 3 if the map is open and not busy.

The level-3 sets of arm 3, with the argument counts taken from the cleanups:

| Site | Call | Cleanup | List |
| --- | --- | --- | --- |
| 0068C932 | 004F8670 | `ADD ESP,0Ch` at 0068C93C | screens {4Fh, 50h} |
| 0068C94F | 004D8B70 | `ADD ESP,10h` at 0068C954 | contexts {14h, 0Eh} when `game+19C4h` is set |
| 0068C967 | 004D8B70 | `ADD ESP,1Ch` at 0068C96C | contexts {14h, 4, 0Bh, 6, 0Ah} otherwise |

The `PUSH 0` at 0068C946 is **shared by both branches**: it is the varargs terminator, which is why
the decompiler renders the first list as three arguments while the cleanup says four.

The same level-3 collapse appears at four sites in this body (0068C569, 0068C7C0, 0068C9ED,
0068CA68) and again inside 0068C0B0, 0068B3F0 and 0068B470. All are byte-identical: the vector must
be non-empty; `[this+BCh]+0Ah` is raised when `[00E198C4+BCh]+30h` is set; both level-3 setters take
the empty list; then the scene request is re-issued for `004B4B00()`'s unit.

### Block F, the ambience and the audio environment (0068CAD9..0068CC5D)

Runs on every frame, camera modes included.

| Host method | Call site | Native | Contract |
| --- | --- | --- | --- |
| `refresh_camera_transform` | 0068CB07, 0068CBA7 | 00B6DB70 | only when `camera->[5Ch] & 2` is clear |
| `set_ambient_volume` | 0068CB45 | 00A79880 | `ECX = [this+104h]`, one float pushed. `BSP_SoundInstance_ScaleVolume` in the ledger |
| `water_height` | 0068CBDE | 0078CF20 | `ECX = [game+19F0h]`, arguments `(camera+120h, camera+124h)` |
| `controlled_unit_is_submarine` | 0068CC04 | `vtable+5Ch` | `IsKindOf(8)` on 00E188D8 |
| `refresh_controlled_unit_pose` | 0068CC1B | 00414DB0 | only when `unit->[C8h]` is clear |
| `set_audio_environment` | 0068CC4F | 00A7B710 | `ECX = [00F8BBD8]`, the sound manager |
| `tick_profile_hints` | 0068CC56, 0068CC5D | 004C1E90 then 00427190 | `004C1E90(0Ch)`, then the result in ECX to 00427190 |

The volume curve, 0068CB0C..0068CB37:

```
value = (float)(((double)camera->[124h] - 2.0) * 0.25)   ; 00D7A308, 00D7A348
if (value < 0)   value = 0
if (value > 1.0) value = 1.0                             ; 00D7A24C
[this+104h]->00A79880(value)
```

It runs only when `[this+104h]` is non-null and `game+19FCh` is non-null. So the unit ambience the
manager started in `ApplyPendingInterface` is re-scaled from the **camera's** height every frame:
silent at or below 2.0 and full at 6.0 and above.

The environment name, 0068CB4A..0068CC48, picks one of three literals and hands it to 00A7B710:

- `Cockpit` (00CF7A90) when `[this+6Ch]+4h` is set and `00604F30() == 1`, or `[this+84h]+4h` is set
  and `[this+84h]+74h == 2`.
- otherwise `Air` (00CE9B1C) when the camera height `camera+124h` is strictly above
  `0078CF20(camera+120h, camera+124h)` **and** the controlled-unit term is 1, else `Underwater`
  (00CE5484).
- the controlled-unit term is 1 unless 00E188D8 exists, is `IsKindOf(8)` (a submarine) and its
  `+100h` is strictly below the `-4.0f` at 00CF1430. The two terms are combined with `TEST BL,AL`, a
  byte AND of two already-computed flags, so the water height is always computed.

## 3. Teardown

`0068BC60` is the scalar deleting destructor, `__thiscall(this, byte flags) -> this`, `RET 4` at
0068BC7B: it calls 0068B630 and then frees the object when bit 0 of `flags` is set.

`0068B630`, `__thiscall(this)`, `RET` at 0068BC5F, SEH handler 00C7DDD4.

| # | Host method | Call site | Native | Contract |
| --- | --- | --- | --- | --- |
| 1 | `restore_vtable` | 0068B651 | - | `[this] = 00CF7A60` |
| 2 | `stop_ambient_sound` | 0068B67A | `vtable+8h` | on `[this+104h]`, argument 0 |
| 3 | `release_ambient_instance` | 0068B68A | `[00CE2220]` | `InterlockedDecrement(inst+4h)`, then the instance's virtual `+00h` when the count reaches zero |
| 4 | `collect_screens` | 0068B6CA..0068BA75 | 00686C20 | 44 calls; the callee de-duplicates, so the vector is the distinct set of the 42 pointers |
| 5 | `screen_visible` / `screen_hide` | 0068BAC3 / 0068BACF | `vtable+1Ch` | pass one skips a null entry (0068BAB2) and hides only a screen whose `+5h` is set |
| 6 | `screen_clear_flags` | 0068BAD3 | - | `+4h` and `+5h` cleared unconditionally |
| 7 | `screen_commit_visibility` | 0068BAD9 | 004F83B0 | |
| 8 | `screen_delete` | 0068BB34 | `vtable+0Ch` | pass two, argument 1: the screens' own deleting destructors, which also unregister them from 00E18B60 |
| 9 | `clear_manager_global` | 0068BB54 | - | `00E198C4 = 0`, **before** the atlas unload |
| 10 | `unload_texture_atlas` | 0068BB96 | 00AEFA30 | the same pooled stem Init loaded |
| 11 | `close_menu_command_screen` | 0068BBC1, 0068BBC8 | 00425D10(1), 00530630 | |
| 12 | `free_screen_vector` | 0068BBD6 | 00BF65AC | |
| 13 | `release_ambient_instance` | 0068BBFD | `[00CE2220]` | the second release of `+104h` |
| 14 | `release_ambient_source` | 0068BC27 | `[00CE2220]` | `+100h` |
| 15 | `destroy_base` | 0068BC49 | 00684FA0 | |

`python tools/bsp.py ghidra flow 0068b630` reports one fall-through gap, 0068BBDB..0068BBDE (three
bytes, the `ADD ESP,4` after the `_free`), which is why the export shows steps 13-15 as unreachable.
The gap was **not** repaired; the reconstruction follows the real control flow.

### What the mission exit calls

The request 10h arm of `BSP_Game_DrainStateRequestQueue` 004E4430, entered at 004E458A, calls
`BSP_Game_TeardownSessionState` 004DA780 at **004E4710** (`docs/GAME_EXECUTABLE.md` milestone 2g).
Three sites inside 004DA780 touch the manager, in listing order:

| Call site | Effect | Guard |
| --- | --- | --- |
| 004DAB53 | `[00E19894] = 0`, releasing the front-end interface lock | `[00E198C4] != 0` |
| 004DAB82 | the manager's virtual `+00h` with `flags = 1`, i.e. 0068BC60 with the free bit | `[00E198C4] != 0` |
| 004DAB94 | `00E198C4 = 0` | inside the same guarded block |

The camera at `game+19FCh` is released at 004DAB77 (00B6DFA0) **between** the lock release and the
destructor call, so the manager's audio tail can no longer reach a camera after that point. The
`MOV ECX,[00E198C4]` at 004DAB65 is dead: every path that reaches 004DAB88 reloads ECX at 004DAB82.
004DA780 is read-only for this packet: it is not leased, not renamed and not reconstructed here.

## Host methods the executable must implement, in call order

For the update, the order is exactly the table order above: `effective_game_mode`, then block A's
camera-screen accessors and the unit walk, then `input_action_pressed(5Ah)` and the free-camera
arm, then block C, then `input_action_pressed(89h)`/`(8Ah)`, then the input block's eleven action
and device reads with `camera_screen_frame_step` and `map_screen_frame_step` between them, and
finally `set_ambient_volume`, `set_audio_environment` and `tick_profile_hints`.
`reports/hud_updates.json` carries the same rows as data, with `address` (call site), `native`
(callee) and the containing function for every one.

The minimum an executable milestone needs to run a mission frame with a real manager is: Init's
eight steps, the update's `effective_game_mode`, `input_action_pressed`, `camera_screen_*`,
`level3_*` and the audio tail, and the teardown's fifteen. Everything else can report "nothing
pending" without changing the observable frame.

## Run-time evidence

`bsp_game.exe --frames 40 --log local/hud_updates_run.log --game-root "<install>"`, exit 0. The run
reaches the title screen and does not enter a mission, so it never executes 0068C1F0's body: the
mission-frame host at `src/game_hosts_mission_frame.cpp:744` records step 17 and returns, which is
exactly the slot this packet's reconstruction is meant to fill. **No claim here about what the
update does on a given frame rests on a run.** One thing the run does settle is Init's first step:

```
atlas interface/textures/game_dxt1.ats             items= 32 texture=interface/textures/game_DXT1.dds
atlas interface/textures/game_dxt5_1.ats           items=173 texture=interface/textures/game_DXT5_1.dds
atlas interface/textures/game_dxt5_2.ats           items= 70 texture=interface/textures/game_DXT5_2.dds
```

The loader really does expand the `interface/Textures/game.ats` stem into the three per-format
files, which `docs/IN_MISSION_INTERFACE_MANAGER.md` inferred from the installed tree alone.

## Corrections

- **`FUN_0068c1f0`'s `+104h` test is not a mask test.** The decompiler renders 0068CAD9..0068CAE9 as
  `(-(uint)([this+104h] != 0) & 0xE19311) != 0`. The instructions are `NEG EDX; SBB EDX,EDX; TEST
  EDX,0E19311h`, the MSVC idiom for "the pointer is non-null"; the constant is whatever immediate
  the encoder had to hand and has no meaning. The same idiom opens the destructor at 0068B665.
  Was: a bit test against E19311h. Is: `[this+104h] != 0`. Evidence: the three instructions and the
  identical pair in 0068B630.
- **The audio environment's third term is the controlled unit, not a device.**
  `include/bsp/simulation_gate.hpp` records `AudioEnvironmentInputs::device_allows_underwater` as
  "the device-capability path at 0068CC0A, which forces Air when no device reports capability 8 or
  its float +100h is below 00CF1430". 0068CBF3 loads **00E188D8**, the player's controlled unit
  (`docs/UNIT_INSTANCE_UPDATE.md`: 004C0890 writes a unit instance there), calls its
  `IsKindOf(8)` — `kUnitTypeSubmarine` in `bsp/ingame_interface.hpp` — and compares its world height
  `+100h` against `-4.0f`. There is no device and no capability. The term is 1 (allowing `Air`)
  unless the player is in a submarine deeper than four metres. That header belongs to
  `game_simulation_gate` and was not edited; `in_mission_audio_environment_0068cb4a` in this
  packet's header takes the corrected inputs.
  Was: a device-capability flag. Is: the controlled unit's submarine-and-depth test. Evidence:
  0068CBF3..0068CC3A and the writer of 00E188D8.
- **The 0068C946 zero is a varargs terminator, not an argument of one branch.** The decompiler
  prints `FUN_004d8b70(DAT_00e188a8, 0x14, 0xe)`; the cleanup at 0068C954 is `ADD ESP,10h`, so the
  call has four arguments and the fourth is the shared `PUSH 0`.
  Was: three arguments. Is: four. Evidence: the cleanup instruction, checklist rule 7.
- **`00647300` is a method, not a free function.** Ghidra types it as `void FUN_00647300(undefined4)`
  because its body never uses `this` directly. `MOV ECX,[00E198C4+40h]` at 0068C3A1/0068C3AB and the
  `MOV ESI,ECX` at 00647311 show it is `__thiscall(hudRootScreen, unit)`, `RET 4`; ESI is the
  receiver of both 00645600 and 00647040.
  Was: `__cdecl(unit)`. Is: `__thiscall(hudRoot, unit)`. Evidence: the two call sites and 00647311.
- **`0068B3F0`'s conditional write is dead.** The body clears `[00E198C4+BCh]+30h` and then, three
  instructions later, tests the same byte to decide whether to set `[this+BCh]+0Ah`. The test can
  never pass. `docs/IN_MISSION_INTERFACE_MANAGER.md` describes the shared collapse as always able to
  raise `+0Ah`; inside 0068B3F0 specifically it cannot.
  Was: the collapse may raise `+0Ah` at every site. Is: not at 0068B3F0. Evidence: the two accesses
  to the same byte in one basic block.
- **`0051E8E0` in 0068AB80 has a receiver.** `docs/IN_MISSION_INTERFACE_MANAGER.md` lists the
  `run_extra_hook` arm as `0051E8E0()`. The call at 0068ABE5 is preceded by
  `MOV ECX,[ESI+4Ch]`, so it is a method on the registry-slot 26h screen.
  Was: a free call. Is: `[this+4Ch]->0051E8E0()`. Evidence: 0068ABE2.
- **Registry slot 29h is not allocated with `operator new`.** Init uses 00BF681B for 41 screens and
  00BF55BE plus a `memset(p, 0, F0h)` for the `+CCh` screen at 0068D47F..0068D492.
  Was: 42 `operator new` calls. Is: 41. Evidence: 41 `CALL 0x00bf681b` against 42 `CALL EAX`
  register calls in the Init listing.
- **Two constructor site addresses in `docs/IN_MISSION_INTERFACE_MANAGER.md` point inside an
  instruction.** It cites 0068A9AE for the `+ECh` clear and 0068A9B6 for the `+FDh` clear; the two
  `MOV` instructions start at 0068A9AC and 0068A9B5 and are six bytes each. The fields and the
  behaviour are unchanged.
  Was: 0068A9AE and 0068A9B6. Is: 0068A9AC and 0068A9B5. Evidence: the listing of 0068A990.
- **The packet brief's "pending-interface apply" is not in the update.** 0068C1F0 pushes requests
  through 004CC460 and never calls 0068ACA0. The apply happens at frame steps 45 and 48 through the
  drain 006840F0. Evidence: the callee list of 0068C1F0 and `docs/MISSION_STATE_FRAME.md` rows 45
  and 48.

## Coverage

| Routine | Coverage |
| --- | --- |
| 0068A990 | complete (the derived part; the base 00684E10 is another packet's) |
| 0068CC70 | complete as a sequence; the SEH unwind of a partially built screen list is not modelled |
| 0068C1F0 | complete (0068C1F0..0068CC68, every branch) |
| 0068B630 | complete |
| 0068BC60 | complete |
| 0068AA40 | complete (reconstructed as the predicate `scene_request_accepts_unit_0068aa40`) |
| 0068C0B0, 0068B3F0, 0068B470 | partial: read end to end and named, reconstructed only as host methods, not as their own sequences. Unread: 0066FFA0, 0068BC80, 0065BE80, 0065F170, 0068AC30 |
| 00647300 | partial: the sequence is read; `00645060`'s nine clauses and `00645600`/`00647040` are not reconstructed |
| 00673130, 00612DA0 | partial: entry guards read, bodies not reconstructed |
| 004DA780 | read only, not claimed, not reconstructed |

## no_ghidra_function

none. Every address this packet reads has a Ghidra function: 0068C1F0 (body 0068C1F0-0068CC68),
0068A990 (0068A990-0068A9BE), 0068CC70 (0068CC70-0068D759), 0068B630 (0068B630-0068BC5F), 0068BC60
(0068BC60-0068BC7D), 0068AB80 (0068AB80-0068AC23), 0068C0B0 (0068C0B0-0068C1E6), 0068B3F0
(0068B3F0-0068B469), 00647300 (00647300-00647360), 0068AA40 (0068AA40-0068AA86), 0068B470
(0068B470-0068B50E), 004DA780 (004DA780-004DB02C) and 004E4A40 (004E4A40-004E5537). 0068ACA0, which
`docs/IN_MISSION_INTERFACE_MANAGER.md` records as having none, now has one
(0068ACA0-0068B38F); that was created by the integrator, not by this packet.

## Follow-up packets

- `in_mission_spectate_predicate` — 00645060, 00645600, 00647040, 00927C50 and the HUD root's
  `+1Ch`. Contract: the nine clauses that decide whether a unit can be spectated, and what
  00647040 does when the controlled unit still exists.
- `in_mission_tactical_overlay` — 0068C0B0, 0066FFA0, 0068BC80, 0065BE80, 0065F170 and the level-3
  set {4Eh, 50h}. Contract: what the strategic overlay is, and what `game+19C4h` distinguishes in
  the two input-context lists both this routine and 0068C1F0 choose between.
- `in_mission_camera_screen_flags` — 006529B0, 006529C0, 006529E0, 00652A20, 00652A30, 00652A50 and
  00673130 on the 480h-byte slot 4Eh screen. Contract: name the bytes `+8h`, `+9h`, `+0Ah`, `+30h`,
  `+5h` and `+66h` that this update reads and writes eleven times a frame.
- `in_mission_input_device_bytes` — `[[input+4h]+2F3Ch]` and `[[input+4h]+2F6Ch]`, bytes `+0Bh` and
  `+10h`. Contract: which two bindings these are; they drive three of the four overlay arms and are
  read every frame before any of them is tested.
- `in_mission_support_requests` — 008ED9C0 on 00F88C30 and the per-player list at `+20h + slot*0Ch`.
  Contract: what "pending" means; it gates both the map's auto-close and the level-3 install.

## Uncertainties

- What `game+19C4h` selects. It picks between a two-entry and a five-entry level-3 input-context
  list here and in 0068C0B0; "networked" fits the pattern of the other `game+19xx` bytes but is an
  inference and is not asserted.
- The meaning of the camera screen's `+8h`, `+9h`, `+0Ah`, `+30h` and `+66h`. The update treats
  `+8h` as "spectating this frame" and `+0Ah` as "an overlay was collapsed", which is consistent
  with every site read here but is not established from a writer outside this routine.
- `00611750`'s two bytes `+1F9h` and `+13Fh` on the slot 4Fh screen: "busy" is the role they play at
  both call sites, not a recovered meaning.
- Whether `0053D290`'s argument is a mode or a boolean. The two call sites pass 0 and 1 and nothing
  else in this body reads it back.
- The unit field `+100h` the environment tail compares against `-4.0f`. `00414DB0`
  (`BSP_EntityPose_RefreshWorld`) refreshes the object before the read, so it is a world-space
  component, but the entity pose layout is a different type from the camera's and the offset was
  not confirmed against a writer.

## State reached

| Address | State |
| --- | --- |
| 0068C1F0 | exported, analysed from the listing, reconstructed as a sequence over a host, build-tested |
| 0068A990, 0068CC70, 0068B630, 0068BC60 | exported, analysed, reconstructed as sequences and rules, build-tested |
| 0068AA40 | analysed, reconstructed as a predicate, build-tested |
| 0068C0B0, 0068B3F0, 00647300, 0068B470 | analysed from the listing and named; not reconstructed |
| 00673130, 00612DA0, 00611750, 00645060, 004B4B00, 00644220 | read for their contracts only |
| 004DAB53, 004DAB82, 004DAB94 | analysed from the disk bytes; recorded as data, not reconstructed |
| 004E5252 | analysed; the guard is `docs/MISSION_STATE_FRAME.md` row 16 |

Nothing here is game-validated and none of it is a drop-in binary replacement.

# cc13-fix24: two HUD host stand-ins (0077A2F0, 0054D3C0)

Two HUD host hooks modelled image functions with the wrong shape. Each fix sits behind its own
new switch, OFF by default. With both switches OFF the host behaves exactly as before. Neither
switch was flipped, and no game run was made.

| Switch | Header | Host method | Image function |
|---|---|---|---|
| `kHudUnitHealthColourBound` | `include/bsp/game_hosts_hud_world.hpp` | `MarkerRuntimeBinding::unit_health_colour_0077a2f0` (new hook on `bsp::HudMarkersRuntimeHost`) | 0077A2F0 `BSP_Unit_HealthColour` |
| `kHudHintsSecondsLeftBound` | `include/bsp/game_hosts_hud.hpp` | `HudUpdateBinding::screen_34_query` (`src/game_hosts_hud.cpp`) | 0054D3C0 (hints screen seconds left) |

## (a) 0077A2F0: an RGBA colour, not a float

The image body is `__thiscall(ECX = float out[4]; float health) RET 4`, with EAX = out. It
blends three static RGBA colours: 00F8715C (red), 00F8716C (yellow) and 00F8717C (green). They
are set once under the guard bits 1, 2 and 4 of 00F8718C, and the scaling goes through 00BEE140.
The body is already reconstructed as `bsp::cc13_front6::unit_health_colour_0077a2f0`
(src/cc13_front6.cpp; docs/CC13_FRONT6.md). The host's `float unit_health(void* unit)` has
neither the image's input nor its output.

### Callers

A rel32 scan of the PE (CALL/JMP E8/E9 plus absolute dwords) found nine calls and no absolute
references.

| Call | Function | Argument | How the result is used |
|---|---|---|---|
| 005AF6BB | 005ADDA0 `BSP_HudMapScreen_UpdateUnitIcon` | 1.0f (FLD1; FSTP [ESP]) | RGBA read at EAX (cc13_big5ad.cpp, its own host interface) |
| 005B1F6C | 005ADDA0 | 1.0f | `MOV ECX,[EAX]`... (cc13_big5ad.cpp) |
| 00641B6E | 00641910 `BSP_InGameHudMarkers_AddKind1CMarker` | 1.0f | RGBA (cc13_pkt642.cpp) |
| 00641D88 | 00642040 `BSP_InGameHudMarkers_BuildUnitMarker` | 1.0f | RGBA at EAX (cc13_pkt642.cpp) |
| 0064240B | 00642040 | 1.0f; ECX = ESP+38h | 00642413..0064241B copy the four dwords to locals |
| 0064265E | 00642040 | 1.0f | RGBA (cc13_pkt642.cpp) |
| 00642865 | 00642040 | 1.0f | RGBA, from 0064286E |
| 0077A4ED | 0077A4B0 (FUN_) | float at [ESP+1Ch], a real fraction | EAX pushed as an argument to the next call |
| 0077A557 | 0077A510 (FUN_) | float at [ESP+0Ch] | EAX pushed as an argument |

At every marker site, the call to the unit's vtable +140h just before it (for example
006423FF) does not feed the argument. FLD1 loads the argument fresh, so all seven HUD
marker and map calls compute 0077A2F0(1.0) = 00F8717C, the opaque green {0.392157,
0.909804, 0.145098, 1.0}. Only 0077A4B0 and 0077A510 pass a varying health value.

### Host callers today and with the switch ON

- `HudMarkersRuntimeHost::unit_health(void*)` has **no caller** anywhere in `src/`. The
  marker runtime (`src/hud_markers_runtime.cpp`) reconstructs 006430C0's dispatch and the
  geometry helpers, not 00641910 or 00642040. Those builders live in `cc13_pkt642.cpp` and
  `cc13_big5ad.cpp` behind their own host interfaces, which no game host implements. The
  `HudRowWidget` hits on `unit_health` in `hud_root_rows.cpp` and `unit_fire_flooding.cpp` are
  other interfaces (00923BE0 / +370h) and are not affected.
- The fix adds a correctly shaped hook, `unit_health_colour_0077a2f0(float* out, float
  health)`, to `HudMarkersRuntimeHost`.
  - **ON**: it runs the front6 body through `HealthColourFront6Binding`. That binding maps
    00F8715C/6C/7C/8C to host-owned copies (zero-initialised like the image's .bss) and
    00BEE140 to `bsp::engine_services_cc13::scale_vector4_00bee140`. Every other Front6Host
    callback throws, because 0077A2F0 never reaches them.
  - **OFF**: it records 0077A2F0 as unimplemented and zero-fills the four floats, matching the
    old 0.0f stand-in.
- The old `unit_health(void*)` is unchanged and its comment now says it is a mis-shaped
  stand-in.
- **Behaviour change today: none.** Nothing calls either hook. The first observable effect
  will come when 00642040/00641910 (cc13_pkt642) or 005ADDA0 (cc13_big5ad) are wired to a
  game host. Their side-marker widget colour (screen 4Dh, the in-mission unit side markers)
  and the HUD map unit-icon colour would then become the green 00F8717C instead of whatever
  stand-in that wiring uses. Those screens appear in every mission with a HUD.

## (b) 0054D3C0: a truncated int, not a bool

The body is `F3 0F 2C 41 54 / C3` (`CVTTSS2SI EAX,[ECX+54h]; RET`), `__fastcall(ECX = hints
screen)`. +54h is the hints screen's seconds-left float (vtable 00CEE290, 7Ch bytes, HUD manager
+A8h; docs/CC13_GUI_SCREENS_3.md). CVTTSS2SI truncates toward zero; NaN, infinities and values
outside the int32 range give 80000000h.

### Callers

The rel32 scan found five calls and no absolute references.

| Call | Function | Use of EAX | Host today |
|---|---|---|---|
| 0068C52C | 0068C1F0 `BSP_InGameInterface_Update` (spectate-next gate) | `TEST EAX,EAX; JNE 0068C54E`: any non-zero int admits | `screen_34_query` -> `record_false` (false) |
| 0069174A | 00690FD0 `BSP_AwardTracker_TriggerHint` | `MOV [ESP+14h],EAX; FILD`: compared with `[EDI+24h] * 0.25` (double 00D7A348) | `gui_screens_3.cpp` `hints_seconds_left_0054d3c0()`, already int; no game host implements it |
| 00691FE2 | 00690FD0 | `CVTSI2SS XMM0,EAX` stored as the tracker's last timed seconds | same |
| 008D2A50 | 008D2850 `BSP_LuaBinding_IsHintActive` | pushed as the `duration` of the returned table (00B67460) | same |
| 008D2B53 | 008D2850 | pushed as the integer result | same |

Only the first is a bool consumer, and only it goes through the host stand-in this packet
owns. The other four already take an `int` through `gui_screens_3`'s host interface, which has
no game-host implementation, so this packet leaves them alone.

### 0068C52C with the switch ON

With the switch ON, `screen_34_query` returns `hints_screen_seconds_left_0054d3c0(screen) != 0`
(the front6 body over the host's copy of the hints screen):

- +54h in the open interval (-1, 1), including -0.0 and the subnormals: the int is 0, so the
  query is false.
- +54h >= 1.0 or <= -1.0 (and inside the int32 range): the int is non-zero, so the query is
  true. Exactly 1.0 gives 1, which is true. 0.999999 gives 0, which is false.
- NaN, ±infinity or |+54h| >= 2^31: the int is 80000000h, so the query is true.
- A plain bool test of the float would differ from the image throughout (0, 1) and (-1, 0),
  where it is true and the image is false. NaN would agree, since both are true.

The host copy of the screen (`GameHudHost::Impl::hints_screen`, 7Ch bytes, only +54h read)
is zero. In the image, Register 0054D860 stores +54h = 0.0f at 0054D900. ShowHint 0054E020,
the countdown in Update 0054E6B0 and Dismiss 0054E610 are the only other writers, and the host
runs none of them: the mission frame's hint passes only record. So with the switch ON the query
still answers false on every frame. That matches OFF, except that the call is logged as
implemented. The spectate-next block's admission (`run_spectate_next_block`,
in_mission_interface_runtime.cpp) therefore does not change. It is the in-mission
spectate/next-unit switch, which is reachable in any mission when the player has no controlled
unit. It would start to change once a host path shows a timed hint. While a hint shows at least
1 s left, the next-unit request is admitted even with no controlled unit.

## Uncertainty and skips

- The hints screen copy models only +54h. The other 7Ch bytes stay zero and nothing reads
  them.
- 0077A2F0 is modelled with one rounding per stored float, at either x87 precision
  (docs/CC13_FRONT6.md). 00BEE140 is the engine_services_cc13 reconstruction.
- Skipped: wiring cc13_pkt642, cc13_big5ad or gui_screens_3 hosts into the game. Those files
  and interfaces are outside this grant.

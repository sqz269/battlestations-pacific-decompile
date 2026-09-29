# Making a unit the controlled unit (packet `cc_controlled_unit`, part A)

Addresses: 004C0890, 00645600, 00645060, 00954990, 00817380, and read-only 0080E290,
00B0D7B0, 007F3A60, 00959450, 00694A60, 006952A0.

Worker `agent/cc-controlled-unit`, 2026-09-11. Ghidra was read-only for this packet.
Every body below was read from the live disassembly, not from the pseudocode.

## The short answer

`004C0890` does far less than the packet brief assumed. It writes one global, decides
which object is actually being driven, and pushes one handle into the render-resources
singleton. That is the whole body, `004C0890..004C0924`, 51 instructions. Everything a
player would call "taking control of this ship" is in its caller `00645600`.

## `004C0890`, `__thiscall(unit)`, `RET 0`

```
004C0893: DAT_00E188D8 = unit                     ; before any test, null included
          target = unit->IsKindOf(5)   ? unit
                 : unit->IsKindOf(18h) ? [unit+3D0h]
                 : null
          if (target && (target->IsKindOf(0Fh) || target->IsKindOf(18h))) {
004C08F7:     h = target->vtable[18h]()
004C0900:     DAT_00E188DC = h
004C0905:     00B0D7B0(h) with ECX DAT_00F8D39C
          } else {
004C0914:     DAT_00E188DC = 0
004C091E:     00B0D7B0(0) with ECX DAT_00F8D39C
          }
```

The resolution itself was already reconstructed as `resolve_controlled_unit_004c0890` in
`bsp/unit_instance.hpp` (`docs/UNIT_INSTANCE_UPDATE.md`); this packet adds the two global
writes and the publish, and reuses that rule rather than repeating it.

`00B0D7B0` is `__thiscall(obj, void* handle)`, `RET 4`: `obj+1C0h = handle`, then
`00B4EC90(obj+30h)` when that pointer is set. `DAT_00F8D39C` is the render-resources
object published by `00B0F076` (`docs/APP_INIT_RENDERER.md`). So the controlled unit's
driven object hands one handle to the renderer once per change, and the handle is
`vtable[18h]`, a slot the unit's primary vtable fills with `006D1E80` for a destroyer.

**The null path is a contract, not an error path.** Of the five call sites, three reach
the routine with `ECX` already zero:

| site | containing routine | argument |
| --- | --- | --- |
| `00645687` | `FUN_00645600` | the filtered candidate, only when it equals the current unit |
| `006456C0` | `FUN_00645600` | the filtered candidate |
| `007F3AF4` | `FUN_007F3A60` | `XOR ECX,ECX` at `007F3AF2` |
| `00959601` | `BSP_Unit_OnDestroyed` | `XOR ECX,ECX` at `009595FE`, a tail jump |
| `00644A38` | undefined region, nearest preceding function `006446E0` | not read |

`004C0893` is the only WRITE to `DAT_00E188D8` among the cross-references read; every
other reference in the image reads it. `004E4A80` inlines the whole body behind
`DAT_00E188DC == 0 && DAT_00E188D8 != 0`, which is the republish-if-lost path once a
frame (`docs/GAME_ON_MOVE_MAP.md` step 5).

## `00645600`, the sequence that matters

`__thiscall(hudRoot, Unit* candidate)`, `RET 4`, body `00645600..00645702`. `EDI` is the
HUD root object and `ESI` the candidate, taken from `[ESP+0Ch]` after the two pushes.

```
1. 00645603  if (g_controlled && g_controlled->IsKindOf(6)) {
   00645622      00817380(g_controlled, 0)          ; per-part broadcast, released value
   0064562D      0080E290(g_controlled)             ; per-node release
              }
2. 00645637  hudRoot+1Ch = 0
3. 0064564D  ok = 00645060(candidate, [game+18ECh], 1)
   00645654  if (!ok) candidate = null
   0064565F  else if (candidate->IsKindOf(1) && !candidate->vtable[124h]()) candidate = null
4. 0064567B  if (g_controlled == candidate) 004C0890(candidate)
5. 00645692  if (g_controlled) {
   00645699      006952A0(g_controlled, hudRoot+8h)         ; observer unregister
   006456B9      if (g_controlled->IsKindOf(5)) 00954990(g_controlled, 0)
              }
6. 006456C0  004C0890(candidate)
7. 006456C5  if (g_controlled) {
   006456D3      00694A60(g_controlled, hudRoot+8h)         ; observer register
   006456FA      if (g_controlled->IsKindOf(5)) 00954990(g_controlled, 1)   ; tail jump
              }
```

The global is re-read at `0064567B`, `0064568C`, `0064569E`, `006456B1`, `006456C5`,
`006456D8` and `006456F4`, so step 5 acts on the outgoing unit and step 7 on the incoming
one. Step 4 is written exactly as the bytes have it (`CMP ECX,ESI` / `JNZ 00645692` at
`00645683`): the setter is called when the controlled unit is **not** changing, which
republishes the listener handle. Nothing in the surrounding code explains why, so it is
recorded and not rationalised.

## The three unit-side callees

`00817380`, `void __thiscall(unit, bool released)`, `RET 4`, body `00817380..008173D4`.
Stores the flag at `unit+1008h`, then walks the `[unit+1000h]`-element pointer array at
`[unit+FFCh]` calling `00815370(element, released ? 0.0f : 1.0f)`. The float is built
with `XORPS` or a load of `00D7A24C` at `008173A6`/`008173AB`.

`00954990`, `void __thiscall(unit, bool released)`, `RET 4`, body `00954990..00954A0B`.
When the unit answers `IsKindOf(6)` and `[unit+BBCh]` is set, calls that object's
`vtable[20h](released ? 0.0f : 1.0f)`. `unit+BBCh` is already
`kUnitOffRpmEmitterA` in `bsp/unit_motion.hpp`, so the broadcast is an engine-sound
emitter parameter: the controlled hull gets `0.0f` and a released one `1.0f`. Then it
walks the child list at `[unit+428h]` (link `+4h`) calling `0072B540(child+8h, flag)`, and
finally stores the flag at `unit+6B4h`.

Both routines therefore mean "this unit is (not) the one the player is aboard", and both
invert the flag on the way to the float. Calling the emitter parameter an interior or
exterior mix is a guess; the call site and the value are not.

`0080E290`, `void __thiscall(unit)`, `RET 0`, body `0080E290..0080E2B6`. Walks the linked
list from `[unit+48h]` by `+44h`, calling `0085AD00` on each node answering
`IsKindOf(22h)`. Read for its shape only; `0085AD00` was not read, so the reconstruction
keeps it as one host method named by its address.

## `00645060`, the selectable test

`bool __fastcall(unit = ECX, teamIndex = EDX, bool allowSpectate = [ESP+4])`, `RET 4`,
body `00645060..00645159`. Complete; every branch is covered.

```
00645071: if (game && game+2194h == 0) allowSpectate = 0
00645084: team = [game + 18CCh + teamIndex*4]
          reject unless: unit != null
00645091:                unit+5Ch  != 0
0064509B:                unit+5Dh  == 0
006450A5:                unit+60h  == 0
006450AF:                unit+5Eh  == 0
006450B9:                [unit+54h] == [team+28h]
006450CA:                unit->IsKindOf(2)
006450D7:                unit->IsKindOf(0Fh)
006450E6:                !unit->IsKindOf(2Ah)
006450F5:                !unit->IsKindOf(46h)
00645104:                !unit->IsKindOf(45h)
00645110:                unit->vtable[124h]()
0064511E: if (00927C50(unit, teamIndex)) return true
0064512A: return allowSpectate && [team+19h] == 0 && [unit+188h] != 8
```

`00645071` reads the *global* game object and inverts the caller's flag when
`game+2194h` is clear, which is why the one call site passes a literal `1`. `00927C50`
and `vtable[124h]` were not read; both are host methods.

## Corrections

**To the packet brief.** It named `004C4300` (input contexts) and `00651760` (HUD unit
view binding) as part of what making a unit the controlled unit sets. Neither is reachable
from `004C0890` or from `00645600`. `004C4300` is `BSP_GameInputContexts_ApplyLevels`,
called only from `004D6410` and `004D8C00` in the front-end screen-set path, and
`00651760` is `BSP_HudUnitView_BindUnitAndReset`, called only from `0068ACA0`
`BSP_InGameInterface_ApplyPendingInterface`. `004C0890`'s single callee is `00B0D7B0`, and
`00645600`'s are `004C0890`, `00645060`, `00694A60`, `006952A0`, `0080E290`, `00817380`
and `00954990`. The controlled unit does not carry an input context with it; the input
context is chosen by the interface level.

**To `docs/GAME_ON_MOVE_MAP.md`, `00B0D7B0`.** Its table calls the value a device handle
pushed into a particle-segment object and proposes the name
`BSP_ParticleSystem_SetDeviceHandle`. The receiving object `DAT_00F8D39C` is the
render-resources singleton from `docs/APP_INIT_RENDERER.md`, and the value comes from the
controlled unit's `vtable[18h]`, so it changes whenever the player changes ship. That
doc's own correction note already says `DAT_00E188D8` is the player-controlled unit rather
than a device; the callee's name should follow. This packet does not rename it: the
particle work owns that address.

## Routines and coverage

| routine | state | coverage |
| --- | --- | --- |
| `004C0890` | reconstructed as a sequence over a host, build-tested | complete |
| `00645600` | reconstructed as a sequence over a host, build-tested | complete |
| `00645060` | reconstructed as a pure rule, build-tested | complete for the readable gates; the two virtual probes and `00927C50` are host methods |
| `00954990`, `00817380` | analysed; named | complete as read |
| `0080E290` | analysed | complete as read; `0085AD00` not read |
| `00B0D7B0` | analysed, read-only | complete as read; `00B4EC90` not read |
| `007F3A60`, `00959450` | read for their call sites only | partial: only the argument setup at `007F3AF2` and `009595DF..009595FE` |

## no_ghidra_function

none. Every routine named or reconstructed here has a Ghidra function. The `00644A38`
call site lies in an undefined region whose nearest preceding function is `006446E0`; it
was not read and nothing is claimed about it.

## Uncertainties

1. Why step 4 of `00645600` republishes when the controlled unit is unchanged.
2. What `vtable[18h]` returns, and therefore what the renderer does with it.
3. The meaning of trait ids `1`, `6`, `22h`, `2Ah`, `45h` and `46h`; only `5`, `0Fh` and
   `18h` have recorded readings, from `docs/UNIT_INSTANCE_UPDATE.md`.
4. Whether `00954990`'s emitter parameter is a mix, a gain or a filter selector.
5. `vtable[124h]`, `00927C50`, `0085AD00`, `00694A60` and `006952A0` were not read.

## Follow-up packets

* `controlled_unit_listener` - addresses `006D1E80` (the destroyer's `vtable[18h]`),
  `00B0D7B0`, `00B4EC90`; files `docs/CONTROLLED_UNIT_LISTENER.md`. What the handle is and
  what the renderer does when it changes.
* `unit_trait_ids` - addresses `006FE530` and every `IsKindOf` call site in the image;
  files `docs/UNIT_TRAIT_IDS.md`. A single table for an id space eight packets now guess
  at individually.
* `hud_root_unit_selection` - addresses `00647300`, `006485A0`, `00649860`, `00927C50`,
  `00645060` (read); files `docs/HUD_ROOT_UNIT_ROWS.md`. Who calls `00645600` and with
  what, which settles the spectator half of the selectable test.

## SetSelectedUnit and 00647300 (packet `cc9_set_selected_unit`, `kSetSelectedUnitBound`)

Worker cc9-movie-camera, 2026-09-27, base main 83ece7303. Ghidra was read only.

### The path (V)

- **008AB260 `SetSelectedUnit`.** It reads argument 0 as an entity (`BSP_ObjectHandle_FromLuaTable`)
  and calls 00647300 `BSP_InGameHudRoot_SetSpectatedUnit` on the HUD root.
- **00647300** (`__thiscall(hudRoot, unit)`, RET 4 at 00647352 and 0064735E):
  1. 00645060(unit, `[game+18ECh]`, 1) at 00647317. If it rejects, the routine returns.
  2. 00645600(unit) at 00647323.
  3. If `[00E188D8]` is null and the manager's applied pair equals its pending pair, it pushes
     34h `INTF_LIMBO` (0064734B).
  4. Otherwise it calls 00647040.
- **00647040** (00647040..0064707C):
  - 0059DA80 on `[manager+54h]`, which is a bare `RET`;
  - 005251C0 on `[manager+CCh]`, a screen reset that is unread here;
  - then 004CC460(20h, `[00E188D8]->vtable[140h]()`).
  - For a ship (vtable 00CFB738) slot 140h is 0047F320, `MOV EAX,ECX; RET`, so the payload is
    the unit itself.
- **00645060's two host probes, read for this packet:**
  - `vtable[124h]` for a ship is 006D1EF0 (no Ghidra function, 006D1EF0..006D1F10 inclusive,
    `RET` at 006D1F10). It answers `+5Ch` set and `+5Dh`, `+60h`, `+5Eh` clear, the same four
    bytes as 0043F080.
  - 00927C50(unit, team) is true when any of the nine words `unit+188h..` is 9 or equals
    `team`. That is the role permission table `SetRoleAvailable` writes.
- **In single player the spectator door is shut.** `game+2194h` is clear, so BL = 0 at
  0064507A.

### Corrections to the section "`00645060`, the selectable test" (V, listing)

- **006450DF is `JNZ` to the reject exit.** A unit answering `IsKindOf(0Fh)` (a plane) is
  rejected. The first reconstruction required it.
- **0064512E `CMP [EBP+19h],AL` with AL = 0, then `JZ` to the reject exit.** The spectator door
  needs `[team+19h]` **set**. The first reconstruction rejected when it was set.
- **0064507A.** BL is zeroed when `game+2194h` is **clear**, not when it is set.
- **00645600 step 4 compares identity** (`CMP ECX,ESI` at 0064567B..00645681). The host
  interface gained `controlled_is_candidate()`.

`src/controlled_unit.cpp` and `include/bsp/controlled_unit.hpp` carry the fixes. Nothing called
either routine before this packet.

### The binding

- **The switch.** `kSetSelectedUnitBound` in `include/bsp/game_hosts_hud.hpp`, committed OFF.
- **The Lua row.** `src/game_hosts_lua.cpp` routes row 008AB260 to
  `hud_set_selected_unit_00647300`. It resolves the argument through the entity `ID`, as the
  objective rows do.
- **The HUD side.** `GameHudHost::set_selected_unit_00647300` runs 00645060 with inputs from
  the units host, then `bsp::select_controlled_unit_00645600`. Its setter is the units host's
  004C0890. Then it runs 00647040's push, which re-arms the pending/applied pair so the next
  006840F0 service runs 0068ACA0's 20h classifier with the new controlled unit.
- **Substitutions (labelled in the code):**
  - The local player's side record `+28h` is party 0. game_hosts_ai.cpp makes the same
    assumption.
  - `vtable[124h]` is read for ships only. Any other class is the record
    `SetSelectedUnit::unit_vtable_124` and answers false.
  - `[team+19h]` and `game+2194h` are not modelled. Both only matter on the spectator door,
    which single player never opens.
- **Records left:** 005251C0 (screen reset), 00817380 and 0080E290 (the release broadcasts),
  006952A0 and 00694A60 (the observer pair), 00954990 (audio), and 0064734B (the 34h push).
- **`ForceSelectUnit` 008AAF30 does not share the path.** It calls 006485A0, which runs 00645600,
  00647040 and the unit-list rebuild 00648290 with no argument. It is not called within USN04's
  4,500 frames, since `luaEndZuikakuDeadMovie` never runs. It stays unbound.
- **What the host does on USN04 today.** The Lexington is already the controlled unit by
  another route: the mission frame's `set_controlled_unit_004c0890(0)`, the first created unit.
  The one `SetSelectedUnit` call is `luaWeHere` (`usn_19_coralus.lua` 3352,
  `SetSelectedUnit(Mission.Lex)`) at t = 25.0 s, and today it is an UNIMPLEMENTED record.

### Predictions (written before the pairs; the same tree, switch only, `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`)

**USN01 3200/3000:**
1. The native row `SetSelectedUnit` goes from UNIMPLEMENTED calls=2 to concrete calls=2.
2. **Call 1**, `luaIn` at about t = 20.1 s:
   - Northampton is accepted.
   - `00e188d8` moves from `Airfield2` to `Northampton`.
   - A second `controlled unit:` line appears.
   - The next pump applies 25h: `applied as 25h`, screens `29h 49h 44h 27h 4Dh 45h 46h 26h 2Eh 35h 50h`.
   - `mission camera: ShipCaptain mover bound to "Northampton"` appears.
3. **Call 2**, `luaPh2MovieEnd` at about t = 100 s: `SetSelectedUnit(Mission.ScoutBomba)`, the
   generated `ScoutDauntless`, is rejected with `kind0F=1`.
4. **The pick:**
   - The `unit pick: first camera basis` line appears once, at about hud update frame 400.
   - `UnitPickScreen::camera_basis` UNIMPLEMENTED falls from 6,160 to about 800 calls (2 per
     frame before the publish).
   - Pick casts stay about 5,999, because screen 29h is in both sets.
   - Pick land hits: from 0 to somewhere in [0, 2,600], expected small. The camera sits behind
     and above Northampton, looking along its bow at the initial pitch. Land is hit only when the
     10,000-unit ray reaches an island inside the bounds of `docs/SCENE_CONTENTS_HOSTS.md`
     section 8.
   - `UnitPickScreen::owner_140` appears with calls equal to the frames whose pick resolved to a
     unit. The band is [0, 2,600], expected nonzero, since the formation (SaltLakeCity, Dunlap)
     follows Northampton inside the ray.
5. **Screens.** The 25h screens start updating on USN01: ship screen 45h, weapon groups 2Eh
   (enter 005494C0), role 27h and warning 50h. The role screen takes role 0 on Northampton for
   the local player. Screen 27h's 0067BB50 now sees a kind-2 controlled unit, so the units host's
   `role_message_4b` count grows by at least 1.
6. **Gameplay.** It is identical unless one of two routes carries a change:
   - a resolved pick reaches the weapon-group fire path, which needs player input, so it is not
     expected;
   - the role-0 take makes Northampton player-held: the `unit+184h` readers 009F3DF3 and
     009F5E06 in the bot fire-target path.
   If rows move, the prediction is that only Northampton's own gunnery rows move first (its
   shots and hits from t >= 20.1 s), and everything after is RNG-coupled.

**USN04 4700/4500:**
1. The native row `SetSelectedUnit` goes from UNIMPLEMENTED calls=1 to concrete calls=1 at
   t = 25.0 s. The Lexington is accepted.
2. 00645600 finds the Lexington already controlled, so `refreshed_in_place=1`.
   `ControlledUnit::set_controlled_unit` goes from 1 to 3 calls (the republish and the setter).
3. The 20h push re-runs the 25h arm:
   - `MissionCamera::bind_ship_view` and `HudWeaponGroupScreen::bind` go from 1 to 2 calls;
   - there is a second `applied as 25h` line;
   - the ShipCaptain keeps its target (no retarget), so only its sway and focus reset.
4. **Gameplay identical.** The controlled unit does not change, and the weapon-group release
   finds no selected group.

### Pairs and verdict

**Setup.** The same tree at 0aac5988c, switch only. Binaries are `local\bin\ss_off` and
`local\bin\ss_on`, both runs had `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`, and every
log shows the harness override lines and its own module directory.

| row | USN01 OFF | USN01 ON | USN04 OFF | USN04 ON |
| --- | --- | --- | --- | --- |
| `pair_diff` exit | | 3 (controlled row only) | | 1 |
| deaths / hit records / damage / shots | 7 / 150 / 2690.0 / 583 | same | 43 / 788 / 11917.1 / 5075 | same |
| death rows, plane death modes, unit table | | identical | | identical |
| controlled unit | Airfield2 | Northampton | Lexington-class01 | same |
| `SetSelectedUnit` | UNIMPLEMENTED 2 | concrete, accepted 1 | UNIMPLEMENTED 1 | concrete, accepted 1 |
| `UnitPickScreen::camera_basis` UNIMPLEMENTED | 6,160 | 805 | 3 | 3 |
| pick casts / land hits | 5,999 / 0 | 5,999 / 0 | | |
| `owner_140` | 0 | 0 | | |
| `ControlledUnit::set_controlled_unit` | 1 | 2 | 1 | 3 |
| player role takes / releases | 0 / 0 | 4 / 4 | 3 / 0 | 5 / 2 |

**USN01 ON, from the log:**
- `SetSelectedUnit 00647300: "Northampton" accepted` appears at t = 20.1 s.
- It is followed by `applied as 25h` with the eleven screens, and by `ShipCaptain mover bound to
  "Northampton"`.
- `unit pick: first camera basis ... at hud update frame 403: from=(6300.0,37.8,-3377.3)
  forward=(0.0000,-0.1736,0.9848)`, a pitch of -10 degrees behind the bow.
- The second call, `"ScoutDauntless" rejected ... kind0F=1`, lands at about t = 100 s.
- `landscape attach traces` falls from 6,401 to 1,207. The zero-length ray at the origin sat
  inside a landscape's box, while the real rays miss every landscape box.

**Failed or unpredicted:**
- `owner_140` stayed 0 where I expected nonzero. The ray pitches 10 degrees down from about
  37.8 above the water, so it enters the sea about 214 units ahead. The formation follows behind
  Northampton, and nothing lies in front of it in that range.
- On USN04, `released_previous=1` was not predicted. The Lexington is kind 6, so step 1's release
  runs even when the unit is re-selected.
- On USN04 the role takes and releases rose by 2 each, from the weapon-group rebind.
- Every other prediction held.

**Verdict: ON.** Gameplay is identical on both missions, and every change is the binding's own
row. `kSetSelectedUnitBound` is set true in the verdict commit.

## ForceSelectUnit and 006485A0 (packet `cc9_force_select_unit`, `kForceSelectUnitBound`)

Worker cc9-hud2, 2026-09-27, base main 34f7f95cd. Ghidra was read only.

### The path (V, listings)

- **008AAF30 `ForceSelectUnit`** takes no argument. It loads `[00E198C4]+40h`, the HUD root
  (screen 44h), and calls 006485A0 at 008AB01F. It returns its empty call frame's result count, 0.
- **006485A0** (`__fastcall(root)`, 006485A0..00648644):
  1. It clears `+C0h` (006485A4), then runs 00648290 and 00645710.
  2. **Empty `+8Ch` vector:** it calls 00645600(null). Then:
     - with `[mgr+4] == 34h` in multiplayer, it pushes 2Ah when `[mgr+20h]` is also 34h and
       `[mgr+1Ch] == [mgr+38h]`;
     - otherwise, with `[mgr+4] == [mgr+20h]` and `[mgr+1Ch] == [mgr+38h]`, it pushes 34h.
  3. **Otherwise** it calls 00645600(00644A60()), the unit at the cursor. With a controlled unit
     it tail-jumps to 00647040, the same 20h push as `SetSelectedUnit`'s tail.
- **00648290, the rebuild** (`__fastcall(root)`, 00648290..00648598). Screen 44h's enter 006488D0
  also calls it, at 00648913 (vtable 00CF5A9C `+18h` = 00CF5AB4 holds 006488D0).
  1. **The re-find flag.** It is set when a controlled unit exists and its list kind has changed.
     That is either cursor `+C2h` set and the unit not a formation follower (007788B0), or `+C2h`
     clear, the unit a follower and the *old* `+9Ch` vector longer than one.
  2. **Both vectors are cleared** (00645CA0).
  3. **`+8Ch`** comes from the walk of `[game+1974h]`, the head of game+1970h. That is 004C3CB0's
     walk 0 without ordnance (`docs/LOCAL_PLAYER_UNIT_LISTS.md`). A unit is appended when
     00645060(unit, `[game+18ECh]`, 1) accepts it and either it is no follower, or its leader
     (007788D0) exists and 00645060 **rejects** the leader.
  4. **`+9Ch`** is filled only with a controlled unit whose leader (007788D0) exists:
     - first the leader, when `[leader+188h]` is 9 or `[game+18ECh]`;
     - then each group member (0070D060, `[[leader+284h]+4F8h]` slots) that is not null, not the
       leader, and has the same role word.
  5. **With the flag set**, the entry at the cursor in the new vectors is read. When it is the
     controlled unit, the cursor becomes 00644CC0(controlled). If that has index -1, the cursor
     becomes 00644C20(controlled).
  6. It then runs 00645710, and tail-jumps to 006485A0 when `+C0h` is set.
- **00645710, the settle** (00645710..0064582B). Indices are compared unsigned after sign
  extension, so -1 is out of range.
  1. With `+C4h == -1` and a non-empty `+8Ch`, the cursor becomes (0, 0).
  2. An index out of range of its own vector becomes (0, 0).
  3. Then the cursor unit is read (00644A60):
     - null gives (0, -1);
     - a unit that 00645060 accepts stays;
     - a rejected unit becomes (0, 0) when `+8Ch` holds more than one unit, else (0, -1).
- **00644A60** returns 0 for index -1. Otherwise it returns entry `+C4h` of `+9Ch` when `+C2h` is
  set, else of `+8Ch`, or 0 when out of range.
- **Nothing else moves the cursor on an idle run.**
  - 00644DB0, the poll, writes `+C2h`/`+C4h`/`+C0h` only on actions 8Ch..8Fh.
  - The update's 0064A114..0064A153 re-find follows a pending `MW_MultiSelectUnit` handle, which
    stays 0 here.
  - The root's register 00645F10 clears `+C0h` only. No constructor store of `+C2h`/`+C4h` was
    found. The settle maps either a zero or a -1 start to (0, 0) on the first non-empty rebuild,
    so the host starts at (0, -1).

### The binding

- **Switch:** `kForceSelectUnitBound` in `include/bsp/game_hosts_hud.hpp`, committed OFF.
- **The reconstruction** is in `src/hud_root_rows.cpp`:
  - `hud_root_rebuild_unit_lists_00648290`, `hud_root_settle_cursor_00645710`,
    `hud_root_cursor_unit_00644a60` and `hud_root_force_select_unit_006485a0`;
  - the existing `hud_root_find_unit`/`hud_root_find_primary_unit` (00644CC0/00644C20, packet
    orch2_hud_root_unit_rows).
- **Routes:**
  - the Lua row 008AAF30 goes to `GameHudHost::force_select_unit_006485a0`;
  - the menu host's 44h enter goes to `enter_hud_root_screen_006488d0`, which runs only its
    00648290 call.
- **Sources:**
  - game+1970h is the world host's last 004C3CB0 build (`game_local_player_unit_lists`, walk 0
    rest);
  - 00645060 is `selectable_inputs_00645060`, as `SetSelectedUnit` uses it;
  - the group queries are the units host's `unit_formation_group_0284` family;
  - 00645600 goes through the same `SelectControlledUnitBinding` as `SetSelectedUnit`.
- **SUBSTITUTIONS, labelled:**
  - `[game+18ECh]` is the local player index 0, as `SetSelectedUnit` takes it.
  - The 44h enter's other steps stay records: its three widget `+4Ch(0.0f)` calls, `+24h` and
    `+F4h = 0`.
  - The empty-vector arm is a record: 00645600(null) does not clear the units host's controlled
    unit, and the manager fields are not read, so neither push is made.
  - The initial cursor is (0, -1) (see above). Settled later: the root constructor stores it at
    0068BFDC..0068BFE2 ("SetSelectedUnit's records" below), so it is the image's value.

### Reach

No idle run of USN01 3200/3000, USN02 or USN04 4700/4500 calls `ForceSelectUnit`.
- USN04 (`usn_19_coralus.lua`) calls it from `luaMoveToPh2` (line 2195, then `GetSelectedUnit`)
  and `luaEndShohoMovie` (line 2225, after `SetSelectedUnit(Mission.SelUnit)`).
- It also calls it from the Zuikaku movies (lines 998, 1032, 1967), which need later phases.
- Phase 2 starts when the fourth bomber wave has spawned or the Lexington's bombers are all dead
  (lines 560..587).
- An idle USN04 9200/9000 run on base 2026-09-19 (`gh_on_usn04_long.log`, worker
  cc8-gunnery-host) ran the `luaMoveToPh2` callback and logged `ForceSelectUnit` UNIMPLEMENTED
  calls=2.
- The other callers in this installation are training grounds, `ijn/ESMP` and `usn/LOMP`
  missions, which the harness has not run.

### Predictions (written before any run; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player)

**Regression pairs, USN01 3200/3000 and USN04 4700/4500: gameplay identical, `pair_diff` exit 1.**
- The only changes are the 44h enters.
  - `FrontEndScreen::enter` falls by one per 44h enter. Each OFF log has two 44h enters: on
    USN04 at the first 25h and at `SetSelectedUnit` at 25 s, and on USN01 at its two 25h applies.
  - Each enter logs one `hud root unit lists 00648290` line and one concrete
    `HudRootScreen::rebuild_unit_lists_00648290`.
- Nothing reads the vectors, so every other row is identical.
- The summary line shows `calls=0 selects=0`.

**Measuring pair, USN04 9200/9000 (the handoff budget):**
- **OFF:** `ForceSelectUnit` UNIMPLEMENTED calls=2, if phase 2 still starts on this base. If it does
  not, the pair measures nothing and is reported as such.
- **ON:** `calls=2`.
  - At each call the cursor is (0, 0), and entry 0 of `+8Ch` is the Lexington. It is the first
    selectable own unit, because game+1970h follows the recon triple's unit order, in which the
    first created unit comes first.
  - 00645600 refreshes the Lexington in place (it is already controlled since
    `SetSelectedUnit` at 25 s). 00647040 then re-pushes 20h, so `selects=2`.
  - The controlled unit does not change. `Mission.SelUnit` is the Lexington on both sides.
- **Gameplay identical, exit 1.** The re-pushed 20h re-applies 25h with the same unit. 0064DA40
  finds 46h's `+20h` set and keeps the ShipCaptain, so the camera and the player seat do not
  move.
- **The risk,** named in advance: if the re-apply rebuilds the ShipCaptain, USN04 moves through
  the seat, as in the 25 s case of `docs/HUD_PICK_SEGMENT_QUERY.md` 8.7.

### Pairs and verdict

The OFF side is this tree's build at 39e587a19. The ON side is the `tools/pair_export.py`
export `local\fs_on` of the same commit with the switch flipped. Streams are ON, with
`BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000 | exit 1 | gameplay, death rows and unit table identical. Two 44h enter rebuilds (frames 1 and 403); `FrontEndScreen::enter` 16 -> 14. At frame 403: `+8Ch` 0 units, `+9Ch` 3 units (Northampton's group), cursor (0, -1). Also `SetSelectedUnit::unit_vtable_124` 1 -> 39 (see below) |
| USN04 4700/4500 | exit 1 | gameplay, death rows and unit table identical. Two 44h enter rebuilds (frames 1 and 502), each with `+8Ch` = {Lexington}, `+9Ch` = {Lexington} and cursor (0, 0) at the Lexington; `FrontEndScreen::enter` 22 -> 20 |
| USN04 9200/9000, OFF only | no pair | `ForceSelectUnit` was **not called**: phase 2 never started (below) |

**Predictions.**
- Identity held on both regression pairs, and so did the 44h enter counts.
- The Lexington at cursor (0, 0) on USN04 held for the enters.
- **Failed detail:** "every other row identical". The rebuild's 00645060 calls reuse
  `selectable_inputs_00645060`. That function records `SetSelectedUnit::unit_vtable_124` for
  every non-ship it tests, so that record grew from 1 to 39 on USN01. It is a record count only.
- **Unpredicted:** USN01's `+8Ch` is empty.

**Why USN01's `+8Ch` is empty.** This finding is upstream of this packet.
- The host's game+1970h on USN01 is the **Japanese** side for the whole run: Katori, the convoy,
  the Mavis flying boats, CB2, the coastal guns and Airfield2, 26 units that are all not party 0.
- The world host builds the lists from the recon triple of the side of the controlled unit.
  That is a labelled substitution for the local slot `[game+18CCh + game+18ECh*4]`, in
  `include/bsp/game_hosts_world.hpp`.
- On USN01 the list was built once, while the host's first controlled unit was Airfield2, and it
  is never rebuilt after `SetSelectedUnit(Northampton)`.
- So 00645060 rejects every entry (`party0=0`). The image walks the local player's own units and
  would hold Northampton there.
- The same list feeds the 29h pick (`UnitPickScreen::list_1970`).
- This binding reads that list as it is and does not correct it.

**Why the measuring pair measured nothing.**
- On this base `Blackout(true, "luaMoveToPh2", 3)` is armed 75 times from t = 225 s. Each arm
  resets the remaining time to 3.0 s, and the callback never runs.
- `summary mission blackout`: `arms=81 completions=5 callbacks=2`. On base 2026-09-19 it was
  `arms=35 completions=9 callbacks=4`, with `blackout callback luaMoveToPh2 ran`.
- `Objectives_Completed` 008BD340 is UNIMPLEMENTED and is called once. The re-arms therefore
  come from later passes of the phase-1 check (`usn_19_coralus.lua` 560..587).
- The cause of the difference from the 09-19 base was not traced in this packet.
- No other mission within the budget that the harness has run calls `ForceSelectUnit`.

**Verdict: ON** (`kForceSelectUnitBound = true`).
- Both regression pairs are gameplay-identical, and the binding is the image's path over the
  host's inputs.
- `ForceSelectUnit` itself is **not measured**: no measured run reaches it.
- On USN04 its `+8Ch` would hold only the Lexington, so a call would re-select the controlled unit
  and re-push 20h.
- On USN01 it would take the empty-vector arm (a record) until the game+1970h side is corrected.

## The initial controlled unit (packet `cc9_initial_controlled_unit`, `kInitialControlledUnitBound`)

Worker cc9-hud2, 2026-09-27, base 718162a82 (the list rule and `ForceSelectUnit` ON). Ghidra was
read only.

### The image (V, listings)

- **The candidate named in the packet brief does not run in single player.** 004C9CA0's
  mission-entry arm tests `CMP [ESI+1FE4h],EBX` / `JZ 004C9EA2` at 004C9E32..004C9E38. It
  therefore skips 00A933F0, 008053C0, 008073C0, the latch clear, 004C3CB0 and 006485A0 unless
  game+1FE4h is set, which is a network session. The host's comment in `apply_in_game_interface_004c9ca0`
  already says so.
- **The scene load's step 19 does run in single player.** It is 004DFB70 at 004E0565..004E05B7
  (`docs/MISSION_SCENE_LOAD.md` step 19), gated only on the local slot being in [0, 8):
  1. 008053C0 into the local slot's `+30h` (004E0583);
  2. 008073C0 on it (004E059B);
  3. `game+193Ch = 0` (004E05A2);
  4. 004C3CB0 (004E05A9);
  5. then 006485A0 on `[00E198C4]+40h` (004E05AE..004E05B7).
- So the image's first controlled unit is the HUD root's `+8Ch` at its cursor, the first
  selectable unit of the local player's game+1970h. It is not the first created unit.

### The binding

- **Switch:** `kInitialControlledUnitBound` in `include/bsp/game_hosts_world.hpp`, committed OFF.
  It needs `kLocalPlayerUnitListRuleBound` and `kForceSelectUnitBound`, which are both ON.
- **ON behaviour:**
  - The mission frame's `set_controlled_unit_004c0890(0)` (the first created unit) is not made.
  - It arms `GameWorldHost::request_scene_load_force_select_004e05b7`.
  - The world host then calls `hud_force_select_unit_006485a0` after the first 004C3CB0 build
    over a published own triple.
  - With no HUD attached yet, the request stays armed for the next build.
- **SUBSTITUTION, labelled:** the image runs step 19 at the end of scene load, after its own
  008073C0 on the local slot. Here it runs at the first in-mission list build, when the gunnery
  host has published the own triple. On the OFF logs that is mission clock 0.05, the first tick.
  Until then no unit is controlled.

### Predictions (written before any run; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player)

The initial unit is taken from `cc9_local_player_unit_list`'s ON runs, whose first 44h-enter
rebuild at frame 1 shows `+8Ch`. That vector does not depend on the controlled unit, except
through followers of the controlled unit's group.

**USN01 3200/3000:**
- **The initial controlled unit is Northampton, instead of Airfield2** (a Japanese airfield,
  wrongly the first created unit). `+8Ch` = {Northampton} and the cursor is (0, 0).
- **`SetSelectedUnit(Northampton)` at 20.1 s still lands.** 00645060 accepts, and 00645600
  refreshes Northampton in place.
- **Gameplay moves, exit 3.** From t = 0:
  - The 20h push gives 25h on Northampton, so the ShipCaptain and the player gun seat run on
    Northampton from the first frames instead of from 20.1 s. The intro movie still replaces the
    ShipCaptain at 4 s.
  - Northampton's ship AI and seat behaviour as the player's unit start 20 s early, so its path
    and its gunnery in 0..20 s move.
  - The later combat follows from that. No band is predicted for the size of the move.

**USN04 4700/4500:**
- The initial controlled unit is the Lexington, the same as the first created unit. `+8Ch` =
  {Lexington}.
- `SetSelectedUnit(Lex)` at 25 s refreshes it in place, as today.
- **Gameplay identical, exit 1,** apart from the rows of the moved call. The controlled unit is
  bound at the first tick instead of at load, and one more 20h push is made at the first tick.
- **The risk,** named in advance: if that one-tick gap or the extra 20h shifts the first 25h
  apply or the ShipCaptain's seed, USN04 moves through the seat.

**USN02 9200/9000:**
- **The initial controlled unit is Alden, instead of DeRuyter.** `+8Ch` holds 10 units and the
  cursor is (0, 0) at Alden.
- **`SetSelectedUnit(Houston)` at about 25 s still lands.** It moves control from Alden to
  Houston.
- **Gameplay moves, exit 3.** Alden, not DeRuyter, is the player's ship for the first 25 s, so
  the seat, the ShipCaptain and the ship AI exemption move from DeRuyter to Alden in that window.

### Pairs and verdict

- **A first ON export (71457582b) was invalid.** The host's call ran inside the first 004C3CB0
  build, before `game_local_player_unit_lists` published it. It therefore saw an empty `+8Ch`,
  and no unit was controlled until `SetSelectedUnit` (USN01: gameplay identical).
- a3a03908f moves the call after the publication. The pairs below are the same tree at
  a3a03908f, switch only, with streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000 | exit 1 | 006485A0 at frame 0: `+8Ch` = {Northampton}, cursor (0, 0), control "none" -> Northampton. `SetSelectedUnit(Northampton)` at 20.1 s refreshes it in place. **Gameplay, 7 death rows and 28 unit rows identical**; seat messages 2,801 -> 2,961, role takes 4 -> 5 |
| USN04 4700/4500 | **exit 3** | 006485A0 at frame 0 controls the Lexington, as predicted. **Gameplay moved:** deaths 44 -> 43, hit records 789 -> 779, damage 11985.6 -> 11851.8, shots 6321 -> 6528, dive-bomb releases 4 -> 8 of 19; 9 death rows and 18 unit rows changed |
| USN02 9200/9000 | exit 3 | 006485A0 at frame 0 controls **Alden**, as predicted. `SetSelectedUnit(Houston)` moves it to Houston. Gameplay moved: hit records 652 -> 645, damage 49661.5 -> 51163.0, shots 1095 -> 1169; deaths 21 = 21, with 2 rows swapped; mission failed at 39.65 s on both sides |

**Predictions.**
- **Held:** the initial unit on all three missions (Northampton, Lexington, Alden), the later
  script selections landing, and USN02's move.
- **Failed, USN01:** gameplay did not move. Northampton is controlled for 4 s before the intro
  movie takes the camera, and nothing it does in that window reaches the combat rows.
- **Failed, USN04:** the named risk happened, through a path other than the one named.
  - The first difference is at t = 0.05. The ON run logs `formation slot swap: group=0 swaps=1`,
    and Fletcher-class07 and Fletcher-class08 follow on different targets from ship-AI step 10.
  - With the switch ON no unit is controlled during the first tick, until the world host's list
    build runs 006485A0. The first tick's formation assignment therefore sees no player ship.
    With the switch OFF the Lexington is controlled from load.
  - In the image 006485A0 runs at scene load, before any tick, so the move comes from this
    binding's timing substitution, not from the image's rule.

**Verdict: not flipped** (`kInitialControlledUnitBound` stays false).
- The rule is read (V), and the host's first unit is wrong on USN01 (Airfield2) and USN02
  (DeRuyter).
- But the ON path controls nothing during the first tick, which the image never does. That
  one-tick gap moves USN04.
- **What would fix it:** run step 19 at the load site itself. That needs the local slot's own
  triple before the first tick: the gunnery host's 008073C0 pass (or its publication) run once at
  load, as 004E059B does, then the world host's build and 006485A0, with the HUD attached. That
  touches `src/game_hosts_gunnery.cpp` and the mission frame's load order, so it is a follow-up
  packet.

### Step 19 at the load site (packet `cc9_initial_controlled_unit_load`)

Worker cc9-hud2, 2026-09-27, base ad46022e6. The switch is still `kInitialControlledUnitBound`,
committed OFF.

**The change.**
- **The mission frame's load block runs step 19 itself** when the switch is ON. It runs after
  `attach_world_2k` and before the HUD's scene interface request, so before the first tick:
  1. it arms the request;
  2. it runs `GameGunneryHost::run_recon_pass_at_scene_load_004e059b`: 008073C0 as 004E059B calls
     it, outside 008079B0's countdown, which is left untouched;
  3. it runs the world host's 004C3CB0 build (004E05A9), which then runs 006485A0 (004E05B7)
     once it has published the lists.
- If that pass or build publishes no own triple, the request stays armed for the first tick's
  build, as in the earlier binding.
- **The periodic pass.** `step_recon_sensor_pass_008073c0` is split into its countdown and
  `run_recon_sensor_pass_body_008073c0`. The OFF path runs the same code in the same order.
- **SUBSTITUTION, labelled:** this host's pass covers every side at once, where 004E059B rebuilds
  only the local slot. The first-tick substitution of the earlier binding is gone whenever the
  load-time build publishes.

**Predictions** (written before any run; the same protocol; the rows come from the measured pairs
at a3a03908f):

- **USN01 3200/3000:**
  - Northampton is controlled from load (the log line precedes hud update frame 0), and
    `SetSelectedUnit(Northampton)` at 20.1 s refreshes it in place.
  - **Gameplay identical, exit 1.**
  - Seat messages about 2,961 (OFF 2,801), as measured.
  - Pick `owner_140` unchanged from OFF. Nothing in the 0..4 s window before the movie casts
    differently: the pick's ShipCaptain camera exists from load either way.
- **USN04 4700/4500:**
  - The Lexington is controlled from load, the same unit as OFF's first created unit, because
    `+8Ch` = {Lexington} is the only selectable party-0 unit.
  - The first tick's formation assignment now sees the player ship, so the t = 0.05 swap of the
    earlier ON run does not occur.
  - **Gameplay identical, exit 1.** The added rows are the load-time recon pass (sensor-pass
    counts +1), one more list build, and one `ForceSelectUnit` with its 20h push. Seat messages
    equal OFF's.
  - **The risk,** named in advance: if a recon value carries from one pass to the next, the
    load-time pass moves the recon rows and, through them, the AI.
- **USN02 9200/9000:**
  - Alden is controlled from load, then Houston from the script's `SetSelectedUnit`.
  - **Gameplay moves, exit 3,** in the measured direction: hit records 652 -> about 645, shots
    1095 -> about 1169. The exact rows may differ from a3a03908f's, because the first tick now
    sees Alden controlled.
  - Judged on the per-entity tables and the player-seat rows.

**Pairs and verdict (load-time fix).** The same tree at e8ed6b35c, switch only, with streams ON,
`BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player. Export `local\ic_on`.

| pair | `pair_diff` | initial controlled unit (006485A0 at load, before hud update frame 0) | result |
| --- | --- | --- | --- |
| USN01 3200/3000 | exit 1 | **Dunlap** (`+8Ch` 7 units); `SetSelectedUnit` moves control to Northampton at 20.1 s | gameplay, 7 death rows and 28 unit rows identical; seat messages 2,801 -> 2,961 |
| USN04 4700/4500 | exit 1 | **Fletcher-class01** (`+8Ch` 18 units); `SetSelectedUnit(Lex)` at 25 s | gameplay, 44 death rows and 81 unit rows identical; seat messages and casts 8,157 -> 7,997 |
| USN02 9200/9000 | exit 1 | **Kortenaer** (`+8Ch` 14 units); `SetSelectedUnit(Houston)` | gameplay, 21 death rows and 32 unit rows identical |

**Predictions.**
- **Held:** gameplay identity on USN01 and USN04, USN01's seat messages (2,961), the script
  selections landing, and USN04 no longer moving. The t = 0.05 swap is gone, because a unit is
  controlled before the first tick.
- **Failed:** every initial unit name, and USN02's move.
  - At load the scripts have not yet run `SetRoleAvailable`, so every own ship's role word is
    open. The frame-1 44h rebuild is also no guide, because it ran after the scripts. `+8Ch` then
    holds every selectable own unit that leads no selectable leader, and the cursor takes the
    first of the local party's own triple.
  - That order is the gunnery host's publication order, "scanned classes in bucket order". It is
    the image's order; see "The own triple's order" below (packet `cc9_own_triple_order`), which
    supersedes the doubt this line first recorded.
  - USN02 does not move: Kortenaer, controlled until Houston, changes no combat row. The a3a03908f
    move came from Alden being controlled from the first tick after an uncontrolled load window.
    That window is gone.

**Verdict: ON** (`kInitialControlledUnitBound = true`).
- The image's step 19 now runs at the load site before the first tick, and gameplay is identical
  on all three pairs.
- The first unit's identity rests on the own-triple order and on role words that are open at
  load. Both are read (V) in "The own triple's order" below.

### The own triple's order (packet `cc9_own_triple_order`, read only)

Worker cc9-hud2, 2026-09-27, base c2f07e359. This settles which unit the image controls at load.

**How 008073C0 orders triple 0** (`docs/RECON_SLOT_LISTS.md` section 3, V):
- **The buckets.** 00807529..00807545 walks the 61h own buckets from slot `+34h` upward, stride
  `0Ch`, ECX = triple 0 at `+DD8h`, and appends each (00804E10). So triple 0 is ordered by
  **class id, ascending**, then the squadron and convoy aggregates `A[18h]`/`A[1Ah]` at step 11.
- **Inside a bucket.** The records follow the scan of that class's world registry list,
  `[game+19CCh] + 18h + id*0Ch`. 008065B0 appends a new record and splices a carried one to the
  tail, and step 3 empties A before every scan.
- **The registry list order.** It is insertion order: each unit's `vtable[130h]` registration
  (`GameEntity::register_in_parent_entity_list`, 00928560) pushes it at creation. The scene read
  creates the load-time units in scene-file order.
- **No sort** appears anywhere in the rebuild.

**What 006485A0 takes.** The cursor at load is (0, -1). The settle 00645710 turns it into (0, 0)
on a non-empty `+8Ch`, so the unit is **entry 0 of `+8Ch`**: the first unit of game+1970h (walk 0
of triple 0, without ordnance) that 00645060 accepts and that is not a follower of a selectable
leader.

**The inputs at step 19 in the image:**
- **Role words.** `unit+188h + role*4` starts at 9 for every role, from 00928630's
  `vtable[148h](1FFh, 9)` (`docs/SCRIPTED_HELM.md`). The mission script's `luaStageInit`, where
  `SetRoleAvailable` runs, is scene-load step 25 (`0045F440`), **after** step 19. So every own
  ship is selectable at step 19.
- **Liveness.** `+5Ch` is set by `BSP_SEntity_InitAll` (00925F20 -> 00922F30). The scene read runs
  InitAll inside step 11/13 (0046EB4B..0046ED0F), before step 19.

**The host against the image.**
- The host's publication (`publish_recon_triples_008073c0`) walks the class ids ascending and,
  inside each, `world_list_entry`, the host's per-class list.
- `register_in_world_lists` fills that list by the reconstructed 00928560 push-back at creation,
  and `create_units` runs over the scene read's entities in file order.
- The host order is therefore the image's rule over the same creation order.
- The load-time selections, Dunlap (USN01), Fletcher-class01 (USN04) and Kortenaer (USN02), are
  the image's too.
- Each is the first unit of the lowest own class bucket present (USN01's class 7 destroyers first,
  then class 9 Enterprise, then class Ah Northampton).

**Remaining assumption, labelled.** The equality of creation order rests on
`scene_contents->entities()` being the scene read's instantiation order. That is the host's
existing contract, not re-read here.

**Outcome:** no binding. The doubt recorded in "Pairs and verdict (load-time fix)" is withdrawn.

## SetSelectedUnit's records (packet `cc9_set_selected_unit_records`, read)

Worker cc9-hud2, 2026-09-27, base 03a43858b. Ghidra was read only.

The path is 008AB260 -> 00647300 -> 00645600 -> 00647040. Each record the host still leaves on it
was read and given one outcome: bind, display-only, subsumed, unreachable, or blocked. **No
binding lands in this packet**, so there are no pairs.

| record | what the image does (V) | outcome |
| --- | --- | --- |
| 00817380 `SetControlledUnit::release_unit_parts` (00645622) | stores the flag at unit+1008h, then `BSP_EffectGroup_SetScalar` 00815370 on each effect group of `[unit+FFCh]`. unit+1008h's one reader, 0081C8CF (body 0081C830..0081C938, no Ghidra function), builds the same effect scalar | **display-only** (effects) |
| 00954990 `SetControlledUnit::controlled_audio` (006456B9, tail 006456FA) | the engine emitter `[unit+BBCh]->vtable[20h]`, 0072B540 on each child (a gun's effect mix), and the flag at unit+6B4h. Its readers 0072F98E/0072FAEF in `BSP_Gun_SpawnShotAndEffects` pick an effect scalar | **display/audio-only** |
| 0080E290 `SetControlledUnit::release_unit_nodes` (0064562D) | each child on unit+48h answering `IsKindOf(22h)` gets 0085AD00 `BSP_TurningGun_AimToRestAngles`, target = RestAngles unless `+94h` is FLT_MAX | **gameplay state, subsumed.** The gunnery host's gun bot aims every targetless gun at `rest_horz`/`rest_vert` each tick (`src/game_hosts_gunnery.cpp`, `want_horz = gun.rest_horz`), so a binding would move a released gun's target at most one tick early. **Difference noted for the gunnery owner:** the image skips a gun whose RestAngles were not authored (FLT_MAX at 00D7A278), while the host's `rest_horz` defaults to 0.0 |
| 00645637 root `+1Ch` = 0; 006952A0 / 00694A60, the root observer at `+8h` (vtable 00CF5A84, stored by the constructor at 0068BF99) | the observer's slot `+4h` is **00644A20** (body 00644A20..00644A50, no Ghidra function). On the destruction notice of the controlled unit it sets `+14h` (root+1Ch) to 0, calls **004C0890(null)**, the controlled unit is released, and sets the cursor `+BAh`/`+BCh` (root+C2h/+C4h) to (0, -1) | **gameplay state, blocked.** A binding needs a units-host way to clear the controlled unit, and `include/bsp/game_hosts_units.hpp`/`src/game_hosts_units.cpp` are leased to cc9-ships2 (cc9_bsm01_think_natives) |
| 005251C0 `SetSelectedUnit::screen_reset_005251c0` (0064A0DE, and 00647040's) | on `[manager+CCh]`, the 29h pick screen: `+C0h` = 0, clears the vector `+C4h`, `+D4h` = -1, and in modes 4..6 sets input context levels 4 and 0Eh | **display/input-only** for an idle player: the pick's lock branches are input-driven records in this host |
| 0064734B `SetSelectedUnit::push_limbo_34h` | 34h when no unit is controlled after 00645600 | **unreachable:** 00645600 binds an accepted unit |
| `SetSelectedUnit::unit_vtable_124` (00645110), non-ship classes | 006D1EF0, the four-byte liveness test, is the `+124h` slot of **23** vtables, among them the plane unit's (00D05F20) and the ship's (00CFB738). It is the base implementation, not ship-only | **open.** The host answers false for every non-ship, which is wrong wherever the class shares 006D1EF0. It needs a class-to-instance-vtable map the host lacks. On the measured missions it can only add forts and airfields behind the ships in `+8Ch` (higher class ids), so the initial unit would not change |

**Predictions for the blocked observer binding**, written now for whoever binds it:
- USN01 and USN04: identity. Northampton and the Lexington are not destroyed within 3,000 and
  4,500 frames.
- USN02 9200/9000: Houston, controlled, dies at 74.55 s. From its destruction notice the
  controlled unit is null and the cursor is (0, -1). The player seat, the ShipCaptain's unit and
  the pick then lose their unit.
- Whether gameplay moves depends on what the seat and camera do with no unit. The mission has
  already failed at 39.65 s.

**Also settled:** the root constructor stores `+C2h` = 0 and `+C4h` = 0FFFFh at
0068BFDC..0068BFE2. So the initial cursor (0, -1) that `ForceSelectUnit`'s binding took as a
substitution is the image's value.

## The controlled unit's destruction notice (packet `cc9_controlled_unit_observer`, `kControlledUnitObserverBound`)

Worker cc9-hud2, 2026-09-27, base 02b0c988f + 06f268757 (corrected below). It binds the record of "SetSelectedUnit's records"
that was blocked on the units-host lease.

### The binding

- **Switch:** `kControlledUnitObserverBound` in `include/bsp/game_hosts_hud.hpp`, committed OFF.
- **Registration.** 00645600's 006952A0 (00645699) and 00694A60 (006456D3) now move the HUD
  root's observer: the host's `observed_unit` follows the controlled unit that
  `SetSelectedUnit`, `ForceSelectUnit` and the load's step 19 bind.
- **The notice.** When the observed unit is destroyed, 00644A20 runs:
  - it does nothing unless the unit is still `[00E188D8]` (00644A2D);
  - root+1Ch = 0 (00644A31; a record, since the host has no field);
  - `GameUnitsHost::clear_controlled_unit_004c0890` (00644A38, new: 004C0890(null) stores the
    empty global and publishes no listener);
  - the cursor becomes (0, -1) (00644A3D/00644A44).
- **SUBSTITUTION, labelled:** the image delivers the notice in the on-killed dispatch
  (009274DE..00927510 -> 00696330 -> 00693550 -> `observer->vtable[4h]`). Here the units host's
  destroyed-unit record (a sunk gunnery row) stands for it, checked once per interface update
  before 0068C1F0.

### Predictions (written before any run; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player)

- **USN01 3200/3000 and USN04 4700/4500: identity, exit 1.**
  - Neither controlled unit is destroyed: Northampton, and before 25 s Fletcher-class01, then the
    Lexington.
  - The summary shows `notices=0 releases=0`. Only the observer rows change: 006952A0 and 00694A60
    become concrete.
- **USN02 9200/9000:**
  - Houston, controlled from `SetSelectedUnit`, dies at 74.55 s. The next interface update
    releases control: one `controlled unit: 00e188d8 = null` line and `notices=1 releases=1`.
  - After that:
    - the player gun seat has no unit, so its messages and casts stop from 74.55 s;
    - the pick keeps casting from the ShipCaptain's camera, because the camera's unit is not
      cleared by this binding;
    - every host reader of `controlled_bound()` answers false.
  - **Gameplay identical, exit 1,** because Houston is already dead and the mission failed at
    39.65 s.
  - **The risk,** named in advance: if a host AI path treats the controlled unit specially, for
    example the player ship's exemption from ship AI or formation leadership, the escorts' AI
    moves after 74.55 s.

### Pairs and verdict

**Base correction.** This packet's tree was 02b0c988f plus 06f268757, not b00fb0f15. `bsp.py sync`
refused while 06f268757 was unlanded. The pairs are the same tree at 2abbbef25, switch only, with
streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000 | exit 1 | identical; `notices=0` |
| USN04 4700/4500 | exit 1 | identical; `notices=0` |
| USN02 9200/9000 | **exit 3** | Houston destroyed; the notice fires at hud update frame 1490 (about 74.5 s); `controlled unit: 00e188d8 = null (was "Houston")`; `notices=1 releases=1`. **Gameplay moved:** deaths 21 -> 22 (John2 only ON), hit records 652 -> 650, damage 49661.5 -> 50259.2, shots 1095 -> 1089; 8 death rows and 11 unit rows changed (Alden dies at 336.69 s instead of 315.70 s, killed by Haguro instead of John1) |

**Predictions.** USN01, USN04 and the USN02 release held. **USN02's "gameplay identical" failed:**
the named risk fired, through the host rather than the image.

**Why USN02 moved.**
- Both runs log Houston's ship-AI lines equally (133 each). What changes is who drives the wreck.
- While Houston is the controlled unit, the host holds it on the idle player's standing order
  (throttle 0, rudder 1; the OFF log's `controlled unit frame` lines continue after 74.55 s).
- Once control is released, its ship AI's output drives the sunk hull (`attackmove`,
  `navigate_astern`). Its position moves (`controlled moved` 1167.61 against 1155.93), and the
  escorts' and enemies' geometry follows.
- In the image a killed ship's AI is gated. 009F50E0's gate requires `unit+5Dh` clear, and the
  kill flush 009273A0 sets `+5Dh` and `+60h` (section "SetSelectedUnit's records",
  `docs/ENTITY_LIFECYCLE_TAILS.md`). The host's controller never sees a sunk ship as gated. That
  gap, not the notice, is what moves USN02.

**Verdict: not flipped** (`kControlledUnitObserverBound` stays false).
- **Follow-up for the ship-AI owner:** the gate on a killed ship's `+5Dh` (or the units host's
  `pending_destroy`/`destroyed` flags) in 009F50E0.
- With that gate in place, this pair is predicted to be identity. Then flip.

**Superseded: flipped ON (lead ruling 2026-09-27).**
- cc9-ships2 read the USN02 move (its section in this doc, landed at ce8e85b75). The host already
  treats a released wreck as the image does.
- The move is the image's own formation slot-swap rule 0070DB60, which never swaps a group that
  contains the controlled unit:
  - OFF, dead Houston stays controlled and group 0 never swaps again;
  - ON, the release lets it swap between 80 and 240 s, and the followers move.
- So the move is this binding's consequence, not a host gap.
- `kControlledUnitObserverBound = true`, with no re-run. The expected rows are the measured ON
  side: USN02 deaths 22, hit records 650, Alden dead at 336.69 s (Haguro); USN01 and USN04
  identity.

## Handoff (cc9-hud2 retires after this commit)

Worker cc9-hud2, 2026-09-27. The branch is `agent/cc9-hud2` and the worktree
`J:\PROG\battlestations-pacific-decompile-cc9-hud2`. It holds no leases.

### Switches this worker set

| switch | file | state | doc |
| --- | --- | --- | --- |
| `kMovieStepDrawBound`, `kMovieParentKilledByteBound`, `kMovieTerrainAvoidBound` | `include/bsp/game_hosts_hud.hpp` | ON | `docs/HUD_PICK_SEGMENT_QUERY.md` 10 |
| `kForceSelectUnitBound` | same | ON (the call itself unmeasured) | this doc, "ForceSelectUnit and 006485A0" |
| `kLocalPlayerUnitListRuleBound` | `include/bsp/game_hosts_world.hpp` | ON | `docs/LOCAL_PLAYER_UNIT_LISTS.md` |
| `kInitialControlledUnitBound` | same | ON | this doc, "The initial controlled unit" |
| `kControlledUnitObserverBound` | `include/bsp/game_hosts_hud.hpp` | **OFF** | this doc, "The controlled unit's destruction notice" |

### Open, in order

1. **The after-row-9 order queue** (`kAfterRow9OrderQueueBound`, not yet written).
   `docs/SENTITY_INIT_ATTACH_ORDER.md` section 22 has the design, the per-callback census and the
   predictions. It waits on `src/game_hosts_script_orders.cpp` and its header: cc9-ships2 holds
   them under `cc9_get_hp_percentage`.
   - Set a flag around `GameScriptOrdersHost::mission_lua_call_named_00887e50` when
     `src/mission_blackout.cpp` (005B9800) calls it.
   - While the flag is set, the 58h (0077D600) and 5Eh (00835860) order natives enqueue.
   - The next `GameFixedStepHost::pump_session_00778450` applies the queue in post order.
   - `GenerateObject` and `SetSelectedUnit` stay direct.
   - Predictions: USN01 moves through `luaIn`'s five `PilotSetTarget`; USN04 is identity, census
     0; USN02 moves through `luaMoveToPh2`'s `NavigatorAttackMove`.
2. **The observer switch** waits on a ship-AI fix:
   - 009F50E0 must stop driving a killed ship, gating on `+5Dh` as the image's kill flush sets
     it.
   - With that gate, USN02 9200/9000 is predicted identical. Then flip
     `kControlledUnitObserverBound`.
3. **Non-ship `vtable[124h]`** ("SetSelectedUnit's records"). 006D1EF0 is the base liveness test
   of 23 vtables. The host answers false for every non-ship and needs a class-to-vtable map.
4. **For the gunnery owner:** 0085AD00 skips unauthored RestAngles (FLT_MAX at 00D7A278), while
   the host's `rest_horz` defaults to 0.0.
5. **For the world host:** USN01's intro movie poses vary from run to run
   (`docs/HUD_PICK_SEGMENT_QUERY.md` 10.4); the source is not found.
6. **Movie camera parse keys** left unbound (`docs/HUD_PICK_SEGMENT_QUERY.md` 10.2): `flyalt`
   (x87 constants), the `goaround`/`gamecamera` modifiers, `event` and `finishscript`. None is
   reached by an idle measured run.

### Local files

- **Pair binaries:** `local\{sd_on,pt_on,fs_on,lp_on,ic_on,kk_ctl}`, each an export with its
  build.
- **Logs:** `local\{kk,sd,pt,fs,lp,ic,ic2,ld,ob}_*_usnNN.log`.
- **Scripts:** `local\cc9-hud2-*`.

## What moved USN02 in the observer pair (packet `cc9_wreck_released_order`, a read, 2026-09-27)

Worker cc9-ships2. No code changed. The evidence is cc9-hud2's own pair (`local\ob_{off,on}_usn02.log`
in its tree) and this tree's USN02 logs.

**The released wreck does not move; the host already matches the image there.**
- **The AI stops at the kill.** The row-15 flush sets `+5Dh` for every dead ship
  (`kSunkShipFlushBound`), and 009F50E0's gate stops the controller. Houston has 1491 controller
  steps in both OFF and ON, which is its death at 74.55 s. DeRuyter (3845) and Exeter (716) stop
  the same way, and USN02's `gated=42666` counts the skipped wreck steps.
- **The throttle is cut.** The wreck handler's cut (0082524B, `kWreckThrottleCutBound`) zeroes the
  ring's newest throttle. OFF shows the controlled frames going to throttle 0 at 75.0 s and the hull
  at rest by 76.5 s.
- **Houston's unit row is identical in the two runs.** The `controlled moved` difference
  (1167.61 against 1155.93) comes from where the frames stop printing: OFF prints the controlled
  frames to 450 s, and ON stops at the release at 74.5 s.

**What moved the pair: the formation slot swap's controlled-unit gate.**
- 0070DB60 BSP_UnitGroup_SwapSlotsByDistance returns without swapping for a group that holds the
  controlled unit (0070DB87..0070DBA0, `src/unit_group_slot_swap.cpp`).
- Houston is a member of DeRuyter's group 0 (the `formation group 0` line lists it).
  - On OFF the dead Houston stays the controlled unit, so group 0's swaps are gated for the rest of
    the mission.
  - On ON the release clears `00E188D8` at 74.5 s, the gate lifts, and group 0 swaps slots at 80,
    90, 130, 200, 210 and 240 s. These `formation slot swap` lines appear on ON only.
- The reshuffled stations move the followers.
  - The first differing ship-AI sample is step 3690 (184.5 s): Alden, John1 and John2 in `follow`,
    with different station targets.
  - The death rows follow from there: Alden 315.70 -> 336.69 s, John2 dies only on ON.

**The rule is the image's**, given the release: 0070DB60 reads the controlled unit, and 004C0890(null)
clears it. So the pair's move is a consequence of the observer switch, not a wreck-control gap.

**One open question for a later packet:** does the image keep a dead ship in its group's member
list? The host keeps Houston in group 0 after its death. If the image's kill path removed it, the
gate would have lifted at 74.55 s even without the release, and OFF would swap too.

## The unit vtable[124h] map (packet `cc9_vtable124_liveness`, `kUnitVtable124MapBound`)

Worker cc9-hud3, 2026-09-27, base 21b62fc38. Ghidra was read only.

### The map (read)

Every `.rdata` dword equal to 006D1EF0 was located with `tools/callsite_census.py`. There are 23,
and each is a vtable's `+124h` slot. The classes come from `docs/ENTITY_CLASS_IDS.md`'s vtable
column. The image carries no RTTI (the dword before each vtable is not a locator).

| `+124h` | body | classes |
| --- | --- | --- |
| **006D1EF0** | 006D1EF0..006D1F10: `+5Ch` set and `+5Dh`, `+60h`, `+5Eh` clear (0043F080's four bytes) | 05 (00D1A698), the ship base 06 (00D09678) and ships 07-0E (00CFC3D0, 00D0BF80, 00D01630, 00CFB738, 00CFA778, 00CFFA30, 00CF90B0, 00D0C648); the plane base 0F (00D05F20) and planes 10-17 (00D06638, 00D1A000, 00D19D28, 00D06920, 00D00070, 00D0BA80, 00D00308, 00D1A2D8); MLandVehicle 19 (00CFFDE0); 35 (00CFCD60); MAirfield 45 (00CF8C08); MShipyard 46 (00D0B770) |
| **006F5920** | 006F5920..006F5958, **no Ghidra function** | MCommandBuilding 1C (00CFB028): false when `+790h` is set and `[[+538h]+70h] == 58h`, else the four bytes |
| **00745A50** | 00745A50..00745A52 (`XOR AL,AL`, `RET`), **no Ghidra function** | MLandFort 1B (00CFF3F8): false |
| **007EE670** | 007EE670..007EE6AF, **no Ghidra function** | PlaneSquadronGen 18 (00D087C0): false when `+361h` or `+3B0h` is set, else the four bytes of the slot-0 plane `[+3D0h]`, false when that is null |
| **00927800** | 00927800..00927802 (`XOR AL,AL`, `RET`), **no Ghidra function** | the class-02 family outside 05 and 18: 02 (00D03E80), 04 (00D0DF70), 1E (00CFDC58), the guns 20-28 (00CFE0A8, 00CFE308, 00CFBD20, 00CFE548, 00CFBF58, 00CF96A8, 00CF9918, 00CFC190, 00CFAAB8), LandConvoy 1A (00CEA570): false |

**Coverage.** Every class-05 descendant in the table of `docs/ENTITY_CLASS_IDS.md` appears
above, and so does every class-02 descendant that can reach the test. Classes 2A and later are
rejected by 00645060 before `+124h` (`!IsKindOf(2Ah)`). 01, 03 and the other non-02 families fail
`IsKindOf(2)` first.

**Two single-player reductions (V):**
- **MCommandBuilding `+790h`.** Its one setter is 006F292D in 006F2780, reached through a
  vtable (00CFB0C8). It stores 1 only when 004BCA50 `BSP_Game_GetEffectiveGameMode` returns 2 or
  3. In single player 004BCA50 reports 8 (the mode is not forced, `game+1FE4h` is 0, and the mode
  is neither 8 nor 9). The constructor clears it at 006F5761. So 006F5920 reduces to the four
  bytes.
- **PlaneSquadronGen `+3B0h`.** Every store to the squadron's `+3B0h` is a clear: `MOV
  [ESI+3B0h],BL` in the constructor at 007F2DD3, and `MOV [ECX+3B0h],DL` with `DL = 0` at 007EFB69.
  The byte-store scans (`C6 ?? B0 03 00 00 01`, `88 ?? B0 03 00 00`, `C7`/`89`/`66 89` forms) find
  no other store in the squadron code. The same `88` pattern does find those two, so the negative
  is not vacuous. `docs/PLANE_SQUADRON.md`'s "any plane ready scratch" row names no setter.

**Where the test is asked.** 00645110 in 00645060, after `IsKindOf(2)`, `!IsKindOf(0Fh)`,
`!IsKindOf(2Ah)`, `!IsKindOf(46h)` and `!IsKindOf(45h)`. So planes, airfields and shipyards never
reach it through 00645060. Also 0064565F in 00645600, for a candidate answering `IsKindOf(1)`.

### The binding

- **Switch:** `kUnitVtable124MapBound` in `include/bsp/game_hosts_hud.hpp`, committed OFF.
- **The code:** `GameHudHost::Impl::unit_vtable_124` answers both sites by the map. It tests 1C
  before 1B and 1B before 05, because 1C descends from 1B and 1B from 05.
- **SUBSTITUTION, labelled:** the squadron's `+361h` is set by 007F31A0 when the leader leaves
  the map with survivors. This host does not model it, so it reads clear.
- **OFF keeps the old rule:** ships answer the four bytes, and every other class answers false as
  the record.
- **The census**, on both sides:
  - `summary mission hud unit vtable124 map`: asks, and the units the map answers true for where
    the old rule answered false, with class and first site;
  - a `+8Ch members:` line after each root-list build, with class ids.

### The OFF census (this commit's OFF build, `local\vt_off_*`)

| mission | asks | map-only units | `+8Ch` builds |
| --- | --- | --- | --- |
| USN01 3200/3000 | 59 | **ScoutDauntless/12h at 00645110** | frame 0: Dunlap, Ralph, McCall, Blue (07), Enterprise (09), Northampton, SaltLakeCity (0A); frame 1: Northampton; frame 403: Dunlap, Northampton, SaltLakeCity |
| USN04 4700/4500 | 62 | none | frame 0: nine Fletchers (07), Lexington, Yorktown (09), seven cruisers (0A); frames 1 and 502: Lexington |
| USN02 9200/9000 | 70 | none | frame 0: 14 ships; frames 1 and 524: 10 ships |

No fort, command building, land vehicle, class-35 unit or squadron reaches either site with the
map answering true, on any of the three missions. The `+8Ch` lists hold ships only.

**ScoutDauntless** is asked by `SetSelectedUnit(Mission.ScoutBomba)` in `luaPh2MovieEnd`. The
host's unit 62 is the squadron slot fused with its leader, and its class is the leader's
dive-bomber class 12h. 00645060 rejects it at 006450DF (`IsKindOf(0Fh)`), before `+124h`, on
either side.
- **Observation, outside this switch.** In the image, `GenerateObject("ScoutDauntless")` returns
  the PlaneSquadronGen entity (18h). It answers `IsKindOf(2)` and not `IsKindOf(0Fh)`, and its
  `+124h` 007EE670 is true while its leader lives. So the image **accepts** this
  `SetSelectedUnit`, and the player's controlled unit becomes the squadron at `luaPh2MovieEnd`.
  The host rejects it because its squadron slot carries the leader's class.
- That is a units-host class substitution. It needs the squadron slot to answer as class 18h to
  00645060.

### Predictions (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player)

- **USN01 3200/3000:**
  - ScoutDauntless's `+124h` answer goes from false to true, and 00645060 still rejects it at
    `IsKindOf(0Fh)`. The `SetSelectedUnit` line is unchanged except `vt124=1`.
  - The `+8Ch` builds are identical (the same members, order and cursor). The initial controlled
    unit stays Dunlap, and then Northampton.
  - Pick counts are unchanged.
  - The native table swaps the `SetSelectedUnit::unit_vtable_124` record for a concrete
    `Unit::vtable_124` row.
  - **Gameplay identical, exit 1.**
- **USN04 4700/4500:** census none; the same table change. **Identity, exit 1.**
- **USN02 9200/9000:** census none; the same table change. **Identity, exit 1.**

### Pairs and verdict

- **The pairs.** The OFF side is this tree's build at c30b838cd (`local\vt_off_*`). The ON side
  is `local\vt_on`, a `tools/pair_export.py` export of the same commit with only the switch
  flipped.
- **Run parameters:** streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000 | exit 1 | gameplay, 7 death rows and 28 unit rows identical. Every `+8Ch` build is identical (members, order, cursor), and so are the initial unit (Dunlap) and `SetSelectedUnit(Northampton)`. The one moved line is ScoutDauntless's rejection with `vt124=0 -> 1`, still rejected at `kind0F=1`. The native table swaps the record `SetSelectedUnit::unit_vtable_124` (25 calls) for `Unit::vtable_124` (59, concrete) |
| USN04 4700/4500 | exit 1 | gameplay, 44 death rows and 81 unit rows identical; the table gains `Unit::vtable_124` (62) |
| USN02 9200/9000 | exit 1 | gameplay, 19 death rows and 28 unit rows identical; the table gains `Unit::vtable_124` (70) |

**Predictions:**
- **Held:** identity on all three missions; ScoutDauntless's answer; the `+8Ch` builds; the
  initial controlled unit.
- **Failed, minor:** USN04 and USN02 had no record row to remove. No non-ship unit is asked there,
  so the old rule never reached its record arm, and the table only gains the concrete row.

**Verdict: `kUnitVtable124MapBound = true`.**

**Open (units host, not this switch):** the host's squadron slot carries its leader's class.
`SetSelectedUnit(Mission.ScoutBomba)` in USN01's `luaPh2MovieEnd` is therefore rejected at
`IsKindOf(0Fh)`, where the image's PlaneSquadronGen (18h) would pass. That would make the squadron
the controlled unit, since its `+124h` 007EE670 answers the leader's liveness.

## The squadron slot in the selection tests (packet `cc9_squadron_slot_class`, `kSquadronSlotClassBound`)

Worker cc9-hud3, 2026-09-27, base 3e754f54e. Ghidra was read only.

### The read

- **In the image, a squadron is selectable.** `Mission.ScoutBomba = GenerateObject("ScoutDauntless")`
  is the PlaneSquadronGen entity (class 18h). 00645060 then passes it:
  - it answers `IsKindOf(2)` and none of `0Fh`, `2Ah`, `46h` or `45h`;
  - its `vtable[124h]` 007EE670 answers the slot-0 plane's four bytes (previous section);
  - 00645600's `IsKindOf(1)` holds.
  So `SetSelectedUnit(Mission.ScoutBomba)` in USN01's `luaPh2MovieEnd` makes the squadron the
  controlled unit `00E188D8`.
- **What a controlled squadron changes in the image:**
  - **The interface.** 0068AE0B's 20h classifier tests 18h last and redispatches with the
    squadron's `+3D0h` plane (0068AF18), which answers `IsKindOf(0Fh)`. So the final interface is
    INTF_PLANE or INTF_PLANESPAWN with the plane as payload, whose hand-offs are `+6Ch`/`+68h`
    (0068B03B, 0068B047).
  - **The plane-side readers of `00E188D8`**, all display-only (census of the absolute address,
    `tools/callsite_census.py`):
    - 007BBD1A in `BSP_PlaneEntity_ApplyHitRecord` compares it with the plane's squadron
      `+9D4h` and feeds the HUD hit indicator 0064B170;
    - 008259EB (`BSP_UnitInstance_Update`) and 0080FC9B compare it with the unit itself and pick
      an effect intensity (1.0 or `[00CE3800]`);
    - 007F3A60 `BSP_PlaneSquadron_ReleaseControlledUnit` is the squadron's `+7Ch` slot, which
      releases the controlled unit when the squadron itself goes.
    No pilot or squadron AI reader was found in 007A0000..0084FFFF or 00990000..009CFFFF.
- **The host fuses the squadron with its wing-0 plane in one slot.** That slot is
  `PlaneSquadronHostRecord::squadron_unit` (`src/game_hosts_script_orders.cpp`), and it carries the
  plane's class (ScoutDauntless: 12h). So 00645060 rejects it at 006450DF, `IsKindOf(0Fh)`.
- **The role screen** (0067BB50, units host) needs only `IsKindOf(2)`, which the plane class also
  answers. It takes role 0 (mask 1) on the controlled slot. Only the pilot role (mask 2) sets
  `+184h`, so the plane's AI is not handed to the player.

### The binding

- **Switch:** `kSquadronSlotClassBound` in `include/bsp/game_hosts_hud.hpp`, committed OFF.
- **The code:** `GameHudHost::Impl::selection_class_id` answers 18h for a squadron's fused slot.
  `selection_is_kind_of` then serves:
  - 00645060's five class tests;
  - the `vtable[124h]` dispatch (so the slot takes 007EE670's arm, the slot-0 plane's liveness);
  - 00645600's `IsKindOf(1)`.
  The HUD root lists and ForceSelectUnit use the same tests.
- **Everything else keeps the plane's class:** the interface classifier (whose image path ends on
  the plane anyway), the units host and gunnery.
- **SUBSTITUTIONS, labelled:**
  - **The slot's own four bytes are the plane's.** After the wing-0 plane dies, the image's
    squadron entity is still alive, but this host's slot reads dead.
  - **The HUD observer releases on the slot's death**, which is the wing-0 plane's death. The
    image releases when the squadron entity goes (007F3A60 or its own destruction notice).
  - **The plane interface's camera hand-off** (`+6Ch`/`+68h`) is not modelled. After a plane is
    selected, the mission camera stays unbound.
- **The census:** `summary mission hud squadron slot class`, listing each squadron slot the
  selection tests saw, with its class (host -> selection).

### The OFF census (this commit's OFF build, `local\sq_off_*`)

| mission | squadron slots asked |
| --- | --- |
| USN01 3200/3000 | ScoutDauntless (12h -> 12h), by `SetSelectedUnit(Mission.ScoutBomba)` at mission frame 2003 (about 100.2 s) |
| USN04 4700/4500 | none |
| USN02 9200/9000 | none |

The installation's scripts were grepped read-only. `usn_2_java.lua` selects only Houston. USN04
(`usn_19_coralus.lua`) selected only the Lexington in the OFF log. USN01's other selections
(`luaKatMovieEnd`, `luaConHitMovieEnd`) are not reached within 3,000 frames.

### Predictions (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player)

**USN01 3200/3000:**
1. The census reads `ScoutDauntless/12->18`.
2. `SetSelectedUnit("ScoutDauntless")` is **accepted** at mission frame 2003, with `kind0F=0` and
   `vt124=1`.
   - 00645600 releases Northampton through its ship arm and moves `00E188D8` to ScoutDauntless.
   - 00647040 pushes 20h with the unit. The 20h classifier picks INTF_PLANE (in flight).
3. The role screen releases role 0 on Northampton and takes role 0 on ScoutDauntless. `+184h`
   stays clear. The planes' AI is unchanged.
4. The controlled-unit observer (ON) registers on ScoutDauntless. At its death (129.85 s on OFF)
   it releases the controlled unit (`004C0890(null)`), so the controlled unit is none from then on.
   The world summary's `controlled=` reads `none`.
5. The mission camera is not rebound after the phase-2 movie. The pose, pick and camera lines
   after about 100 s move. `ray_pick_*` and `owner_140` move or stop.
6. **Gameplay.** The slot-swap gate on Northampton's group (Northampton, SaltLakeCity, Dunlap)
   lifts after 100.2 s. The OFF run logs no `formation slot swap` at all, even for the ungated
   groups. So no swap is predicted, and the ships' paths are unchanged.
   - Deaths stay 7, with every death row identical in time and killer.
   - The ScoutDauntless row may differ only in a controlled flag, if the death table prints one.
   - **Gameplay identical, `pair_diff` exit 1.**

**USN04 4700/4500 and USN02 9200/9000:** census `(none)`, and every line identical except the
switch's own summary. **Identity, exit 1.**

### Pairs and verdict

- **The pairs.** The OFF side is this tree's build at f9de31c1f (`local\sq_off_*`). The ON side is
  `local\sq_on`, a `tools/pair_export.py` export of the same commit with only the switch flipped.
- **Run parameters:** streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000 | **exit 3** | hits 150, shots 561, damage 2690.0, 7 death rows, 7 plane death modes and 28 unit rows all identical. The exit comes from `controlled moved`: Northampton -> `(none)`. Script timer failures go 0 -> 5 |
| USN04 4700/4500 | exit 1 | census `(none)`; only the switch's summary line changed |
| USN02 9200/9000 | exit 1 | census `(none)`; only the switch's summary line changed |

**USN01 in detail:**
- **Held:**
  - The census reads `ScoutDauntless/12->18`.
  - `SetSelectedUnit("ScoutDauntless")` is **accepted** at mission frame 2003. 00645600 moves
    `00E188D8` from Northampton to ScoutDauntless and pushes 20h.
  - The HUD observer's destruction notice fires once, at ScoutDauntless's death (129.85 s).
    `004C0890(null)` releases the controlled unit, so the run ends with none.
  - Every death row is identical.
- **Failed 1, the interface.** It is 24h INTF_PLANESPAWN, not 22h INTF_PLANE. The host's
  007BB9A0 (`plane_is_in_flight`) is unimplemented and answers false.
- **Failed 2, the exit code.** `pair_diff` counts `controlled moved` as gameplay, so the
  predicted release makes exit 3. Every combat number is identical.
- **Failed 3, "no swap".** On ON only, Enterprise's group (group 2) swaps once at 110.00 s, and
  Ralph and McCall exchange stations (across 68.92 and -561.47). No other ship line differs.
  - That group never held the controlled unit, so 0070DB60's own gate (0070DB87..0070DBA0, per
    group) does not explain it.
  - The caller's walk over the groups was not read. One reading is that it stops at the first
    gated group, which was Northampton's on OFF. That is **unverified**.
- **Not predicted, the blocker: five `luaTimetable` failures between about 100 s and 130 s**:
  `commandhelpers.lua:330: attempt to index field '?' (a nil value)`.
  - USN01's `luaCheckObjectives` runs the music check first:
    `luaCheckMusic(GetSelectedUnit())` -> `luaGetShipsAround`.
  - That reads `recon[thisTable[target.ID].Party][allegiance]`. The generated squadron's Lua
    table carries no `Party`: the GenerateObject route's attach writes none, and
    `write_party_race_fields`'s callers all pass -1. So `recon[nil]` is nil.
  - Each failure also skips the rest of that objective check.
  - The image would not fail there: its squadron entity's table carries its party (the 00928F50
    mirror).
  - The failures stop once the controlled unit is released at 129.85 s.

**Verdict: `kSquadronSlotClassBound` stays false.** The selection now matches the image. But
turning it on exposes three host gaps whose effects the image does not have. Bind them first, in
this order, then re-run this pair:
1. **`Party` (and `Race`) on a GenerateObject'd entity's Lua table** (00928F50's mirror; the
   caller of `GameMissionLuaHost::write_party_race_fields` on that route). This removes the
   script failures.
2. **007BB9A0 `plane_is_in_flight`**, for INTF_PLANE.
3. **The walk that calls 0070DB60**, to establish whether the group-2 swap is the image's
   consequence of the new controlled unit.

The plane interface's camera hand-off (`+6Ch`/`+68h`) stays a labelled record. It only affects
display and the pick.

### The slot swap after the selection (packet `cc9_slot_swap_walk_read`, a read)

Worker cc9-hud3, 2026-09-27. Ghidra was read only. This settles the squadron pair's "Failed 3".
- **The caller.** 0070DB60 has one caller, 00826D3E in 00825F20
  `BSP_UnitInstance_UpdateShipMotion` (`tools/callsite_census.py`: one rel32, no absolute
  reference). It is **not a walk over the groups**:
  - Each ship's motion update asks 00778890 whether the ship leads its group (00826CF5).
  - A leader counts its own timer `+E48h` down by the step (00826CFE..00826D13).
  - When the timer passes below zero, it re-arms it with `[00424C40()+430h]` (00826D21..00826D38)
    and calls 0070DB60 on its own group `[EDI-8Ch]` (00826D32, 00826D3E).
  - So one group's gate never stops another group's swap.
- **What moved in the pair.** USN01's `luaMoveToPh2` (about 91 s) runs
  `JoinFormation(unit, Mission.CVGroup[1])` over the live `Mission.BmdGroup` (Northampton,
  SaltLakeCity, Dunlap; `usn_1_marshall.lua` 274-277, 699-701). Northampton, the controlled unit,
  thereby joins Enterprise's group, the host's group 2.
  - **On OFF,** 0070DB60's own gate (0070DB87..0070DBA0, a member equal to `00E188D8`) holds
    group 2 for the rest of the run. The summary reads `2:runs=16,swaps=0,gated`.
  - **On ON,** `00E188D8` becomes ScoutDauntless at 100.2 s. Group 2's next tick, at 110.00 s,
    swaps Ralph and McCall (`2:runs=16,swaps=1,gated`; the gated mark is from before 100.2 s).
- **Verdict:** the swap is the image's consequence of the new controlled unit, under 0070DB60's
  per-group rule. **No host gap.** The downstream counters in the combined pair are that swap's
  effect on the escorts' paths: `gunfire avoidance detect` 4462 -> 4402 and the 0041BC20
  avoid-zone samples.

### The re-pair with the party and 007BB9A0 switches

- **The pairs.** The OFF side is this tree's build at 519df06ec, with all three switches off
  (`local\gp_off_*`). The ON side is `local\all_on`, an export of 519df06ec with
  `kGeneratedEntityPartyBound`, `kPlaneInFlightTestBound` and `kSquadronSlotClassBound` all ON.
- **Run parameters:** streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000 | exit 3 | hits 150, hull hits 85, damage 2690.0, shots 561, first hit 53.75 s, 7 death rows, 7 plane death modes and 28 unit rows identical. The exit comes from `controlled moved` Northampton -> `(none)`. Script timer failures stay 0 (fires 85). The interface is 22h |
| USN04 4700/4500 | exit 1 | identical; only the switches' summary lines changed |
| USN02 9200/9000 | exit 1 | identical; only the switches' summary lines changed |

**On USN01:**
- ScoutDauntless is accepted at frame 2003. It is the controlled unit from 100.2 s until its
  death at 129.85 s, when the observer releases it.
- The group-2 swap at 110 s is the image's consequence (above).
- **This is the lead's expectation: combat identical, with the squadron controlled from about
  100 s to 129.85 s.**

**Verdict: `kSquadronSlotClassBound = true`**, together with `kGeneratedEntityPartyBound` and
`kPlaneInFlightTestBound`.

The substitutions labelled in the binding stand:
- the slot's own liveness is the wing-0 plane's;
- the observer releases at that plane's death rather than at the squadron's;
- the plane interface's camera hand-off is not modelled.

## A controlled plane under INTF_PLANE, with an idle player (packet `cc9_controlled_plane_seat`, a read)

Worker cc9-hud3, 2026-09-27, base 92ded9747. Ghidra was read only, and **no code changed**. The
image's rule for an idle player is what the host already does, so there is nothing to bind.

**The two producers of a plane's pilot command block** (`docs/PILOT_COMMAND_PATH.md`,
`docs/PILOT_CONTROLS.md`). Both write `unit+9FCh..+A10h` through 007B8C90.
- **The pilot bot**, 0099ACD0 (PilotBot `+0Ch`), pushes every tick at 0099B0B9.
  - Its only gates are at 0099ACD6..0099AD09: the unit pointer, `+5Dh`, `+60h`, `+61h` and the
    squadron's `+61h`.
  - **No role test and no `00E188D8` test.**
  - The `+61h` writers are the setters 007B8840/007B8860, the launch routine 007C6760 (from
    006C3E50) and the constructors. None is in the pilot screen or the role path
    (`C6 ?? 61 01`, `88 ?? 61` over the image).
- **The player**, 00519520 (from the INTF_PLANE HUD screen update 00519BB0 at 00519CF7), runs only
  when 007BB9A0 is true. It pushes its command at 00519818 **only when
  `00927F30(unit, 1)` holds**, that is when the local player holds role 1, the pilot seat
  (00519807).
  - With role 0 only, it *tracks*: `unit+9F0h` goes into the screen's own `+24h` (005197F9..
    005197FF).
  - It takes the pilot seat through `0077C470(2, 1)` only when the stick moved more than 0.2
    (`00CE3D10`).

**What the controlled squadron gets on USN01:**
- The role screen 0067BB50 takes role 0 (mask 1) on the controlled slot. The combined re-pair
  shows it: `player roles: takes 8 -> 9`.
- Role 1 is never taken, because an idle stick never moves.
- So 00519520 never pushes, and the pilot bot keeps flying ScoutDauntless as an autopilot, attacks
  included. **The host's behaviour (the AI flies it; combat identical) is the image's.**
- **The "held level" and "last input latched" readings are both excluded:** the player's block is
  never written without role 1.

**What the plane interface adds for an idle player is display only:**
- 00519BB0's `role_Text` shows `ingame.pilotai`, because role 1 is not the local player's
  (00519BF0..00519C92);
- 005191B0 and 00519020 update HUD pages and the part flags `vtable[21Ch](2Ah/2Bh)`;
- the camera hand-off `+6Ch`/`+68h`.
All stay records or labelled, as before. If 007BB9A0 turned false, the screen would hand the unit
to the spectator HUD (`00647300(root, unit+9D4h)`, then `004CC460(24h, unit)`). It is true for
ScoutDauntless while it flies.

**Consequence for measured pairs:** until a run gives the plane stick input (a scenario like
`BSP_PLAYER_HELM` for ships), no plane-seat binding can move gameplay. The ship-side equivalent is
the scripted helm option (`docs/SCRIPTED_HELM.md`).

## Handoff (cc9-hud3 retires after this commit)

Worker cc9-hud3, 2026-09-27. The branch is `agent/cc9-hud3` and the worktree
`J:\PROG\battlestations-pacific-decompile-cc9-hud3`. It holds no leases after this commit.

### Switches this worker set

| switch | file | state | doc |
| --- | --- | --- | --- |
| `kAfterRow9OrderQueueBound` (+ delivery continuations) | `include/bsp/game_hosts_script_orders.hpp` | ON | `docs/SENTITY_INIT_ATTACH_ORDER.md` 22.7-22.9 |
| `kUnitVtable124MapBound` | `include/bsp/game_hosts_hud.hpp` | ON | this doc, "The unit vtable[124h] map" |
| `kSquadronSlotClassBound` | same | ON | this doc, "The squadron slot in the selection tests" |
| `kPlaneInFlightTestBound` | same | ON | `docs/IN_GAME_INTERFACE_SCREEN_SETS.md`, "007BB9A0 in the plane arm" |
| `kGeneratedEntityPartyBound` | `include/bsp/game_hosts_lua.hpp` | ON | `docs/SENTITY_INIT_ATTACH_ORDER.md` 23 |
| `kObjectiveStatusBound` | same | ON | `docs/MISSION_OBJECTIVES.md` 8 |
| `kGeneratedWingPartyBound` | same | ON | `docs/SENTITY_INIT_ATTACH_ORDER.md` 23.5-23.6 |

Reads closed without a switch:
- the 47h create path (SENTITY 22.7);
- USN01's intro pose noise, not reproducible (`docs/HUD_PICK_SEGMENT_QUERY.md` 10.5);
- the movie parse keys (same doc 10.6);
- the slot-swap caller (this doc);
- the controlled plane seat (this doc).

### Open, in order

1. **A plane-seat scenario.** No idle pair can exercise the player's pilot seat: 00519520 pushes
   only with role 1, taken through `0077C470(2, 1)` after a stick move above 0.2. A measurement
   option like `BSP_PLAYER_HELM` for planes would be the only way to exercise it.
2. **The squadron-slot substitutions** (units host). The squadron entity and its wing-0 plane share
   one slot.
   - After that plane dies, the slot reads dead, while the image's squadron outlives it.
   - The HUD observer therefore releases the controlled unit at 129.85 s on USN01. The image would
     release at the squadron's own end, through 007F3A60 or its destruction notice.
   - The squadron's `+361h` (set by 007F31A0 at a map exit) is not modelled.
   - A separate squadron slot (class 18h) would retire all three.
3. **007BB9A0's inputs** (`GameUnitsHost::plane_local_input_gate_007bb9a0`).
   - `+C0Ch` comes from 007C11E0's flight-state rule only. Its other writers are not modelled:
     007B8C30 (through 008A5BD0), 007C6871 in 007C6760 (from 006C3E50) and the ground roll
     007CC0E5.
   - `+904h` and `+AA0h` are not carried. `+AA0h` is 5.0 only for a `ShipYardLaunch`, which would
     turn the gate false.
4. **The plane interface's camera hand-off** (`+6Ch`/`+68h` in 0068B03B/0068B047) is a record.
   After a plane is selected, the mission camera stays unbound (display and pick only).
5. **The bot's task-install trigger.** The setter of the pilot bot's dirty byte `+7Ch` (0099ACD0
   installs a task while it is set; the constructor zeroes it at 0099A91E) was not found. The host
   installs at the order's delivery, one bot tick early (SENTITY 22.7).
6. **GenerateObject's party.**
   - The image's 00928F50 call site on that route is unread, so the host writes Party and Race at
     pass A (labelled).
   - The wing planes of a generated squadron now get the squadron's Party and Race
     (`kGeneratedWingPartyBound`, ON, packet `cc9_generated_wing_party`, SENTITY 23.5-23.6).
   - 0046D930's bag handling 0046DA33..0046DB02 is unread.
7. **Objectives.** The kind `+18h` is not recorded, so 008DFE50's hidden-objective early return is
   not applied. No measured objective holds a unit. The announcement 008E1D30 (sound) and the
   marker refresh are records.
8. **Movie camera.**
   - `flyalt`'s update arm 0079B3E9..0079B62F needs 0042AE80 and 0042B2F0 read before binding.
   - `finishscript` would bind onto 00887E50 with the `+390h` gate.
   - The `+64h` register store at 0079CFE8 (in 0079CFC0) is unattributed.
   - No measured mission reaches any of these (`docs/HUD_PICK_SEGMENT_QUERY.md` 10.6).

### Local files

- **Pair binaries:** `local\{cq_on,vt_on,sq_on,gp_on,all_on,ob2_on}`, each an export with its
  build.
- **Logs:** `local\{cq,vt,sq,gp,all,ob2}_{off,on}_usnNN.log` and `local\cam_*_usn01.log` (the
  intro-camera diagnostics).
- **Scripts:** `local\cc9-hud3-*`.

## Note from packet `cc9_gun_rest_angles` (2026-09-27)

`kGunIdleRestBound` is ON (docs/GUN_REST_ANGLES.md section 9). The row above that calls
`0080E290`'s `0085AD00` call "subsumed" no longer holds. The host no longer rests a targetless gun
every tick. After a unit release, the idle timer waits out `NoTargetTimeUntilRest` before it rests
the guns, where `0080E290` rests them at once. Idle-player reference runs never release a unit, so
no reference row moves. Binding `0080E290`'s call is the fix.

## Handoff (cc9-lua2, 2026-09-28)

Worker cc9-lua2. The branch is `agent/cc9-lua2` and the worktree
`J:\PROG\battlestations-pacific-decompile-cc9-lua2`. It holds no leases after this commit. Most of
the work is in `docs/LUA_BINDING_MISSION.md`, whose sections are named below.

### Switches this worker set

| switch | file | state |
| --- | --- | --- |
| `kGetPropertyClassReadersBound` | `include/bsp/game_hosts_lua.hpp` | ON (`docs/MISSION_LUA_GETPROPERTY.md` 9) |
| `kSquadronObserverLivenessBound` | `include/bsp/game_hosts_hud.hpp` | ON (`docs/PLANE_SQUADRON.md`) |
| `kObjectiveKindBound` | `include/bsp/game_hosts_ai.hpp` | ON (`docs/MISSION_OBJECTIVES.md` 9) |
| `kLuaKillBound` | game_hosts_lua.hpp | ON |
| `kLuaListenersBound` | same | ON |
| `kLuaReconListenersBound` | same | ON |
| `kReconListenerResetCycleBound` | same | ON |
| `kLuaHitListenersBound` | same | ON |
| `kForcedReconLevelBound` | same | ON (the table is in `src/recon_sensor_pass.cpp`) |
| `kLuaAddDamageBound` | same | ON (phase-2 USN02 pair) |
| `kLuaAAEnableBound` | same | ON |
| `kLuaSetShipSpeedBound` | same | ON |
| the SetInvincible native (no switch; inert until gunnery3's `kUnitInvincibilityFloorBound`) | `src/game_hosts_lua.cpp` | bound |

`read_difficulty_multipliers_0087d7b0` has no switch and is read by the gunnery host.

### Open, in order

1. **UnitGetAttackTarget, `008A6DE0`.** Read the director slots `48h` and `2Ch` and the command
   kinds 1 and 2, then bind it OFF with a LOMP06 prediction. See the section "UnitGetAttackTarget,
   008A6DE0".
2. **The rest of the refreshed ranking,** in this order:
   - `SquadronSetSpeed` `0089F780` (USN13, 15 calls);
   - `IsClassChanged` `008CC4B0`;
   - `SetSubmarineDepthLevel` `00893F40` (JM06, 5 calls);
   - `SetAirBaseSlotCount` `008963E0` (USN13).
3. **Listener channels still unfired:** `command`, `input` (movie skip) and the other channels
   are registered but never call back. Measured: JM06's `submove` (`command`).
4. **Hit-listener filters not modelled:** `targetDevice`, `attackerPlayerIndex`, `fireCaused` and
   `leakCaused`. They need attacker player indices and the device hit on gunnery3's hit event.
5. **Idle-run blind spots:** the hit, AA and forced-recon bindings were identity on idle runs.
   The first run with the player or a convoy leader hit, or the BSM01 air raid, measures them.

### Local files

- **Pair exports:** `local\{gp,kill,sq,ok,ls,rl,rc,fr,ad,ht,aa,sp}_on`.
- **Logs:** `local\<prefix>_{off,on}_<mission>.log`.
- **Scripts:** `local\cc9-lua2-*`.

## Handoff (cc9-lua3, 2026-09-28)

Worker cc9-lua3 took over cc9-lua2's lane (handoff above). The branch is `agent/cc9-lua3` and the
worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua3`. It holds no leases after this
commit. Everything below is on main except the last two rate-limit commits and this handoff,
which were reported to the lead.

### Switches this worker set (all ON by their pairs)

| switch | file | doc |
| --- | --- | --- |
| `kLuaUnitGetAttackTargetBound` | `include/bsp/game_hosts_lua.hpp` | `docs/LUA_BINDING_MISSION.md`, "UnitGetAttackTarget, 008A6DE0" |
| `kLuaSquadronSetSpeedBound` | same | "SquadronSetSpeed, 0089F780" |
| `kLuaIsClassChangedBound` | same | "IsClassChanged, 008CC4B0" |
| `kLuaSetSubmarineDepthLevelBound` | same | "SetSubmarineDepthLevel, 00893F40" |
| `kLuaSetAirBaseSlotCountBound` | same | "SetAirBaseSlotCount, 008963E0" (USN04 caller miss recorded) |
| `kLuaHitFilterFieldsBound` | same | "The unmodelled `hit` filters, bound" |
| `kLuaHitRateLimitBound` | same | "The hit-callback rate limit" |
| `kSubmarineDiveBound` | `include/bsp/game_hosts_units.hpp` | `docs/SUBMARINE_MODEL.md` section 12 (the dive law, 00936DC0) |
| `kSubmarineAirBound` | same | section 13 (air 00855250, crush 008551C0, `SetUnlimitedAirSupply`) |
| `kSubmarineSeabedBound` | same | section 14 (the scan 00855420, the clamp, the order-ring throttle bounds) |
| `kSubmarineDiveTeleportBound` | same | section 15 (the `Dive` teleport) |

The four new Lua rows are in the dispatcher's `handled` list (`src/game_hosts_lua.cpp`).

### Closed without a binding

- **The `command` listener channel.** No reachable mission registers one: JM07, JM02, JM13 and
  IJN13 run header scripts without it. The scripts that register one are not in
  `missiontree.lua`.
- **The `+928h` script table.** It is already bound by `kSceneStageScriptBound`. The log line now
  names its source.
- **`attackerPlayerIndex`.** The fire-time producer of `[src+1Ch]` is unread, and gunnery4's field
  stays -1. It is counted `unmodelled`.

### Open, in order

1. **The natives ranking refresh** on the current head, asked for by the lead. USN02 fails at
   29.75 s, so its phase-2 rows are gone. JM06 and LOMP06 carry the four submarine flips; rebaseline
   them first.
2. **`attackerPlayerIndex`** needs the producer of the ordnance record's `+1Ch` (stamped at fire
   time). `00988510`'s send block compares it with `attacker[+1ACh + category*4]`.
3. **The rate limit's `this+1A4h` list test** and a kamikaze shooter's forced kind 11h are not
   modelled.
4. **Submarine leftovers.** The ship-AI depth callers (`009E4B90`, `009E4EE0`, `009EA8DE`,
   `009EA949`), the message `A2h` echo, and the unread `00852970` periscope object tick.
5. **Noise to know.** The LOMP06 movie camera pose at frame 441 varies by 0.1 m run to run. The
   one at frame 201 moves with the seabed binding (section 14).

### Local files

- **Pair exports:** `local\{ga,ss,cc,sd,sc,dv,ar,hf,sb,dt,rl}_on`, and `local\sd_off`.
- **Logs:** `local\<prefix>_{off,on}_<mission>.log`.
- **Scripts:** `local\l3_*`.

## Why a controlled plane reads 0.00 m (packet `cc9_controlled_plane_ai_moveto`, `kPlaneRowPositionBound`, committed OFF)

Worker cc9-lua4, 2026-09-28. The question came from ships4's CAUTIOUSATTACK pairs
(`docs/PLANNER_TASK_CHOICE.md` section 11.4): on USN07 and USN09 an AI moveto issued through
`00A02020` to an air-group leader left it at 0.00 m.

### The image (V): the pilot bot's role gate

`BSP_PilotBot_Tick` (`0099ACD0`) runs its task logic only while the pilot role is AI-held.
`0099AE3F` calls `006DEEC0(bot, 1)`:

```
006deec0  MOV ECX,[ECX+0x50]            ; the unit
006deec7  MOV EAX,[ECX+EAX*4+0x1ac]     ; its role-1 (pilot) slot
006deece  CMP EAX,8 / JE -> return 1     ; PLAYER_AI
006deed4  CALL 00927F10 / JNZ -> return 1 ; the slot is AI-held
006deedd  XOR EAX,EAX / RET 4            ; a player holds the pilot
0099ae44  TEST AL,AL / JNZ 0099AE5C      ; AI: go on to the task logic
0099ae4a  CALL 0099A0A0 / CALL 007B8DC0 / JMP 0099B11D   ; player: skip it
```

So a plane whose pilot role a player holds ignores every task, AI moveto included. That is gate
7 of `docs/PILOT_BOT_TICK_GATES.md`. The host models none of the gates except the think interval.

### Why the gate does not explain the 0.00 m

- **USN07.** The controlled unit is `PBY Catalina 01`. Its roles are `held=088888888`: the player
  holds role 0, and the pilot role 1 is 8. The gate therefore lets the moveto through, in the image
  as in the host. The host installs the script's own `PilotMoveTo` task on it (`009C3000`), and
  every one of the 3000 plane steps is a free-flight step.
- **USN09.** The controlled unit is first `Maury`, a destroyer, and ends as `Enterprise_sqn01`,
  which the summary reports. (This read first took the earlier line; the pairs below correct it.)
- **The cause is the host's row, not the plane.** `refresh_row` (`src/game_hosts_units.cpp`)
  copies `motion.position` into the unit row. It runs only at the end of the ship-motion loop, and
  the plane branch `continue`s before it. A plane's row therefore keeps its spawn position. On
  reference h's USN04 every aircraft row shows `moved 0.00` at its spawn coordinates, although
  those aircraft fly and drop torpedoes.
- **What reads that row.**
  - The "controlled moved" summary.
  - `GameScriptOrdersHost::entity_pose_translation_008a7c3c`, the host's copy of `entity+FCh`
    that `GetPosition` (`008A7B00`, the read at `008A7C3C`) returns.
  - So a script's `GetPosition` on a plane returns its spawn point. No other native reads a unit
    row's position.

**Verdict on the question:** the image's gate is not what freezes these leaders, and the host's
pilot task already accepts the moveto. The 0.00 m is the stale row.

### The binding

`kPlaneRowPositionBound` in `include/bsp/game_hosts_units.hpp`. After `007CE040`'s fixed step, a
plane's row takes its motion position, and its moved distance follows. The ship-only row fields
(speed, throttle, rudder) are left alone. The census is
`summary mission plane row position bound=.. refreshes=..`.

### Predictions, before any run (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player)

- **USN07 3200/3000: exit 3.** "controlled moved PBY Catalina 01" rises from 0.00, because the PBY
  flies its moveto. `refreshes` equals the plane steps (3000). The rest of gameplay should be
  identical unless a script reads a plane's position.
- **USN09 3200/3000: exit 1.** The controlled Maury's "moved" is unchanged. Only the census
  differs, unless one of its `GetPosition` calls names an aircraft.
- **USN01 3200/3000: exit 3.** The controlled unit ends as the `ScoutDauntless` plane (reference
  h prints it last), so "controlled moved" rises from 0.00 if that plane flies. USN01 makes no
  `GetPosition` call.
- **USN04 4700/4500, USN13 3200/3000: exit 1, gameplay identical.** Their controlled units are
  ships. USN04 makes 8 `GetPosition` calls and USN13 27 (reference h). If any of them names an
  aircraft, that value moves, and the pair shows it as exit 3.

### The pairs and the verdict

OFF is this tree's build at `36326a0fd`. ON is `local\pr_on`, a `pair_export` of `36326a0fd` with
`kPlaneRowPositionBound=true`. Logs are `local\pr_{off,on}_<mission>.log`.

| mission | pair_diff | controlled moved OFF -> ON | death rows / unit table |
| --- | --- | --- | --- |
| USN07 3200/3000 | exit 3 | PBY Catalina 01 0.00 -> 9866.80 | identical (0 / 7) |
| USN09 3200/3000 | exit 3 | Enterprise_sqn01 0.00 -> 10176.60 | identical (0 / 31) |
| USN01 3200/3000 | exit 3 | ScoutDauntless 0.00 -> 4582.21 | identical (5 / 28) |
| USN13 3200/3000 | exit 1 | Enterprise, unchanged | identical (20 / 211) |
| USN04 4700/4500 | exit 1 | Lexington-class01, unchanged | identical (44 / 81) |

- **The mechanism held.** The controlled planes do fly: the PBY is 9866.80 m from its spawn, and
  the Dauntless 4582.21 m. Only the row that reports them moved.
- **The USN09 prediction failed on its premise.** I predicted exit 1 because I took Maury as the
  controlled unit. It ends as Enterprise_sqn01, whose row now follows it, as on USN07.
- **No `GetPosition` value moved any gameplay on these rows.** Every death row and unit-table row is
  identical.
- **Consequence for `docs/PLANNER_TASK_CHOICE.md` 11.4.** The "0.00 m both ways" of the two air
  leaders was the stale row. The leaders were moving.

**Verdict: `kPlaneRowPositionBound = true`.** The mechanism matches the image's `entity+FCh`, and
the one miss was on premise.

## Handoff (cc9-lua4, 2026-09-28)

Worker cc9-lua4 took over cc9-lua3's lane (handoff above). The branch is `agent/cc9-lua4` and the
worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua4`. It holds no leases after this commit.
Commits from `c19b2ea78` on were not yet landed when this was written. `bsp.py sync` refuses until
they are, so the next worker should start from main after the lead merges.

### Switches this worker set

| switch | file | state | doc |
| --- | --- | --- | --- |
| `kLuaDeviceReloadEnabledBound` | `include/bsp/game_hosts_lua.hpp` | ON | `docs/LUA_BINDING_MISSION.md`, "SetDeviceReloadEnabled, 008C1350" and its feed sections |
| `kLuaFormationQueryBound` | same | ON (spread miss recorded) | "IsInFormation 008996A0 and LeaveFormation 00899EB0" |
| `kSquadronTravelAltBound` | `include/bsp/game_hosts_units.hpp` | ON | "SquadronSetTravelAlt, bound" |
| `kPlaneRowPositionBound` | same | ON (premise miss recorded) | this doc, "Why a controlled plane reads 0.00 m" |

Also landed: the inert accessors `command_building_capture_range_07a0` and
`plane_class_max_speed_0188` (`fed901e6a`) for ships4's Capture path.

### Reads closed without a binding

- **The `attackerPlayerIndex` producer**: `0072BF10` stamps `shot+1Ch` (`docs/LUA_BINDING_MISSION.md`,
  "the fire-time producer"). Gunnery5 has since landed the field (main `e49be76ba`).
- **The rate limit's `this+1A4h` set and the forced kind 11h** are unreachable on this installation.
- **The submarine leftovers** are in `docs/SUBMARINE_MODEL.md` section 16. The AI depth states
  went to ships4.
- **The convoy back pointer `+738h`** is zero on every measured row (`docs/LAND_AND_STRUCTURES.md`,
  last section).

### Open, in order

1. **The `attackerPlayerIndex` hit filter** (the lead's queue). Sync first: the field
   `GameGunneryHitEvent::attacker_player_index` is on main at `e49be76ba`.
   - Under a new switch, keep the entry's `attackerPlayerIndex` set in `src/game_hosts_lua.cpp`,
     where the `hit` listener parse now counts it `hit_filters_unmodelled`.
   - At dispatch, test membership of the event's index, as the `recon` party set does.
   - Only JM06's `hshit` uses it (`{PLAYER_1}`, PLAYER_1 = 0), on a hospital ship that the idle
     runs never hit. Predict identity on JM06 and USN01.
   - The scenario that would exercise it is the player's own gun seat firing on the hospital ship.
2. **The LOMP10 obedience cases** (the lead's extension of the controlled-plane read). On LOMP10
   1200/1000, the party brain issues 117 `returntobase` tokens (`00E08F98`, zero position, flags 1)
   to the B-25 and Lightning squadrons, and movetos to the CargoShip leader, and nothing moves in
   150 s (`docs/PLANNER_TASK_CHOICE.md` section 13). Questions:
   - Does the image accept a brain returntobase for an AI squadron?
   - Does it accept a brain moveto for a CargoShip leader?
   - What do the host's pilot task and ship order intake do with each?
   - Before trusting "nothing moves", check the plane rows with `kPlaneRowPositionBound` ON: the
     old 0.00 m figures were the stale row.
3. **GetClosestBorderZone** (`008AECD0`) needs `004C7730` reconstructed and border-zone data.
4. **Ghidra definitions requested:** `0074DFC0`-`0074E0E4` (`MLandVehicle` `vtable[A4h]`, the
   `convoyID` reader).

### Local files

- **Pair exports:** `local\{dr,fq,rf,pr,ta}_on`.
- **Logs:** `local\<prefix>_{off,on}_<mission>.log`. The census logs are `local\l4_rk_*.log`.
- **Scripts:** `local\l4_*`.
  - `l4_runs.ps1` launches detached runs with the pair environment.
  - `l4_wait.sh` is the foreground wait on the final COM release line.
  - `l4_rel32.py` scans rel32 and absolute references to a target in the image.

## The LOMP10 obedience cases: `returntobase` and the CargoShip `moveto` (cc9-lua5, a read)

Item 2 of the cc9-lua4 handoff. The question was why the SELLING tick's orders on LOMP10 moved
nothing in 150 s (`docs/PLANNER_TASK_CHOICE.md` section 13.4). **No binding came out of it.** The
image's answer needs four bodies the host does not have, listed at the end.

### Re-measured on the current head

- The run is LOMP10 3200/3000 on this tree's build of `583e3eee1`, with the pair environment.
  The log is `local\rt_base_lomp10.log` in worktree cc9-lua5.
- The SELLING census is unchanged: 92 ticks, 46 approaches, 117 `returntobase`.
- **The ship half obeys now.** CargoShip is in `movetopos` from the first steps and moves
  1818.89 m.
- **The planes move, but not because of `returntobase`.** With `kPlaneRowPositionBound` ON, B-25 01
  flies 7044.64 m and B-25 01|.-2 6919.30 m. They close 7279.4 m and 7358.7 m on their
  `levelbomb` target from `PilotSetTarget`. The old "nothing moved" was the stale plane row.
- The host places every `returntobase` on each member plane (`ai_selling_tick`, command 21). The
  plane's bot intake then drops it, because the host's `resolve_return_to_base` answers 0. So the
  squadrons keep bombing.

### What the image does with a brain `returntobase` (V, from the listings)

**It goes to the squadron, not to the planes.**
- SELLING's air arm (`00A12040`-`00A12101`) walks the group's member list at `+563Ch`. Each member
  must answer `vtable[5Ch](18h)`; a member that does not gives ECX = 0 and a null read at
  `00A12087`. With `+361h` and `+3B0h` clear, `00A120EB` calls `0077D600` with ECX still the
  squadron.
- The flags argument 1 lands at message `+21h` (`007798F7`).
- The squadron's `vtable[160h]` (vtable `00D087C0`, slot `00D08920`) is `007F1940`. Its arms:
  - with `+5Dh` set it ignores the message (`007F194A`);
  - with no members (`+3D0h` = 0) it ignores it (`007F1AE4`);
  - `returntobase` goes through `007F16D0` with ECX = the squadron (`007F1AF9`), then
    `0071DB50`;
  - when the result is not null, it clears the squadron's commands and issues the result, because
    `+21h` is 1. With `+21h` clear it would first ask the director's `vtable[34h]`.

**`007F16D0` resolves it, in this order (V, listing checked at the head):**
1. `[sq+35Ch]` is the **plane class descriptor**, not an assigned base. `+198h` is `MinWaterSpd`
   (`docs/PLANE_CLASS_FIELDS.md`). `007F16F0`-`007F16FB` (`UCOMISS`, `LAHF`, `TEST AH,44h`,
   `JNP`) take the null result when it **equals** 0.0.
   - This installation's B-25 Mitchell (class 118) and P-38 Lightning (class 104) author 22.222221
     (`vehicleclasses.lua`, mtime 2026-05-09), so both pass.
2. The member-array head `+3D0h`: an `MPlaneKamikaze` (`17h`) with `+C24h` clear gives the null
   result.
3. With `+369h` set: `006BCD20(+404h, 1)`, the home's air-ops block, then `006C4790` and
   `006BED30`. That gives `land` at the home through `007F1000`.
   - **LOMP10 authors no `HomeBase`** (`summary squadron scene home base ... keys=0`), so `+404h`
     is 0 and this arm fails.
4. Still under `+369h`: `006C0840`, the nearest landing site. From the pseudocode it tests
   airfields (`45h`) and carriers (`9`) and walks the air-ops list at `00E19948` by side. Its register ABI was not traced. `006BC120` must be false, and then
   `land` goes through `007EF8B0` at the site's `+7Ch` owner.
5. Otherwise `004C7730` on the world object, for the squadron's side `+54h`. With a zone, the result
   is a **`retreat`** (`00E08F90`) toward a point built from the zone record's corner floats,
   scaled by `00D7A348` (the float block is not decoded).
6. Otherwise the result is null, and the order is dropped.

**So on LOMP10 the image either lands the B-25s and Lightnings at a friendly airfield or carrier, or
sends them to retreat. Both clear the `levelbomb`.** The host keeps them bombing. That is the
behavioural difference. Which arm is taken depends on `006C0840` and on the border-zone records.

### The border zones are reachable

This corrects the deferral of `GetClosestBorderZone` (handoff item 3).
- `004C7730` (`004C7730`-`004C7A96`) is small. It walks four lists at `world+7134h`, one per map
  edge, each 0Ch apart. It keeps the records whose `+4` is the side (any side when the side is
  negative), and clamps the position onto each record's edge (`+1Ch`, `+24h`, `+28h`, `+30h`). It
  returns the nearest record, the clamped point, and a direction taken from one of two constants,
  `00D7A24C` or `00D7A260` (values not read).
  - With no match it returns 0 for sides 0 and 1. For any other side it retries with -1.
- **The data is the map's own bounds.** The literal `+7134h` has six sites: the storage constructor
  and destructor, `BSP_Game_ApplyMapBounds` (`004D61CA`), and the merge pass `004CA930`, which
  joins adjacent records of one side.
  - `docs/WORLD_MAP_BOUNDS.md` already reads `004E6C00`'s `BorderSizeX/Y` and the NW/SE selection.
  - It records the rest as contracts: the twelve `004C7150` calls and the record rebuild.
- So the zones follow from scene properties the host already parses. The next step is
  `004C7150` and the rebuild in `004D5BD0`. No new data source is needed.

### What a binding needs, in order

1. The zone records: `004C7150`, the rebuild in `004D5BD0`, and the merge `004CA930`. Then
   `004C7730`. This also unblocks `GetClosestBorderZone` (`008AECD0`) and `PilotRetreat`'s zone
   (`docs/PILOT_ORDER_BINDINGS.md` step 3').
2. `006C0840`, the nearest landing site, read from the listing (register arguments).
3. The squadron intake `007F1940` for `returntobase`: resolve on the squadron, clear, issue.
4. **A flown `retreat` and `land`.** The host has no bot task flight for either (`kRetreat`
   `009CA2B0` and `kLand` `009B41C0` are table rows only), and `PilotRetreat` is not bound.
   - Until one exists, an intake binding would clear the `levelbomb` and leave the squadrons
     without a task. That changes gameplay for the wrong reason, so no switch was added.

`docs/ATTACK_COMMANDS.md` ("`returntobase` and `land`") read `+35Ch` as an assigned base; it now
carries a correction.

## `returntobase` on LOMP10 lands the squadrons; it does not retreat them (cc9-lua5, a read)

The lead asked for the squadron intake with the retreat arm as the resolution LOMP10 reaches. The
listing of the fourth arm says LOMP10 reaches the **land** arm first. So no switch was added.

**`007F16D0`'s nearest-site arm (V, listing `007F177E`-`007F17DF`).**
- `006C0840` is called with ECX = the squadron's side `+54h` and EDX = the member-array head
  `+3D0h`. Its stack arguments, in order, are:
  - 0, the distance out-pointer, unused here;
  - `0047B850(head)`: 1 when the plane answers class 10h or 16h;
  - 1: `PUSH 1` at `007F1788` is `006C0840`'s third argument. `0047B850` takes none and returns
    with a bare `RET`, and `006C0840` ends `RET 0Ch`.
- `006C0840` (`006C0840`-`006C0B3F`):
  - It needs the plane's `+9D4h`.
  - It first tries the plane's own `00923810(1)` site: an airfield (45h) or mothership (9) that is
    not dead and whose side matches.
  - Otherwise it walks the air-ops list at `00E19948` (next at `+BCh`). A block qualifies when:
    - its object `+4` and owner `+7Ch` exist;
    - the owner is not dead (`+5Eh`) and not remote (`+5Dh`, because of the third argument);
    - when the second argument is set, the block's `+20h` bit 1 is set;
    - the owner's side is the squadron's (or at least 2 outside multiplayer).
  - It keeps the nearest block, preferring those for which `006BC530` answers true. It returns the
    block even when none answers true.
- **Back in `007F16D0`:** block `+4`, then `006BC120` answers 0 when that object's `+7Ch` owner
  exists with `+5Dh` clear. Then `007EF8B0(land, 00465080(owner, 0.0))`: **`land` at the site.**

**On LOMP10.** The scene has one Allied airfield, `CB4_AF` (`MultiAirField`, party Allied(0), an
air-ops deck of 4 slots; `local\rt_base_lomp10.log`). The B-25 and Lightning squadrons are Allied,
so the image sends them to land at `CB4_AF`. The retreat arm is reached only when no friendly live
airfield or carrier with a deck exists.
- **Two qualifications from the same log.**
  - B-25 01 is the controlled unit (`00E188D8`, instance class id 16 = 10h). So `0047B850` answers
    1 for its head, and `CB4_AF` qualifies only if its block's `+20h` bit 1 is set, which is not
    read.
  - If the squadron's `+5Dh` marks player control, `007F1940` ignores the order for B-25 01
    altogether (`007F194A`). The Lightning squadron is not controlled.
- **Not settled:** class 16h, `006BC530`, the `+20h` bit and the meaning of `+5Dh`.

**What a binding needs.** A flown `land` task (`kLand` `009B41C0`) that the host lacks, besides
the intake `007F1940` and a reconstruction of `006C0840`. Binding the retreat arm alone would send
LOMP10's squadrons somewhere the image does not.

## Handoff (cc9-lua5, 2026-09-28)

Worker cc9-lua5 took over cc9-lua4's lane (handoff above). The branch is `agent/cc9-lua5` and the
worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua5`. It holds no leases after this commit.

### Switches this worker set

| switch | file | state | doc |
| --- | --- | --- | --- |
| `kLuaHitAttackerPlayerIndexBound` | `include/bsp/game_hosts_lua.hpp` | ON | `docs/LUA_BINDING_MISSION.md`, "`attackerPlayerIndex`, bound" |
| `kLuaClosestBorderZoneBound` | same | ON (exit-code miss recorded) | `docs/LUA_BINDING_MISSION.md`, "`GetClosestBorderZone`, bound"; `docs/WORLD_MAP_BOUNDS.md`, "Border zones" |

The border-zone records and queries are new C++ in `src/world_map_bounds.cpp`. The mission frame
hands the Lua host the map bounds at the avoid-zone load (`src/game_hosts_mission_frame.cpp`).

### Reads closed

- **The `hit` filters are all modelled now.** 84 `hit` listeners in this installation name an
  `attackerPlayerIndex` set, not one. No measured row reaches the test.
- **The LOMP10 obedience cases** (this doc, "The LOMP10 obedience cases"):
  - the CargoShip obeys its `moveto`;
  - the planes fly their `levelbomb`;
  - the image would clear that and land or retreat the squadrons, and the host drops
    `returntobase`.
- **The fourth natives ranking** (`docs/LUA_BINDING_MISSION.md`): after `GetClosestBorderZone`, no
  unimplemented native with gameplay reach is left on the measured rows.
  - USN02's `CountdownCancel` cancels a countdown that was never started.
  - `Scoring_SetMissionCompleted` and `BannSupportmanager` are the failure path's scoring.
  - JM06's `LoadCheckpoint` answers "no checkpoint", as a fresh run would.

### Open, in order

1. **`returntobase` for AI squadrons.** In order:
   - `006C0840`, the nearest landing site: read its register ABI from the listing;
   - a flown `retreat` and `land` bot task (`009CA2B0`, `009B41C0`), which the host lacks;
   - then the squadron intake `007F1940`: resolve through `007F16D0`, clear, issue.
   The retreat arm's zone query is ready (`closest_border_zone_004c7730` with the squadron's side).
   Until a flown task exists, binding the intake would strand the squadrons. It belongs with
   whoever owns plane flight.
2. **`PilotRetreat` (`008A4300`)** can now take its zone from `closest_border_zone_004c7730`
   (`docs/PILOT_ORDER_BINDINGS.md` step 3'). It needs the same flown `retreat` task.
3. **USN02's reference row moved** after its failure (`GetClosestBorderZone`). Re-anchor it at the
   next re-baseline.
4. **Records for the lead:** the ledger and Ghidra names and comments for `004C71C0`, `004C7150`,
   `004C7730`, `004CA930` (unreferenced), `008AECD0`, `007F1940` (the squadron's `vtable[160h]`)
   and the `007F16D0` correction.
   - Ghidra was read-only for this worker.
   - `tools/const_width_sweep.py` was not run on the new constants.

### Local files

- **Pair exports:** `local\{ap,bz}_on`.
- **Logs:** `local\{ap,bz}_{off,on}_<mission>.log`, the LOMP10 re-measure `local\rt_base_lomp10.log`,
  and the census `local\l5_rk_*.log`.
- **Scripts:** `local\l5_*`.
  - `l5_runs.ps1` and `l5_wait.sh` are lua4's with the tree renamed.
  - `l5_api_census.py` lists the `attackerPlayerIndex` users.
  - `l5_minwater.py` maps `MinWaterSpd` to classes.

### Addendum after the handoff (cc9-lua5, same day)

The lead's later queue, taken after the handoff above:
- **`GetClosestBorderZone` on USN04** (`a93cd9467`): gameplay identical, as predicted.
- **`returntobase` on LOMP10** (`ff3f69a18`, "`returntobase` on LOMP10 lands the squadrons"): the
  image lands the Allied squadrons at `CB4_AF` through `006C0840` before any retreat. No switch.
  Open item 1 above now reads: a flown `land` task first, then `006C0840`, then the intake.
- **The LandConvoy roster** (`22747653d` OFF, `ec9b39927` ON, `docs/LAND_AND_STRUCTURES.md`,
  "The LandConvoy roster, bound"): `kLandConvoyMembersBound` is ON. On JM05, five members are
  created at their convoys' frames.

**New open items:**
1. **The convoy formation** (`00742400`, `00743060`). The first step is `007AF150`, the Path knot
   derivation, with its helpers `007AE330` and `007AE3E0`. The details are in the LAND doc's
   "Still open".
2. **Gunnery's `00805680` fold** over `GameUnitsHost::unit_land_convoy_738`, routed by the lead.
3. **`HudMinimap::land_vehicle_player_query 008DDF00`**: unimplemented, and now reached on JM05.

**New local files:**
- **Pair exports:** `local\{bz_off,lc_on}`.
- **Logs:** `local\lc_{off,on}_*.log` and `local\bz_{off,on}_usn04.log`.

### The `00805680` fold lines for the gunnery host (routed by the lead)

For `src/game_hosts_gunnery.cpp`, beside the `00805490` `group` lambda (about line 4778). The
fold rule is the squadron pass's, over class 19h and the back pointer `+738h`
(`docs/RECON_TEAM_LISTS.md`). The `m.level < 1` gate is assumed to be `008054D7`'s; it is not read
in `00805680`.

```
        // 00805680(B[19h], B[1Ah]), packet cc9_land_convoy_members: each class-19h
        // member with level >= 1 folds into its convoy (+738h, 00743A34); the
        // convoy's level is its members' maximum. SUBSTITUTION, labelled: the
        // convoy has no unit in this host, so the group is keyed by its scene
        // name and no class-1Ah record is pushed into the triples.
        const auto group_convoys = [&](std::map<int, std::vector<Rec>>& arr) {
            const auto found = arr.find(0x19);
            if (found == arr.end()) return;
            std::map<std::string, int> convoy_level;
            for (const Rec& m : found->second) {
                if (m.level < 1) continue;
                const std::string& convoy = units.unit_land_convoy_738(m.unit);
                if (convoy.empty()) continue;                 // 00805680's +738h != 0 test
                int& lv = convoy_level[convoy];
                if (m.level > lv) lv = m.level;
                ++recon_convoy_member_records;
            }
            recon_convoy_groups += convoy_level.size();
        };
```

- Call it after `group(enemy)` and after `group(neutral)` under `kReconAggregatesBound`, and
  before the own `group(own)`.
- `recon_convoy_groups` is a new counter beside `recon_convoy_member_records`.
- Replace the comment `// 00805680(B[19h], B[1Ah]): no convoy membership in this host.`.
- Turn the `record(...00805680)` into a `done(...)` once it runs.
- **Reach:** only JM05 has members, and they are Neutral, so only the neutral pass folds anything.
  The reference rows have no members, so this is identity there.

### The next plane packet: `cc9_squadron_land_task`

This is the piece that lets the image's `returntobase` answer run. The read is in this doc, "`returntobase` on LOMP10
lands the squadrons".
1. **The flown `land` bot task** (`kLand`, factory `009B41C0`, constructor `009B3240`, approach
   `009B2E50`, size 670h; `src/bot_tasks.cpp` table row). Not read.
2. **`006C0840`'s site lookup.** It takes ECX = side, EDX = the head plane, and three stack
   arguments: 0, `0047B850(head)`, 1. It ends `RET 0Ch`. The loop is at `006C0967`-`006C0AF3`.
3. **`006BC120`**, then **`007EF8B0(land, 00465080(owner, 0.0))`**.
4. **The squadron intake `007F1940`** for `returntobase`: resolve through `007F16D0`, clear
   (`+21h` = 1), then issue.

Predict it on LOMP10 3200/3000 with **Lightning 01** as the clean case. The controlled B-25's
`+20h` bit-1 gate and `+5Dh` byte stay labelled until read. Identity is expected on the
reference rows.

## The squadron's `returntobase` resolution, and the `land` task's shape (cc9-lua6, packet `cc9_squadron_land_task`, part 1)

Worker cc9-lua6, 2026-09-28. This continues "`returntobase` on LOMP10 lands the squadrons" above.
The resolution `007F16D0` and the site search `006C0840` are reconstructed in
`src/return_to_base.cpp`. The host binding is written but not committed: `src/game_hosts_units.cpp`
was leased to cc9-gunnery7 when it was ready. It is kept as `local\l6_rtb_units.patch` in worktree
cc9-lua6.

### `006C0840`, the nearest landing site (V, listing `006C0840`-`006C0B3C`)

**The ABI.** `__fastcall`:
- ECX = side, kept in EBX;
- EDX = the head plane, kept in EDI;
- stack: a distance out pointer, the "need approach bit" byte and the "local only" byte;
- `RET 0Ch`.

A null head, or a null `[head+9D4h]`, returns 0. The multiplayer flag is `00927C90(0) != 0`
(`006C0866`, `SETE`).

**The own-site branch.** `00923810(1)` is the head's scene parent `+3Ch`.
- **An airfield:** the parent answers `vtable[5Ch](45h)`, is not dead (`+5Eh`), and its own parent
  is null or not dead. Then:
  - a non-null distance pointer gets 0;
  - it returns `[site+7ACh]` when the side test passes, else 0.
- **A carrier:** otherwise, the parent answers `vtable[5Ch](9)` under the same liveness test. It
  returns `[site+1208h]` under the same side test, else 0.
- **The side test** (`006C08CE`..`006C08E4`): the first compare is unsigned (`CMP EBX,1`/`JA`), so
  any side other than 0 or 1 passes. Otherwise the site's `+54h` must equal the side, or be 2
  outside multiplayer.

**The walk.** The air-ops list is at `00E19948`, next at `node+BCh`. `node+4` is the block, and
`block+7Ch` is its owner. A node qualifies when:
- the block and the owner exist and the owner is not dead;
- with "local only" set, the owner's `+5Dh` is clear;
- with "need approach bit" set, `block+20h` bit 1 is set;
- the side is negative, or the owner's `+54h` matches, or, outside multiplayer, the owner's side is
  2 or more (`JL` at `006C09F8`, signed here, unlike the own-site test).

**The choice.** Two values decide it:
- **Acceptance:** `006BC530(head position, 0)`.
- **The key:**
  - for an accepting node, it comes from `block+88h` and `head->vtable[50h]()` through `00438B10`
    and `0042BE90`;
  - otherwise it is `00427E30` of `006BCC90`'s offset. That offset is the head's position in the
    block frame `block+48h` (`004142E0`), minus `block+A4h..ACh`. Its y is scaled by `00CE3DC8`
    below `00CE3948`.

The best key starts at -1.0 (`00D7A260`). An accepting node beats a non-accepting one, and within
a class the smaller key wins (`006C0AA3`..`006C0ADB`). It returns the **node**. When the distance
pointer is non-null, it gets 0 for an accepting best, else `sqrt(key)`.
- **Unread:** `006BC530`, `00438B10`, `0042BE90` and `00427E30`. With one node past the filters,
  neither value decides.

### `007F16D0`, re-read (V, listing `007F16D0`-`007F191F`)

- **The null arm** (`007F18EB`) writes record `[0] = 0` and the zero position
  `00F87574..00F8757C`.
- **`sq+369h` is `ReloadEnabled`,** default 1 at `007F2D09` (the units host's
  `control_flag_369`). It is not a deck flag. Both land arms need it.
- **The site arm.** `006C0840`'s stack is `(0, 0047B850(head), 1)`, where `0047B850` is
  `vtable[5Ch](10h) || vtable[5Ch](16h)`.
  - From the node: `node+4` must be non-null, and `006BC120(block)` must be 0. `006BC120` answers 0
    when `block+7Ch` exists with `+5Dh` clear.
  - Then `007EF8B0(out, land 00E08FA0, 00465080(block+7Ch, 0.0))`.
- **The retreat record:**
  - `[0]` = `00E08F90`; word `+4` = 0; byte `+5` = 1;
  - `+0Ch` = 0.25 × (0 + A.x + B.x + C.x + D.x), each sum stored as binary32, over `+10h`, `+1Ch`,
    `+28h` and `+34h`;
  - `+10h` = 0;
  - `+14h` = the same over `+18h`, `+24h`, `+30h` and `+3Ch`. `00D7A348` is the double 0.25.

**LOMP10, by these rules.**
- There is one air-ops deck, `CB4_AF`, Allied, and no `HomeBase`.
- **Lightning 01**'s head is class 13h, neither 10h nor 16h, so the approach bit is not needed.
  `CB4_AF` passes the filters and it is **land at CB4_AF**.
- **B-25 01**'s head is class 10h. `CB4_AF` qualifies only with `block+20h` bit 1, and that bit's
  producer is unread.

### The `land` task (`009B3240`, size 670h): what the flown task needs

**The per-tick arm** is slot `+64h` = `009B3EB0` (no Ghidra function; `009B3EB0`-`009B3F49`,
`RET 4`). It:
1. sets `+4ACh` = FFh;
2. calls `009B3900(approach, onGround, dt)`, where `onGround` means the current state is `+5D8h`,
   `+5F8h`, `+620h` or `+64Ch`;
3. when `+424h` is set, calls the state rule `009B3CF0` and then the current state's
   `vtable[0Ch](dt)`;
4. copies `+4ACh` to `+2E4h`.

**The states.** These are whole-object offsets. The approach is at `+3F8h`.

| offset | vtable | enter / exit / tick | reached when |
| --- | --- | --- | --- |
| `+4C4h` | moveto (`009C2AC0`) | shared | approach `+448h` = 1, through `009AFA50` |
| `+500h` | follow | shared | the same, for a wingman |
| `+598h` | `00D1FEA4` | `009B02E0` / `009B02F0` / `009B0300` | mode 2, wingman |
| `+5B8h` | `00D1FEF4` | `009B0230` / `009B0240` / `009B0FE0` | mode 2 leader, mode 3, or `+66Ch` set in `+64Ch` |
| `+5D8h` | `00D1FF14` | `009B13E0` / `009B0240` / `009B1D70` | mode 4 |
| `+5F8h` | `00D1FF44` | `009B1E60` / `009B1E90` / `009B1ED0` | from `+5D8h` when `009B3C00` answers true |
| `+620h` | `00D1FF60` | `009B21A0` / `009B21C0` / `009B22C0` | the plane's `+900h` is 4 or 5 (on the ground), from `+5F8h` when `009B3370` answers true, or at construction for a plane whose `(+72Ch)->vtable[38h]` is false |
| `+64Ch` | `00D1FED4` | `009B0980` / `009B09A0` / `009B09C0` | from `+5D8h`/`+5F8h` when mode is not 4 |

- **The mode `+448h`** (approach `+50h`) is written in the approach update `009B3900` and its
  callee `009B34D0`. Those are tied to the air-ops landing request `006C54C0(plane, approach+38h)`,
  which is paced by `+B0h`/`+ACh`.
- **The cruise profile** `009B3C60` writes `ctl+394h` = `Pilot/Landing/CruisingAlt` (1400).

**What a flown `land` needs**, beyond the moveto the pilot-moveto packet flies:
- the landing request `006C54C0` and the mode writer `009B34D0`;
- the six states' bodies, about 12 KB of listing from `009B0230` to `009B2C70`. `009B1420` and
  `009B22C0` are about 2.5 KB each and x87-heavy;
- the deck and runway side: touchdown `007CA3F0`, the slot release `006C65B0`, and the taxi.

**On the measured row.** In the current LOMP10 log, Lightning 01 ends about 13 km from `CB4_AF`
and flies about 9.5 km in the run. So the land task would spend most of the row in mode 1, the
moveto. Whether it reaches mode 2 depends on when the first `returntobase` lands and on
`009B34D0`.

### The record switch `kSquadronReturnToBaseResolveBound` (cc9-lua6, part 2, ON)

`issue_script_command` runs the resolution when the SELLING tick places `returntobase`
(`00E08F98`) on a squadron's flight leader. The inputs are labelled in the code
(`record_return_to_base_007f16d0`). The command is still placed as before, so the switch is a
record only.

**Predictions** (written before the ON runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player, present interval immediate):

| row | prediction |
| --- | --- |
| LOMP10 3200/3000 | **exit 1.** One `returntobase 007F16D0` line each for `B-25 01`, `Lightning 01` and `Warhawk 01`, all `-> land at site CB4_AF` with `sites=1 passed=1`. B-25 01's line carries `approach-bit-20-unread`, because its head is class 10h. None carries `home-arm-unread`. The summary reads `null=0 home=0 retreat=0 squadrons=3`, and `site` equals the number of leader placements. Gameplay identical |
| USN01 3200/3000 | **exit 1.** No SELLING returntobase, so the summary line reads all zeros with `squadrons=0` |

#### Record switch pairs and verdict

- OFF is this tree's build of `1749c12d1`.
- ON is `pair_export --commit 1749c12d1 --flip kSquadronReturnToBaseResolveBound=true`
  (`local/rtb_on`).
- The logs are `local/rtb_{off,on}_<mission>.log` in worktree cc9-lua6.

| row | pair_diff | what moved | verdict |
| --- | --- | --- | --- |
| LOMP10 3200/3000 | exit 1 | the record lines and the summary, and the same-binary noise below | held |
| USN01 3200/3000 | exit 1 | the summary line, all zero | held |

**LOMP10, ON.** All three squadrons resolve to `land at site CB4_AF` at 8.65 s, the first SELLING
tick:
- MinWaterSpd is 22.2222 for all three;
- each line reads `sites=1 passed=1`;
- B-25 01's line carries `approach-bit-20-unread`.

The summary reads `null=0 home=0 site=117 retreat=0 squadrons=3`. The 117 are the SELLING
tick's squadron-level placements. Gameplay is identical: deaths, hits and the 34 compared
unit-table rows.

**The moved presentation lines are same-binary noise.** A second OFF run
(`local/rtb_off2_lomp10.log`) against the first gives the same kind of drift:
- the minimap heading 0.4535 against 0.4537 rad;
- the landscape attach cells 46835 against 46833;
- 70 movie-camera pose lines moving by about 0.1 m;
- the doubled `ShipAiSectorScan` counters.

Those LOMP10 variances are not yet in the deterministic-noise list.

**Verdict: `kSquadronReturnToBaseResolveBound = true`.** It stays a record. The issue waits for
the flown `land` task, which is parked (the land-task section above).

## Handoff (cc9-lua6, 2026-09-28)

Worker cc9-lua6 took over cc9-lua5's lane (handoff above). The branch is `agent/cc9-lua6` and the
worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua6`. Everything is landed on main at
`d384f1ef5`. The worker holds no leases.

### Switches this worker set (all ON)

| switch | file | doc |
| --- | --- | --- |
| `kLandConvoyMovementBound` | `include/bsp/game_hosts_units.hpp` | `docs/LAND_AND_STRUCTURES.md`, "The convoy formation, bound" |
| `kSquadronReturnToBaseResolveBound` (record only) | same | this doc, "The squadron's `returntobase` resolution, and the `land` task's shape" |
| `kLuaSpawnNewIdQueriesBound` | `include/bsp/game_hosts_lua.hpp` | `docs/LUA_BINDING_MISSION.md`, "`SpawnNewIDIsRequested` and `SpawnNewIDRemove`, bound" |
| `kSquadronAttackAltBound` | `include/bsp/game_hosts_units.hpp` | `docs/LUA_BINDING_MISSION.md`, "SquadronSetAttackAlt, bound" |
| `kDiveProfileDrawBound` | same | `docs/DIVE_BOMB_TASK.md`, "The cruise profile's begin-altitude draw" |
| `kDepartedWingmanTaskBlockBound` | same | `docs/DIVE_BOMB_TASK.md`, "Departed wingmen keep their task's squadron block" |

**Other commits:**
- the inert `GameUnitsHost::set_formation_member_offset_0070d080`, for the ships lane's wedge;
- the OverrideHP dispatch in the Lua host, gunnery7's lines;
- the deletion of `submarine_depth_bands`;
- `007F16D0`/`006C0840` in `src/return_to_base.cpp`;
- `007AF150` in `src/camera_path_sampler.cpp`;
- the Landscape's local height and normal slots (`00ADA160`, `00ADA1C0`) in the scene-contents
  host.

### The natives ranking after these packets

This is the fifth refresh (`docs/LUA_BINDING_MISSION.md`, head `2291cd778`) with its outcomes:
- **Rank 1, `OverrideHP` `008C1930` (LOMP10):** the gunnery lane's (`cc9_override_hp`). The Lua
  dispatch is on main; the gunnery side is behind `kLuaOverrideHpBound`, and cc9-gunnery7 was
  measuring it.
- **Rank 2, `SquadronSetAttackAlt`:** ON. On LOMP10 the dive planes glide-bomb from 150 m.
- **Rank 3, `SpawnNewIDIsRequested`/`SpawnNewIDRemove`:** ON; identity on the measured rows.
- **Ranks 4-7 have small reach:**
  - `AddUntouchableUnit` (JM05, 3);
  - `GetLastCatapulted` (JM05, 47; nil also means "nothing catapulted");
  - `GetFormationLeader` (JM05, 49; nil falls back to the first ship);
  - `NavigatorEnable`, `GetFailure`, `Countdown`.
- **Everything else is presentation:**
  - `GetCapturePercentage`: nil raises "arithmetic on a nil value" at `jm05.lua` 5216, 48 times per
    150 s, inside an objective-text timer that `luaTimetable` retries; no gameplay;
  - `IsHintActive`, `DisplayScores`, `SetGuiName`, `PrepareClass`, `IsGUIActive`, and so on;
  - the failure-path scoring natives.
- **JM05's `FindEntity` UNIMPLEMENTED status is correct.** Its nine misses are names the scene does
  not author (`BSP_LUA_FIND_ENTITY_MISSES=1` lists them).

### Open, in order

1. **The SpawnNew queue never places on JM05** (for cc9-lua7). The brief is in
   `docs/LUA_BINDING_MISSION.md`, "Unowned: the SpawnNew queue never places on JM05".
   - **Measured:** `summary SpawnNew 0094c480 ... attempts=290 fulfilled=0 requeued=290 units=0
     still_queued=2` (`local/l6_rk_jm05.log`).
   - **The queue:** the two stage-init `SH2SpawnRequest` Fletchers (`luaJM5Shipyard2Spawned`,
     `refPos ABSENT`, `angleRange given`) are the only records. The drain takes one per 0.5 s
     (`SpawnAttemptDelay`) and requeues it every time.
   - **The drain** is `run_spawn_queue_0094c490` (`src/game_hosts_lua.cpp`, over
     `src/lua_spawn_new.cpp`). Its placement is `0094A140` (`spawn_reference_frame_0094a140` /
     `spawn_candidate_frame_0094a140`), then the fulfil test.
   - **Not yet found:** which test refuses every candidate. I did not trace it. Start with the
     reference frame for a request with no `refPos`, and with the `SpawnPlacementWorld` answer for
     a shipyard's water.
2. **The death-tick own-block draw (LABELLED)** in `kDepartedWingmanTaskBlockBound`.
   - A dying plane has left `member_units` (the removes-dead clear) but is not yet in
     `departed_units` (the `007F3970` compaction). For that one tick it resolves no record and
     draws once on its own slot.
   - Fix it by filling `departed_units` at the clear, or by resolving through `member_names`.
3. **Untraced: whether the image's bot think stops after `007F3A07` clears `plane+9D4h`.** The
   host keeps a dead plane's task running (the powerlost glide under pilot steering,
   `docs/PLANE_DEATH_MODES.md`). The task's `[task+404h]` is a construction-time copy, so it still
   reaches the squadron. Whether `0099ACD0` or the arm gates on `+9D4h` is unread.
4. **LandConvoy as a unit for the `00805680` fold (gunnery lane, routed by the lead).**
   - The members exist and move (`kLandConvoyMembersBound`, `kLandConvoyMovementBound`), and
     `GameUnitsHost::unit_land_convoy_738` answers their convoy by name.
   - The convoy itself has no unit slot: it is a scene record, and `+738h` holds its name.
   - The fold's convoy side needs either a unit-like convoy object or the fold keyed on the name.
   - `HudMinimap::land_vehicle_player_query 008DDF00` is still unimplemented and is now asked on
     JM05.
   - Also still open for the convoy: the member-death kill `007422A0` and the convoy's `+5Eh`.
5. **The flown `land` task is PARKED.**
   - The brief is this doc's section "The squadron's `returntobase` resolution, and the `land`
     task's shape": eight states, the rule `009B3CF0`, the per-tick `009B3EB0`, the mode writer
     `009B34D0`, and the landing request `006C54C0`.
   - The measured LOMP10 row cannot judge it: Lightning 01 ends about 13 km short of `CB4_AF`.
   - **What is bound:** `007F16D0`/`006C0840` resolve the squadron to `land at site CB4_AF` from
     8.65 s, as a record only.
   - **Unread:** B-25 01's `block+20h` bit, and `006C0840`'s ordering key when two sites pass.
6. **Found, not modelled:** the other ordnance profiles (`docs/BOT_TASKS.md` step 5 table) likely
   draw before their gates as the dive-bomb profile does. Only `009C8920` was read.

### Local files

- **Scripts:** `local\l6_*`.
  - `l6_runs.ps1` and `l6_wait.sh` are lua5's with the tree renamed.
  - `l6_paths.py` measures scene paths. It reads points in file order; use the numeric order,
    the note in `docs/LAND_AND_STRUCTURES.md`.
- **Pair exports:** `local\{cm,sq,rtb,aa,dp,dw}_on`.
- **Logs:** `local\{cm,sq,rtb,aa,dp,dw}_{off,on}_<mission>.log`, the census `local\l6_rk_*.log`,
  and the traces `local\l6_fe_jm05.log` and `local\dt_usn04.log`.

## Handoff (cc9-lua7, 2026-09-28)

This is written at about 80% context. Branch `agent/cc9-lua7`, worktree
`J:\PROG\battlestations-pacific-decompile-cc9-lua7`.

### Done this session (all switches ON unless stated)

| packet | switch | record |
| --- | --- | --- |
| `cc9_spawn_new_shipyard` | `kSpawnNewEntityRefPosBound` | LUA_BINDING_MISSION "SpawnNew with an entity refPos and a surface group" |
| `cc9_land_convoy_unit_plan` | none (plan, PARKED) | LAND_AND_STRUCTURES, the last section |
| `cc9_dead_plane_bot_think` | `kDeadPlaneBotThinkBound` | PLANE_DEATH_MODES section 7 |
| `cc9_death_tick_draw_recheck` | closed, no switch | DIVE_BOMB_TASK, beside cc9-lua6's labelled note |
| `cc9_ships7_entry_points` | none (inert accessors) | `GameUnitsHost::unit_class_yaw_rate_0082ecb0`, `GameMissionLuaHost::sub_attack_submarine_lost_time_04d4` |
| `cc9_natives_ranking_6` | none (a read) | LUA_BINDING_MISSION "sixth refresh" |
| `cc9_get_formation_leader` | `kLuaFormationLeaderBound` | LUA_BINDING_MISSION "GetFormationLeader, 00899AF0" |
| `cc9_add_untouchable_unit` | `kLuaAddUntouchableUnitBound` (inert) | LUA_BINDING_MISSION "AddUntouchableUnit, 008AC140" |

**Unlanded at this handoff:** `fb297db29` and `b5bc97bd1` (GetFormationLeader), `11bad8895` and
`38d8d14db` (AddUntouchableUnit), this handoff, and the main merges between them.

### Open, in order

1. **The untouchable gate `00862440` (gunnery lane, cc9-gunnery8).**
   - The flag is `bsp::game::lua_unit_untouchable_1d4(std::size_t index)`
     (`include/bsp/game_hosts_lua.hpp`).
   - The lead's brief asked for `GameUnitsHost::unit_untouchable_1d4`. The units host files were
     leased to cc9-gunnery8 (`cc9_periscope_out`), so the flag lives in the Lua host instead.
     gunnery8 can wrap it as that member.
   - JM05 marks `PT Boat 80' Elco 01`/`02` and `Event2Pt` at stage init. USN01, USN04 and LOMP10
     mark nothing.
2. **`GetLastCatapulted` `00892860`** (JM05, 47 calls), rank 3 of the sixth refresh. Small reach:
   it sets an elite skill on the catapult plane at 5812. First read whether the host catapults
   anything at all.
3. **The LandConvoy handle** is PARKED, because no reader in this host consumes a placed convoy.
   The plan names two open reads: the convoy's `+1E4h` sensor category, and when its detection
   record is created.
4. **Known noise worth adding to the deterministic list:** LOMP10's minimap heading, landscape
   attach cells, movie-camera poses and `ShipAiSectorScan` counters. They drifted between
   same-binary LOMP10 runs in every pair this session.

### Working notes

- In the Git Bash tool, a heredoc or `printf` that contains an apostrophe (`80'`, `lua6's`) breaks
  the whole command. Write such text with the Write tool and `git commit -F`.
- `pair_export --out` needs `local/<name>` with a forward slash in bash. `local\\name` lost its
  backslash and created `locall7_on` at the tree root.
- The units host files (`game_hosts_units.*`) were leased to cc9-gunnery8 at this handoff.

## Handoff (cc9-lua8, 2026-09-28)

Branch `agent/cc9-lua8`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua8`. Written at
about 70% context. No leases are held.

### Done this session (both switches ON)

| packet | switch | record |
| --- | --- | --- |
| `cc9_land_task_reach` | `kSquadronLandTaskBound` | `docs/SQUADRON_LAND_TASK.md` |
| `cc9_get_last_catapulted` | `kLuaLastCatapultedBound` | LUA_BINDING_MISSION "`GetLastCatapulted`, 00892860" |

- **The land task.** On LOMP10 the Lightning 01 and Warhawk 01 squadrons now leave the fight at
  3.80 s and fly `moveto (land)` / `follow (land)` to CB4_AF, where they circle. No plane lands,
  because every landing request misses the deck's assignment vector. Deaths are 11 -> 3 on
  LOMP10 9000. B-25 01 is refused (unread approach bit) and keeps bombing.
- **Ledger names:** `009B3CF0`, `009B3900`, `009B34D0`, `009B3560`, `009B3680`, `009AFA50`,
  `006C54C0`, `006C4790`, `006BD080`, `009B3750`. `009B3750` still needs a Ghidra function
  (`009B3750`-`009B376A`).

### Open, in order

1. **The deck's landing sequencer `006CC9F0`** (from `006CD240`; inserts through `006CAA10`,
   updates through `006C7960` and `006C3F80`). It is the only producer of modes 2-4, so it is
   what lets a plane reach `land/standby`, `land/line` and `land/begin`. Read it with `006C0B50`'s
   queue at block `+98h`, `006C3E50` and `006C5380`. The first landing state to bind after it is
   whichever the LOMP10 row then enters; the refusal counters in the land summary will say.
2. **B-25 01's approach bit** (`block+20h` bit 1 for a class 10h/16h head).
3. **The follow law on a circling leader.** Two Lightning wing members drift 5-6 km from their
   leader at the reseeded 173 m/s. This is the shared follow law, not the land task.
4. **Known:** the leaders climb to about 1.25 km on `009C18C0`'s far-distance glide before
   descending to 149.9 m, and the wingmen-wait term holds them near 31.5 m/s. Both come from
   code the land task reuses.

### Working notes

- In Git Bash, a heredoc with an apostrophe breaks the whole command, and so did a long
  `ledger add-name` chain. Write text with the Write tool; do multi-step edits from a script in
  `local\` (`l8_ins.py` inserts at a unique anchor).
- A locked RDP desktop (`logonui=1`) fails renderer init with `0x8876086A`.
  `local\l8_probe.ps1` retries a 300-frame smoke until it passes.

### Addendum (cc9-lua8, same day, after the lead's reorder)

- **Inert accessors for cc9-gunnery9** (`0b535255d`): `GameUnitsHost::add_hull_torque_message_93h`
  and `GameMissionLuaHost::read_physics_torpedo_force_0083b5e0`. No caller yet.
- **`kUnitYawRateForwardSpeedBound` ON** (`docs/UNIT_YAW_RATE_FORWARD_SPEED.md`).
  `unit_current_yaw_rate_00811940` now sets the binding's forward speed.
  - JM06, USN02 and USN04 move; USN12 and USN01 are identical.
  - cc9-ships8 can re-pair its rank 2 yaw-rate half on it.
- **Not started: packet `cc9_landing_sequencer`.** Read `006CD240` -> `006CC9F0` whole, find what
  creates a sequencer entry, and find what fills the assignment vector at block `+A8h` (inserting
  via `006CAA10`) and when. Then bind it OFF behind `kLandingSequencerBound`, predict on LOMP10
  9200/9000 (which plane gets an assignment, when the mode rises past 1, the first landing state
  entered), pair and flip by verdict. The land task's refusal counters in the LOMP10 summary are
  the measurement to start from.
- **Renderer-init outages.** 18:02 to about 18:40 UTC, the session was RDP with the desktop
  locked. Check `query session` first; `local\l8_probe.ps1` retries a smoke.
- **`GameCommandUnit::class_id` is filled** in the `command_units` loop from the slot's `class_id` (+C4h), for cc9-gunnery9's `kSquadronSetCommandBound`. It is inert until that switch flips.

## Handoff (cc9-lua9, 2026-09-28)

Branch `agent/cc9-lua9`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua9`. Written at
about 75% context. No switch was flipped this session.

### Done

| packet | commit | what |
| --- | --- | --- |
| `cc9_command_unit_class_id` | `6cbe2462d` | the command-units loop copies the slot's `class_id` (unit `+C4h`) into `GameCommandUnit::class_id`; inert |
| `cc9_unit_class_lands_troops` | `a24c4aa86` | `GameUnitsHost::unit_class_lands_troops_vtable_2c`; the Lua row reader now reads `LandingShip`, `LandingShipAmount` and `Rocketer`; inert |
| `cc9_landing_sequencer` (read) | `3f8267c99`, `7635037a4` | docs/SQUADRON_LAND_TASK.md section 5b; nothing bound |

The slot 2Ch values for `unit_class_lands_troops_vtable_2c` came from the disk image by a script
(`local\l9_vt.py`, which reads the PE file):
- `00827FB0` sits in the seven ship-leaf descriptors and the ship base `00D1ACC4`;
- `00963C70` sits only in MLandingShip `00D1AD78`;
- plane, land and building kinds answer false, because their slot 2Ch was not read.

### Open, in order

1. **`cc9_formation_join_follow` waits on one commands-host method.** The lead has the proposed
   `GameCommandsHost::issue_follow_command_00720cd0(unit_index, target_index)` for cc9-gunnery9:
   00720CA0's clear, then a direct 008358D0 `follow` push with no message.
   - The image, read whole: in `0077F940`, the loop `0077FA81`..`0077FABF` runs once per brought
     unit. It calls `0070EF30`, then `ESI->vtable[114h]()`, where ESI is always the ordered unit.
     When that is non-null, `director->vtable[58h]([ESP+80h])` follows.
   - `[ESP+80h]` holds the target group's leader, which `0077F9E6` stored.
   - So only the ordered unit gets `follow`, issued `brought.size()` times with the same argument.
     The one-level recursion over `brought[1..]` in `GameUnitsHost::formation_join_0077f940` must
     not issue it.
   - Once the method lands, add the call at the end of a successful top-level join behind
     `kFormationJoinFollowBound` (OFF), and write the predictions first. Every row has
     scene-formation followers. Scene groups are not built through `0077F940`, so only runtime
     joins (script orders and the ship-AI follower pass) issue it.
   - With `kAutoTargetFollowerGateBound` OFF, expect idle_follow re-issues to fall and queues to be
     cleared at join. Pair on USN01, JM06, JM08 and USN04, then re-pair
     `kAutoTargetFollowerGateBound` (docs/SHIP_AI_OPEN_ITEMS.md section 12).
2. **`cc9_landing_sequencer` binding.** Before binding:
   - confirm `006C6020`'s arc term (`00BF9940` has an x87 register argument) and `006C3F80`'s
     `00419010` argument roles from the listing;
   - read block `vtable[10h]`, the CB4_AF scene's `RunwayWidth`/`RunwayLength` and `0085DEA0`
     (the inverse at holder `+48h`).
   Then bind the holder frame from the owner's pose, the queue, the sequencer and `006C7960`
   behind `kLandingSequencerBound`. Also bind `006C54C0`'s found arm. The predictions for the
   LOMP10 9200/9000 row are at the end of section 5b.
3. **`cc9_plane_follow_law_drift`** (SHIP_AI_OPEN_ITEMS queue item 6): not started.
4. **B-25 01's approach bit**: not started.

### Working notes

- A Git Bash heredoc still breaks on an apostrophe. Write text with the Write tool or into
  `local\` files.
- `python tools/bsp.py ghidra decompile <addr> --start N --lines M` pages long bodies.
  `show` works only for exported functions.
- **`GameCommandUnit::class_id` is filled** in the `command_units` loop from the slot's `class_id` (+C4h), for cc9-gunnery9's `kSquadronSetCommandBound`. It is inert until that switch flips.

### Addendum (cc9-lua9, same day): item 3 committed OFF, pairs not run

- **`ef1fdd1a6`**: `kFormationJoinFollowBound` (OFF), wired to cc9-gunnery9's
  `issue_follow_command_00720cd0`. Predictions are in docs/SHIP_UNIT_GROUP_FOLLOW.md section 5g,
  written before any ON run.
- **The pairs were not run.**
  - The four OFF runs (`local\l9_off_<row>.log`, rows jm06/usn01/jm08/usn04 via
    `local\l9_run.ps1`) all died at startup with `_FMOD_EventSystem_Init result=61`.
  - `query session` showed session 1 `Disc`. That is the known no-audio-endpoint environment
    failure, not a code failure.
- **The ON export** is `local\l9_ff` (`pair_export --commit ef1fdd1a6 --flip
  kFormationJoinFollowBound=true`, log `local\l9_ff_export.txt`). Its build was still running at
  handoff.
- **Next:**
  - Once `query session` shows session 1 active, re-run
    `./local/l9_run.ps1 -Prefix l9_off -Rows 'jm06:JM06:3200:3000','usn01:USN01:3200:3000','jm08:JM08:3200:3000','usn04:USN04:4700:4500'`.
  - Run the same with `-Exe local\l9_ff\build\win32\Release\bsp_game.exe -Prefix l9_on`.
  - Then `python tools/pair_diff.py` on each pair, check the section 5g mechanism clauses, and
    flip by verdict.
  - cc9-ships9 then re-pairs `kAutoTargetFollowerGateBound`.

## Handoff (cc9-lua10, 2026-09-28)

Branch `agent/cc9-lua10`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua10`. Final state
at about 72% context; `land/standby` is ON and `land/line` is read in part.

### Done (every switch below is ON)

| packet | commits | switch | evidence |
| --- | --- | --- | --- |
| `cc9_landing_sequencer` | `cce28dd60`, `c3b35987f` | `kLandingSequencerBound` | docs/SQUADRON_LAND_TASK.md 5c; pairs gameplay identical |
| `cc9_formation_join_follow` | `c05ff3746` | `kFormationJoinFollowBound` | docs/SHIP_UNIT_GROUP_FOLLOW.md 5g; four pairs held |
| `cc9_plane_follow_law_drift` | `f9efe504d`, `3198cc88c`, `97d1c44bf` | `kFollowLeaderTurnRateBound`, `kFollowLeaderLiveSpeedBound` | docs/PLANE_FOLLOW_LAW.md 17.5, 17.7 |
| `cc9_follow_target_dir_acos` | `1d7b7045e`, `196d0c3d9` | `kFollowTargetDirAcosBound` (tuning loader) | docs/PLANE_FOLLOW_LAW.md 17.4, 17.6; LOMP10 drift 6442 m -> 131 m |
| `cc9_landing_approach_bit` | `5abf6e808`, `8c2cc67ca` | `kLandingApproachBitBound` | docs/SQUADRON_LAND_TASK.md 5d; B-25 01 lands instead of bombing |
| `cc9_land_standby_state` | `ac4f1d6fe`, `5f2f7bde5` | `kLandStandbyStateBound` | docs/SQUADRON_LAND_TASK.md 5e; the flight leaders fly their landing circle |

### Open, in order

1. **`land/line`**: read in part, in docs/SQUADRON_LAND_TASK.md section 5f.
   - Its tick `009B0300` has no Ghidra function. Continue the raw listing from `009B04D9`.
   - Then bind it OFF behind `kLandLineStateBound`. The template is standby's binding: search
     `run_land_standby_tick_009b0fe0` and `land_enter_standby_009b0230` in src/game_hosts_units.cpp.
   - Write LOMP10 predictions for the wing members at mode 2, pair it, and flip by verdict.
   - Then `land/begin`, which also needs the launch-site arm of `006C3F80` (block `+3Ch`, its
     `+40h` and `vtable[30h]`). A mother-ship holder is still refused.
2. **The collision-box extent binding** in ShipHullBodyInputs (GUNNERY_OPEN_ITEMS 47.3 step 3). It
   waits on cc9-gunnery10's per-class boxes.
3. USN01 came out identical on three follow-law pairs where it was predicted to move. Its 81 fly-to
   ticks are all dive-bomb done-law ticks. A row with torpedo-follow wings would test the follow law
   more widely.

### Working notes

- Runs died at renderer init (`hr=0x8876086a`, `logonui=1/1`) whenever the RDP session was locked.
  They came back at 15:28 on their own.
- The `land follow trace` diagnostic prints every 200 `follow (land)` ticks. It exposed the acos
  correction.
- Ghidra names were sent to the lead: `007D7DA0` (the leader turn rate) and `006CA5A0` (35.0).
- In Git Bash, a heredoc or a `-m` argument with an apostrophe breaks. Put scripts and messages in
  `local\` files.

## Handoff (cc9-lua11, 2026-09-28)

Branch `agent/cc9-lua11`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua11`. The brief's
queue is done at about 60% context. No lease is held.

### Done

| packet | commits | switch | state | evidence |
| --- | --- | --- | --- | --- |
| `cc9_land_line_state` | `9cdf5ee0e`, `8a1936324`, `7f36065fd` | `kLandLineStateBound` | ON | docs/SQUADRON_LAND_TASK.md 5g |
| `cc9_land_begin_state` (site arm) | `87895cada`, `8de33f8be` | `kLandingSiteSpacingBound` | ON, gameplay identical | 5i |
| `cc9_land_begin_state` | `8abe8cee0`, `70f425145` | `kLandBeginStateBound` | OFF | 5h |
| `cc9_land_final_state` | `880219379`, `a33bfc522` | `kLandFinalStateBound` | OFF | 5j |
| `cc9_hull_inertia` | `5125fc9a6`, `5a68b4d19`, `664b571f7` | `kHullInertiaFromShapesBound` | OFF | docs/GUNNERY_OPEN_ITEMS.md 51, 51.1 |

### Open, in order

1. **The touchdown.** Begin and final's airborne half match the listing on LOMP10. They stay OFF
   because this host has no ground contact: the planes fly on past T.
   - What is missing: plane `+BF8h`/`+BF4h` and `007CA3F0`, which move the control mode off 7
     (docs/PLANE_GROUND_OPS.md).
   - Then come final's on-ground half (`009B1FEA`-`009B207A`), `land/park` (vtable `00D1FF60`)
     and `land/abort`.
   - Re-pair begin and final together, with the rule in 5j.
2. **The hull inertia's hydro torque.** With the shape-derived inertia, `009329C0`'s AddTorque
   turns every hull (51.1). A packet has to predict:
   - the restoring moment and the rocking of a living hull;
   - the capsize of a sinking wreck;
   - the rise in hit records.
   Then the switch can be re-paired.
3. The W substitution in begin (the pilot bot's lifetime at `bot+80h`) is saturated. A host
   `bot+80h` would remove it.

### Working notes

- `local\l11_run.ps1 -Exe <exe> -Prefix <p> -Rows 'tag:MISSION:frames:mission_frames'` launches
  rows with the reference-k options.
- `local\l11_sweep.py <lo> <hi> <disp>` lists stores to `[reg+disp]` over a range.
- `local\l11_vcall.py <lo> <hi> <slot> <owner_disp>` finds a load from `+owner_disp` followed by a
  `vtable+slot` call. MSVC calls virtuals through a register, so a byte scan for `FF 50 xx` misses
  them.
- Traces: `land line trace` (every 10 ticks), `land begin trace` and `land final trace` (every 10),
  and `summary hull tilt` (one per ship).
- One smoke died at renderer init (exit 4) with the console active. It passed on the retry 10
  minutes later.

### Handoff supplement (cc9-lua11, after the pause notices)

The five pause messages reached me only after my turn ended. By then I had already bound
`cc9_hull_inertia`: it is committed OFF with its pairs and verdict (`5125fc9a6`, `5a68b4d19`,
`664b571f7`), and I took no work after it. Everything below is for the successor.

- **land/line (ON, landed as `97a9ac3ba`).** The height clause is reclassified in 5g as a spread
  miss of the vehicle response. The follow-up is the airframe's descent under `009FB800` and the
  follow speed law (the plane flight controller at unit+AB0h).
- **land/begin (OFF, landed as `a7035f516`) and land/final (OFF, landed as `b7564ea25`).**
  - Re-pair plan: begin and final flip together on LOMP10 9200/9000 and USN01, with the flip rule in
    5j.
  - Both need the touchdown first: plane `+BF8h`/`+BF4h` ground contact and `007CA3F0`
    BSP_Plane_HandleTouchdownOrCrash, which moves the control mode off 7. Without it every head
    flies on past T.
  - After the touchdown come final's on-ground half (`009B1FEA`-`009B207A`) and `land/park`
    (vtable `00D1FF60`: enter `009B21A0`, exit `009B21C0`, tick `009B22C0`, unread), then
    `land/abort` (`+64Ch`).
  - Labelled substitutions in begin:
    - W, the pilot bot's lifetime at `bot+80h`, is saturated; land tasks younger than 10 s are
      counted.
    - holder+8Ch is 0 and the owner velocity is 0 (a static airfield).
    - vtable[38h] is the live |v|.
    - The 007C07A0 direction hold has no consumer and is counted.
- **The launch-site arm (ON).** Section 5i.
- **The hull-inertia extent binding (OFF, not landed: `5125fc9a6`, `5a68b4d19`, `664b571f7`).**
  - It follows the 49.10 rule: `shape_count == 0` keeps the zero defaults.
  - The periscope exception (`009396BA`-`009399BF`, the node named `periszkop` when class+510h and
    +514h are both <= 0) is recorded and not bound. It affects i-400, kaiten, minisub, type7 and
    type_b, all without a root ConvexObject; `0071AD50`'s key form is unread.
  - Why OFF: the hydro torque of `009329C0` wakes (GUNNERY_OPEN_ITEMS 51.1).
- **Ghidra, to define (names provisional, exclusive ends, RET then INT3):**

  | range | name |
  | --- | --- |
  | `009B02E0`-`009B02ED` | BSP_BotStateLandLine_Enter |
  | `009B02F0`-`009B02F5` | BSP_BotStateLandLine_Exit |
  | `009B0300`-`009B08F5` | BSP_BotStateLandLine_Tick |
  | `009B13E0`-`009B13F7` | BSP_BotStateLandBegin_Enter |
  | `009B1D70`-`009B1DE3` | BSP_BotStateLandBegin_Tick |
  | `009B1E60`-`009B1E8B` | BSP_BotStateLandFinal_Enter |
  | `009B1E90`-`009B1E9E` | BSP_BotStateLandFinal_Exit |
  | `009B1ED0`-`009B211F` | BSP_BotStateLandFinal_Tick |
  | `006CE230`-`006CE23E` | BSP_AirOpsSite_StampTime |

- **Ghidra, to name (existing FUN_s):**

  | address | name |
  | --- | --- |
  | `009B1420` | BSP_BotStateLandBegin_Steer |
  | `009B3C00` | BSP_BotStateLandBegin_FinalRule |
  | `009B3370` | BSP_BotStateLandFinal_ParkRule |
  | `009AFAF0` | BSP_BotApproachLand_Geometry |
  | `006BCA80` | BSP_AirOpsHolder_PathPoint |
  | `009B1300` | BSP_BotApproachLand_MinSpeed |
  | `007DB4D0` | BSP_PlaneClass_LeaderTurnRadius |
  | `006CF100` | BSP_AirOpsSite_Construct |

## Handoff (cc9-lua12, 2026-09-29)

Branch `agent/cc9-lua12`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua12`. No lease
is held. The lead asked me to stop after `cc9_hull_flooding_trace`.

### Done

| packet | commits | switch | state | evidence |
| --- | --- | --- | --- | --- |
| `cc9_plane_touchdown` | `6369a63c4`, `6ad4b7fcd`, `257d94741` | `kPlaneTouchdownBound` | ON, gameplay identical | docs/SQUADRON_LAND_TASK.md 5k, 5l |
| `cc9_land_begin_final_repair` | `257d94741` | `kLandBeginStateBound`, `kLandFinalStateBound` | OFF: no touchdown | 5l |
| `cc9_land_park_state` | `aa541a3e1` | none | head read, not bound | 5m |
| `cc9_land_abort_state` | `aa541a3e1`, `f157b5478` | `kLandAbortStateBound` | OFF (mechanism held; flips with begin and final) | 5m |
| `cc9_landing_descent` | `046998bd5`, `e39d2cf1c`, `534019cd6` | none | no host substitution found | 5n |
| `cc9_squadron_ordnance_state` | `fd28a0931`, `0fdb01ab3`, `e52c36714` | `kSquadronOrdnanceReaderBound` | ON | docs/SQUADRON_ORDNANCE_STATE.md |
| `cc9_hull_flooding_trace` | `9ff6f59a8` | env `BSP_HULL_FLOODING_TRACE=1` | diagnostic | below |

The stale leak comment gunnery11 routed is fixed in `534019cd6`
(`src/game_hosts_units.cpp`, the ship leak gate).

### Open, in order

1. **The landing descent.** Under the image's laws as reconstructed, the ApproachPitch hold climbs
   at the final speed command `r = LevelFlight x StallSpd`, which is q = 1 (5n). The blend never
   lowers the command. Until something sinks the flare, the heads porpoise 4 to 16 m over the
   runway and begin, final, abort and park stay OFF.
   - Not yet read: the pitch arm's nose-up floor and slew in mode 1 (`0099DC9E`-`0099E512`) as
     they apply to the flare, and `007D9140`'s drag, which the host supplies.
2. **Park** (`009B22C0`-`009B2C68`) and final's on-ground half need a touchdown and the
   ground-roll arm `007CBFA0`, which the host does not run.
3. **The gear channel** `(+DECh)+28h` with its request `+C1Ch` (5k): only the touchdown gates read
   it.
4. **The begin W substitution** (`bot+80h`), unchanged.

### Working notes

- `local\l12_run.ps1 -Exe <exe> -Prefix <p> -Rows 'tag:MISSION:frames:mission_frames'` launches
  rows with the reference options. `local\l12_run_flood.ps1` adds `BSP_HULL_FLOODING_TRACE=1`.
- `local\l12_sweep.py`, `l12_calls.py` (rel32 and absolute references), `l12_abs.py` (absolute
  dwords in every section), `l12_jt.py` (MSVC two-level switch tables), `l12_dec.py` (uses of
  `+DECh`), `l12_flood_sum.py` (flooding summary).
- Diagnostics:
  - `land final flight` (every 10 final ticks, only while final is bound);
  - `summary plane touchdown`;
  - `plane ground contact first`;
  - `land abort trace` and `summary land abort` (abort bound);
  - `squadron ordnance` (reader ON, on change).
- Bash heredocs holding an apostrophe fail in this harness: write scripts and messages with the
  Write tool.

## Handoff (cc9-lua13, 2026-09-29)

Branch `agent/cc9-lua13`, worktree `J:\PROG\battlestations-pacific-decompile-cc9-lua13`. The lease
`cc9_plane_ground_roll` is released with this addendum.

### Done

| packet | commits | switch | state | evidence |
| --- | --- | --- | --- | --- |
| `cc9_landing_descent_2` | `546fe663d`, `4229cb934` | `kPlaneDirectionHoldBound`, with `kLandBeginStateBound`, `kLandFinalStateBound`, `kLandAbortStateBound` | ON: the heads land | docs/SQUADRON_LAND_TASK.md 5n (second pass), 5o |
| `cc9_land_begin_w` | `ec607a509` | none | W is the bot's age; the saturation is exact | 5p |
| `cc9_gear_channel_scope` | `88e341d87` | none | gates and final's done test only | 6 |
| `cc9_plane_ground_roll`, stage A | `33b530f29`, `638ff834f` | `kPlaneGroundRollBound` | ON: landed heads brake and stop | 5q |

### Open, in order

1. **Stage B of the ground roll: why the followers abort, then park.**
   - The abort is decided by the landing sequencer, not by runway occupancy. `006C7960` gives a
     follower mode 4 only with `rec.spacing_8 > 0`. `006C3F80`'s k = 0 site arm
     (`landing_spacing` code around `006C42F4`-`006C4405` in `src/game_hosts_units.cpp`) zeroes
     it when `tp < 0.25 x FollowDistTime` (9 s) and the time since the site's last touchdown
     stamp (`006CE230`, at the leader's touchdown) is under `0.4 x FollowDistTime`.
   - Lightning 01|.-4 aborted at 165.41 s, 0.25 s after Lightning 01's touchdown, at A8 708 m and
     68 m/s. Check which arm its record took (k and `tp = path_4 / speed`) before assuming the
     occupancy vector matters. `site_occupants_34` is only appended to (`006CED90`); no reader was
     found in this host.
   - Park: vtable `00D1FF60`, tick `009B22C0`-`009B2C68`. Its taxi math is unread (docs/AIRFIELD_TAXI.md 6
     and 8). So are the site calls `006CF420` (taxi target, the last point of the hangar exit
     path), `006CF520` (queue origin) and `006CF5B0` (vtable `34h`). The host carries no hangar
     paths yet.
   - The rule's first arm (`009B3D38`, `+900h` 4 or 5) is what refuses park today.
2. **Lightning 01's approach moves when Warhawk 01 stops** (5q verdict, 4). Some planner term
   reads the stopped plane's position or velocity; it was not identified.
3. **The ground laws not carried** (5q): the runway steering band `007DA380`, the rate law's
   ground arm `007DA542`, the ground pose (`ctl+80h..8Ch`, `007D9C80`, `007D80C0`), the wire
   (class-9 holders), and the lift-off message `007C7110`.
4. **The direction hold's launch arm** `007C705C` (0.8 s at BeginFlying), not bound (5o).

### Working notes

- `local\l13_run.ps1 -Exe <exe> -Prefix <p> -Rows 'tag:MISSION:frames:mission_frames'` launches
  rows with the reference options (a copy of cc9-lua12's `l12_run.ps1`).
- `local\l13_calls.py <addr>...` finds rel32 and absolute references (from `l12_calls.py`).
- Listings saved in `local\`: `l13_core.txt` (`007DB680` whole), `l13_d8470.txt` (`007D8470`
  whole), `l13_final.txt` (`009B1ED0` onward).
- Diagnostics:
  - `summary plane direction hold` and `summary plane ground roll`, one per landing plane;
  - `hold=` in `land final trace`.
- Pair logs: `local\l13_off_*`/`l13_jon_*` (5o) and `local\l13_goff_*`/`l13_gon_*` (5q).

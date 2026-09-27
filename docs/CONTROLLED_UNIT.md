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
  - The initial cursor is (0, -1) (see above).

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

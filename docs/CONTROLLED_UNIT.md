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

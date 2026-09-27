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

(filled after the runs)

# CC13 GPR 47: segment 47 reference models (plane damage and crash)

Packet `cc13-gpr47` (cc13 lineage, Claude). Segment 47 is `007B2DD0`-`007D1DC0`. cc9 approved this
reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg47_cc13.cpp` and `include/bsp/gameplay_seg47_cc13.hpp`,
  registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

Skipped by the terms: the plane crash sweep `007C0910`, the water power-lost arms `007CB7F0` and
`007CBE02` (both hosted), the carrier-landing chain (cc9's lua56), the plane bot tasks and the air
operations. Met and left alone: `007CA3F0` (the touchdown/ground-state step, hosted as
`ground_state_from_locked_007ca3f0`), `007C71E0`, `007CB5F0`, `007C3EA0` (touchdown and land arms),
the rocket-task band `007B3BA0`-`007B7FD0`, and the path/net-path band `007B2DD0`-`007B3A80`.

Method. Each comparison was re-read from the disk bytes with Capstone `disasm-raw`; the x87
compares (`FCOMI`/`JA`, `COMISS`/`JBE`) are transcribed with their NaN direction. The constants were
read from the PE on disk. The models are **reconstructed and build-tested**. None is ABI-compatible,
wired or game-validated. Every value the image stores is a float, so the models compute in float.

## 1. Models

### 1.1 The named-effect dispatch (`007BBFA0`, plane vtable `+194h`)

`__thiscall(unit, NativeString* name, LuaObject* args)`, `RET 8`, always returns `AL = 1`. The death
mode chooser `007CA8A0` calls it with `args = 0`; Lua and the water arms call it with a name.

- **The forced predicate (BL, `007BBFC7`-`007BC031`).** It is true when the name is `"explosion"`, or
  the flight state `unit+900h` is not 4..7, or the controller gate `(unit+72Ch)->vtable[38h]()` is
  clear and `0.2777778f > unit+B1Ch` (`00CF8918`, 1 km/h in m/s; `unit->vtable[38h]` is
  `007B8E60`, `FLD [ECX+B1Ch]`). `FCOMI`/`JA`: a NaN speed does not force.
- **The explosion arm.** A forced call, whatever its name, or `"explosion_delayed"`, sets `unit+C10h`
  and routes message `63h` with `+20h = C10h`. Only an unforced `"explosion_delayed"` on a plane whose
  `unit+DFCh` (rammed) is clear draws the delay: `U([00E18710], [00E1870C])` on **stream 0** at
  `007BC3C7`. Every other case stores `-1.0f`.
- **`"splash"`** routes message `64h` (`0075C450`, `+20h = 256.0f`, `00CF8B3C`).
- **`"enginefire"`** returns at once when `[unit+538h]+7Ch <= 0` (`JLE`). The engine index is Lua
  argument 2 when `args` is set, else `NextU32` on **stream 1** (`ECX = EDI + 2 = 1`, `007BC0FC`)
  modulo the count (unsigned `DIV`). A Lua index of -1 also draws; a Lua index is not range-checked.
  Message `65h` carries `+20h = 99.0f` (`00D059A0`) and `+24h = index`. `C10h` is not written here.
- **`"powerlost"`**: `C10h = 1000.0f`, message `66h` with `+20h = 1000.0f`.
- **`"spinning"`**: `C10h = 1000.0f`; when `unit+C36h` is clear, side `= 0.5f > U(0, 1)` on
  **stream 1** (`007BC1F2`) and message `67h` (`+20h = C10h`, `+24h = side`). `"leftspinning"` and
  `"rightspinning"` pass side 0 and 1 with no draw.
- The power-lost and the three spinning arms then route `007BA8D0`'s message `C8h`
  (`PLANE_DEADMEATROT`) when `[00E188A8]+1FE4h != 0`; engine fire and splash do not.

### 1.2 The failure query (`007BC460`, vtable `+198h`)

`"enginefire"` writes 0 and answers false; `"powerlost"` writes `C39h` and answers true; `"spinning"`
writes `C36h` and answers true; any other name answers false and leaves the output alone.

### 1.3 The failure message handler (`007D0B80`, vtable `+208h`)

Each message class's IsKindOf slot `+0Ch` answers its own kind, `62h` and `49h` (`0075C400`,
`0075C4D0`, `0075A1A0`), so the handler's kind tests are exclusive.

1. `C10h = msg+20h` whenever the session mode is 2 or `unit+5Dh` (dead) is clear. So a live plane's
   engine fire sets `C10h = 99.0f`, and a splash sets `C10h = 256.0f`.
2. `unit+5Eh` set: return.
3. Kind `66h`, or kind `63h` with `C10h > 0.0f` (`COMISS`/`JBE`, `00D7A218` is 0.0) **and** the
   controller gate: stop every engine (`007DE600`), `C39h = 1`, and clear `969h` when the plane has a
   model (`[[unit+360h]+160h]`).
4. Kind `63h` otherwise: when `unit+100h > -5.0f` (`00CFBC84`; `unit+FCh` is the position, so this is
   y above -5 m), the marker service (`00498F80` into `C4Ch`, then `0049A760`, only when
   `[[00E188A8]+19CCh]+4ACh`), then the class explosion effect (class `+214h`, `00868420`).
   Then `Kill(unit, 1)` (`00926D90`) and `C10h = -1.0f`.
5. Kind `64h`: `007BC4E0` (bind the crash effect), stop every engine, `C39h = 1`.
6. Kind `65h`: `007EB0B0(msg+24h)` destroys that engine; when dead, not removed and `C4Ch` is null,
   create the marker. **Jumps past the tail.**
7. Kind `67h`: `C36h = 1`. Without a model, **jumps past the tail**. With one, `msg+24h != 0` sets
   `C37h` and breaks the right wing (`"rwing"`, `"aileron_r"`, `"jobbszarny letorik"`), else `C38h`
   and the left wing (`"lwing"`, `"aileron_l"`, `"balszarny letorik"`). The break hides the wing's
   parts, spawns a point effect at their centre, calls `007CAAD0` per `"...letorik"` node, then
   `007D0820`.
8. The tail `007D12FC`: when `C3Ah` and `5Dh` are both clear, `C3Ah = 1`, `unit->vtable[70h](1)`, and
   `9E8h = 0` (the pitch input) when `0.1745329f > unit+C68h`.

### 1.4 Hit and destroy (`007BBCF0` vtable `+ECh`; `007BCAA0` vtable `+7Ch`)

`007BBCF0` clears `520h`, latches `DFCh` when the shooter's `vtable[5Ch](5)` answers, registers the HUD
damage direction when `[00E188D8] == unit+9D4h` (true also when both are null), forwards to the
squadron's `vtable[ECh]` when `C3Ah`, `5Dh` are clear and the squadron is set, calls `00999AA0` when
`DF4h` is set, copies `[[hit+38h]+4h]` into `800h` (the spin side `007CA8A0` reads) when
`hit+34h != -1`, then calls `008777D0`.

`007BCAA0` draws `C14h = U([00E18718], [00E18714])` on **stream 0** at `007BCABB` (the dead-engine
fire delay, read by `007CAF10` at `007CAF7E`, `007CAFD0`, `007CB02C`), copies `1B0h` to `748h` unless it
is 8, calls `00959450`, removes the plane from its squadron (`007F3970(unit, 0)`), then retires `DF4h`
and `6DCh` through their `vtable[10h]` and nulls them.

### 1.5 The leave-map countdown (`007C6C30`, vtable `+1D8h`) and `007CA790`

Dead planes return at once. A plane inside the map (`0071C4F0` false), **or one with a squadron that
is not leaving on purpose** (`007EE2E0(1)` false), resets: `6F0h = tuning+34Ch`, `6ECh = 1`, and the
squadron's `+36Ch = 0` when `9D8h` is null. Otherwise `6F0h -= step` (float), and state 3 with
`6F0h = -1.0f` when `0059C9B0(pos, tuning+348h)` or `0 > 6F0h`, else state 2. With `9D8h` null the
squadron's `+36Ch` counts down for the local player's role and is pinned to 1.0f otherwise; when the
old state was 1 or `+36Ch < 0`, `00982120(unit, 0)` warns and `+36Ch` is reseeded from
`tuning+350h`. Finally `0 > 6F0h` sends `007EE410(squadron, unit, 00489C40(pos))` each tick. Note
that `ECX = [unit+9D4h]` there even when it is null; the host would need to guard that.

So a squadron plane that strays out of bounds is never destroyed by this countdown; a squadron that
is leaving on purpose, or a plane without a squadron, is. The host's substitution
(`src/game_hosts_units.cpp:31102`) runs it for retreating planes only, which leaves out the
squadron-less planes.

`007CA790` is the message's handler: under the mission-entity lock it sets the Lua
`self.ExitZoneParty = party`, then calls `007F31A0(unit, party)` when `9D4h` is set.

### 1.6 The debris spec (`007CAAD0`)

For one scene node, `007CAAD0` scans the model's record pairs `[model+50h..+54h]` (record, node). For
a non-null record it copies the node's world matrix (`+F0h`), adds `U(-0.001, 0.001)` to x, y and z of
its translation (`007CAC5D`, `007CAC8B`, `007CACB9`), and builds the spec `00447510` reads:
`+4h = 15.0f`, `+8h` the plane's velocity (`unit->vtable[34h]`), `+14h..+1Ch` three `U(-1, 1)`
(`007CAD58`, `007CAD78`, `007CAD98`), `+24h..+2Ch` record `+14h..+1Ch`, `+30h = -50.0f`,
`+54h = FLT_MAX`. All six draws are **stream 1**. How `00447510` consumes the jittered matrix copy
was not traced; the model returns it as `position`, a hypothesis.

### 1.7 Control factors (`007BBDC0` vtable `+1A0h`, `007BBF10` vtable `+1A4h`)

Neither had a Ghidra function: `scan-bytes` attributed both to `007BBCF0`, whose body ends at
`007BBDBA`. The plane vtable `00D19D28` holds them at `+1A0h` (`00D19EC8`) and `+1A4h` (`00D19ECC`);
their callers are the Lua natives SetControlFactor / GetControlFactor (`0088EB50`, `0088EDC0`). The
ship pair is `gameplay_seg50_cc13` (`008131E0` / `00813250`).

The block is `unit+BCCh..BE4h`: roll `BCCh`, pitch `BD0h`, the power mean `BD4h`, and four engine power
values from `BD8h`. The setter (`RET 0Ch`, `(name, index, value)`):
- `"roll"` (`__stricmp`) stores `BCCh`; `"pitch"` stores `BD0h`;
- `"power"` calls `007B8140(index, [unit+538h]+140h, value)`, which stores `BD8h + index*4` and sets
  `BD4h` to the float running sum of the first `count` entries over `FIDIV count`; then, when
  `0.0f >= value` (`COMISS`/`JB`, NaN excluded), `007DE5E0(index)` stops that engine (that function's
  ledger entry said "unreferenced"; this is its caller);
- any other name returns `AL = 0` with no write; a match broadcasts message `C9h`
  (`PLANE_CONTROLFACTOR`, vtable `00D02E20`) with the seven floats through `0077C7B0(msg, 0)`.

The getter compares `"roll"`, `"Pitch"`, `"Power"` (the last with the unchecked index). `007CAF10`
subtracts `BCCh`, `BD0h` and `BD4h` from its three latched control magnitudes (`007CB226`, `007CB27A`,
`007CB2B1`), which the host models as `effective_control_007caf10` (`src/pilot_controls.cpp:130`).
`007B97F0` (RestoreFullHealth) calls `00878340`, sets the seven factors to 1.0f and `BE8h..BF0h` to
0.0f, then tail-jumps to `007DE630` (the engine list's ResetAll).

### 1.8 The dead plane's splash (`007CC2F0` tail, `007CC375`-`007CC41F`)

A dead plane (`5Dh`), unless the session mode is 2, skips the landing probe and the water contact.
When its latch `unit+911h` is clear and `3.0f > unit+100h` (`00CE3854`), it raises `"splash"` through
`vtable[194h]` and sets the latch. With `007BBFA0` and `007D0B80` that is: message `64h` (its
`+20h = 256` is not copied, the plane being dead outside session mode 2), the crash effect `007BC4E0`, every engine stopped, `C39h = 1`. The live branch (landing probe, the
`C5h` touch message, `007CB7F0`) is outside the model.

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `named_effect_007bbfa0` | `plane_death_timer_c10_007bbfa0` (`src/plane_death_modes.cpp:39`) and the mode-to-message step at `src/game_hosts_units.cpp:17261` | the effect name; `900h`; the controller gate; `B1Ch`; `DFCh`; `C36h`; class `+7Ch`; the Lua index; the session mode; `[00E18710]`/`[00E1870C]` | **stream 0** `007BC3C7` (the delay draw the host already makes, key (unit, 0) at `src/game_hosts_gunnery.cpp:1702`); stream 1 `007BC1F2` (`"spinning"` only, which `007CA8A0` never raises) and `007BC0FC` (engine fire without a Lua index) | OFF: none. ON: a plane that dies outside flight states 4..7 (landed, parked, deck) or below 1 km/h with the gate clear explodes at once, with no stream-0 delay draw. That removes one stream-0 draw per such death, shifting later consumers. |
| `failure_query_007bc460` | none | `C39h`, `C36h` | none | none (Lua query only) |
| `state_message_007d0b80` | `plane_death_flags_007d0b80` (`src/plane_death_modes.cpp:51`) | the message; `C10h`, `5Dh`, `5Eh`, `C36h`..`C3Ah`, `969h`, `C4Ch`, `C68h`; the gate; `unit+100h`; the session mode | none here (the wing break's debris draws stream 1, see `debris_spec_007caad0`) | OFF: none. ON: a delayed explosion while the gate is clear kills at once instead of powering down; engine fire and the model-less spin do not raise the death (`C3Ah`); a splash sets `C10h = 256` and an engine fire `C10h = 99`, which `007CAF10` then compares with the dead timer. |
| `plane_hit_record_007bbcf0` | the state writes of `notify_plane_hit_007bbcf0` (`src/game_hosts_gunnery.cpp:2815`) | the hit record (`+4h`, `+34h`, `+38h`); `[00E188D8]`; `9D4h`, `C3Ah`, `5Dh`, `DF4h` | none | OFF: none. ON: the spin side `800h` and the rammed latch `DFCh` follow every hit, which `007CA8A0` reads. |
| `plane_on_destroyed_007bcaa0` | `squadron_leave_on_death_007bcaa0` (`src/game_hosts_units.cpp:6730`) | `1B0h`; `[00E18718]`/`[00E18714]`; `9D4h`, `DF4h`, `6DCh` | **stream 0**, one draw per destroyed plane at `007BCABB` | OFF: none. ON: every plane destroy draws once on stream 0, shifting every later stream-0 consumer. Bind it on stream 0 only with the dead-engine fire it feeds (`007CAF10`); otherwise keep it OFF. |
| `out_of_action_countdown_007c6c30`, `exit_zone_007ca790` | the empty override `src/game_hosts_units.cpp:17364` and the retreat-only SUBSTITUTION at `:31102` | GameTuning `+348h`, `+34Ch`, `+350h`; `6F0h`, `6ECh`, `9D4h`, `9D8h`, the squadron's `+36Ch`; the map tests `0071C4F0`, `0059C9B0` | none | OFF: none. ON: planes without a squadron that leave the map are removed through `007EE410`; retreating squadrons behave as the substitution does. |
| `set_control_factor_007bbdc0`, `get_control_factor_007bbf10`, `restore_full_health_007b97f0` | none: SetControlFactor / GetControlFactor are listed unbound in `src/mission_lua_host.cpp:45-46`; the host feeds `effective_control_007caf10` (`src/pilot_controls.cpp:130`) its own loss values | the name, index, value; class `+140h`; the factor block | none (`007DE630`'s rotor Resets draw stream 1; see gameplay_seg48_cc13) | OFF: none. ON: a script's power factor of 0 stops that engine, and the factors limit the three control magnitudes in `007CAF10`. A full-health restore resets them to 1.0. |
| `free_flight_tail_007cc2f0` | none (the host has no dead-plane splash) | `5Dh`, the session mode, `911h`, `unit+100h` | none | OFF: none. ON: a dead plane that falls below y = 3.0 runs the splash chain once: the crash effect `007BC4E0`, the engines stop, `C39h = 1`; `C10h` keeps its value. |
| `debris_spec_007caad0` | none (the host builds no wing break) | the node, its record, the plane's velocity | **stream 1**, six draws per debris node | OFF: none. ON with the wing break: every right/left spin death shifts stream 1 by 6 x the model's `"letorik"` node count. |

## 3. For cc9 (no host edits made)

- **`include/bsp/plane_death_modes.hpp:51` says every arm of `007D0B80` reaches the tail `007D12FC`.**
  The engine-fire arm (`007D0D9A`) and the spinning arm without a model (`007D0DBB`) jump to
  `007D1339` and skip it. The power-lost arm also needs the controller gate (`007D0C0A`), and its
  threshold is `[00D7A218] = 0.0`.
- **The death step's `C10h` inputs.** Engine fire and splash messages overwrite `C10h` (99, 256) on a
  live plane, so `007CAF10`'s `0 < C10h < C3Ch` explosion test fires 99 s (or 256 s) after death for
  those paths.
- **`007BCAA0` draws on stream 0** at every destroy. The host's death block does not make that draw.

## 4. Ledger

Records added (`config/reconstruction/`, `semantic_build_tested`, source `src/gameplay_seg47_cc13.cpp`):
`007BBFA0`, `007BC460`, `007D0B80`, `007BBCF0`, `007BCAA0`, `007C6C30`, `007CA790`, `007BBDC0`,
`007BBF10`, `007B97F0` (complete) and `007CAAD0`, `007CC2F0` (partial).

Host pointer record, checked against the listing (`007CA8C1`-`007CAA9D`):
`007CA8A0` -> `plane_death_mode_007ca8a0` (`src/plane_death_modes.cpp:19`). One divergence: the image's
`COMISS`/`JB` returns on a NaN health, the host continues.

Names added (`config/names/`, no model): see section 5.

## 5. Named, not reconstructed

| address | name | evidence |
| --- | --- | --- |
| `007D0820` | `BSP_Plane_HideOtherMeshHolders` | collects the note nodes engine, rotor, rotor_still, rotor_still_dam (and numbered copies through `007D06A0`), aileron_l/r, kulsocockpit, belsocockpit; then hides every `"MeshHolder"` child of `[unit+4A4h]+34h` not in that list (visibility 0, `vtable[2Ch]` with 100000.0). Called at the end of the wing break. |
| `007D06A0` | `BSP_Model_CollectNumberedNoteNodes` | builds `prefix + itoa(i)` and appends each found note node. |
| `007D05E0` | `BSP_Node_AppendChildrenByNamePrefix` | `BSP_Node_CollectChildrenByNamePrefix` then appends each to the caller's vector. |
| `007D1C90` | `BSP_PlaneUnitInstance_Create` | `operator new(0E94h)`, `memset 0`, `BSP_PlaneUnitInstance_Construct`. |
| `007D0350` | `BSP_Plane_GetField93C` | returns `[ECX+93Ch]`. |

## 6. Verification

- `scripts/build.ps1` in this tree: passed (`bsp_game.exe` linked, `gameplay_seg47_cc13.obj` built,
  ctest 2/2).
- `tools/verify_report_calls.py` takes a JSON report, so the call sites this doc cites were written to
  the tree-local `local/g47_calls.json` (32 rows: every RNG draw site, the message constructors, the
  route, and the callbacks of `007D0B80`, `007BCAA0`, `007C6C30`): 32 checked, 0 failed.
- The control-factor call sites (`007BBE65` -> `007B8140`, `007BBE7B` -> `007DE5E0`, `007BBED8` ->
  `0077C7B0`) were read from the disk bytes only: the verifier needs a Ghidra function, and
  `007BBDC0`/`007BBF10` have none. Their names (`BSP_Plane_SetControlFactor`,
  `BSP_Plane_GetControlFactor`) are in the names ledger only; `ghidra_annotate.py` refuses an address
  with no function, and creating the two functions was left to the lead.
- Ghidra: the five names of section 5 applied with `tools/ghidra_annotate.py --apply` (prior values in
  the tree-local `local/ghidra-annotations-20261013T094334Z.json`).

## 7. Open and handoff

- `007C4DD0` (engine effects, 1136 lines), `007C3700` (bow wave), `007CC580` (OnKilled, effect
  teardown) and `007CBA50`'s other arms are presentation and were not modelled.
- `0049A760`/`00498F80` (the `C4Ch` marker service on `[00E188A8]+21D0h`) is unread.
- `007BA450`, `007BAD00` and `007BBA10` stay raw (see docs/CC13_GP_007A_007F.md).

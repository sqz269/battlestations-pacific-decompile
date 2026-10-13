# CC13 GPR 50: segment 50 reference models (barrel counts, ship command availability, control factors)

Packet `cc13-gpr50` (cc13 lineage, Claude). Segment 50 is `0080DA00`-`0081AA10` (keywords steeringjam,
enginejam, periscope, torpedostock, thrust). cc9 approved this reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg50_cc13.cpp` and `include/bsp/gameplay_seg50_cc13.hpp`,
  registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

Most of the segment's gameplay is already hosted:
- The jams and failures are in `src/game_hosts_gunnery.cpp` (packets cc9_component_failures and
  cc13_natives1, `script_set_failure_008132c0` at `:11095`).
- Torpedo stock and loaded tubes are in the gunnery host (`:2952`, `:3010`).
- The turn circle, torpedo threats and ready torpedo barrels are in the ship AI host.
- The transport-ready test is in the AI host.

The work here is therefore: models for the unhosted bodies with gameplay reach, and pointer records for
hosted bodies after checking them against the listing. Every comparison and constant below was read from the
disk bytes (Capstone `disasm-raw`; strings and lists from the PE). Everything is **reconstructed and
build-tested**. None of it is ABI-compatible, wired or game-validated.

## 1. Models

### 1.1 Barrel counts (`00812310`..`00812730`)

These are the inputs that `band_0080_cc13::refresh_firepower_ratios_008127e0` takes as given. The totals
are stored by `0081F980` at unit `+103Ch`..`+1048h`. The live sums are read only by `008127E0`. The ratios
it writes (`+101Ch`..`+1028h`) are read by the Lua native `GetFirepower` (`0088DC10`).

The device iterator `00952080`/`009520E0`/`00952140` walks a category list (ended by a value of at least
0Ch) and, for each category c, the unit's linked list at `unit+390h + c*0Ch` (count `+394h`, head `+398h`,
node next `+4h`, device `+8h`). The lists, read from the PE:

| list | categories |
| --- | --- |
| `00E0A4F8` artillery | 1, 2, 3, 4, 6 |
| `00E0A510` AA | 1, 5, 6 |
| `00E0A520` torpedo | 7 |
| `00E0A528` depth charge | 8, 9 |

| address | model | what it sums |
| --- | --- | --- |
| `00812310`, `008123A0`, `00812430`, `008124C0` | `total_*_barrels_*` | `FILD dev+448h` (the barrel count) over artillery, AA, torpedo and depth charge |
| `00812550`, `00812680`, `00812730` | `live_*_barrels_*` | the same, but a device with `+3B8h` (disabled) set adds 0 |
| `00812600` | `aa_mount_count_00812600` | 1.0 (qword `00D7A210`) per AA-list device, disabled or not |

Each step is `FILD int; FADD float; FSTP float`. That is exact for integer totals below 2^24, so the walk
order does not change the result. Categories 1 and 6 are in both the artillery and the AA lists, so such a
device counts in both.

### 1.2 Ship command availability, vtable[168h] (`00812B00`)

It is installed in eight ship vtables (`00CFC3D0`+168h and seven more), and the mothership's `007582F0`
calls it too. It is the ship counterpart of the fort's `00743F50` (docs/CC13_GPR_41.md). The compares are
`00438E10`, in this order:

| command | answer |
| --- | --- |
| `settarget`, `clearorders` | true |
| `cleartarget` | the weapon director `[+738h]->vtable[2Ch]()` is non-zero |
| `stop` | true |
| `artillery` | a `+48h`/`+44h` child of kind 20h that is an artillery kind (`005459B0`) and operational (`00729F10`) |
| `moveto`, `attackmove`, `stop` (a second, unreachable compare at `00812BE0`), `retreat`, `repair`, `ZigZag` | true |
| `torpedo` | a kind 20h child with class function `[+3F4h]+80h == 7` that is operational |
| anything else | false |

`ship_ai_states.hpp`'s `ShipAiCommandAvailabilityHost::entity_offers_command_vtable168` is this call for
ships.

### 1.3 Control factors, vtable[1A0h] `008131E0` and [1A4h] `00813250`

`"engine"` (`00CE4438`; `__stricmp` on the data, skipped when the data is null) selects `+9D8h`.
`"steering"` (`00CE4408`; `00425850` on the native string) selects `+9DCh`. The setter stores the value;
the getter (no Ghidra function, `00813250`-`008132B0`) copies it out. Both answer 1 on a match, else 0.
- The only callers are the Lua natives `SetControlFactor` (`0088EB50`, `0088ED30`) and `GetControlFactor`
  (`0088EDC0`, `0088EF56`).
- None of the 621 `.lua` files in this installation calls either, so `+9D8h`/`+9DCh` keep their constructor
  values in every shipped mission.
- `008127E0` reads both (band_0080_cc13) and `007B8AD0` tests `+9D8h == 0` (`src/bot_tasks.cpp:116`).

### 1.4 Avoid-zone point test, vtable[170h] (`00812C90`)

It takes x (`+0h`) and z (`+8h`) of the point and asks the class navigation group at depth 0 (`0082ADA0(0)`)
for a containing zone (`004178F0`). It answers true when there is none.

### 1.5 Helm record keys (`00815520`)

`{thrust +0h, toTurn +4h}` (`00D09478`, `00D09470`), both type 2. `00815520` reads them through the
archive's vtable[10h] (`{0, key}`, `{2, &field}`); its sole caller is `0081F980`. `00815590` (no Ghidra
function) writes the values through vtable[0Ch].

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| barrel counts | none; `band_0080_cc13.hpp:122` (`FirepowerInputs`) takes them as given | the unit's guns: category, barrel count `+448h`, disabled `+3B8h` (the gunnery host's per-gun state) | none | OFF: none. ON with `GetFirepower` bound (a Lua native, cc9's lane): scripts see live/total firepower ratios. Without that binding there is no consumer. |
| `ship_command_available_00812b00` | none; `ShipAiCommandAvailabilityHost` (`include/bsp/ship_ai_states.hpp`) has no host implementation. The script-order host asks `entity_may_follow_target_00779d50` directly (`src/game_hosts_script_orders.cpp:3046`). | the command token; the director's target; the ship's guns (kind 20h, artillery kind, operational, class function) | none | OFF: none. ON: when `008162B0` is bound for ships, commands outside the twelve names are refused. `artillery`/`torpedo` are refused once no operational gun of that kind remains, and `cleartarget` once the director has no target. |
| `set_control_factor_008131e0`, `get_control_factor_00813250` | none; the natives are unbound (`src/mission_lua_host.cpp:45`-`46`) | the name string and value | none | none in this installation: no script calls SetControlFactor |
| `point_outside_avoid_zones_00812c90` | none found (the callers of vtable[170h] were not traced) | the avoid zones (`include/bsp/avoid_zone_boundary.hpp`) | none | none known until a caller is bound |
| `kHelmSaveKeys_00815520` | none (save I/O) | the helm record | none | none |

## 3. Pointer records (hosted; checked against the listing)

| address | host | check |
| --- | --- | --- |
| `00811A30` | `game_hosts_ship_ai.cpp:864`, `:9846` (`0082E960(throttle)`) | the `[00F88C30]` modifier arm (`00811A3B`..`00811A81`) divides by 1.0 because `00F88C30` is never created in single player; this matches the null branch `00811A92` |
| `0080DF40` | `game_hosts_ship_ai.cpp:4740` | `00729F10` at `0080DF59`, `00727D70(0.0)` at `0080DF6A`, summed over `+3ECh`'s list |
| `00810E90` | `game_hosts_gunnery.cpp:2952` | the `+3ECh` device list (`00810E91`), barrel count `+448h` read at `00810EB6` |
| `008120E0` | `approach_target_ref.cpp:203` | cites `008120F8`..`00812101` in the body |
| `008128E0` | `game_hosts_ai.cpp:3374` | class vtable[2Ch]; `+1124h > 0` refuses (COMISS/JA `0081290B`); `006F2C30(&+FCh, +54h, 2)` then `006F2A50(0)` |
| `00814390`, `00814420` | `game_hosts_ship_ai.cpp:1798`, `:5669` | the host's notes match the body ranges |
| `008132C0` (partial) | `game_hosts_gunnery.cpp:11095` | `push 6Bh` at `008134C4` (SteeringJam); Explosion (6Ah) and Periscope per the host's notes |

`008173E0`, `00810630`, `00811150`, `00813950` and `008193A0` are referenced by hosts too, but their
host bodies were not checked here, so they have no records yet.

## 4. Open questions

- **The helm estimator** (`00810900`, `00810A90`, `00810CF0`; ship+`FACh`; constructor `0080DE00`, reset
  `0080DEA0`) is ship vtable[4Ch]. It infers a rudder estimate (`+FDCh`) and a throttle estimate (`+FC4h`)
  from observed headings and positions. Its only readers found are `BSP_UnitInstance_UpdatePropellers`
  (`00834E90`), so it is presentation (propellers and rudder). It is not modelled.
- `00815EE0` FlipPendingAiOrderSlot (the 54h-byte order double buffer at `+A98h`/`+B38h`/`+B40h`) has no
  direct caller found.
- `008129E0` (a record-kind-1 test against kind 29h entities whose command `+174h`+8 is 4, 5, 6, 7, 9 or
  0Ah) and `00812960` (HUD) were left.
- `00818110` ShipExplosionFailureTask is referenced by the gunnery host (`:2792`, `:11063`); its record waits
  for that check.

## 5. Ledger

Models (`config/reconstruction`): `00812310`, `008123A0`, `00812430`, `008124C0`, `00812550`, `00812600`,
`00812680`, `00812730`, `00812B00`, `008131E0`, `00813250`, `00812C90`, `00815520`. Host pointers:
`00811A30`, `00810E90`, `008120E0`, `008128E0`, `00814390`, `00814420`, `0080DF40`, `008132C0` (partial).

## 6. Verification

- `./scripts/build.ps1` passed in this tree (bsp_game built with `gameplay_seg50_cc13.cpp`; ctest 2 of 2). No
  test was added.
- `tools/verify_report_calls.py local/g50_calls.json` checked 15 call-site rows with 0 failures. The file is
  not committed.
- `tools/x87_dc_form_scan.py --include-st0` finds no DC-form site in `00812310`-`00812C90` or in
  `008131E0`-`008132C0`.

## 7. Handoff

Done: 13 models (eight barrel counts, ship command availability, the control factor setter and getter, the
avoid-zone test, the helm keys) and 8 host pointers. Left, by reach:
- Check and record the five hosted bodies listed at the end of section 3, and `00818110`.
- The helm estimator (presentation) and `00815EE0` (no caller found).
- About 60 functions that are message constructors and serializers (SHIP_CREATE, SHIP_GUNS_SYNC, the unit
  state message), STL or template helpers, and the unnamed `00811820`, `008138A0`, `008160F0` and `00819CC0`.

# TorpedoDirector's submarine class override

Packet `cc11_director_class8`, 2026-10-06. Owned addresses: `00822C20`,
`00853050`. Ghidra was read only, with target `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe` verified through `bsp.py`.

**Finding:** the native explicitly enables a submarine's torpedo director even
when its bag says `TorpedoDirector = false`. The test is the constructed unit's
`vt[5Ch](8)`, not a scene class token or a property-group name. The scene host
currently retains false from `Sub(Ship)` without that test. The candidate fix
uses the same resolved `VehicleClass.Type` descriptor as `create_units` and the
existing decoded kind predicate. **`kShipDirectorClass8Bound` is OFF pending a
JM06 3000-frame OFF/ON pair.** The existing director switch remains unchanged.

## Native producer and consumer

| Routine | Coverage | Evidence |
| --- | --- | --- |
| `00822C20` | partial: `00823924..0082395A`, message stores/publication through `00823B6B`; other initialization arms not reconstructed | Torpedo bag lookup, absence, false/class-probe branch |
| `00853050` (`00853050..0085308A`) | complete | Submarine vtable slot `5Ch`, accepted query set and `RET 4` |
| `00852F10` | complete supporting read | Installs primary vtable `00D0BF80`; writes class id 8 at `+C4h` |
| `008531A0` | complete supporting read | Descriptor allocator calls `00852F10` |
| `00853630` | partial: entry through `0085364C`; remaining submarine initialization unread in this packet | Submarine init calls base `00822C20` |
| `004F05F0` | complete supporting read | Creator resolves the bag's Type through `00964790` |
| `00964790` | partial: Type read and Submarine comparison/constructor branch | The leaf follows `VehicleClass.Type` |

At `00823932`, `00822C20` calls `008F2260` for `TorpedoDirector`:

1. Missing record: `00823939` branches directly to `00823955`, storing 1.
2. Present nonzero `record+0Ch`: `0082393F` also branches to `00823955`.
3. Present zero: `00823941..0082394A` loads the unit's slot `5Ch`, passes 8,
   and calls it with ECX = the unit. `0082394E` seeds the local byte with 0;
   a nonzero AL falls through to the store of 1 at `00823955`.

`00823B02` puts that local byte into state-message `+42h`; the controller slot
`3Ch` call at `00823B6B` submits it. The earlier packet established the
message/controller stores and sparse shipyard bag. Nothing here changes the
native absence default, including shipyard-built Gyoraitei.

The class probe is resolved from the object producer:

- `004F0601` finds Type; `004F060D` passes its integer to `00964790`.
- `00964790` reads `VehicleClass[classIndex].Type`. At `00964AFD..00964B06`
  it compares with `00CEB7B0` (`Submarine`), then calls descriptor constructor
  `00963DB0` at `00964B2F`.
- That constructor installs descriptor vtable `00D1AE38`. Slot `+28h`,
  `00D1AE60`, contains `008531A0`, whose call at `008531ED` constructs the unit.
- `00852F10` installs unit vtable `00D0BF80` and writes `unit+C4h = 8` at
  `00852F96`. Vtable slot `00D0BFDC` contains **`00853050`**. Slot `+A0h`
  (`00D0C020`) contains `00853630`, which calls base init at `0085364C`.
- `00853050` accepts `{0,1,2,4,5,6,8}` plus the dynamic `+C4h` value.
  `CMP EAX,8` at `00853054` reaches `MOV EAX,1` at `00853083`; return at
  `00853088` is `RET 4`. Thus an actual submarine answers both ship-family
  query 6 and this override query 8. The helper `unit_is_kind_of` already
  carries this exact body and accepted set.

The `SubmarineGen` and other common unit creators allocate from the resolved
Type descriptor. A label alone does not force a submarine leaf.

## Groups, types, and current host admission

Installed `universe/library/ship.props` lines 2..9 define `Ship(Common)` with
torpedo false. Lines 199..203 define `Sub(Ship)`, adding only Dive and
NavigatorAllowMaxDepth. Therefore an ordinary submarine scene entity's merged
bag inherits false. Scene group merge runs before authored assignments; this
patch leaves that precedence unchanged. The native class override happens
after the final bag value, so even an authored false does not disable an
actual class-8 submarine at initialization. A later Lua/message write remains
able to change the retained enable through the existing runtime writer.

Installed PRCP JM06 has `Sub`-group entities at lines 5143..5161 (Narwhal),
5163..5182 (Gato), and 5615..5642 (TypeB w Jake, initially Hidden). The global
enums map Narwhal to 31, Gato to 30, and TypeB_Jake to 93. Their VehicleClass
rows have Type `Submarine` (lines 18768, 18468, 43772). No inference from the
scene entity's group or label is needed. Asset hashes are recorded in the report.

The host resolves the merged bag's Type enum first. `create_units` reads the
corresponding Lua row, maps its Type string through `vehicle_class_kind_row`,
and stores that kind as the actual slot class id. The candidate follows those
same two existing operations, restricted to scene creators admitted through
`UnitClassFactory`. Only a present false key triggers the class lookup; a
known kind answering query 8 changes false to true. Unknown/missing Type rows,
a missing Lua host, and other creator families retain the existing raw value.
Those are explicit host limits; no fallback class is invented.

Gunnery's existing `kShipDirectorEnablesBound` is ON. For any actual unit
answering ship-family query 6, its stance push looks up the per-name table and
copies the four enables. Since `00853050` accepts 6, real submarines consume
this table. Runtime TorpedoEnable/CLOSEATTACK updates keep their existing table
writer and order; the new binding runs only while retaining initial scene bags.

## Candidate and validation boundary

The implementation is confined to scene source/header. Its false-to-true
candidate is gated by `kShipDirectorClass8Bound = false`, and ON emits
`ship director class8 override: unit=... type_id=... class_id=8 raw_false=1 torpedo=1`.
This is a process binding of native initialization, not a native ABI replacement.

Use the existing **JM06 3000 mission frames at 0.05 s** row with all other
switches, orders, timing and inputs fixed. Compare OFF/ON using `pair_diff` and
the existing per-unit director/gunnery diagnostics. ON should produce class-8
override lines and enable the inherited-false submarines' torpedo masks;
surface ships with false retain false. Shipyard boats with absent keys keep
true on both candidates. Gameplay may move because the previously masked
submarine tubes become eligible; a change needs review before the switch is ON.

The worker compiled the scene translation unit as MSVC Win32 with `/W4 /WX
/fp:strict`, for OFF and an ignored local ON-header variant. Direct native call
receipts are checked by `verify_report_calls`; vtable resolution is separate
byte/listing evidence. No new broad tests or concurrent full build were run.
`verify_report_calls` passed eight direct rows with zero failures; two indirect
rows were explicitly excluded. `git diff --check` passed. Both ignored object
hashes and sizes are recorded in the report.
Fresh linked build, the OFF/ON runtime pair and original-game validation remain
with the primary integrator. These qualifications prevent treating successful
source compilation as gameplay or ABI proof.

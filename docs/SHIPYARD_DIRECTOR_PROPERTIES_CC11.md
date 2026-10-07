# Shipyard-created boats and director properties

Packet `cc11_director_props`, 2026-10-06. Addresses owned for this read-only
question: `00844FC0`, `008238F0` (the latter is an interior byte, not an
instruction or function start). Ghidra target verified through `bsp.py`:
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

**Verdict:** a Gyoraitei created by the shipyard does **not** inherit
`universe/library/ship.props`' `TorpedoDirector = B false`. The launch producer
constructs a sparse bag, and the native director arm treats an absent key as
true. The current host's missing-name fallback therefore agrees with this
path. Applying the `Ship` group to every dynamic ship would change the native
behavior. This settles GUNNERY_OPEN_ITEMS 163.2 queue 1's scene-contents question;
the goal-range / pass-byte `+7Dh` question remains in the ships lane.

## Producer, clone, and consumer

| Routine | Coverage in this packet | Evidence |
| --- | --- | --- |
| `00844FC0` (`00844FC0..0084559D`) | complete producer inspection; pose arithmetic not reconstructed | Empty bag, explicit launch writes, creator dispatch, clone and publication |
| `004F06C0` (`004F06C0..004F0780`) | complete supporting read | The TBoatGen creator does not merge a property library group |
| `00822C20` | partial: `008238B1..00823B6B`; other initialization arms not reconstructed | The property-holder arm, director-key defaults, state-message publication |
| `008F41A0`, `008F2260`, `008F41F0`, `00922E20` | complete supporting reads | Empty construction, missing-key lookup, deep clone, holder construction |
| `0046CF40` | partial: `0046D2A2..0046D353` | Scene group-list merge precedes authored-body parsing |

`00845001..00845014` passes owner 0 to `008F41A0`. That callee zeroes the
entry count and all 64 bucket heads; it loads no defaults. The producer writes
`Type`, `Skill`, `Race`, `Party`, `OwnerPlayer` and `ShipYardLaunch` at the call
sites `00845037`, `00845055`, `0084506A`, `0084507F`, `00845097`, `008450AA`.
The integer writer `008F3710` produces a type-0 record with its value at `+0Ch`;
the boolean writer `008F3940` produces type 3 with a byte at `+0Ch`. Aircraft
add `WingCount`, `VelocitySI`, `State`, optional `Equipment`; submarines add
`Dive`. No branch writes a director key, reads a named property group, or calls
a bag merge.

The class-id-`0Eh` branch calls `004F06C0` at `008453AA`. Register/stack evidence
at `00845395..008453AA` passes the production-entry name in EDX and this same
bag as stack argument 3. `004F06C0`'s listing reads that slot at `004F06C2`
and ends with `RET 10h` at `004F077E`: ECX class id, EDX name, then parent,
frame, bag, unused. It resolves `Type`, allocates a unit, places or defers its
hierarchy, assigns its name, and applies `Command` through `004E6B30` at
`004F0773`. The latter queues nothing when `Command` is absent. There is no
scene-entity reader or library merge on this creator path.

`00845440` calls `00922E20` with the launch bag. `00922E20` calls the deep-clone
routine `008F41F0` at `00922E2D` and creates `{vtable, refs=1, clonedBag}`.
`0084544C` publishes that holder at new-unit `+C0h`. Clone iterates only the
source entries; it adds no group defaults. The director arm reads the bag at
`[unit+C0h]+8`, after selecting holder type 1 at `008238B1`.

The actual four-key span is **`008238FD..008239A8`**, inside `00822C20`, rather
than the older approximate `008238F0..008239A3` span.

| Key | Lookup call | Absent / nonzero | Explicit false |
| --- | --- | --- | --- |
| `ArtilleryDirector` | `0082390B -> 008F2260` | 1 at `0082391F` | 0 at `00823918` |
| `TorpedoDirector` | `00823932 -> 008F2260` | 1 at `00823955` | 0 at `0082394E` if `unit->vt[5Ch](8)` returns zero; otherwise 1 |
| `AADirector` | `00823968 -> 008F2260` | 1 at `0082397C` | 0 at `00823975` |
| `DCDirector` | `0082398F -> 008F2260` | 1 at `008239A3` | 0 at `0082399C` |

The torpedo absence branch at `00823939` goes directly to `00823955`; it does
not execute the class probe. `008F2260` returns null for an absent key, with no
library fallback. `00823B02` transfers the torpedo local byte to state-message
`+42h`, and `00823B6B` submits that message to director slot `3Ch`. This is an
explicit native true default, even though the host obtains the same result by
leaving its constructor default in place.

## Installed assets and scene precedence

Installation: `I:/SteamLibrary/steamapps/common/Battlestations Pacific`.
SHA-256 receipts and timestamps are in `reports/shipyard_director_properties_cc11.json`.

- `universe/library/ship.props`, lines 2..9: `Ship(Common)` declares the four
  keys, with torpedo false and the others true.
- `universe/scenes/missions/COTP-IJN/PRCPIJN/prcpijn_08_defend_guadalcanal.scn`,
  lines 26498..26502: `Shipyard 01` uses `Common, LandingZone, MultiEntity,
  CommandBuildingInferior`. Lines 26590..26594: stock 1 is Count 12,
  Names `Gyoraitei`, Type `VehicleClasses : JapPT`. This is stock metadata,
  not an authored `Gyoraitei #Y1..#Y4` scene entity with a `Ship` group list.
- `scripts/missions/COTP-IJN/PRCPIJN/prcpjm08.lua` has no `TorpedoEnable`
  occurrence; no script enable is needed to explain the native initial true.

Ordinary scene entities enter `0046CF40`, construct a bag at `0046D2C3`, resolve
each named group with `00469B60` at `0046D304`, merge at `0046D30C`, and parse the
authored body at `0046D34E`. Authored assignments occur after the group values.
The host mirrors those stages in `bind_entity_properties`; the existing
`merge_group_into` / authored overwrite order is unchanged. Shipyard launch
bypasses that reader and has no group-list stage. Neither the loaded `Ship`
definition nor the presence of a `Type` field makes a sparse bag inherit it.

## Changes and verification limits

Only comments change in scene contents and its header: the exact key span,
native absent-key default, and distinction between a merged scene bag and a
sparse launch bag. No default, property precedence, table entry or executable
behavior changes. The explicit-false torpedo class-id-8 exception remains a
separate follow-up: the host currently retains raw false unconditionally.

`python tools/verify_report_calls.py reports/shipyard_director_properties_cc11.json`
passed: 20 direct call rows checked, zero failed. Two indirect rows are explicitly
excluded from that check. `git diff --check` passed.
No new tests, Ghidra annotations, definitions, exports or flow repair are needed.
This packet establishes original-source semantics and installed-asset contents;
it does not claim a native ABI replacement, a fresh executable build, a current
JM08 run, or original-game validation. Earlier `mask=3` host logs are consistent
with this static result but were not rerun here.

# Runtime unit class predicates

Addresses: `006FE460` (constructor identity producer), `006FE530` (Destroyer
predicate, reused), `00964790` (existing descriptor factory), `004C0890`
(controlled-unit dispatch), `009329C0` (hydro), `00937C90` (hull construction).

`GameUnitsHost` now dispatches kind queries through the existing compiled-body
table in `unit_kind_query.hpp`. Its previous adapter applied Destroyer's ancestry
`{0,1,2,4,5,6,7}` to every instance, removing only literal 7 for other classes.
That made planes and structures answer ship 6 and discarded intermediate
ancestors such as plane 15, recon 20, and LandFort 27 for CommandBuilding 28.
The correctly specialized `unit_is_kind_of_006fe530` is unchanged.

## Identity and dispatch

The existing `VehicleClass.Type` reader and `vehicle_class_kind_row` implement
the 22-branch string chain of `00964790`. A descriptor's `+28h` allocator constructs
its corresponding instance. The existing descriptor and entity tables establish
the matching descriptor-kind and instance-class values; the scene generator's
class name and numeric Lua row index are not the instance class id.

This packet rechecked the complete Destroyer producer as a concrete witness:

| Routine | Coverage | Evidence |
| --- | --- | --- |
| `006FE460..006FE4C2` | complete analysis of constructor; existing implementation reused | ECX copied to ESI at `006FE466`; call `0081ED40` at `006FE468`; primary vtable `00CFC3D0` at `006FE46D`; most-derived `+C4=7` store at `006FE4B3`; EAX returns ESI; `RET 4` at `006FE4C0` |
| `006FE590..006FE612` | complete listing checked for identity and argument provenance; existing allocator reused | Sole direct caller; allocation/zero fill precede ECX=ESI and the forwarded stack flag at `006FE5D6..006FE5DD`; `RET 4` at `006FE610` |
| `006FE530..006FE56A` | complete listing rechecked; external compiled predicate reused | Query comes from `[ESP+4]`, owner from ECX; accepted chain or equality with `[ECX+C4]`; both returns use `RET 4` |
| Other predicates and descriptor factories | existing external contracts | `docs/UNIT_KIND_QUERY.md`, `docs/ENTITY_CLASS_IDS.md`, `docs/VEHICLE_CLASS_DESCRIPTORS.md`; no new implementation or ABI claim |
| `004C0890..004C0924` | existing routine reused; complete listing rechecked for query order | Unit queries 5 then 18; target queries 15 then 18; listener `+18h` call is `004C08F7`, not the vtable load at `004C08F2` |
| `009329C0..00933BA9`, `00937C90..00939C8F` | partial: predicate call sites only | Query 8 at `00932A42`, `00932E2F`, and `00937CFD`; surrounding force and body logic is unchanged |

For the factory-selected leaf classes, the body owner and most-derived class id
coincide, so `bsp::unit_is_kind_of(actual_class_id, query)` supplies the exact
accepted set. The separate core `unit_kind_body_answers(owner, dynamic, query)`
is required for a future entity that inherits a different owner's predicate;
the present vehicle factory does not supply such an instance. This binding does
not invent an owner for unrelated effects or other scene-object classes.

`GameUnitSlot::class_id` starts at existing `kVehicleClassKindUnknown` (-1), and
only a recognized loaded Type replaces it. Missing rows and unrecognized Type
values retain explicit unresolved identity, log it, and answer false even for
query -1. The sentinel is a host boundary, not a claimed native constructor
value. `UnitInstanceState::class_id` receives the same value. Public signatures
are unchanged; `unit_class_id` documents -1 for unresolved identity as well as
invalid indices.

The known `DummyTargetVehicle` descriptor maps to kind 53 but its native `+28h`
allocator `00749150` returns null (`XOR EAX,EAX; RET 4`). Its descriptor mapping
is retained. The existing represented-unit creator must not be mistaken for
proof of a native instantiated object for that row; creation is outside this
packet.

## Four bindings and diagnostics

| Binding | Native predicate site | Result |
| --- | --- | --- |
| Public `GameUnitsHost::unit_is_kind_of` | callers' instance `vtable+5Ch` | Loaded class's compiled predicate; invalid index or unresolved identity is false |
| `ControlledUnitQueryBinding` | `004C08A2`, `004C08BD`, `004C08DB`, `004C08EA` | Same predicate for every query literal; existing absent-subunit boundary retained |
| `ShipHydroBinding::unit_category_8_vtable5c` | `00932A42`, `00932E2F` | Same class dispatch for literal 8; diagnostic records the actual predicate address per class |
| Hull-body input | `00937CFD` | Same class dispatch for literal 8; accepting store at `00937D09` chooses record 2 |

`GameHostLog` groups records by method name, so hydro uses a class-qualified
method name to keep different native predicates distinct. The controlled-unit
diagnostic now reports actual query answers instead of assuming neither plane
nor squadron ancestry. Correct plane dispatch reaches the existing listener
handle adapter, which still returns false; renderer publication is also an
explicit unresolved host. No native handle or renderer ownership was fabricated.

The existing ship navigation construction guard in `GameShipAiHost` stays at
known loaded kinds 7 through 14. Correcting these predicates also changes
local-player list classification (`004C3CB0`), HUD selection (`0068ACA0`), script
kind filtering (call site `008AD44F`), and generic AI queries. Those consumers
reuse the corrected public API; their other partial host branches are not
implemented by this packet. The separate legacy world-registration `+130h`
adapter is also unchanged.

## Verification and limits

`reports/game_unit_kind_binding.json` records the exact 77 previously registered
USN01/Marshall units, their numeric Type ids, loaded Lua type strings, native
predicate addresses, and before/after accepted sets. The ignored fixture joins
the primary agent's existing runtime registration log to the preserved installed
633-row Lua extraction and classifies it with the compiled current core. It also
enumerates every branch of the 22-kind descriptor table and verifies unresolved
identity answers no query in the class-id domain. Source scene, Lua files, input
log, extraction, classifier, and output hashes are retained there.

All 77 registered units resolved: 37 distinct Type ids, drawn from 11 instance
classes. Exactly 14 ship-family units kept all answers. The other 63 lost the
incorrect ship-6 answer; 20 aircraft gained plane-15, five large recon aircraft
also gained recon-20, and the command building gained LandFort-27. The corrected
categories are 14 ships, 20 planes, 42 land structures, and one airfield. All 633
installed Lua rows map to one of the factory's 22 branches (21 Type strings are
used by that installation).

This is classification of a previously observed runtime domain, not a new
mission run or an additional original-byte differential. The core's existing
native evidence is reused. Root runs the combined mission after integration;
this packet makes no new gameplay, listener-publication, binary replacement,
or complete native object-construction claim. Win32 Release compilation and the
two existing tests passed; `verify_report_calls` checked seven direct call rows
with zero failures and reported eight explicit indirect boundaries. Those
indirect sites were checked in the live listings. No tracked tests or new framework were
added. Ghidra was read-only, with project/program verification in each wrapper
batch; no new function definitions or false `_free` flow repairs are needed.

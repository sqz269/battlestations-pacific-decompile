# Actual unit secondary observer callbacks

Addresses: `007C6E90`, `008455A0`, `0080DFC0`, `00952050`.

The four source providers follow the actual secondary tables recovered in
[NATIVE_UNIT_OBSERVER_ENDPOINT.md](NATIVE_UNIT_OBSERVER_ENDPOINT.md). They do not
add runtime endpoints or replace a whole unit constructor, destructor, class
hierarchy, or observer dispatcher. Names are provisional hypotheses.

Plane tables select `007C6E90` in callback slot+4. The shipyard selects the
distinct `00952050` no-op in slot+4 and `008455A0` in slot+8. The report retains
each exact creator/table row; callback providers are not interchangeable defaults.

| Native body | Coverage | Native ABI | Source |
|---|---|---|---|
| `007C6E90..007C6F16` | complete normal body, external calls retained | ECX=plane secondary at unit+10h, one delivered first endpoint, RET4 | `native_plane_observer_callback_007c6e90` |
| `008455A0..0084566D` | complete reachable body, validation boundary retained | ECX=shipyard secondary at unit+10h, one delivered first endpoint, RET4 | `native_shipyard_observer_callback_008455a0` |
| `0080DFC0..0080DFC2` | complete | `C2 04 00`; consumes one stack word, reads no inputs, no defined return | `native_unit_observer_noop_0080dfc0` |
| `00952050..00952052` | complete | same three bytes, separately selected provider | `native_unit_observer_noop_00952050` |

Root formally defined, saved and exported the two short bodies. They are not
adjustor thunks. Their parameterless C++ interfaces infer no unused ECX or stack
type. Generic `0042B120` remains its existing consumed provider. These are new
C++ interfaces, not drop-in binary replacements; register preservation, native
fault addresses and arbitrary SEH are not supplied by them.

## Plane callback

At `007C6E92` EDI captures the secondary receiver. The original stack argument
becomes ECX at `007C6E94`; its vtable+4 getter runs once at `007C6E9D`. ESI then
holds the getter result. The result may differ from the delivered first endpoint;
the source keeps both identities. Its actual vtable+5Ch provider is queried for
literal `45h` at `007C6EAE`. Only a successful nonnull result skips the fallback:
query literal `9` at `007C6EBD`, then effective game mode `004BCA50` must differ
from 9. `004BCA50` receives the current owner at global `E188A8` and no stack
arguments; the complete existing `simulation_gate` implementation remains the
binding target for its raw mode/forced/session fields.

A null getter takes the fallback and dereferences null at `007C6EB4`; it is not
silently ignored. The source passes null to the explicit actual-vtable provider
boundary, whose native-valid domain requires a live entity. Its host must not
turn null into a harmless false answer. The fixture records the original access
violation and uses an explicit faulting host boundary on the source side; it does
not claim native SEH replacement semantics.

The callback then requires unit+9D4h nonnull, +5Ch nonzero, and +5Dh/+60h/+5Eh zero,
then reloads +60h and +5Dh and requires them zero again. Volatile borrowed field
references preserve those loads. Existing `plane_unit_off::kSquadron` and
`PlaneSquadronOffsets::kPlaneBackPointer` already establish +9D4h; original stores
`007ED0E6` and `007F4B49` write the actual squadron backpointer. Base unit flag
storage comes from the existing unit state/`00925CE0` producer, not another flag
owner invented by this packet.

`007C6EE0` computes whole raw unit as secondary-10h. `007C6F03..007C6F0A` load and
push exact `BF800000` (-1.0f), and `007C6F0D` calls `007C5AC0`, whose normal exits
at `007C5F38` and `007C5F5C` both RET4. That entire callee was read to confirm the
float ABI and meaningful negative-input path; its shared prepass remains external,
as in `plane_ground_ops` and `airfield_taxi`. The address-named provider retains
the float, with no invented broad prepass implementation or narrowed universal
contract. Actual +5Ch class bodies remain producer-specific; existing `unit_kind`
providers can only be selected after resolving that actual producer.

## Shipyard callback and producer

Its EBP is the actual secondary base. Secondary+770h/+780h are whole unit+780h/
+790h checked-vector headers. The source borrows their actual begin/end lvalues
(header+4/+8), leaving header word0 and capacity untouched. It allocates no vector
or record and ports no STL operation. Numeric opaque record fields retain their
native widths instead of acquiring inferred semantic types.

The whole-owner constructor `00848080` establishes ESI=original ECX at `008480A0`,
EDI=0 at `0084809C`, publishes secondary table `D0B754` at `008480B7`, and zeros
+784/+788/+78C at `00848109/0F/15` and +794/+798/+79C at `0084811F/25/2B`.
`00844CE0` walks whole-owner+780h records with stride10h and returns its selected
record. In `00844FC0`, EDI=whole owner at `00844FE8`, EBP captures that returned
record at `00844FFF`, and ESI loads the supplied 4Ch record at `00844FF8`.
`00846D90` proves that argument's source: EBX=index*4Ch+unit[794h] at
`00846DE0..00846DED`, then PUSH EBX/MOV ECX,ESI/CALL `00844FC0` at `00846F86..89`.

The build producer writes literal 3 to record+4 at `0084500A` and consumes its
existing descriptor at +8. It stores the newly created first endpoint to +30h at
`00845413` and the same pointer to the selected 10h record+Ch at `0084541B`.
`008454CE..008454D4` pass that same first in ECX and whole shipyard+10h in EDX to
the existing `00694A60` registration. These are direct observed-prefix identities,
not semantic world-list pointers or results from an event getter.

`008455A0` compares the delivered pointer directly, including a null argument.
It clears only the first matching +Ch in the 10h range, then only the first
matching +30h in the 4Ch range, clearing that record's +4/+8/+30h words in order.
All other bytes, duplicates after the first match, capacities, and endpoints
remain untouched. It neither unregisters nor releases the stored pointer.

Each range retains its initial captured iterator, then per loop captures end,
checks begin<=captured end, tests iterator equality, checks iterator<reloaded end,
reads the record, and checks iterator<reloaded end before advancing on a miss.
The existing `ObserverLifetimeServices::invalid_parameter_00bf6713` can return;
no repair/restart/default range is invented. Two `CMP EDI,EDI` error edges are
unreachable and disappear in source. `00BF6713` pushes five zeros, calls
`00BF66EF`, then ADD ESP,14h/RET; its decoded-handler path may return. Invalid
record addresses after a returning handler retain an external memory-fault limit.

## Existing owner mapping

The application keeps its one stable `GameUnitSlot*` canonical world identity and
one raw prefix member in that same owner. Resolve the received callback prefix to
`NativeUnitObserverAlias{&slot, slot.observer_prefix}` through the existing binding.
The plane view borrows that owner's live squadron and flag lvalues. The shipyard
view borrows that owner's actual two vector begin/end lvalues; it does not copy
the records to a sidecar or derive them from semantic slot offsets.

Delivered first and getter result remain raw observed-prefix identities. Resolve
each to its canonical owner only when an external whole-unit provider needs it.
The prepass host verifies/resolves the alias and invokes the existing plane owner.
Subtracting10h from the raw secondary finds its raw prefix, never the allocation
address of a semantic `GameUnitSlot`. Keep borrowed fields, vectors and aliases
live through callback return, under the existing observer dispatch/lifetime order.
This packet does not bind these providers into `GameObserverRuntime`; root owns
that integration. No executable frame-behavior claim or gameplay validation is made.

The report contains all 21 exact producer rows, rechecked against live and disk
table bytes, along with numeric direct-call rows and explicitly indirect slots.
The focused ignored fixture retains all four original bodies (347 bytes), twelve
relative-call and two absolute-address relocations, native/source record captures,
source/objects/executable, linker libraries and logs. Its provider stubs test the
wrapper contract, not the external class queries, game owner or shared prepass.
The Win32 Release build and both existing CTests passed. The focused fixture
matched 12 captures, 34,384 result bytes and 39,072 exact pre-call input bytes per
side with zero mismatches. Its first launcher failure and both successful probe
versions remain retained. The live call verifier passed all 16 direct rows;
three indirect virtual calls remain explicitly identified by their actual slots.
The JSON report records the exact local proof manifest/hash, copied repository
include dependencies, objects, linker libraries, and installed tool paths/hashes.

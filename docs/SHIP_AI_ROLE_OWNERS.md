# Ship AI role ownership prerequisites

Packet `orch6_role_owners_j` is a read-only producer assessment. No runtime
source, native function, annotation or ledger is changed. It establishes the
constructor and receive-side ownership needed by the cruise avoidance request;
it does not implement the session's role-message delivery.

## The cruise receiver is the attached ship

The historic description of `[[unit+740h]+50h]` as a formation-group object is
wrong on the proven ship construction path. `00810DF2` calls `009F3F20` and
`00810DF7` stores its returned outer bot at `unit+740h`. `009F3F59` constructs
that object through `009F3BA0`, carrying the original ship owner. This object
contains the inherited bot at offset zero and the separate ship brain at
`+58h`; those addresses must not be conflated.

`009F3C36` forms `owner+310h`, then `009F3C43` calls `008FBC80`. The tick-node
constructor's `008758B7` store produces `node+28h = owner`. Attach loads that
payload at `008FBC90` and stores it at `bot+50h` at `008FBC95`. Thus the native
cruise loads at `009E118E/009E119C/009E119F` reach **the attached ship's role 1**
at `unit+1B0h`. This proof covers this construction/attachment chain; a later
detach or reattach must continue to use its actual attached owner.

The two relevant consumers are:

| Consumer | Actual field and predicate |
| --- | --- |
| `009E11AD` | Attached owner's role 1: value 8 bypasses the call; otherwise `00927F10(slot)` decides the cruise versus helm arm. |
| `009E12E4 -> 00521E70(unit,0)` | Ship's role 0 at `+1ACh`: value 8 or an AI-held assigned slot enables the torpedo request. |

`00927F10` is **stdcall with one stack argument**, `RET 4` at `00927F24`,
complete body `00927F10..00927F26`. It overwrites ECX itself and returns the
byte `[[game+18CCh+4*slot]+9h]` in AL. Loading an entity into ECX before a call
does not make it a second argument. Neither a scene Party nor the local-player
index is interchangeable with the role's slot index.

## Constructor values and actual writers

`00928630` initializes the current assignments explicitly: `00928713` loads
EAX=8; nine stores at `00928718..00928748` initialize `+1ACh..+1CCh`. These are
produced sentinel values, not inferred from zeroed allocation. The constructor
also sets `+180h=9` and calls virtual `148h(1FFh,9)` before those nine stores.

There are two distinct nine-word tables:

| Storage | Producer and role |
| --- | --- |
| `+188h..+1A8h` | `00927D20(mask,slot)` writes the selected availability/owner-policy entries. Ship virtual `148h` is `0077F360`, which calls that base routine. This is the table tested for 9 or the requested slot during assignment. |
| `+1ACh..+1CCh` | Constructor writes 8; `009281C0(role,slot)` writes the current assignment. Ship virtual `154h` is `0077F480`, which calls that base routine at `0077F48F`. |

Vtable bytes confirm base `00D192E0+154h = 009281C0` and ship
`00CFC3D0+154h = 0077F480`. `009281C0..009281FE` uses ECX for the entity and
two stack arguments, with `RET 8` at `009281EC/009281FC`. Role 0 clears the
player-controlled byte at `+184h` when assigning 8, invokes the scoring
notification at `009281DD`, then writes the current slot at `009281E2`.
Other roles take the direct store at `009281F3`. The ship wrapper also has a
suppression-map side effect; a future full writer must preserve it.

The inspected kind `4Bh` branch of `00780120` calls virtual `154h` at
`00780387`. For its acquire path it first requires the current slot to be 8
or AI-held (`00780346..0078035C`), and the corresponding `+188h` entry to be 9
or the requested slot (`0078035E..0078036D`). Release requires the current
slot to equal the request and passes 8. The receiver includes other owner,
controlled-byte, observer and relay work. This assessment does not reduce the
whole receive path to an unconditional array store.

Virtual `144h` is another distinct operation: ship `0077F2D0` calls
`009278A0`, which writes `+180h` and invokes virtual `148h(1FFh,slot)`. Likewise
the already reconstructed `004C3840` selects candidates through `+188h`,
calls virtual `148h` to unbind, or routes a kind `53h` owner-change message.
Neither operation directly proves a write to the current `+1ACh` table.

## Session ownership and current process

`004BB160` initially points game `+18CCh` at the mission-record pool starting
at game `+1008h`, stride `118h`, and sets each applicable record's `+9h` to 1.
That is not a permanent property of a slot number. `004BB440` claims records
from the separate player pool at game `+748h`. The native local-load caller
replaces the active pointer; `004BB630` also installs an actual supplied
record in `game+18CCh+slot*4`, while `004BB660` restores the corresponding
mission-pool record. An assigned role must consult the current pointed-to
record's byte, not an assumed AI value derived from Party.

At assessment base `b40c84e6`, `GameUnitSlot` has no current-role array.
`GameScriptOrdersHost::entity_route_slot` records an unresolved read and
returns -1. Its role-availability setter and role-message router are also
recorded as unimplemented. No live kind `4Bh` assignment receiver is present.
`GameUnitsHost::set_controlled_unit_004c0890` publishes the selected unit; it
does not establish an assignment to the nine role entries.

`GameMissionFrameHost::Impl::slots` and `entry_slots` represent scene-entry
state. The host sets slot 0 `in_use` and entry records `device_bound`; this
does not construct the two native participant pools or the active pointer
table with its `+9h` bytes. They cannot supply `00927F10` by themselves.

## Smallest subsequent binding

The proposed canonical storage is one nine-element signed integer array in
the existing `GameUnitSlot`, initialized once to the existing
`kUnitRoleTableFill` (8), with the count taken from `kUnitRoleTableEntries` (9).
No second array should be kept in ShipAI or Commands. A typed read should
report unavailable for an absent unit or unrepresented constructor state.
The current-role array must remain separate from the `+188h` policy table.

For the proven attached-ship path, read roles 0 and 1 from that same owner.
When the value is 8, the predicate is available and true without any session
lookup. For an assigned value 0..7, require a live session-slot read interface
which can report unavailable until its active record and `+9h` byte are
represented. An unavailable assigned-slot byte must not become false, true,
or an unassigned sentinel. Out-of-domain values should likewise be reported
unavailable; the native helper itself performs no bounds check.

This constructor projection can represent the process until an actual
delivered writer is implemented. Script intents and selected-unit publication
are insufficient evidence that delivery occurred. Later delivery must update
the same array through the real receiver contract and preserve the role-0
side effects. Do not expose an always-local replacement for the routed sender.

Suggested ownership for that next implementation is the two Units host files
(after their active motion lease clears), plus a narrow role-state core only
if native writer reconstruction is required. Candidate addresses for such a
core are `009281C0`, `0077F480`, and the relevant `00780120` branch, checked
against leases first. Full assigned-slot support additionally needs the
mission/session owner and its `004BB160/004BB440/004BB630/004BB660` lifecycle;
the existing entry-slot owner alone is insufficient. The integrator owns
corrections to old request-header wording and the AI call-site binding.

## Evidence limits

The report records exact native call rows and source/listing hashes. This is
a bounded constructor/attach/receiver assessment, not an exhaustive scan of
all writes, all callers, or all class vtables. The receiver and session routines
were inspected only for the stated producer contracts. No code changes,
build, tests, native fixture, or mission validation are claimed in this packet.

# Canonical game unit scene lifecycle view

Addresses: none newly reconstructed. This application binding consumes the
existing `00926390`/`009263C0` lifecycle providers and T observer aliases.

`GameUnitsHost::scene_lifecycle_view(identity)` lends the actual five byte
lvalues of the same slot returned by `unit_identity` and `observer_alias`.
`UnitInstanceState::active` and `simulate` now use `uint8_t`, as do the same
slot's existing destroyed, removed and pending-destroy cells. No owner, native
unit object, flag copy, raw layout cast or callback routing is added.

| Native field | Existing source owner | Producer / retained default |
| --- | --- | --- |
| `+5C` | `UnitInstanceState::active` | constructor `925E0E` clears; represented creator activation `923855` writes 1 |
| `+5D` | `UnitInstanceState::simulate` | constructor `925E14` clears; host creation retains 0 |
| `+5E` | `GameUnitSlot::scene_destroyed_005e` | constructor `925E11` clears |
| `+5F` | `GameUnitSlot::scene_removed_005f` | constructor `925E0B` clears |
| `+60` | `GameUnitSlot::scene_pending_destroy_0060` | constructor `925E08` clears |

The constructor's `XOR EBX,EBX` at `925CFD` establishes BL for those stores;
the complete stored listing was filtered for every EBX/BL write. The generic
`UnitInstanceState` defaults remain 1/1; the existing host construction path
still explicitly supplies 1/0. Unknown creators do not gain flag provenance.

The lookup first requires the existing live observer runtime and a published
unit prefix, then requires the same slot's state and flag provenance. Null,
foreign, unresolved and unbound identities return no view. The interface is
new C++, not an ABI-compatible native object layout or callable native vtable.

All direct state consumers were audited. The only consumers outside the owned
host are the nonzero conditions in `src/unit_instance.cpp`; that file needs no
change. Existing Boolean getters and `SceneNodeFlags` snapshots explicitly
test `!= 0`, so values such as 2 through 7 retain native byte semantics.

`GameUnitRow::active` remains derived diagnostic data. Ship AI's nearby-unit
scan reads it through `unit_row`; the trajectory stream obtains `units()`.
Both getters now refresh activity from the canonical byte before returning a
row or copying the flat table. Existing world and motion gates already read
the canonical state. A stored row is a snapshot: request it again after a
lifecycle write. There is no independently authoritative cached activity flag.

| Scope | Coverage |
| --- | --- |
| Borrowed view and current activity snapshot reads | complete application binding |
| Native lifecycle wrappers | existing U source, unchanged |
| Game renderer, scene handle and lifecycle virtual tails | external; no binding added |
| Original native ABI, EH transport and gameplay lifecycle | unproved |

The focused actual-host fixture uses two Lua-created Destroyers, the existing
application singleton/observer runtime, actual endpoint registration, and
explicit callback providers selected by the verified leaf table. The slot18
provider consumes an explicit null fixture input matching the seven-byte
`6D1E80` field-read contract; it does not claim to expose an actual host scene
handle. Slot7C (`824B60`) and slot80 (`951FB0`) throw a named fixture boundary.
Assertions cover the real flag and observer effects completed before those
tails, not successful execution of either tail or renderer behavior.

The fixture also checks noncanonical byte-to-Boolean conversion, same-lvalue
identity on repeated borrowing, callback reads of refreshed rows, and inactive
world/motion gates. It explicitly resets state between the two lifecycle
cases. It disposes of all views before destroying their host and verifies the
surviving endpoint is detached while the actual runtime remains live.

Alias withdrawal ordering is a separate source check: the destructor clears
`observer_prefix_ready` before callback-owner destruction and then observed-
owner destruction (`92589D` before `9258AC`). New lookups therefore fail once
withdrawal begins. A previously borrowed C++ reference is **not** mechanically
invalidated; the caller must finish using it before teardown. The fixture does
not instrument destruction to re-enter that lookup.

The report retains exact compilation/link inputs, application objects, all
resolved libraries, toolchain binaries, source, native evidence, build/CTest
logs and probe output under ignored `local/scene_flags_proof`. No game install,
renderer host, core instance implementation or native lifecycle source changed.

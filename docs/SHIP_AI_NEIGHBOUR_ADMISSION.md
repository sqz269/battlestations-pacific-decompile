# Neighbour admission and refresh

Addresses: `009F0D20`, `00827F70`. Packet `orch6_neighbour_admission_n`.

The new `ship_ai_neighbour_admit_009f0d20` covers the complete normal admission,
duplicate refresh, allocation and publication sequence. The older helper in
`ship_ai_sector_scan.cpp` remains an explicitly partial projection; this packet
does not change it. The new caller uses the existing `ShipAiObstacleNode`, list
capacity, exact lifetime constant and unit/class kind definitions. Its actual
constructor, allocator and virtual dispatch services remain external.

| Routine | Native boundary and ABI | Coverage |
| --- | --- | --- |
| `009F0D20` | `009F0D20..009F0E83`, 356 bytes, 109 instructions; ECX controller, one stack candidate, `RET 4` at `009F0E81` | Complete normal caller sequence; external complete constructor, allocation policy, raw ABI and CRT EH/unwind |
| `00827F70` | `00827F70..00827FA3`, 52 bytes, 26 instructions; ECX descriptor, AL Boolean, three plain returns | Complete normal predicate over actual descriptor virtual dispatch and produced byte |

Both stored Ghidra bodies have zero flow gaps. The sole admission call is
`009F1A25`, inside `009F1420..009F1BBA`: `PUSH ESI` at `009F1A21` passes the
candidate; `LEA ECX,[EDI+8]` at `009F1A22` selects the navigation block. The caller
ignores the result. This does not establish runtime membership in its world list.

## Gates and field ownership

The first check is signed `count+604 >= 128`. It returns before any self, candidate,
settings or duplicate access. A full list therefore does not refresh an existing
candidate. Otherwise the routine reads current self `+3FC`, then its signed
`+6B8`. When that value is nonnegative and candidate is nonnull, candidate virtual
`+5C` with literal 8 rejects on a nonzero AL. Literal 8 is the existing unit-kind
Submarine query. The meaning/producer of the unit's `+6B8` remains unresolved;
the interface requires that actual signed field and supplies no default. It is
not the ship **descriptor**'s unrelated `DamageThreshold` at the same offset.

Current self is reloaded, then descriptor `+538` is captured for `00827F70`.
That helper asks this same descriptor's virtual `+18` for kind 14 (TorpedoBoat).
A nonzero result immediately returns true. Otherwise it asks kind 12
(LandingShip); only a true answer permits reading descriptor byte `+808`, and
only a zero byte returns true. The whole predicate is therefore
`TorpedoBoat || (LandingShip && !BigLandingShip)`, with ordered short-circuit
dispatch. This predicate **bypasses** the next admission rejection; its historical
close-approach callback name is not a universal meaning for its other callers.

Existing `VehicleClassKind`, `kVehicleClassDescriptorTable` and
`ShipLeafDescriptorOffsets::kLandingShipIsBig` establish the two descriptor kinds
and byte. The writer in `0074C630` fetches `BigLandingShip` through `00B67800` at
`0074C66A`, passes a zero default to Boolean reader `00B662F0` at `0074C67B`, and
stores AL at `0074C687`. The reader accepts an actual Lua Boolean from a valid
kind-2 reference and otherwise uses the supplied default byte; it is not a bare
Lua truthiness conversion. The new predicate consumes the stored byte and does
not read Lua or invent a descriptor. See `VEHICLE_CLASS_LUA_LOAD.md`.

When the class predicate is false, current self is reloaded for unit virtual
`+5C`, again query 8. A true answer admits. Otherwise candidate party `+54` is
captured **before** reloading current self for its party `+54`. Matching parties,
or candidate party 2, admit. For differing parties other than 2, candidate virtual
`+5C` query 8 rejects only when it answers true. A candidate can consequently be
queried twice; neither query is cached. Native null handling is local to the first
gate: a null candidate reaching this party path is dereferenced, while a null
candidate admitted by an earlier bypass can reach construction.

The unit-kind and descriptor-kind virtuals are different dispatch domains. The
host must bind the actual receiver's implementation at each site, including any
derived-class behavior. The existing unit-kind table is available for represented
unit identities; a hardcoded Destroyer predicate is not supplied here.

## Duplicate refresh and allocation

`00424C40` is called once after the gates. Its returned stored `+194` float is
loaded by x87 and added to double `00CEC160`, whose bits are
`3FF3333340000000`: exactly widened binary32 `1.2f`, not double literal `1.2`.
`kShipAiNeighbourLifetimeSeconds` already carries this value. The sum spills to
binary32 at `009F0DE1` before comparisons and construction. It is a duration,
not an absolute timestamp; no current-time input occurs here. See
`SHIP_AI_AVOIDANCE_TUNING.md` for the retained settings producer.

The loop scans all live slots in order, comparing each node's owner `+14` with
the unchanged candidate identity. Every match suppresses allocation. A match's
`+78` lifetime is replaced only when `FCOMI fresh,old` followed by `JBE` does not
take: fresh must be strictly greater and ordered. Equal or unordered values do
not overwrite. Multiple matching owners are all visited; there is no early exit.
The native fresh x87 value is retained across the loop, while the store uses its
original binary32 spill. The new helper preserves those value/spill/comparison
semantics and x87 `FCOMI` behavior; its typed interface is not a register-level
replacement and the temporary stack schedule is not identical.

With no match, operator new receives exactly `90h` at `009F0E2F`, with caller
`ADD ESP,4`. A nonnull result is passed to `009E52E0` at `009F0E53`: ECX allocation,
stack `(candidate, controller, lifetime)`, `RET 0Ch` in the constructor at
`009E53A3`, EAX constructed node. The existing L initializer remains **partial**:
it projects represented owner/lifetime/validity/cached-Y fields but omits observer
prefix construction, registration, controller `+1C` and other stores. The new
host requires the complete constructor service; it does not silently replace it
with that projection. See `SHIP_AI_OBSTACLE_OWNER.md` for all stores and preserved
bytes, including registration `009E532D -> 00694A60`.

After either allocation outcome, the caller reloads current count, stores EAX into
that slot, then increments count. A null allocator result skips construction but
still appends null and increments. The actual native operator-new library body
normally retries a new-handler and throws on exhaustion; the null arm is present
in this caller but is not evidence that native out-of-memory normally returns
null. No additional capacity check occurs after callbacks. Existing null nodes,
invalid storage/counts, concurrent mutation and observer lifetime are not repaired.
Callbacks must preserve the actual backing storage required at every access.

## Call contracts and missing definitions

| Caller site | Callee / input | Binding |
| --- | --- | --- |
| `009F0D6C`, `009F0DBB` | candidate virtual `+5C`, query 8, callee `RET 4` | Actual receiver dispatch |
| `009F0D98` | current self virtual `+5C`, query 8 | Actual receiver dispatch |
| `009F0D82` | `00827F70`, ECX captured descriptor | New direct predicate |
| `00827F7A`, `00827F8D` | descriptor virtual `+18`, queries 14 then 12, callee `RET 4` | Actual receiver dispatch |
| `009F0DC8` | `00424C40`, no arguments | Actual settings singleton service, then retained float `+194` |
| `009F0E2F` | `00BF681B`, size `90h` | External CRT allocation policy |
| `009F0E53` | `009E52E0`, allocation/owner/controller/lifetime | Complete external constructor |

All 15 known `00827F70` call sites were inspected; 14 have verified containing
functions. They pass descriptor receivers from unit `+538`, navigation descriptor
`+AAC`, or the captured descriptor parameter in `009FE270`; no stack parameters
are passed. `reports/ship_ai_neighbour_admission.json` records every site and
keeps `0096ACB4` separately under `orphan_call_sites`: the call and preceding
`MOV ECX,[ESI+538]` are byte-verified, but Ghidra has no containing function.
No caller entry or whole-body boundary is invented for that orphan.

Two actual descriptor vtable targets were initially missing Ghidra definitions:
`00D1AD90 -> 00963C80..00963CA4` and
`00D1AE90 -> 00963E90..00963EB4`. Their complete 37-byte leaf compare bodies accept
respectively `{12,6,5,4}` and `{14,6,5,4}`. The final `RET 4` instructions begin at
`00963CA2` / `00963EB2` and are three bytes long. These are exact definition
requests, not reconstructed substitutes or worker-side Ghidra mutations. The
report preserves their initial `no_ghidra_function` status and raw bytes.

## Verification and limits

The ignored `local/prepare_neighbour_admission_probe.py` verifies both complete
bodies and the constant against the installed PE and live saved Ghidra bytes,
then records every relocation and direct call. The fixture executes original
408 bytes / 135 instructions, including the actual branch into `00827F70`.
It relocates the one absolute data operand and four direct calls. Virtual unit
and class queries, settings, allocation and construction are shared fixture
services; native `009E52E0`, observer registration and original CRT allocation
are not executed in this proof. The fixture constructor reuses L's explicitly
partial initializer only for its own controlled test domain.

The 23 cases match all 3,532 canonical trace/storage words. They cover signed
capacity 128/129, both rejection gates, TorpedoBoat and small/large LandingShip
bypasses (including nonzero byte `80h`), self Submarine, same/party-2 handling,
repeated candidate dispatch, multiple duplicate lifetimes, quiet/signaling NaNs,
exact cancellation of widened `1.2f`, maximum float, negative infinity, null
allocation, an admitted null candidate, captured-class/current-self changes,
settings-created duplicates, and allocator/constructor count changes before
publication. Source alone also verifies an allocator exception propagates before
append; this is explicitly not original-byte CRT exception/unwind proof.

The process uses masked x87 exceptions, 53-bit precision and nearest rounding.
Native and source x87 TOP are balanced; native FS exception-list head is restored.
The copied native SEH handler immediate is not executed; no exceptional native
path, unmasked exception or independent virtual/observer/allocator parity claim
is made. Real fixture malloc buffers are freed and the executable mapping is
released. The ad hoc executable embeds its manifest.

Reproduce from this worktree with `python local/prepare_neighbour_admission_probe.py`,
`cmd /c local\build_neighbour_admission_probe.cmd`, then
`local\neighbour_admission_probe.exe`. The build script compiles the probe and
the two tracked admission/L initializer sources directly; it has no dependency
on an older worker worktree or prebuilt core library. Python dependencies are
`pefile` and `capstone`, and the byte verifier requires the configured Ghidra
project. The tracked report contains hashes of all ignored evidence and sources.

`scripts/build.ps1` and its two existing CTests are the compiled checks; the
report records their final result and the live call gate. No tracked test or
runtime host is added. No gameplay/world-list registration, complete native
observer ownership, binary ABI compatibility or full engine equivalence is claimed.

## Correction from docs/UNIT_NEIGHBOUR_FIELDS.md and docs/NATIVE_SHIP_AI_OBSTACLE_NODE.md

The previously unresolved unit field+6B8 is DummyObjectID. Packet O establishes
the actual property and typed-record delivery sites and the complete00953A80
binding operation. This supersedes the meaning-only uncertainty above; the
runtime must still provide the actual retained field and delivery owners.

Packet Q now supplies the complete normal raw90h constructor009E52E0,
common destructor0064A610, scalar wrapper0064B5F0 and callback0064B5C0 through
the existing actual observer lifetime. It preserves all83 untouched bytes,
registration-before-tail ordering and controller identity. This raw API is
separate from the older partial semantic initializer. Admission runtime
integration still needs actual node allocation, endpoint/controller storage
and delivery; a compiled complete constructor alone does not bind those owners.
The native fixture and documented Ghidra body-membership limit remain distinct.

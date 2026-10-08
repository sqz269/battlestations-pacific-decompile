# Native entity registry owner: ordinary Source implementation

This packet implements the fourteen complete ordinary Source behaviors below,
with one permanent canonical owner, real CRT registrations and concrete deletion
dispatch. The normal MSVC Win32 build and all three existing checks pass.
The worker adds no ledger credit; primary acceptance and integration remain separate.

The implementation is based on accepted main `7b244d3b8bc3204b1e37759a90842c28a0b78b63` and the primary-reviewed
registry-owner readiness report. Descriptive role names remain Source hypotheses.
Original function addresses identify evidence; the C++ interfaces are new ABIs.

## Implemented surface

| Original address | Complete Source function | COFF bytes | Normal PE retained |
| --- | --- | ---: | :---: |
| `00923640` | `unwind_native_pending_init_lock_00923640` | 23 | no |
| `00924100` | `construct_native_pending_init_lock_00924100` | 85 | no |
| `00924810` | `get_native_pending_init_lock_00924810` | 220 | no |
| `00925660` | `delete_native_pending_init_lock_00925660` | 105 | yes |
| `00926be0` | `enqueue_native_pending_init_00926be0` | 178 | no |
| `00927a60` | `unwind_native_scene_registry_lock_00927a60` | 23 | no |
| `00928080` | `construct_native_scene_registry_lock_00928080` | 85 | no |
| `00928240` | `get_native_scene_registry_lock_00928240` | 220 | no |
| `009285f0` | `delete_native_scene_registry_lock_009285f0` | 105 | yes |
| `009516b0` | `destroy_native_entity_id_table_009516b0` | 31 | no |
| `00cd39a0` | `initialize_native_pending_init_owner_00cd39a0` | 34 | no |
| `00cdf4c0` | `destroy_native_pending_init_owner_00cdf4c0` | 5 | no |
| `00cd3a70` | `initialize_native_entity_id_pair_00cd3a70` | 123 | no |
| `00cdf500` | `destroy_native_entity_id_pair_00cdf500` | 52 | no |

The fourteen Original spans total **1,044 bytes / 276 instructions**. Their complete
Source extents total **1,289 bytes / 452 decoded instructions**, including compiler
alignment instructions. Exception funclets are additional retained complete extents.
No raw pseudocode, Original numeric profile call or unresolved linker stub is used.

The core declaration is `include/bsp/native_entity_registry_lifetime.hpp`.
`src/native_entity_registry_lifetime.inc` is compiled exactly once by the already
registered `src/native_pending_entity_lock.cpp`. The real application facade is
declared in `include/bsp/game_native_entity_registry_process.hpp` and defined in
`src/game_native_entity_registry_process.cpp`. A single coordinated append-only
`bsp_game` registration was added to `cmake/startup.cmake`; no existing registration
was changed. The deletion dispatcher and its contract header gain the two actual
profile cases.

## One owner and one manager authority

`NativeEntityRegistryProcess` has private construction, deleted copying and trivial
destruction. Its sole process accessor compiles to **MOV address; RET** and returns
one 200-byte writable uninitialized COFF object. There is no allocation, local-static
guard, implicit CRT registration or C++ exit destructor in that accessor.

Its raw CC list is 0Ch. Its primary and alternate ID headers are adjacent raw 54h
objects, with an asserted 54h displacement and A8h pair size. E4 and FC are distinct
volatile pointer cells. The startup marker and two registration statuses are explicitly
Source-only state. This Source container does not reproduce the Original global-address
gaps and does not claim the Original process-wide object layout.

The private `GameNativeEntityRegistryProcess` facade owns two references: that same
core owner and `game_native_string_process().manager_01090aa0()`. Its complete 22-byte
constructor calls the actual two canonical accessors and stores their addresses.
Current `manager_01090aa0()` aliases offset zero of the real string-process object.
There is no second manager, copied publication value, temporary registration domain
or app-only unresolved symbol in `bsp_core`.

The facade is trivially destructible. Its lazy construction binds references only;
`initialize_crt_once()` is explicit. Neither application startup nor any real entity
producer calls the new facade in this packet.

`ObjectHandleTables` contains references to the CURRENT primary +4/+8/+4C and alternate
+4/+4C fields. The compiled view uses five LEAs at process offsets 10h, 14h, 58h,
64h and ACh, then stores those addresses in the returned view. It copies no native
field values and computes no replacement end. The existing `first_end_00f89a10`
interface name still aliases the native +8 field directly.

## Explicit CRT lifecycle

The explicit startup API runs CD39A0 before CD3A70, matching their established relative
CRT table order at CE30CC and CE3120. It records and returns both actual `std::atexit`
statuses. A nonzero pending-list registration status still proceeds to the ID initializer.
The Source-only attempted/completed marker prevents replay after a partial exception
and avoids duplicate successful registrations on repeated API access.

CD39A0 allocates the real 0Ch sentinel through the accepted provider, publishes head,
zeros count and registers a fixed noncapturing CDF4C0 callback. It preserves owner+0
and sentinel payload+8. Allocation failure publishes/registers nothing. Registration
failure keeps the initialized list. The callback targets the same permanent CC owner,
detaches the ring, frees nodes in next-link order, frees the sentinel and clears head.
It never deletes borrowed entity payloads.

CD3A70 constructs primary `(first=0, count=1000h)`, then alternate
`(first=1000h, count=1000h)`, then registers the fixed CDF500 callback. Its compiled
ordinary catch destroys only primary after an exception following primary construction.
It adds no alternate cleanup and no repair of the original dangling fields. Failed CRT
registration retains both initialized tables and the actual status.

CDF500 destroys alternate and then primary. Complete plain9516B0 captures slots+4C,
stamps the actual typed Source D19B88 profile and frees slots. It does not free either
54h header or clear its fields. Both successful registrations therefore run ID teardown
before the pending-list callback. The owner has no second automatic destructor.

Full application CRT scheduling is still a separate contract. The caller must arrange
all other native initializers and keep the process quiescent, the rings intact and
entity/world withdrawal complete before these ID arrays are freed. CDF4D0 is the
different F899C0 owner; it is not the ID-pair callback.

## Locks, deletion and pending append

E4/D190C0 and FC/D19284 constructors allocate the actual tracked critical section and
publish +4 only after its successful creation. Constructor failure clears the supplied
publication and stamps CE3818 without overwriting the +4 preimage or releasing a section.
The ordinary constructor/getter/base-unwind families reproduce the unchanged accepted
E8 family across **16 complete main/catch/guard extents**, after only explicit role-name
and profile substitutions.

Each getter returns its first captured nonnull publication on the fast path. The slow
path captures the first canonical manager section, enters/increments, double-checks,
allocates raw8, constructs, ends allocation cleanup state, publishes, performs the
second manager lookup, then rereads publication for registration. It unlocks the first
captured section and reloads publication for return. Constructor failure frees captured
raw8 after base unwind. Registration failure retains owner/publication and releases only
the captured manager section.

The full scalar bodies stamp their own profile, release actual +4, clear their actual
process publication unconditionally, stamp CE3818 and free only for flags bit0. They
return the captured address bits. The compiled flag test is the low-byte bit0 test.
They do not compare publication identity or unregister the owner. The concrete
`delete_current_profile` cases use these same process E4/FC cells and the popped owner.

Complete926BE0 accepts borrowed actual root bits and requires an initialized intact
finite CC ring. It obtains E4, captures its section, enters/increments and captures
head/previous. The real node allocator reads the root-pointer local; count growth
occurs before either link store. It stores captured head.previous, then reloads the
new node.previous and stores previous.next. Normal exit decrements before Leave.

The producer does not dereference an entity, inspect flags, deduplicate, retain or
translate a semantic object. A count exception intentionally leaves its new unlinked
node allocated. Its sole compiled unwind funclet destroys the captured section guard;
there is no node-free rollback. FC and E4 remain distinct, preserving the outer scene
and inner pending lock roles for the separately admitted future caller.

## Whole artifacts and preservation

The actual normal build ran from **2026-10-08T21:23:32.054970+00:00** to
**2026-10-08T21:24:52.421042+00:00**, after physical pre-build capture completed. All three existing
checks passed. No new test, probe, selected entry, startup or game was executed.

The final provider cohort has **25 whole COFF objects / 425 complete function extents**
and **23 exact unique complete members** of the retained 1,908-member core archive.
The two application objects are separately retained. The static direct-call closure
contains **34 roots, 83 decorated Source functions and 169 edges**, with no unresolved
BSP target. It includes the actual lower string/SBO and singleton-vector providers.

Six additional provider objects were extracted from the already frozen complete
pre-edit/post-build archives using their actual full member offsets and payloads.
They are explicitly late extractions of historical snapshots, never fabricated
individual pre-build captures. **3,547 actual per-TU compiler-read dependencies** match
their physical pre-build and post-build images. Tool binaries, commands, recipes and
the ordinary build/check log are retained; this does not claim loaded-tool tracing.

**355 old complete extents** retain identical bytes and ordered relocations. The sole
changed old extent is the dispatcher. Its whole body and embedded compiler data are
retained; all **73 old profiles plus two default sentinels** keep identical reachable
leaf instruction/relocation graphs after branch-displacement normalization. The two
new profile leaves are D190C0 and D19284. The expanded cohort has 69 new complete
extents, including C++ helpers and EH functions; that is not a count of native APIs.

The same normal map and PE verify **230 complete linked extents and 745 ordered
relocations**, with zero unresolved mapped extents. These are qualifications of COFF
symbol extents: COMDAT folding yields **154 distinct address/length spans and 568
distinct relocation sites** in the actual PE. The remaining **195 compiled extents
are absent from that map**. Of the fourteen new behaviors, only the two scalar
deleters are retained in the normal executable. The unused initializer/getter/producer
and Game facade entry COMDATs are not force-linked. Whole current COFF proof and final
PE presence are reported independently.

The accepted 37 complete Native owner/support/EH spans, **1,678 bytes / 467 instructions**,
were rehashed against the unchanged installed image:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The prior Ghidra memory/export evidence remains
dated prior evidence; this Source packet makes no fresh Ghidra-memory query or mutation.

## Evidence and boundary

Evidence archive: `local/cc12_native_entity_registry_owner_source_evidence.zip`.

- SHA-256: `200a4662168abfb062938004fb0973c4ef258d17794d1538bfc1c58f8666b1fa`.
- Size: 213,920,409 bytes; 13,457 entries.
- Every ZIP CRC and manifest member SHA-256 was verified.
- Manifest SHA-256: `dc0687550e4efc787ea792ec9e8ffb6517e91d9acc2a4016dbec14750de6e342`.
- Structured record: `reports/cc12_native_entity_registry_owner_source.json`.

The final document/report are generated after sealing and deliberately excluded from
the ZIP to avoid circular hashes. The seal receipt remains outside that archive.

This is ordinary Source reconstruction, compilation, artifact/CFG/relocation proof and
three existing checks. Native thiscall/register/RET cleanup, FH3/SEH transport, mutable
native stack aliases and hardware faults are not reproduced. In particular, the Source
ID/free providers are noexcept; the Original CDF500 handler's hardware-fault cleanup is
outside the admitted Source exception domain.

Canonical entity/World production and hierarchy authority, raw Unit/factory/scene-base
construction, descriptor/profile dispatch, raw InitAll consumption and withdrawal,
full startup/CRT composition, Original ABI and gameplay remain separate frontiers.
Existing semantic application registries and entity call sites are unchanged. This
packet does not make the larger native entity factory ready for execution.

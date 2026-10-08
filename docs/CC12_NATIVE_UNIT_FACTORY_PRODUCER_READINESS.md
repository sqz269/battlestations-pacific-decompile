# Destroyer instance producer readiness

Packet `cc12_native_unit_factory_producer_readiness` is a read-only audit of
whole Original `006FE590` and `006FE460`. **New Source: 0. Ready Source owner
or wiring packets: 0.** The allocation and derived-constructor order are now
retained directly, but an allocation-only implementation would still omit the
actual descriptor, constructor chain, publication and destruction domain.

Fresh exports verify project `bsp`, program `/battlestationspacific.exe`.
Installed-image bytes decode to all fresh listing instruction starts:

| Original function | Complete extent | Bytes / instructions |
| --- | --- | --- |
| Destroyer descriptor instance factory `006FE590` | `006FE590..006FE612` | `131 / 40` |
| Destroyer derived constructor `006FE460` | `006FE460..006FE4C2` | `99 / 17` |

Names remain hypotheses. This packet analyzes these two functions only;
dependencies below use existing qualified Source and evidence.

## Exact allocation, preimage and constructor order

`006FE590` receives the real descriptor in ECX and one unchanged 32-bit flag
on the stack, returning the successful unit pointer in EAX with `RET 4`.

| Site | Actual operation |
| --- | --- |
| `006FE5A8 / 006FE5AF` | Request `1188h` bytes from `00BF55BE` |
| `006FE5AD` | Keep the incoming descriptor in EDI |
| `006FE5B4..006FE5BE` | `memset(allocation, 0, 1188h)` through `00BF79F0` |
| `006FE5C3` | Pop all four cdecl argument words: allocation size plus three memset arguments |
| `006FE5C6..006FE5D4` | Save the allocation, test it, store EH state 0, then branch on the earlier test |
| `006FE5D6..006FE5DD` | Pass the original flag unchanged; ECX is the allocation; call `006FE460` |
| `006FE5E2` | Retain the constructor's returned root in ESI |
| `006FE5E8..006FE5F3` | Clear EH state to -1, then call descriptor-owning setter `009553D0(root, descriptor)` |
| `006FE5FC` | Store the same descriptor into root `+354h` |
| `006FE603 / 006FE610` | Return that root in EAX / `RET 4` |

The **whole 4,488-byte allocation is zeroed before the derived and base
constructors run**. That includes the eventual wake range
`[unit+BD0h, unit+FACh)`, but the zeroing belongs to this actual Destroyer
allocation producer. It does not authorize a detached 988-byte reset array,
an extra 1188-byte shadow allocation, or skipping the intervening constructors.
Preservation of particular bytes across every earlier base-constructor effect
is a separate question from the proved initial all-zero preimage.

The memset precedes the null test. A hypothetical null allocator return would
reach a positive-length write to null before the displayed null branch; this
is not a proved safe failure path. The current semantic creation model's
nullable host/result interface must not be promoted to the actual failure ABI.
The factory saves the allocation at entry ESP-16 and its EH state at entry
ESP-4; the caller flag is read back from entry ESP+4. Its handler immediate is
`00C83AB8`. No handler or unwind-table behavior was newly analyzed, so there is
no admitted failure rollback or factory-exception ownership policy.

`006FE460` first calls `0081ED40` with the same root and unchanged flag at
`006FE468`. Only after that call returns does it publish this exact table set:

| Root offset | Original vptr word | Store site |
| --- | --- | --- |
| `000h` | `00CFC3D0` | `006FE46D` |
| `010h` | `00CFC3B8` | `006FE473` |
| `024h` | `00CFC3B0` | `006FE47A` |
| `170h` | `00CFC3AC` | `006FE481` |
| `1E4h` | `00CFC3A4` | `006FE48B` |
| `310h` | `00CFC38C` | `006FE495` |
| `38Ch` | `00CFC388` | `006FE49F` |
| `72Ch` | `00CFC384` | `006FE4A9` |

It then stores class id 7 at `+C4h`, returns the same receiver, and performs
`RET 4`. These are literal Original table addresses, not usable Source virtual
tables. Copying these integers into newly allocated Source memory does not
implement their methods or their base-constructor stages.

## Existing dependencies and real lifetime boundary

The independently accepted wake-owner audit retains whole `0081ED40` and its
call of the wake constructor at `0081F03D`, receiver `unit+BD0h`. Current Source
offers a subobject projection plus the `2Ch` navigator-side-block seed fragment,
not a whole live vehicle-base constructor. Existing constructor-chain evidence
continues through `0095CC90`, `0087B670`, `0077EED0`, `00928630`, and `00925CE0`.
The creation model announces selected weak-owner, matrix, name, world/ID,
registration, tick-node and subobject operations through abstract hosts.
Its result structure records vptr values and class/descriptor fields; it does
not materialize the complete returned root.

The actual raw wake constructor/destructor, tracked-section services,
canonical geometry owner, observer-prefix producers, and a real
malloc/new-handler allocation service exist. Their individual availability
does not supply the missing enclosing constructor, descriptor, dispatch or
allocation-root owner. The observer prefix is only `20h` bytes. Existing raw
vehicle activation APIs also borrow already-valid class storage and retain
required external providers; they do not produce a Destroyer descriptor.

Existing `009553D0` evidence identifies owning descriptor slot `+538h`; the
factory's direct `+354h` store is the second, non-owning link. Source only
exposes `UnitInstanceCreationHost::set_vehicle_class` for this operation.
The current `VehicleClassDescriptor` model is a semantic subset with abstract
construction/loading hosts, not the real descriptor storage and vtables that
this factory requires.

There is **no normal root free in either newly read function**. For the actual
Destroyer, existing destruction evidence identifies `00CFC3D0[0] = 006FE570`,
which delegates to parent destructor `0081F3A0` and frees the root when
`flags & 1` is set (`006FE578`, free site `006FE580`). The parent includes the
wake section cleanup among its ordered subobject/base destruction. Those
destructor bodies and allocation/unwind handlers were not newly analyzed here.
Current `UnitReleaseHost`/`run_unit_release_0081f3a0` is an unbound semantic
plan; `free_instance()` is abstract. A wake-section destructor cannot substitute
for that enclosing lifetime or free the unit allocation.

## Current application handoff

The earlier proved boundary still holds at current main `23a82cd9a`:

- `SceneUnitCreatorBinding::vehicle_class_descriptor` logs `00964790` as
  unimplemented and returns a numeric `type_id + 1` token for nonnegative IDs.
  Its retained compiled body is **49 bytes / 21 instructions**, with the logger
  as its sole call. It does not return a constructed native descriptor.
- `create_instance_from_descriptor` logs `006FE590` as unimplemented, sets
  `entity_.created`, and returns `&entity_`. Its whole **40-byte / 12-instruction**
  body likewise has only the logger call.
- The scene creator's remaining placement/name/command operations operate on
  this record. The mission frame later passes records into `create_units`,
  which separately makes stable `unique_ptr<GameUnitSlot>` instances.
- `GameUnitSlot` remains the genuine application identity and owns the semantic
  trail. Mission teardown withdraws outward borrowers before destroying the
  units; that real existing lifetime is not a Native unit allocation/destructor.

`create_unit_instance` is a **561-byte / 186-instruction** compiled semantic
host-driven model. No concrete application implementation of
`UnitInstanceCreationHost`, `UnitSubObjectHost` or `UnitReleaseHost` was found.
The current source inventory also finds no full raw factory/derived/base
constructor provider that closes this gap. No model is cast to the raw wake.

## Cohesive missing domain

There is no viable factory-only Source implementation packet to admit. A
genuine persistent owner must first close all of the following together:

1. A real Destroyer descriptor and its owning `+538h` / borrowed `+354h`
   relationships, with actual reference-count and dispatch behavior.
2. The actual allocation and full zeroing at the factory stage, followed by
   the complete constructor/base/subobject sequence and functional Source
   dispatch. Original literal vptrs and global addresses cannot fill this role.
3. One canonical unit identity through construction-time world/ID/tick
   publication, scene placement and the existing application slot handoff.
   Adding another raw allocation beside the live semantic slot leaves two
   authorities and does not establish the original producer.
4. Withdrawal of every borrow, ordered real subobject/base destruction and
   matched allocation-root free. The exceptional domain additionally needs the
   actual factory/base unwind ownership contract; no rollback is invented.
5. The same real wake state for every current fill, append, handoff,
   decomposition, distance-sampling and direct diagnostic consumer.

These are explicit missing dependencies, not a proposal to implement the
whole vehicle-class ABI in this read-only packet. A future bounded readiness
packet may investigate one named base constructor or the actual descriptor /
root-destruction boundary under fresh address ownership. Until the connected
owner domain is admitted, bare allocation/zeroing, a callback owner, or a leaf
constructor call earns no producer or Source credit.

## Physical evidence and limits

`reports/cc12_native_unit_factory_producer_readiness.json` indexes the sealed
evidence under `local/cc12_native_unit_factory_producer_readiness_evidence`.
It contains both whole Original functions, stack/call-order checks, current
Source inputs, twelve complete existing worker objects and 751 selected
complete function extents, including six application methods.

Nine core objects match unique whole members of the worker's historical
1,905-member archive. A separate retained Root archive from the accepted prior
review contains 1,908 members and the same nine providers. Their whole function
bytes and actual target-section graphs agree; compiler anonymous-namespace
and lambda names differ in some records. Those are paired one-to-one only
after checking actual complete section bytes, symbol offsets/types/storage,
section flags, and ordered relocation targets. Literal symbol strings and
whole object hashes are not claimed equal across worktrees.

Twelve historical compiler command/read records, currently available physical
dependency files and configured compiler binaries are retained. The worker
archive predates this main merge; Root's separate archive has its own retained
receipt. Neither is a fresh build for this packet or a reconstructed
consumed-input preimage. No C++ changes, build, tests, API/Original execution,
Ghidra mutation, final executable linkage, whole-class ABI or gameplay
validation occurred. Source and ready-packet counts remain zero.

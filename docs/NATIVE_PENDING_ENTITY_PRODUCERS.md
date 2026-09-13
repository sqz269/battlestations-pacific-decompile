# Pending entity destroy and kill producers

Addresses: `00926C80`, `00926D90`. Existing descriptive Ghidra names are retained
as hypotheses. This component borrows the actual entities, the existing two
`NativePendingEntityOwners` lists and the actual pending lock owner. It adds no
queue, entity owner, runtime binding, synthetic native vtable, or STL implementation.

| Routine | Coverage | Original ABI | New source |
|---|---|---|---|
| `00926C80..00926D8A` (267 bytes) | complete normal body, external providers retained | ECX actual entity; stack DWORD recurse; RET4 at `00926D88`; no defined return | `native_pending_entity_destroy_00926c80` |
| `00926D90..00926E7D` (238 bytes) | complete normal body, external providers retained | ECX actual entity; stack DWORD cause; RET4 at `00926E7B`; no defined return | `native_pending_entity_kill_00926d90` |

The interfaces are new C++ APIs, not binary replacements. C++ exception cleanup
releases the captured section. Original FH3 dispatch, hardware faults, instruction
identity and game runtime behavior remain unproved. The semantic implementations
in `unit_damage.cpp` are unchanged; the base already includes its earlier low-byte
recursion and callback-mutated parent-cause corrections.

## Borrowed storage and producers

`NativePendingProducerEntityView` contains references, rather than an object
overlay or copied entity fields. `resolve_entity` is a required pure mapping from
the exact actual identity to that same owner's live fields. It must not invoke
callbacks, allocate, or cast unrelated semantic metadata to a native layout.
Views and callback-visible owners must survive through their native last access.

The parent/sibling/child roles are established by `009258F0`: `00925935` stores
the parent at +3Ch, `00925942` links the previous child through +44h, `00925959`
publishes the first child at parent+48h, and `0092594E/62` terminates the new
child's +44h. This is separate from `CameraTransform`'s different link offsets.
`00925CE0` stores first child +48h at `00925D64`, zeroes destroyed +60h and killed
+5Fh at `00925E08/0B`, and zeroes cause +70h at `00925E1A`.

The lists reuse the producer-established 0Ch owner/node aliases from
[NATIVE_PENDING_ENTITY_OWNERS.md](NATIVE_PENDING_ENTITY_OWNERS.md): owner+4 head,
owner+8 count; node+0 next, +4 previous, +8 actual entity payload. Owner+0 and
sentinel payload are unconsumed. No sidecar counts or private pending lists exist.

## Destroy ordering

The routine obtains `009248D0`, captures its actual owner+4 section in EBP, enters
that section if nonnull, then increments its +18h DWORD. It performs this even
when entity+60h is already nonzero. The source depth update preserves DWORD wrap.

When +60h is zero, `00926CD1` reads the existing cause before `00926CD5` writes
+60h=1. Only an initially zero cause is replaced: use the nonnull parent's cause
when that parent has +60h set; otherwise use 1. At `00926CF7` only the recurse
argument's low byte matters. Thus `0x100` skips children and `0x101` visits them.

Each child comes from +48h then live +44h. Its actual slot+78h receives the parent
at `00926D0E`. If AL is nonzero, `00926D14` reloads the parent's cause after that
callback. A zero value clears child+70h before actual child slot+70h receives 1
at `00926D2A`. The sibling pointer is reloaded after either callback path.

## Kill ordering

Cause 7 maps to stored cause 2, while the original DWORD survives in EBX. After
capturing/entering the same lock domain, an existing +5Fh skips the entire body.
Otherwise `00926DEF` reads +60h before `00926DF3` sets +5Fh. A clear +60h causes
the mapped cause write followed by actual entity slot+70h(1) at `00926E05`.
Original cause 7 writes the mapped value again at `00926E0C`, even if the callback
changed it or +60h was already set.

Every child is then passed the original cause through a **direct recursive call
to `00926D90`** at `00926E19`. This is not a virtual Kill callback. Each recursion
obtains/captures its own current lock and enqueues the child before the parent.
The next sibling is read after the recursive call.

## Allocation, count and links

Both producers read the current list head and its previous pointer only after
the entity/child operations. They capture the old head's previous-cell address,
pass next, previous and the source-pointer-cell address to `00924B10`, then call
`009267F0(list,1)`. Only after successful count growth do they write captured
head->previous and then **new node's current previous**->next. Replacing the live
list head during a provider call does not redirect the captured first store.

| Call sites | Native provider and read contract |
|---|---|
| `00926C9C`, `00926DBE` | `009248D0`, existing pending owner getter; body read; no native inputs, EAX actual 8h owner, RET; null owner is an invalid-memory boundary, null section is allowed |
| `00926D0E` | child's actual vtable+78h; parent stack DWORD, AL result; provider-specific, no universal default |
| `00926D2A`, `00926E05` | actual vtable+70h; entity ECX and literal1 stack DWORD; provider-specific |
| `00926D4E`, `00926E40` | `00924B10`; complete 51-byte body read; stack next/previous/source-cell, RET0Ch; allocates 0Ch through `BF681B`, writes links, then reads payload source; incoming ECX unused; concrete default reuses `create_effect_deletion_node_008665f0` |
| `00926D5C`, `00926E4E` | `009267F0`; complete 147-byte generic STL body read; ECX actual list, increment stack DWORD, RET4; unsigned `3FFFFFFF-count` guard, then captured count+increment publication; concrete default reuses `grow_effect_deletion_list_count_008675e0` |
| `00926E19` | direct `00926D90(child, original cause)`; RET4 |

All callers of the two list helpers were surveyed: these producers, `00926BE0`,
`00927610` and `009269B0`. The common stack cleanup/roles agree. `009267F0` is
already correctly named `STL_xlen_throw_009267f0`: it builds a legacy length-error
object on overflow. Neither library body is newly ported. The integrator proved
the complete allocation/count bodies and count exception graph equivalent to
existing providers in [pending_entity_library_reuse.json](../reports/pending_entity_library_reuse.json):
12 live/disk spans, 520 bytes, with only relative calls and matching EH identities
normalized. Both library methods therefore have concrete canonical defaults;
overrides remain available for explicit provider substitution and focused evidence.
Allocation returning null is not turned into a successful empty enqueue: the
original node helper then reaches its invalid previous-cell address.

Flags and cause writes precede allocation; no rollback is inferred. A count
exception occurs after node allocation and before either ring-link publication.
The source leaves that unlinked node allocated, matching the normal body's
absence of node cleanup and its single native state0 unwind entry.

## Exception evidence and verification limits

Native FuncInfo `DDA0E4`/`DDA110` each has one unwind state. Maps `DDA0DC`/`DDA108`
point to `CA6DC0`/`CA6DE0`, whose complete 8-byte bodies jump to `00411EE0` on the
captured stack guard. The complete 25-byte guard body decrements the captured
section's +18h and leaves that same section; it has no flag/list/node cleanup.
Handler thunks `CA6DC8`/`CA6DE8` have no Ghidra functions; their exact ten-byte
PE/live-matching spans are retained without annotations or invented function
boundaries. The original source body sizes include all bytes through RET4.

The ignored fixture retains 505 original producer bytes and 149 additional
guard/FH3 bytes, with 30 relocation records. Seven normal native/source capture
pairs match 5,852 exact input bytes and 5,856 result bytes per side. Cases cover
low-byte recursion, callback mutation of parent cause/sibling/lock publication,
already-destroyed gating, inherited cause, raw-cause recursive Kill with mapped
cause restoration, already-killed child gating, and provider mutation of head
and new-node previous before the two link stores.

The first native exception attempt exited with uncaught C++ exception
`E06D7363`; the precise compatibility failure is unproved. Its source, objects,
executable and log are retained. Two separate **source-only** cases verify that
allocation/count exceptions preserve published flags, leave an allocated node
unlinked after count failure, and release the captured section. They are not
native EH differential results. No game frame behavior is claimed.

The JSON report records the completed Win32 build/CTest and live call-verifier
results, together with the retained proof manifest, source/headers, objects,
libraries, tools/hashes, exact inputs, outputs and failed attempts.

The final Win32 Release build and both existing CTests pass; all nine direct
call/tail-jump rows pass the live verifier. One additional source composition
inherits both concrete library defaults, creates real malloc-backed destroy and
kill nodes, checks both ring/count/payload publications and final flags/cause,
then frees the fixture nodes. This proves actual provider composition separately
from the seven native pairs that use controlled providers to expose call ordering.

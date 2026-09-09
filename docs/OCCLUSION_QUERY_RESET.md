# Concrete occlusion-query reset owner

`D3D9OcclusionQuery` and `D3D9QueryRegistry` implement the concrete listener
identified in `RESET_LISTENERS.md`. They do not implement arbitrary reset
callbacks or fabricate query results. The device operation is real
`CreateQuery(D3DQUERYTYPE_OCCLUSION)` with native query type 9.

## Owner lifecycle and native ABI

Native constructor `00b5fe90` takes object ECX, consumes no stack arguments,
and returns object EAX. The 20-byte original has intrusive count one, vtable
`00d62ad0`, field `+08h=1`, field `+0ch=0`, and owned query pointer `+10h`.
The new C++ owner preserves the two neutral-named fields and COM ownership;
it does not reproduce the vtable, allocator, intrusive count, or original ABI.

`create_occlusion_query_00b5fe90` requires an empty COM owner, initializes
`field_08=1` and `field_0c=0` before the device call, and adopts its returned
reference. No extra AddRef is introduced. An explicit device argument replaces
the native renderer-singleton getter `00b1fef0`. These field meanings during
query issue/polling remain unaudited, so no visibility or completion semantics
are invented for them.

Reset release `00b5fe20` and restore `00b5fe60` also take native wrapper ECX
and return with ordinary RET; restore has no native stack device argument.
Neither has an internal guard or readiness/lost gate.

- Release calls COM Release once if nonnull, then clears the pointer. It does
  not add a balanced AddRef/Release pair, reset either neutral state field,
  unregister, or destroy the owner.
- Restore obtains a device and creates query type 9 directly into the native
  member. The typed function receives a stable device explicitly, requires an
  empty owner instead of overwriting a live owned pointer, and preserves both
  neutral state fields.

Native ignores CreateQuery HRESULTs. The new interface reports failures,
initializes a temporary output, releases any pointer returned on failure, and
returns E_POINTER for unexpected success without a query. It does not silently
replace an unsupported query with an empty callback or fallback visibility.
Constructor field initialization remains visible on failure; restore leaves
other fields unchanged. There is no pending-query result recovery claim.

The new destructor drops the owned query reference only. Native full destructor
`00b5fda0` also unregisters through renderer virtual `+28h`; typed callers must
remove list membership before owner destruction. No renderer singleton or
hidden registry side effect is fabricated in the C++ destructor.

## Borrowed listener list

The registry projects renderer `+19a0h` / `+19a4h` / `+19a8h` as borrowed
pointers. `append_00b27c20_fragment` preserves append order and adds no COM or
intrusive reference. It is the append phase, not native factory allocation,
constructor, or outer guard. `remove_00b25290` removes the first pointer match,
swaps the final entry into its place when necessary, and reports whether it
found the entry. It performs no release. Duplicate registration is not silently
deduplicated; each registration must be removed before owner destruction.

The parent renderer integration supplies optional guards around registration
and the `00b27cf0` unregister adapter. These methods themselves are unguarded
fragments. Owners, membership, and backing storage must remain stable during
reset traversal; supported sizes fit the native nonnegative signed count.

Release/restore traversal fragments use an index and current vector size,
matching the native signed-index iteration within that stable domain. They
invoke only these concrete query lifecycle functions. Restore attempts every
query even after an earlier HRESULT failure, matching the original lack of
failure-based loop termination, and returns the first failed HRESULT as a new
reporting feature. An explicitly supplied stable device replaces the native
per-listener singleton lookup. The full resource readiness gates, other
resource arrays, outer guard, and Reset call belong to the scheduler.

## Integration and validation boundary

No query Issue or GetData implementation was added. Those methods require a
separate audit before any visibility/completion behavior can be claimed.
A successful creation or reset proves only this COM lifecycle component,
not renderer occlusion correctness or gameplay.

Parent integration provides guarded `create_registered_query_00b27c20_fragment`
and `unregister_query_00b27cf0` methods in D3D9StateCache. The factory fragment
requires a fresh caller-supplied owner and appends it even after a CreateQuery
failure, as native does. It reports HRESULT; unregister reports found, which
is not an established native return contract. Native allocation and automatic
destructor unregister remain outside this typed owner.

The existing real-device reset probe creates two occlusion queries, registers
them, releases their COM owners after defaults and before textures, resets the
device, then restores them after surfaces. Both report type OCCLUSION and
DWORD-sized results through SDK introspection; no query is issued or polled.
The probe checks state-field preservation, first-entry swap removal, missing
removal, retained COM ownership and balanced guards. The build, both existing
CTests and full installed-asset D3D9 probe pass. No new test target was added.
See `reports/query_reset_probe.txt` and `reports/query_reset_audit.json`.

Each evidence batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Complete bodies matched
the installed PE bytes:

| Address | Bytes | SHA-256 |
|---|---:|---|
| Constructor `00b5fe90..00b5fefe` | 111 | `469ab83ca834099c95a66eaf40fced39235cc366516f045a472137cfb587d4f0` |
| Release `00b5fe20..00b5fe3a` | 27 | `9c45583ceed898e5c04ef5b8a6a7100afb71a45a97fc601bd4e1e7751f315ef2` |
| Restore `00b5fe60..00b5fe80` | 33 | `31eeaea774758bf37204434d865460bfd1c18f2746cec2e899b08dd421b2269c` |
| Removal `00b25290..00b252f6` | 103 | `24d2aab156d7889ea7543c821483002b5f93b654671b47704e8253690ef1b8d0` |

Factory, unregister wrapper, full destructor and vtable evidence remain in
`RESET_LISTENERS.md`. Parent integration checked the factory's 204-byte dry-run
body and created its missing function, applied descriptive names/evidence with
prior values preserved, saved the program and refreshed inventory/exports.

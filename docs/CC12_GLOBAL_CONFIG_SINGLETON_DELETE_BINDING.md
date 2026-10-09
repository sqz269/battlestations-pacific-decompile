# GlobalConfig scalar binding in the shared singleton drain

This packet adds the existing GlobalConfig scalar to the finite Source profile
dispatcher. `CE3D98` now calls `scalar_delete_global_config_00432710` when the
appended `GlobalConfigContext* global_configuration` binding is non-null. The
call receives the popped owner, the full flags word, and the borrowed context;
the adapter immediately returns. A null binding still reaches the existing
`std::logic_error`. This is zero additional Original-function credit.

The only Source changes are in `include/bsp/native_singleton_destruction.hpp`
and `src/native_singleton_destruction.cpp`: a forward declaration, one appended
default-null pointer, its offset/size assertions, one include, and one guarded
switch case. Existing GlobalConfig code and the manager's pop/recount schedule
are unchanged. No app assignment, context creation, registration, publication
reset, effects substitution, or method-table installation was added.

## Native target and scalar evidence

Read-only queries used the configured `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe`. Two live byte spans independently match the
installed PE: the complete 30-byte `00432710..0043272D` scalar and four bytes at
`00CE3D98`. The profile's slot zero is `00432710` (`10 27 43 00`). The scalar's
11 instructions decode completely through `RET 4`:

- ECX is the owner; ESI captures it before the unconditional `004325B0` call.
- One stack DWORD carries flags. Only bit zero controls the `00BF65AC` free.
- EAX receives the captured original address; no owner dereference follows free.

The Source dispatcher maps the numeric profile to the recovered Source scalar.
It does not call a numeric Original-process code address. The adapter forwards
all flag bits so the existing scalar remains responsible for `flags & 1`.

## Borrowed context and payload boundary

The popped object must be the actual complete `0x2E8` GlobalConfig allocation.
The binding must retain the same raw construction/publication, string, effects,
and lifetime domain used to create and register it, through the complete drain.
The existing scalar destroys the raw members and unconditionally clears the
context's `F878E4` publication, even if that publication no longer identifies the
popped owner. The new case adds no publication-identity check. Allocation and
free remain in the existing `singleton_lifetime_allocate` /
`singleton_lifetime_free` domain, using Source malloc/free.

Payload admission remains separate. The existing release accesses actual
object+4 and invokes the retained effects contract. If the new current-table
effects provider is selected, payloads must expose callable current-process
tables and nonthrowing callbacks. Numeric Sound profile identities and projected
Sound storage do not establish that contract. The unknown non-null second-array
producer remains unresolved; this packet assumes neither emptiness nor a fixed
payload profile. Original exceptional EH/SEH equivalence is not claimed.

## Source ABI and verification

On the MSVC Win32 target, the struct grows from **180 to 184 bytes**, retaining
four-byte alignment. All 45 existing pointer fields retain their offsets; the
last existing field, `particle_clock`, remains at 176. `global_configuration`
is appended at 180 and defaults to null. Recompile all binding producers and
consumers together: an older 180-byte object cannot satisfy the new access at
offset 180. These are Source layout assertions and inspection evidence; this
worker did not compile a layout probe.

The report preserves all prior member offsets, the exact Source diff, physical
and normalized source hashes, reversible edits restoring both preimage Git
blobs, five existing-source observation pins, and both native byte spans with
the complete scalar decode. `git diff --check` passed. No build, test, probe,
native execution, Ghidra write, or ledger edit occurred in this worker packet.
Primary integration, the normal build, and emitted-code review remain pending;
this is not runtime or game validation.

Evidence: [cc12_global_config_singleton_delete_binding.json](../reports/cc12_global_config_singleton_delete_binding.json).

## Primary build and case review

The primary independently replayed all 34 native bytes, both exact Source preimage reversals and five observation pins/excerpts. Win32 offset/size assertions pass. The compiled switch selects CE3D98, loads context at B4h, sends null to the existing failure, and otherwise pushes context/full flags/captured owner for the named scalar call. It then goes directly to the common frame epilogue. The complete existing scalar is 39 bytes / 17 instructions and preserves flags bit0/free/captured-pointer return. Its presence in the game map follows static dispatcher linkage; no application assigns this context or demonstrates that the case executes. The whole 3193-byte dispatcher span includes embedded switch data; manual instruction review covers the changed case and its selection/failure/return paths, not all existing cases.

The normal MSVC Win32 build exited zero and all three existing checks passed. Seventeen bounded physical Source/recipe fingerprints were captured before the build, rechecked afterward and retained with whole objects, unique exact Core members, library, map and logs in `local/cc12_current_dispatch_primary_review`. No new tests/probes or current application runtime checks were performed. See `reports/cc12_current_dispatch_primary_review.json`.

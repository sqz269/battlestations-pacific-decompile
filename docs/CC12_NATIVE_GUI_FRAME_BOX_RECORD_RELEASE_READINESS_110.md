# Native FrameBox record resource release readiness 110

The existing `dispatch_native_render_actual_owner_zero` is an honest no-flags
post-zero Source service for the **canonical registered 2D texture family traced
here**. It does not decrement again. The first remaining Source work is the
raw AD0860 record operation and its retained, one-shot failure metadata; a new
texture owner or substitute terminal is unnecessary for this family.

This is a read-only readiness result. No Source, CMake, ledger, saved Ghidra or
installed-game bytes changed, and no build, test, probe or runtime was run.
Worker baseline is `93928db54ce14f683dbd23281c0bc44441e995ee`; all 18 relevant
Source files were byte-identical to Root at
`a17278d8358e1234a2dc10aecec9bf5a72e092f7`. The prior 98/103 contracts are reused,
not reaudited. Existing descriptive names remain hypotheses.

## One concrete producer route

The bounded route is AD2B30 record setup -> AA2660 **atlas miss** -> ordinary
renderer profile D5F0A8 -> cold cache with ordinary registry profile D5F088 ->
successful 2D file load, loader/callback word zero. The profile choices are
conditional static contracts, not observations of a running F8D394 publication.
Atlas hits, other renderer profiles, cache-hit families, cube/volume textures,
full AD0D80/ACF8F0 rebuild and general texture ownership are outside this result.

At AD2C31 the resolver result becomes resource DWORD zero of the local actual
2Ch record. AA2660's miss path reloads current renderer F8D394, its current
profile, then slot +64. Actual D5F10C contains B319B0. The renderer uses its
SAME cache at +1A74; the selected cold arm reloads the registry's current table
and slot +08, with D5F090 containing B2C2D0. The successful 2D arm allocates a
real texture slot and enters B3F930. Its named base is followed by numeric
profile D61948. Actual table words are D61948[0]=BD30E0 and
D61948[1]=B3F590. The latter whole 32-byte entry runs B3F2E0 before testing
bit0 and returning the raw slot through the SAME 0108DB38 pool.

The live function at AD2B30 is now present through AD2D4E inclusive; an older
ledger/comment that called it undefined is stale. This packet neither repairs
that history nor claims to have audited its whole setup body. Only the small
resource handoff is used. Ten bounded spans (265 bytes total), including both
actual table cells and the complete scalar entry, match the unchanged installed
Original. Every live query used the BSP wrapper's project/program verification
for `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

Audited direct handoffs (other surrounding body behavior is not inferred):

```text
00ad2c10 CALL 004c12b0
00ad2c2c CALL 00aa2660
00b31a55 CALL 00b30b40
00b2c5e0 CALL 00b3f2b0
00b2c60d CALL 00b3f930
00b3f961 CALL 00b34230
00b3f593 CALL 00b3f2e0
00b3f5a5 CALL 00b3d8d0
```

AA27DA and B30DBD are actual indirect sites, respectively current renderer
slot +64 and current registry slot +08. They are not represented as invented
direct calls. BD30E0 is externally leased: its established Source contract is
reused without a new live body query, export, annotation or ownership claim.
Its native ABI is ECX owner/no stack arguments/RET; it reads the current profile
and invokes slot +04 with flags1, without another decrement. Those internal
flags do not turn AD0860's slot00/no-flags site into a scalar-flags call.

## Why the existing Source service closes this seam

`src/native_render_context.cpp:181` resolves the SAME canonical companion,
checks that its `reference_count` aliases the actual resource +04 atomic, and
invokes `release_zero_references()`. It neither decrements nor accesses resource
payload after that callback. The observed-release wrapper at line196 **does**
decrement, and therefore must not be used after AD0860's own decrement.

This is concrete for the selected family, rather than an assumption about a
generic companion. `NativeTextureLoadOwners::register_completed_creator`
requires a completed, not-yet-registered creator and registers an Entry that
borrows its actual +04. Entry validates the current D61948 identity and actual
terminal table words, reloads the current profile, then invokes the genuine
`delete_native_texture_2d_00b3f590(raw,1,same_context)`. Only after success does
it unregister and erase its own companion. The texture scalar Source body uses
the real nondeleting B3F2E0 body and same actual pool.

The existing production composition is explicit: `game_native_renderer_textures.inc`
uses the renderer's actual registry in geometry's bind/unbind/find callbacks,
then builds `NativeTextureLoadOwners` over that geometry, the existing texture
context and the actual D61948 profile view. Loading and cache contexts borrow
the same VFS strings. `GameNativeRendererApplication::actual_owners()` exposes
that existing registry by reference. This is not a new manager, registry,
profile array, pool or fake terminal. The semantic `gui_framebox.cpp` route still
uses a shared_ptr projection, and semantic `resolve_gui_texture_00aa2660` is
not proof that the raw producer/normal GUI host has been composed.

## Ordering, failure and lifetime boundary

The reused 98 AD0860 contract is 126 bytes/42 starts: capture resource **before**
arming name-cleanup state0; if captured resource is nonnull, decrement actual
+04 once, dispatch current slot00 only on observed zero, then clear record+0
only after that branch succeeds. A null captured resource does not perform the
clear. Capture current name DATA before disarming the cleanup action, then
capture current name length+1 before the genuine pool getter/return. Returning
the name leaves its actual header unchanged. A resource-service escape invokes
41DD20 on the live record+4 through the established native cleanup action.

A Source operation must consume that cleanup action before its potentially
throwing pool getter, retain the exact record/partial graph and metadata before
entry, and never replay a count decrement or already-consumed name return.
A failed zero dispatch can leave record+0 pointing at an already-zero owner;
a successful terminal may have freed/reused it before record+0 is cleared.
Neither pointer is a healthy retry/cleanup capability. Do not discover ownership
from container links after failure or read resource payload after the terminal.

The Source service has an explicit exception limit. Missing-registry/alias
errors can escape before the Entry terminal. Entry's terminal is `noexcept` and
catches texture-terminal exceptions with `std::terminate()`; registration
inconsistency also terminates. Such failures cannot unwind into record name
cleanup. This is the existing Source containment boundary, **not** transparent
native slot00 throwing behavior or private Native FH3/double-exception ABI
proof. A future Source cleanup that throws while handling another exception
must fail closed; it must not retry the getter or invent successful cleanup.

The actual record, external operation metadata, canonical registry/Entry,
renderer and SAME texture pool/owner contexts, VFS/raw string pool, publication
and manager cells must remain live through their applicable terminal and name
cleanup actions. Metadata/bindings stay stable and disjoint from raw payload and
provider scratch; callbacks and raw graph mutation are externally serialized.
An atomic count or registry mutex alone does not establish that graph lifetime.
The record and containing vector cannot be retired while any entered action is
unresolved. No graph-retirement algorithm is supplied here.

## Smallest next Source packet

Append only whole AD0860 to the existing
`include/bsp/native_gui_frame_box_records.hpp` and
`src/native_gui_frame_box_records.cpp`, over the SAME
`NativeRenderActualOwners&` and `NativeStringRawPoolContext&`, plus a persistent
caller-owned one-shot release observation/phase object. A proposed API is
`release_native_gui_frame_box_record_00ad0860(void*, NativeRenderActualOwners&,
NativeStringRawPoolContext&, NativeGuiFrameBoxRecordReleaseOperation&)`.
Names/metadata layout are a Source design proposal, not recovered Original ABI.

Fresh metadata admission must precede native effects; thereafter preserve the
resource/state/clear/name schedule above. Only the selected registered2D domain
is established here. Use the existing zero-only helper after the body has
already observed zero; use genuine raw pool providers and 41DD20 for the
entered name-cleanup action. Keep persistent entered/consumed/failed states,
no reset/retry/implicit graph cleanup, and the Source terminal/cleanup-throws
limits explicit. No vector tidy, rebuild, factory, new terminal, normal GUI
activation or private-ABI replacement belongs in that packet.

Evidence: `local/cc12_native_gui_frame_box_record_release_readiness_110/manifest.json`
freezes Source inputs, prior contracts, bounded byte/Source receipts and this
report's companion documentation. The installed executable SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

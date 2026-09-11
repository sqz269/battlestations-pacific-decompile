# Renderer surface-save worker start discovery

The renderer+1D2C owner reconstructed by B5E270/B5E050/B5E2F0 is a queued
surface-to-file worker. Its launch is inside B5E380, and the actual Win32 thread
entry is B5E230, a small adapter into B5E0C0. This qualifies the earlier generic
"renderer worker" name without changing saved Ghidra names or comments.

This pass examined two complete candidates: B5E380..B5E48D (270 bytes) and
B5E0C0..B5E227 (360 bytes). Directly needed evidence includes the complete
12-byte thread adapter, six-byte D3DX import forwarder, bounded caller/publisher
spans, six import slots and the writable fallback address. Fifteen fresh spans,
774 bytes, matched the original PE and existing guarded Ghidra project. Complete
byte decoding covers the alignment bytes omitted from the worker listing.
No source, runtime test, surface-save thread, or game/analysis mutation ran.

## Launch and surface acquisition

B23C50's B23C64..B23C78 span loads current renderer+1A10 as the device, forms
renderer+1D2C, and calls B5E380 with ECX=that raw owner and a stack device
argument. B5E380 returns a surface pointer in EAX and uses RET4. The constructor
caller at B32756/B32764 passes exactly the same embedded owner.

At B5E386, current owner+50 controls launch. If zero, the routine performs:

```
CreateThread(NULL, 0, B5E230, actual_owner, 0, NULL)
owner[+50] = returned_handle
SetThreadPriority(returned_handle, 0)
```

The original import slots are CE2238/CreateThread and CE22A0/SetThreadPriority.
This is not `_beginthreadex`. The handle is published at B5E3A4 before priority
is set. Both failures are unchecked; null creation still reaches the priority
call and surface acquisition. A preexisting nonzero handle skips both calls.
Neither byte44 nor byte45 is reset, and the constructor/stop evidence establishes
that stop leaves +50 stale. No restart policy can be inferred from this branch.
The real OS can begin the worker before the creator publishes its returned
handle. No new argument allocation, string allocation, or owner retention occurs.

The remainder captures producer index +40 once under current lock+48, then
separately samples consumer +3C under that lock. Each entry increments the
captured section's depth+18; exit rereads current +48, decrements that current
depth and leaves it. Enter and Leave import targets are each captured once.
The routine computes signed `(captured_producer + 1) % 5` with DWORD addition,
CDQ and IDIV5. While this next index equals the sampled consumer, it sleeps10
and resamples only the current consumer. It neither refreshes its captured
producer nor publishes a new producer index in this body.

The chosen surface word is raw owner+4*captured_producer. Nonnull returns that
current word. A null word reads the device argument only after the wait, then
uses these concrete current COM tables:

| Site | Current interface slot | Exact arguments/result use |
|---|---|---|
| B5E455 | device+48, GetBackBuffer | `(0, 0, MONO=0, &local_backbuffer)`; local pointer is first zeroed |
| B5E466 | returned surface+30, GetDesc | `&local_desc`; descriptor is not initialized by this routine |
| B5E482 | device+90, CreateOffscreenPlainSurface | `(desc.Width, desc.Height, A8R8G8B8=21, SYSTEMMEM=2, &owner_slot, NULL)` |

The SDK interface order verifies offsets48/30/90 and the descriptor's Width/Height
at +18/+1C. The actual device pointer is retained while its table is reread before
creation. All HRESULTs are ignored. There is no null result check before GetDesc,
no Release of the temporary backbuffer in this function, and no rollback of the
direct owner-slot output. Invalid counts, failed COM output use and uninitialized
descriptor bits must not be silently repaired in a future raw implementation.

## Actual worker and queue order

B5E230 is exactly `MOV ECX,[ESP+4]; CALL B5E0C0; RET4`. It is the stdcall Win32
entry taking the raw owner as LPVOID; it returns the body's EAX. Ghidra has an
instruction label at this address, not a function definition. The raw body
B5E0C0 takes ECX, uses RET, and returns zero. Both candidates and the adapter
have no local EH/finally handler. This discovery does not create a function or
reinterpret a generic compiler-helper label as implementation evidence.

The worker snapshots current byte44 under current lock+4C before entering its
outer loop. If not stopped, it reads consumer+3C and producer+40 under two
separate lock+48 acquisitions. It processes while the signed remainder
`(producer - consumer + 5) % 5` is positive, preserving the raw DWORD arithmetic
and CDQ/IDIV sequence. In each iteration it captures the filename pointer at
owner+18+8*consumer, then the surface pointer at owner+4*consumer. The filename
header's length is not read. A null filename pointer selects the actual address
0108FE98. The complete call is:

```
D3DXSaveSurfaceToFileA(filename, 0, captured_surface, NULL, NULL)
```

C2DFF8 is the six-byte jump through CE23F4, whose original PE import names
`d3dx9_40.dll!D3DXSaveSurfaceToFileA`. The installed Win32 DLL contains that
concrete export (ordinal284, RVA289C3C); it was inspected without loading it.
No different DLL, image-format policy, or success/failure fallback is proposed.
The HRESULT is ignored. Under lock+48 the worker increments the **current**
consumer modulo5 after the save returns. It then separately rereads current
consumer and producer before deciding whether to continue draining. Thus a
returning provider's mutations must not be replaced with old queue snapshots.

When the observed queue is empty, the worker sleeps10 and samples byte44 again
under lock+4C. Stop is not tested inside the drain loop. A continuously nonempty
queue can therefore delay stop observation. On observing a stop byte, it enters
current lock+4C, increments the captured depth, writes byte45=1, reloads current
+4C, decrements/leaves it, and returns zero. There is no exception/failure path
that guarantees publication of byte45. The existing B5E050 stop helper polls
that byte and closes current +50; it does not wait on the thread handle.

The final owner access is the current +4C read before the final Leave call.
Completion publication is not itself proof that the OS thread has returned or
that external code/data/module bindings can already be destroyed or unloaded.
This pass does not infer complete renderer shutdown or surface-release ownership.

## String boundary and next source proposal

0108FE98 lies in writable `.data` (characteristicsC0000040). Four captured bytes
are zero, and the bounded xrefs show the worker's address use. This does not
establish immutable empty-string semantics or the global's complete extent.
A future implementation must use the actual mutable storage address rather
than copy the observed zeros into a private empty literal. B5E15C is
`B8 98 FE 08 01`, MOV EAX,0108FE98: an immediate address of bytes, not a load
from a pointer variable. The binding therefore borrows `char*` to that same
storage, never `char**`. Its valid domain is a readable NUL-terminated ANSI
byte range. The initial zero needs only its first byte; if those mutable bytes
become nonzero, the caller must keep the eventual terminator readable. The four
captured bytes do not establish a four-byte allocation or a maximum name length.

Neither candidate allocates, resizes, releases, or copies a native string. A
27-byte adjacent publisher span B5E4A3..B5E4BD selects the current producer's
header and calls full 41DD40 with the source header length and preserve=1.
A separate 31-byte tail B5E4D6..B5E4F4 proves that publication increments the
current +40 modulo5 before decrementing/leaving current lock+48. The intervening
copy body was not recovered by this pass. B5E490's full publisher and the B23C50 capture pipeline remain outside this
two-candidate pass. The existing actual-header 41DD40/pool implementation is
available, but no complete enqueue/copy/allocation ordering is inferred from
that bounded span. An end-to-end producer source closure still needs them.

Proposed packet: `native_renderer_surface_save_worker3`, owning exactly
B5E380[270], B5E0C0[360] and B5E230[12], total642 bytes, in four new files:
`include/bsp/native_renderer_surface_save_worker.hpp`,
`src/native_renderer_surface_save_worker.cpp`,
`docs/NATIVE_RENDERER_SURFACE_SAVE_WORKER.md`, and
`reports/native_renderer_surface_save_worker_audit.json`.

The native provider graph for these three entries is closed at concrete OS,
D3D9 and D3DX import boundaries; no private math/string/thread substitute is
needed. The host binding contract needs approval before source: CreateThread
passes only the raw owner, so an extra EDX context cannot reach the asynchronous
adapter without changing the thread argument. The proposed explicit application
binding borrows the actual mutable fallback storage and a concrete named D3DX
import object. That object borrows the caller-owned actual d3dx9_40 module and
resolves exactly `D3DXSaveSurfaceToFileA`, with no arbitrary function-pointer
setter, DLL loader/version fallback, or frontend dependency. The storage address
is fixed while the bytes themselves remain mutable. The binding must exist
before any launch and remain valid through actual thread return. It must not
add owner-tail fields, temporary argument envelopes, or a copied empty name. Existing frontend screenshot
plumbing owns a private D3DX loader with nullable imports and unrelated lifetime;
it is not automatically this raw worker's provider contract.

This is a source-ready native graph conditional on that explicit host binding,
not an implemented worker or a complete producer/lifetime integration. No real
surface-save worker was launched and no surface files were written. Private raw evidence is sealed under ignored
`local/renderer_worker_start_discovery/`; the audit records exact pins.

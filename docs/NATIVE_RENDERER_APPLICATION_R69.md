# Native renderer startup and shutdown in the application (R69)

Addresses: `00B32410`, `00B32920`, `00B32900`, `00B2AEB0`, `00B2ABD0`,
`00B29670`, `00B2DBD0`, `00CE0C10`, `00CE0C30`, `00B33DA0`.

## Application composition

`GameNativeRendererApplication` now runs the complete reconstructed `B32410`
constructor, `B2AEB0` device startup, focused `B2ABD0` reset and `B32920`
destructor. The application no longer constructs a separate parameters owner,
performs a second projected capability gather, or creates its device through
the old prefix. Settings receive a consumer copy of the actual resolution,
antialias and shader capability fields. The parameters accessor borrows the
actual renderer region at `+1A14`.

All ten device request slots reach the native startup, including VSync and
multisampling. `GameDeviceHost` borrows the resulting device and retains only
its frame callbacks and observations. Its `S_OK` summary means that startup
returned with a device; it is **not** the ignored native CreateDevice HRESULT.
The existing direct clear/draw/present bridge remains in use.

The graph borrows the application's raw singleton manager, VFS routes, string
cells, type counter/descriptors and retained-memory accounting. It uses R68's
canonical Lua globals/fundamentals cache, activating the substantive native Lua
bindings for construction and drain. It also borrows the existing renderer
scalar process's synchronization state, worker time bits, serial and tracking
counters, plus the canonical graphics pools, hardware tree and immutable image
profiles. There are no private fixture copies of these domains.

The platform view uses R67's actual HWND/active cells. Device recreation borrows
the same SoundServices online publication and real XLive adapter; the online
owner is still unimplemented, so its current publication remains null. The
system D3DX9_40 module stays loaded through the native drain. The CRT power
binding borrows the existing application dispatch/math runtime and canonical
literal regions; its unused SSE2 route remains unbound.

The module is retained before any native construction. It installs the real
renderer/Lua deletion contexts without replacing VFS's type-counter deletion
binding. The real control worker starts idle and joins at native destruction.
Its active begin/end-frame route remains unbound; this packet does not start it.

## Lifetimes and failure boundaries

The full destructor requires a completed device/default-surface lifetime.
Constructor-only, interrupted startup and interrupted drain states are retained
until process exit through the application's existing `_Exit` policy. No guessed
partial renderer destructor or rollback is added. These exception paths have not
been exercised by this packet.

The composition holds real external factory/device references through the raw
drain, preserving the native destructor's diagnostic COM operations after its
unconditional Releases. The UI bridge closes before that drain; remaining C++
consumers close before the final external references are released. The actual
publication cells and all provider bindings survive the worker join and native
Lua closes.

Source-private renderer/COM scratch has explicit readable zero preimages.
These are source inputs, not recovered original heap/stack contents. The logical
buffer contexts also carry zero input bits for the native uninitialized local
used only with an invalid low flag nibble; that path was not reached. Failed COM
outputs, original FH3/SEH identity, concurrency and arbitrary raw preimages are
not established by successful startup.

## Canonical vertex-format tokens

`GameNativeVertexDeclarationsProcess` owns the original loader-zero usage/type
records and guard, with the verified live signed counts 8/17. Its loading context
uses the same process strings and initialized `0108FD38` pool. The existing full
decoder sets each guard bit and initializes its native table, then registers the
actual `CE0C30` or `CE0C10` continuation with real `std::atexit`.

The process owner and string publication cells outlive application destruction.
C++ destruction does not repeat token cleanup. A focused local probe initializes
both tables through `position.mvfm`, decodes twice, destroys both actual pooled
declarations, drains/destroys the application host, and observes the real CRT
callbacks. They preserve all 25 headers/guard bits and recreate the canonical
manager/string pool; the native nonzero small-return gate protects the stale
buffers. The application itself has not yet loaded these declaration resources.

## Evidence and validation

Fresh verified Ghidra/PE reads cover all ten native function bodies, token table
zero-fill and count cells. The only unlisted instruction bytes are unreachable
`LEA ESP,[ESP]` alignment padding in `B29670`; no flow repair was performed.
The report lists every decoded native call site. This packet executes the
reconstructed C++ providers, not original renderer machine bodies.

The final strict Win32 build and all three existing CTests passed. The token
probe passed through real CRT exit. Two runs of the same final application
executable passed:

- Two frames: focused startup cleared pending/lost to zero, native worker joined,
  renderer/Lua/state-definition/system publications cleared, fundamentals cache
  cleared after 20 getter calls, and final device/API Releases returned zero.
- Eight frames with the existing press-start injection at frame one: shell and
  main-menu manager entered `ScreenVisible`, with 22 cache getter calls and the
  same successful native shutdown/COM observations.

The two-frame capture shows the title/map/sign-in prompt. The eight-frame capture
shows the main-menu frame with an empty interior; the state log does not establish
complete menu rendering or visual parity. No permanent tests were added.

One intervening run of the **same final executable** failed. A debugger captured
`8876086A` (`D3DERR_NOTAVAILABLE`) from both real `GetDeviceCaps` and CreateDevice,
then the native ignored-HRESULT path dereferenced its null device. An independent
D3D probe later succeeded and both final application runs returned zero without
a source/binary change. The exact display/backend transition is not established.
Failed-HRESULT handling remains a known native behavior; the failed run and
debugger records are retained beside the passing evidence.

## Follow-up work

Compose the actual native frame/end-frame providers and worker activation, then
replace the milestone draw bridge. Bind shader-state-list pool `0108FEE4` and the
full resource-loading graph, including genuine texture/geometry owner services.
Complete settings ApplyAll and online startup; extend gameplay and visual
validation. Full game reconstruction is still incomplete.

Tested artifacts and the separate combined-build receipt are recorded in
`reports/native_renderer_application_r69.json`.

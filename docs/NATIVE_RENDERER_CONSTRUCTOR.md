# Actual renderer constructor

Address: `00B32410`, complete 1256-byte body through `00B328F7`. The new
`construct_native_renderer_00b32410` writes the actual caller-owned `1D94h`
renderer and invokes the existing substantive actual providers. It adds a
borrowed context in EDX to the original ECX-owner/EAX-owner/plain-RET contract;
it is not a drop-in executable replacement.

The context carries one application's actual `01090AA0`, `01090AA8`, `01090AA4`,
`00F8D394`, `00F8D434`, `0108FE90`, `0108FE94`, `0108D4B8/B9` and `00D7A24C`
cells. The parent creates the raw string, singleton, Lua, system-registry,
worker and capability adapters from those same references. No projected owner,
second manager, shadow array or successful no-op provider is introduced.
Declaration, texture and effect cleanup borrow the existing concrete contexts;
their pool/cache/canonical-owner domains must match these cells. Existing Lua
bootstrap and current-profile dispatch dependencies remain substantive caller
requirements.

| Native schedule | Actual action |
| --- | --- |
| `B32434..B3246C` | Base publication/registration, renderer profiles, two raw headers, cache at `+34` |
| `B32479..B32509` | Real `Direct3DCreate9(32)`, current owner stores and embedded critical section |
| `B3250C..B32750` | Ordered inline member/profile stores, capability defaults and frame statistics |
| `B32764..B327DB` | Embedded worker construction, final flags and shared frame-byte stores |
| `B327E1..B32854` | Captured `4CC/2C/10h` allocations and actual Lua/state/system singleton constructors |
| `B3285C..B3289B` | Actual resolution enumeration, current COM identifier, NVIDIA substring, raw capabilities |
| `B328A2..B328DF` | Actual `24h` control worker followed by final tracked critical section |

Every native observable parent store, call and state transition has an address
comment in the source. Zero bits produced by native `XORPS`/`MOVSS` are stored
as dwords; `00D7A24C` is captured at the native read before `+1B3C` is written.
`GetAdapterIdentifier` reloads the current `+1990` interface, uses COM slot
`+14`, and ignores its HRESULT. The case-sensitive `strstr` reads Description
at scratch `+200`; its result writes only `+1D88`. The source does not sanitize
partial COM outputs or skip later work after a failing HRESULT.

Caller storage and borrowed lifetime requirements are explicit. The existing
`B29430` source requires a live `CameraPlaneSet` at renderer `+17C0`, with the
intended byte preimages restored before this entry. Mode, identifier and
capability scratch occupy distinct caller-supplied `10h/44Ch/770h` buffers with
valid readable preimages, including reachable Description NULs. Their providers
reuse unwritten data without implicit initialization. Present parameters
`+1A28..+1A5F` remain untouched. The actual control-worker process binding must
already exist and remain valid through every worker join, including a worker
retained by native late-failure behavior.

The native FH3 metadata is `CBDEF5 -> DF66EC`, with 29 entries at `DF6710`.
The source arms states at the actual instruction positions and dispatches the
same descending member actions when an ordinary source C++ exception escapes.
States 25, 26, 27 and 28 first free the captured failed Lua, shader-state,
system-constant or control allocation after its completed inner unwind, then
continue at 24. Raw system construction uses the completed constructor packet
with no retained Operation object. Its inner cleanup can leave a native
published/registered pointer that the parent's captured allocation free makes
stale; the source does not clear it or invent a different ownership policy.

States 24 through 0 invoke the real embedded-worker, four record-array,
capability, remaining-array, shader/state-container, pointer-array, effect,
texture, declaration, embedded-section, resolution/dword-array and renderer-base
cleanup providers. `B321D0` is an exact tail jump to the existing `B32030`
provider. All field offsets and next-state values are recorded in the report.
Source cleanup is noexcept: a second C++ exception terminates, while an ordinary
exception escaping a disarmed normal child path can reach the parent catch.
The existing actual `NativeStringStorage::release` adapter is also noexcept;
a lazy pool-getter exception there terminates. The parent does not broaden
that inherited failure domain.

The native map contains no release of the created D3D9 interface, no later
rollback of completed singleton children, and no destruction of a completed
control worker if the final section creation fails. These retained states are
preserved. The parent does not free its own allocation. Successful singleton
children and failure-retained state require their original publication cells
and providers to remain alive. Existing child native-leak and no-rollback
boundaries remain in force.

Fresh original-PE and read-only live captures cover the full owned body,
constructor actions/handler, complete FuncInfo/map, relevant import/free
thunks and literal values. The complete body is decoded into 244 instructions;
the source audit covers 174 ordered parent events and 19 normal calls. The
report also carries all 29 cleanup transfers. The handler's bytes are captured
even though it is not a separately defined Ghidra function. No Ghidra listing
or annotation was changed by this packet.

The strict Win32 build, eight seed checks and both configured CTests passed.
The one local link/import probe resolved the constructor and 176 mapped core
translation units, recorded 22 loaded x86 images, and exited without invoking
construction. Its embedded manifest and same-process file-handle resolution
preserve the actual WOW64 DLL paths. This is link/import evidence. No valid
full renderer/material/VFS lifecycle fixture
was available, so this packet does not claim parent normal or late-failure
runtime execution. Existing child tests are attributed to their own immutable
reports. Original FH3 frame layout, asynchronous SEH, native CRT exception
identity, full renderer shutdown, ABI interchangeability and gameplay remain
unproved. The new source is a complete parent schedule under the documented
actual-provider and preimage contracts, not whole-program closure.

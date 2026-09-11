# Native renderer surface-save worker

This packet reconstructs three complete entries totaling 642 original bytes.
They belong to the renderer+1D2C surface/name queue identified in
[the bounded discovery](NATIVE_RENDERER_WORKER_START_DISCOVERY.md), and compose
with the separately reconstructed raw worker lifetime. They do not implement
the B5E490 publisher or the complete B23C50 capture pipeline.

| Entry | Complete original range | Raw ABI |
|---|---|---|
| Acquire/launch B5E380 | B5E380..B5E48D, 270 bytes | ECX owner, stack device, EAX current surface, RET4 |
| Worker B5E0C0 | B5E0C0..B5E227, 360 bytes | ECX owner, EAX0, RET |
| Thread adapter B5E230 | B5E230..B5E23B, 12 bytes | Win32 stdcall raw owner, EAX0, RET4 |

The acquire C++ declaration uses an ignored EDX parameter so its actual device
argument remains on the original stack. The adapter receives the raw owner
unchanged, moves it from the stack to ECX, calls the full worker, and returns.
No temporary argument envelope, new allocation, owner-tail context, thread-body
callback or additional queue lock is introduced.

## Concrete external binding

`NativeD3dx9SurfaceSaveImport` borrows the caller's actual `d3dx9_40` HMODULE
and resolves exactly `D3DXSaveSurfaceToFileA`. It does not load or free a DLL,
try another version, use the frontend's private loader, or accept an arbitrary
function pointer. Null modules and missing exports throw during host object
construction, before raw launch. These are host binding checks, not recovered
validation inside B5E380 or B5E0C0. Module identity is a caller precondition.

One explicit application binding borrows this concrete import object and the
actual fallback byte storage corresponding to 0108FE98. Install it before any
launch and keep the binding, import object, module and storage address fixed
and alive until every launched thread has actually returned. Installation and
uninstallation are not concurrent operations. A null/unbound raw invocation is
outside this interface's domain; the worker contains no new unbound recovery.

B5E15C is `B8 98 FE 08 01`, MOV EAX,0108FE98: an immediate address of bytes,
not a pointer-variable load. The observed four zero bytes lie in writable
`.data`, and do not establish a four-byte allocation or immutable empty-name
semantics. The binding borrows `char*` to the same storage; it never copies an
empty literal or takes `char**`. Those bytes may change while their address
stays fixed. The caller provides a readable NUL-terminated ANSI range, including
the eventual terminator after any mutation.

The rebuilt worker replaces only that immediate with loads of the installed
binding and its fallback pointer. Both loads preserve flags. The D3DX call uses
the concrete import object's resolved export through a full stdcall forwarder.
The original C2DFF8/CE23F4 edge is the corresponding external import boundary;
native import-table hot replacement is outside this fixed binding contract.

## Preserved raw schedule

B5E380 checks current +50. Zero calls
`CreateThread(NULL,0,B5E230,actual_owner,0,NULL)`, publishes the returned handle,
then calls `SetThreadPriority(returned_handle,0)`. Creation and priority failures
are unchecked, and no stop/completion byte is reset. A nonzero handle skips
both calls. The thread may run before handle publication. The stale handle left
by native stop is not converted into a restart policy.

Acquisition captures producer+40 once under current lock+48 and reads consumer
+3C under a separate acquisition. It computes signed `(producer+1)%5` using
the original DWORD add/CDQ/IDIV5, then sleeps10 while full, refreshing only the
consumer. It uses raw owner+4*captured_producer; a nonnull surface returns the
current slot. This body does not publish a new producer index.

For an empty slot it reads the stack device only after waiting. Current device
table+48 calls GetBackBuffer(0,0,0,&local), whose pointer is first zeroed. The
returned surface's current table+30 calls GetDesc into an uninitialized stack
descriptor. The retained device's freshly read table+90 calls
CreateOffscreenPlainSurface(Width,Height,21,2,&actual_slot,NULL). All HRESULTs
are ignored, with no null guard, descriptor initialization, temporary backbuffer
Release, or output rollback. The concrete slots are verified from the Win32 SDK.

The worker reads current byte44 under current lock+4C. If not stopped, it reads
consumer and producer under separate lock+48 acquisitions and drains while
the signed DWORD-wrapped `(producer-consumer+5)%5` is positive. Each save
captures current filename data at +18+8*consumer (without reading header length)
and surface at +4*consumer. Null filename data selects the bound actual fallback
address. It calls the actual D3DX export with `(filename,0,surface,NULL,NULL)`
and ignores HRESULT. After return it increments the **current** consumer under
lock, then separately rereads both indices before deciding whether to continue.

Every lock entry increments the captured section's depth+18; every exit reloads
the current owner lock, decrements that current depth, and leaves it. Enter and
Leave import targets are captured in the original order. Raw integer wrap,
signed remainder, current-field loads, and call ordering are retained in the
complete assembly bodies rather than expressed through normalized queue APIs.

Stop is sampled again only after the drain and Sleep(10); a continuously busy
queue can delay it. Once observed, the worker enters lock+4C, sets byte45=1,
reloads current +4C, decrements/leaves and returns zero. There is no new finally
block guaranteeing completion after an exception. The lifetime packet's stop
routine polls byte45 and closes +50, not WaitForSingleObject. Completion is not
proof of actual thread return or permission to unload the borrowed module.

The caller supplies valid raw owner storage, current tracked locks, queue
indices/storage, COM objects and readable names. No malformed-state recovery,
surface-retention policy, producer implementation, or complete renderer lifetime
is inferred from these entries. B5E490 remains separate; no stub was introduced.

## Verification

The final existing `scripts/build.ps1` run passed both CTests; all eight native
differential seeds matched the original PE. An ignored CMake hook includes only
this source and the already committed raw lifetime source. Fresh guarded live
queries matched 15 original spans / 774 bytes, including all 642 owned bytes,
thread adapter, import forwarder/slots, mutable global and bounded caller context.

The actual complete worker library was frozen before fixture linking:
SHA256 `2996be1360b22628c739dd5b0829374ae859613109a6b750ed828f24e0d73c89`.
Six exact archive objects and 21 recursive source/header files are pinned. All
233 mapped COFF sections (158 library, including 22 from this source object),
649 relocations and 14,499 runtime executable bytes were checked. Coverage
includes the full import constructor, save method, stdcall forwarder and binding
function. Map parsing strips every leading `f`/`i` flag. Acquire and adapter
match the original bytes modulo concrete relocations. The complete 363-byte
compiled worker matches the 360-byte original after only the documented fallback
loads, affected branch displacements and concrete relocations.

One ignored nonempty actual-library probe used its own hidden window and real
HAL D3D9 device. Full acquisition launched the actual rebuilt thread and created
an 8x8 A8R8G8B8 SYSTEMMEM staging surface. The fixture filled its pixels, changed
the same fallback bytes after installing the binding, then published one queue
item as explicit fixture input. The worker used the real D3DX export to save a
BMP in the fixture's run directory. All 64 pixels were RGB(51,102,153); the
consumer advanced, actual thread return was zero, and full raw lifetime teardown
preserved the expected stale words. A duplicated handle allowed a separate wait
for actual thread return before unbinding and unloading. The D3DX HRESULT was
ignored by the routine and was not independently observed.

All 79 DLL import targets and five COM/D3DX provider observations matched actual
module images with loader relocations only. Whole raw owner postimages and the
mutable filename bytes are retained without pointer normalization. The fixture
leaves the native acquire's unreleased temporary backbuffer behavior intact;
it does not claim a complete surface-release owner. No live game window or UI
was touched. The original bodies were **not executed**, so this is actual-source
behavioral evidence plus full original/compiled instruction proof, not original
runtime parity. No failure, OS callback mutation, malformed queue, native SEH,
full producer, binary replacement or gameplay claim is made.

Artifacts are immutable under ignored `local/renderer_surface_save_worker/`.
The tracked audit records complete original bytes and proof pins. No permanent
tests, shared CMake, Ghidra, original game or shared metadata files changed.


## Primary integration

Main CMake now registers the unchanged source. The main strict Win32 build,
both existing CTests and eight fresh seeds passed. Primary verified118 sealed
worker pins,21 current files and15 fresh spans,774 bytes. Six exact actual main
archive objects retain all reviewed code/data/relocation contents. The unchanged
fixture linked only the frozen main library and performed one nonempty real
worker save in its own hidden-window/output directory. All64 BMP pixels matched
RGB(51,102,153), and the actual thread returned0. Full233 COFF sections,649
relocations,79 imports,five COM/D3DX observations and14,499 runtime code bytes
passed. Original machine bodies remain unexecuted.

The immutable main bundle is `local/surface_save_worker_primary/`. Primary
defined the complete12-byte Win32 adapter, saved three reviewed names/comments
with prior values retained, registered three raw functions, and refreshed their
exports. The binding, producer, native-exception and full-lifetime/gameplay
boundaries above remain.

# Native GUI raw camera caller, packet 66

`allocate_native_gui_camera` now has an explicit actual-header route into the
existing camera owner/kernel/reference graph. The original helper still selects
the semantic constructor. This packet closes the missing raw caller only; it
does not rewrite a camera/viewport body, pool, ScreenString, store map, outer
scene weak base, or normal host. It creates no second camera or reference count.

Requested Root `2bbcd20d18c8c48c73f9615cb0a0b3cd718c58d7` merged successfully
into the clean worker at `ff5e8916cf110baa79ab6c133cd465b1dbc2d7d0`. The approved
files are `gui_camera_store_owner.hpp/.cpp`, the single metadata inspector in
`native_viewport_registry.hpp`, and this unique doc/report. All three Original
entries and these outputs were leased before editing. No ledger/Ghidra mutation.

## Existing evidence and exact boundaries

Verified existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` with
`bsp.py ghidra ensure` before the bounded `lookup/show` evidence batch. Read
existing pseudocode plus assembly at constructor entry/publication, constructor
return, destructor returning tail, and complete deleting wrapper.

| Original range | Existing behavior / ABI |
|---|---|
| `[00B71A80,00B71CDC)` | 604B; ECX actual45Ch camera slot, stack raw8h name header, EAX same owner, RET4 at B71CD9. B71AA7 calls B6F5A0; B71AD1 constructs viewport; B71ADD publishes +180. Fog184 initializes afterward. Native states0/1/2 govern prefix/raw allocation/438 cleanup. |
| `[00B71F10,00B71FD2)` | 194B; ECX camera, RET at B71FD1. Full camera destruction releases viewport180, fog184, retained438, then B71FBA calls raw-capable Node B6F440. |
| `[00B71FE0,00B72000)` | 32B; ECX owner, stack flags, EAX original address, RET4. Complete destructor first; only flags low bit0 returns SAME pool slot through B711E0/0108FFB0. |

Pool B71770 allocates raw45Ch slots and B711E0 returns them; these existing
bodies are read-only reuse. Existing `native_camera_owner.cpp:266,359,363`
already forwards a raw header unchanged, checks SAME node/viewport D7A24C,
and offers a registered-first-viewport admission overload. `finish_node` at
`:123` already selects direct throwing raw name destruction. Existing canonical
`NativeCameraReference` binds SAME +4 without retaining it, and terminal release
performs full destruction/pool return before lifetime unbinding and companion
disposal. No new native reconstruction or Original differential is claimed.

The old `gui_camera_store_owner.cpp:156` helper still calls the semantic
overload at `:163`. Its local catch/rollback is left byte-for-byte unchanged;
that path is not reused as invented recovery for raw getter/metadata failures.

## Actual shared domains

Normal renderer `game_native_renderer_camera.inc:20..44` already owns one
installed `NativeViewportRegistry`, viewport environment, node constant view
and camera environment. It borrows the canonical resource camera pool, VFS type
counter/descriptors and SAME destruction graph's raw nodes. Its viewport and
node views both reference actual D7A24C. Camera constants borrow CE3D08, CE7D20,
D0C5F8, D5BD98, CE77FC and CE3C88; tables borrow D62CF0 and D62C88. Existing
public accessors `camera_environment()`, `node_constants()` and
`viewport_registry()` return these actual owners (`game_native_renderer_application.cpp:563..570`).
The new helper supplies no dimensions, CRT policy, ID, table, pool or resolver.

The first viewport must join that installed registry. The old bare camera
constructor can construct it without a registry record; that is insufficient
for normal `owner.frame.viewport` resolution and future managed retirement.
The new overload therefore requires an actual `NativeViewportRegistry::Admission`
whose registry is exactly `environment.viewport_views`.

## New caller API and retention order

`GuiCameraRawAllocation` (`gui_camera_store_owner.hpp:91`) is a single-use,
immovable persistent frame. Establish it before this camera's first temporary
name/admission/pool/native effect. The enclosing caller retains the actual
header bytes and unconsumed name constructor/release cleanup, plus the actual
viewport `Storage`. The helper never copies or releases the input header.
Its marker covers this helper, not separate caller name operations: those still
need their own entered/consumed state and outer fail-stop guard.

The overload at `gui_camera_store_owner.cpp:177` takes the SAME camera
environment, frame, exact `const void*` header, actual `NativeNodeRawConstants`
references and required admission rvalue. It performs all rejection before
moving that token or entering the pool: unused frame, nonnull header, supported
table slots/CRT callbacks, node predicate, shared scene/lifetime runtimes, raw
name domain, exact one-cell equality, installed admission/resolver identity and
reserved record state. Other cell/header/context identities remain caller
obligations where this narrow API has no independent owner to compare.

`Admission::storage_identity()` (`native_viewport_registry.hpp:50`) exposes
only the actual borrowed record pointer, null when inactive/moved/consumed.
It performs no dereference, validation, allocation, callback or mutation.
`require_registry()` remains the validation operation. Existing move, cancel
and constructed paths clear the private pointer pair; no registry cpp changes.
The helper derives its record from the validated token, rather than trusting a
second caller-passed record/token pairing.

After preflight the frame captures exact header/record identity, moves the
admission into its member, marks pool entry and records the acquired slot.
`NativeCameraOwner` prepares typed storage and binds its scene association
before native B71A80; its completed pointer is retained in the frame before the
raw kernel runs. Unlike the directional route, that prepared scene binding
exists during camera constructor callbacks. The helper then calls the existing
raw admission overload and constructs the existing canonical reference.

Any entered pool/owner/native/reference admission exception leaves the frame
interrupted with its acquired slot, completed prepared/live/dead companion and
actual viewport record identity. There is no catch rollback, manual cleanup,
slot return, retry or second owner. Existing native callees alone consume their
own prefix/raw viewport allocation cleanup. An unused admission moved inside
the existing kernel may be cancelled by that callee's existing unwind; a
viewport published before a late constructor failure remains live in its record.
This caller does not undo or replay either state. Constructor failures that
have not completed a C++ companion publish no such companion pointer.
Retaining the record identity promises neither a particular reserved/live phase
nor a still-live native object after a callee has consumed its cleanup state.

The metadata predicate reads only the external stage, with no raw access,
locks or callbacks. It stays true during entry/after failure and has no reset.
Its destructor checks before the Admission member destructor could cancel
retained state. External GUI serialization includes reentry. The future host
must inspect the persistent marker and `_Exit` before any outer member,
manager or CRT drain; destroying a local frame after already draining the graph
is not recovery. Existing terminal reference callbacks remain noexcept, so a
terminal raw failure terminates before a partial return/replay can occur.

Success clears slot/header/companion ownership metadata before publication;
no provider/callback follows handoff. Only the actual constructor count1 is
returned. Existing logical release can leave queued references; final zero
retires the registered viewport and camera, returns the same slot, then deletes
the two companions without later access. Record identity remains borrowed
metadata in the frame. Caller `Storage`, registry/resolver, pool, runtime/types,
profiles and raw string domain survive final callbacks; header/constants survive
construction. After all frame/view consumers are quiescent, the caller explicitly
forgets the retired/cancelled record before destroying its Storage/registry.

## Verification and remaining work

`scripts/build.ps1` exited0: MSVC Win32 Release and both enabled CTests
(`reconstructed_math`, `tool_tests`) passed2/2. One focused production Source
case linked the rebuilt `bsp_core.lib` with `/MD /MANIFEST:EMBED` and passed.
It uses actual raw string/camera pools, canonical counter-derived types and
shared scene/lifetime bindings; tables/constants were extracted from installed
Original SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The case checks actual record identity before token consumption, registered
first viewport and same frame view, four current renderer parameter calls,
independent actual name copy, same +4/count1, retained2/logical1 lifetime,
viewport staying live while retained, terminal viewport retirement, explicit
quiescent forget, exactly-once raw name return, same camera slot reuse and
pool/list/string retirement. The fixture's renderer parameter service and CRT
bypass input are explicitly controlled; they are not production providers,
actual D3D startup, Original CRT selector state or drawing proof. Evidence/logs,
probe/preparation scripts, data and executable are ignored under
`local/cc12_native_gui_raw_camera_caller_66/`; exact hashes are in the report.

Normal Screen still needs its own persistent raw name/camera operation frame
and real outer-scene/store composition. No ScreenString/map/weak-base changes,
new runtime/installation/audio behavior or normal native Text activation occur.

Root primary Source review passed the full three-file diff and the existing
Admission move/cancel/validation, owner preparation/scene admission, raw kernel
local token and reference binding paths. The one Source case covers normal
lifetime only; no failure injection or real D3D/CRT selector proof is claimed.

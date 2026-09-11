# GUI camera store and outer scene ownership

Packet `orch2_gui_camera_store_n`, 2026-09-11. Names are descriptive hypotheses,
not recovered symbols. Source: `include/bsp/gui_camera_store_owner.hpp`,
`src/gui_camera_store_owner.cpp`, and the narrow `GuiCameraStoreMap` erase method.

## Evidence and ABI

Read-only Ghidra queries and exports used project `bsp`, configured project file
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 little endian,
base `00400000`. Every `bsp.py ghidra` batch verifies the current project/program.
Disk evidence came from the read-only installed executable at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
No Ghidra mutations, saved-project writes, or game writes were performed.

| Native body, end exclusive | Original ABI | Established behavior |
| --- | --- | --- |
| `[00AA5070,00AA5150)` | ECX manager; camera, scene, descriptor on stack; RET0Ch; EAX store | Allocate 24h record, copy descriptor, publish borrowed pointers, visible count zero, insert same ordered map |
| `[00AA3280,00AA3392)` | ECX manager; descriptor on stack; RET4; EAX store or zero | Existing first-match lookup, excluding Priority |
| `[00AA4B30,00AA4C1D)` | ECX manager; store on stack; RET4 | First matching entry: free record, zero map value, erase; no camera/scene release |
| `[00B724E0,00B72576)` | ECX raw outer scene; NativeString pointer on stack; RET4; EAX scene | Weak base, scene vtable, root/name/lighting fields, name copy, scalar18 |
| `[00B72430,00B724D6)` | ECX scene; RET | Release lighting1C, repeatedly unlink/release current root0C, free name, weak base |
| `[00B72250,00B7226C)` | ECX scene; RET or tail virtual0 | Atomic decrement on the same +04; virtual0 only at zero |
| `[00B72580,00B7259E)` | ECX scene; flags on stack; RET4; EAX original identity | Destructor then optional physical free; this packet composes the flag1 zero-reference path |

Final instructions are AA514D RET0Ch (3 bytes), AA338F RET4 (3), AA4C1A RET4 (3),
B72573 RET4 (3), B724D5 RET (1), B7226B RET (1), B7259B RET4 (3).
These lengths, not the address of the final instruction alone, determine each
end-exclusive extent. All listed entries exist in the saved program.

Current AA4B30 listing has zero flow gaps. Its earlier comment about hidden
AA4BFB..AA4C05 is stale: those instructions are now listed. B72580 still has a
three-byte gap after the incorrectly no-return `_free` call at B72590. Read-only
disk decoding confirms B72595 `ADD ESP,4`, then B72598 `MOV EAX,ESI`, B7259A
`POP ESI`, B7259B `RET4`. This packet records the gap and does not repair Ghidra.
AA4E00 is an MSVC introsort template, unrelated to store retirement; its existing
library classification and name remain unchanged.

## One store and one scene lifetime

`GuiCameraStoreMap` remains the existing canonical registry and draw ordering.
The owner factory allocates exactly 24h bytes on MSVC Win32 and uses the existing
`GuiCameraStore` record. Camera18 points to the actual native camera storage;
scene1C points to the actual outer-scene storage. No second ordered registry or
reference counter was introduced. Native AA5070 and AA3280 do not retain either
pointer. AA4B30 does not release either pointer. Native diagnostic logging reads
the camera's name through B6D800 (AA5123 and AA4BB2 load store+18), not the scene
name. Logging and invalid-STL-iterator diagnostics remain outside the new API.
The flat registry remains a semantic replacement for the native multimap; the
library tree implementation is not reconstructed merely to link this packet.

The outer `cScene` allocated by B724E0 is 24h bytes:

| Offset | Storage |
| --- | --- |
| 00 / 04 / 08 | Native vtable, one atomic reference word, weak handle |
| 0C | Root-list head |
| 10 / 14 | NativeString length and buffer |
| 18 | Live D7A24C scalar word |
| 1C | Owned lighting resource reference |
| 20 | Constructor zero |

It is distinct from the existing `ConcreteSystemSceneResource` constructed at
B83C50, the 3Ch native `GuiLights` resource at outer-scene+1C. The new owner borrows
its own actual +04 through `RenderCommandReference`; its `RenderNodeRootList`
borrows the same +0C/+1C fields consumed by native node root registration. Pointer
fields use the established C++ hierarchy companions, as `NativeNodeStorage`
does, so exact Win32 field offsets do not imply binary compatibility.

The actual native camera constructor/pool and `NativeCameraReference` are reused.
Camera factory allocation returns the initial creator/self reference, and the
existing terminal reference callback performs native cleanup and pool return
before deleting the two host companions. Scene root attachment adds no reference:
the scene's root teardown invokes B6DFA0 and the actual bound node virtual18,
which consumes the node's creator/self lifetime. Queued camera references can
keep a logically released camera alive. Every traversed node must be bound in
the same `GeneratedModelLifetimeRuntime`; no fake camera or light node is supplied.

AC59A0/AC5480 are read-only supporting evidence owned by other packets. When
sharing is enabled and lookup succeeds, the layer sets owns-store false, copies
the scene pointer, increments scene+04 and returns immediately. Otherwise the
creating layer owns its initial scene reference. AC5480 on the creating layer
removes the store first and then decrements the scene; a sharing layer only
decrements the scene. A visible count does not own the record or the scene.
Sharing layers borrow the store itself, so they must not access it after the
creating layer retires it. Their owned scene reference can survive that removal.
The existing Screen service remains responsible for this layer state and order.

## Explicit remaining services and limits

The actual weak-handle base at 925490/925540 needs its real shared pool, critical
section and handle reference operations. `NativeGuiSceneWeakBase` requires those
two operations; it has no fallback. Construction initializes +00/+04/+08, and
destruction invalidates the actual weak handle and releases it under that pool's
lock, ending in the root reference-base vtable. This is not a reconstructed weak
pool, and an application without a concrete provider cannot create a scene.

Full AC59A0 camera naming, GuiLights/ambient/directional setup, fog/viewport
replacement, priority re-registration, and the queue/orthographic pass are not
claimed implemented here. The APIs expose real allocation, native root binding,
lighting publication, store registration and separate scene release for that
composition. The caller supplies the canonical camera and scene companions;
`require_gui_store_camera/scene` validate identity without manufacturing an owner.
The environment and companions must outlive all borrowed and queued references.

Low-level AA5070 insertion failure preserves the native absence of an unwind
cleanup for the record; it does not invent camera/scene releases. The camera
constructor's existing exceptional-domain limits remain, including native
unwind behavior for already published viewport/fog words. This packet does not
claim fault-injection parity, every scalar-delete flag, or a drop-in native ABI.

## Validation

`scripts/build.ps1`: MSVC Win32 Release succeeded. After read-only
`ghidra_export.py verify-seeds`, both existing CTests passed: reconstructed_math
and native_math_differential. These tests validate existing math boundaries,
not original-code execution of these new GUI/scene paths.

One ignored `local/gui_camera_store_probe.cpp` fixture, built with an embedded
manifest, passed: actual scene+04 and root-slot aliasing; same-map first lookup
despite a different Priority; record retirement without a scene retain/release;
second-layer scene reference deferring final destruction; existing concrete
lighting reference release before weak-base cleanup. It uses an explicitly
identified fixture weak-base provider. It isolates store retirement from camera
construction and nonempty-root traversal. Those paths are build-tested and
grounded in the reused native owners, not independently executed by this fixture.
No gameplay, live rendering, or installed-GUI validation was performed.

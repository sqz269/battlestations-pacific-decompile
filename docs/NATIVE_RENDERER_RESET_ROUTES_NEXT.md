# Native renderer reset routes

This discovery covers the complete `B24BF0` unbind body (453 bytes), `B262C0`
release body (468 bytes), and `B23B10` restore body (319 bytes). All three remain
unimplemented as actual renderer operations at source base `b327e7d`. Their
existing typed fragments do not close the original routes.

The next bounded work is texture reset and vertex-layout binding. Query reset
was completed concurrently on main in `f698fb9`; its source and final audit are
pinned separately from this discovery's base.
General cache release and vertex-stream binding still require a concrete
terminal for logical-vertex base field `+4C`; its constructor's null store is
not a whole-lifetime proof. The audit separates ready work from this barrier.

Only this document and `reports/native_renderer_reset_routes_next.json` belong
to the packet. It verifies current Ghidra project/program identity, exact
installed-PE bytes, complete primary control flow, EH metadata, current source
contracts, and concrete profile cells. No source, shared metadata, Ghidra,
build registration, permanent test, or game file changes are included. Source
and instruction evidence do not constitute an original-body fixture, binary
replacement, device-reset test, or game validation.

## Complete route table

All three entries take the actual renderer in ECX, take no stack arguments,
and end in plain `RET`. None has a stable semantic EAX result. Each reloads
actual owner/profile fields at the native accesses; a captured renderer or
device projection cannot replace those reads.

| Entry | Complete sequence | Material source boundary |
| --- | --- | --- |
| `B24BF0..B24DB5` | Texture virtual `+130` for indices 0..19; stream `+134(0,0)` four times; index `+138(0,0)`; clear pixel and vertex shader cache words and conditionally call COM; layout `+E0(0)`; direct color bind `B23D80` for slots 0..3; COM depth unbind | Four renderer binding methods remain typed projections. Color binding and optional synchronization are actual providers. |
| `B262C0..B26494` | Optional guard; gate and clear `+1D8B`; the same texture/stream/index calls; depth then four color wrappers; query, texture, surface registries; empty record walk; clear cache at renderer `+34` | Texture callbacks need raw entries; query callbacks are completed on main. General `B241C0` cannot use its constructor-only all-null fragment. |
| `B23B10..B23C4F` | Run only when `+1D8B==0 && +1D8A==0`; set ready; acquire/rebind color zero, then depth; texture, surface, query registries | No guard/EH. Actual surface operations and main's query callbacks exist; the texture packet remains a prerequisite. |

The four stream calls really pass `(0,0)` each time. Their countdown is never
passed as the stream index. Do not replace them with indices 0..3. Both routes
reload the renderer's primary table for every virtual call. The constructor
store at `B3243B` identifies profile `D5F0A8`; fresh cells resolve `+130` to
`B24710`, `+134` to `B24840`, `+138` to `B24B00`, and `+E0` to `B23F20`.

Unbind clears current `+176C` before conditional `SetPixelShader(nullptr)`
and increments `+1BBC` only after the call returns. It then clears `+1770`
before conditional `SetVertexShader(nullptr)` and increments `+1BC0` after
return. HRESULTs do not gate either counter. Neither cached word is retained
or released. These are direct device calls through current renderer `+1A10`.
The final depth unbind always calls `SetDepthStencilSurface(nullptr)`.

## Current fields and callback order

Release clears ready byte `+1D8B` before its first resource callback. It first
loads optional depth wrapper `+198C`, then visits the four actual pointer cells
`+197C`, `+1980`, `+1984`, and `+1988` in order. Each current nonnull owner gets
its current virtual `+3C`. It does not snapshot the five wrappers together.

| Registry | Release / restore slot | Native traversal |
| --- | --- | --- |
| `+19A0` table, `+19A4` count | `+18` / `+1C` | Signed index; initial count must be positive. Reload current table before each entry and signed count after each callback. No device argument. |
| `+1B00` table, `+1B04` count | `+20` / `+24(device)` | Retain the raw cursor. After each callback reload count, then base; increment the old cursor by four and compare it with the newly computed end. |
| `+1B0C` table, `+1B10` count | `+3C` / `+40(device)` | The same retained-cursor/current-end rule. Restore's post-call reads are count then base, as in the texture loop. |

Pointer endpoint arithmetic is DWORD arithmetic; pointer-loop termination is
equality, not a signed count check. Callback edits to base/count do not rebase
the retained cursor or turn these loops into mutation-safe container walks.
Restore reloads current renderer `+1A10` for every texture and surface callback.
Query restore takes no passed device and obtains the current global renderer's
device inside its concrete provider.

Release's `+1A78` / `+1A7C` record walk has stride `2C` and **no record loads,
callbacks, or stores** in its loop. It computes an initial end, reloads count
once on the nonempty branch, then advances to that second captured end. Do not
invent owner `+28` release calls here; those belong to a different recreate
route. General cache clear is called only after this empty walk returns.

Restore captures the first device before publishing `+1D8B=1`. Its local
surface-output word is explicitly zeroed before `GetRenderTarget(0, &word)`
and again before `GetDepthStencilSurface(&word)`. After each acquire it reloads
the current default wrapper, calls wrapper `+14` with the output value,
reloads that output word, and unconditionally calls its current COM `Release`.
It does not test HRESULT or add a null repair; no rollback clears ready after
a failure. The second device is a fresh `+1A10` load. These explicit zeros
are different from the texture callback's scratch below.

## EH and guards

Unbind has three states, `0`, `1`, and `2`, for its pixel, vertex, and final
depth phases. They reuse one eight-byte guard record. With entry ESP `S`, its
returned-AL byte is `S-20`, retained renderer word is `S-16`, and EH state is
`S-4`. Skipped entry does not initialize the record. Every enter, leave, and
unwind decision uses the native current-mode checks at global `108D6DC`.

Handler `CBD088` names FH3 info `DF5728`. Its three unwind-map entries at
`DF5710` each transition directly to `-1`, using `CBD070`, `CBD078`, and
`CBD080` respectively. All three eight-byte actions take guard `EBP-14` and
tail-jump to actual `B21110`. They are three exclusive scopes, not three
nested guard cleanups. Normal cleanup disarms before `B33B00`; a leave failure
does not cause another cleanup. Initial virtual unbind calls, the layout call,
and the direct color loop execute with the outer state disarmed.

Release has one state `0`, armed after optional entry even on the ready-false
path. Handler `CBD128` names info `DF581C`, with map `DF5814` selecting
`CBD120 -> B21110` and to-state `-1`. The guard uses the same `S-20/S-16`
coordinates. The entire enabled body, including cache clear, lies in that
state. Both normal exits disarm before optional leave.

Restore has no FS registration, optional guard, unwind map, or cleanup scope.
All three complete native spans contain their ordinary return tails; no
no-return annotation truncates these bodies. FH3 handlers and action thunks
are evidence for native cleanup, not new application callback providers.

## Concrete callback providers

| Current profile | Slots and original targets | Current source |
| --- | --- | --- |
| Surface `D619A0` | `+14=B3CC80`, `+3C=B3D510`, `+40=B3D550` | Full operations over `NativeSurfaceOwnerStorage` in `native_surface_owner.cpp`; no typed `D3D9SurfaceBinding` substitute. |
| Query `D62AD0` | `+18=B5FE20`, `+1C=B5FE60` | Completed on main in `native_occlusion_query_device_reset.cpp` / `f698fb9`. Reads the actual publication cell and calls COM into actual owner storage. The typed `d3d9_query.cpp` is not the provider. |
| Texture 2D `D61948` | `+20=B3DD30`, `+24=B3DD90` | Existing `d3d9_reset_texture.cpp` owns a vector, bounds inputs, and branches on HRESULT. It is not the raw callback. |
| Cube `D61870`, volume `D618B0` | `+20=B33F10`, `+24=B33F20` | Exact native `RET` and `RET4` bodies. Source entrypoints remain to be added; these evidenced no-ops are specific to these profiles. |

Fresh constructor stores corroborate these profile identities. Concrete
factory registrations into `+1B00` are documented by the existing generic
reset-resource audit; they do not prove every possible writer or complete
cube/volume unregister lifetime. This packet admits the listed current
profiles with valid reached owner storage, not arbitrary empty callbacks.

The surface provider check includes current source and complete fresh bodies:
`B3CC80` writes actual `+2C`, AddRefs its captured input, then reloads current
`+2C` for `GetDesc`; `B3D510` uses a captured AddRef/Release pair followed by a
fresh owner-field load for the final Release/clear; `B3D550` sends the actual
`+2C` cell directly to COM creation with no preflight or HRESULT branch.
Their descriptor/output validity boundaries remain those of the existing
actual provider. Texture callbacks need only surface initialization/release,
not surface construction, arbitrary destruction, or registry replacement.

## Texture restore scratch correction

The older `GENERIC_RESET_RESOURCES.md` calls the `B3DD90` creation output
uninitialized. Fresh full assembly establishes two precise seeded cells:

1. Entry ESP is `S`; the incoming device argument occupies `S+4`. After
   `PUSH ECX`, `PUSH ESI`, `PUSH EDI`, `PUSH EBX`, and shared-handle zero,
   ESP is `S-20`. `LEA EBX,[ESP+18h]` at `B3DE52` is therefore **`S+4`**.
   `CreateTexture` receives the device argument cell as its output pointer.
   The receiver was captured before the call. Invalid pool codes also copy
   the original device word into the pool register at `B3DDC4`.
2. `PUSH ECX` at entry seeds local `S-4` with the actual owner pointer bits.
   `LEA EDX,[ESP+8]` at `B3DEBB` passes that cell to `GetSurfaceLevel`.
   The cell is neither initialized to null nor cleared between levels.

After `CreateTexture`, the callback reads current owner `+10` and the current
device/output cell. If distinct, it publishes the output into owner `+10`,
conditionally AddRefs the new pointer, and conditionally releases the captured
old pointer. It reloads the output cell after those COM callbacks. A nonnull
current temporary then receives Release, followed by clearing **the argument
cell**, even if that Release changed it. No HRESULT branch is introduced.

For each level, capture current texture/table, then load current record base
and key for `GetSurfaceLevel`. After that call, reload the record base to find
the current nested wrapper at `base + index*8 + 4`; the destination can differ
from the record used for the key. Bind through the current surface profile,
reload the scratch output after binding, Release it unconditionally, and test
the freshly loaded signed count. Scratch survives across iterations.

A future API must make both mutable output cells and their initial pointer
bits explicit before fixture work. It must preserve the receiver/output alias
and callback-visible rereads. A null-initialized temporary, invented default
pool, or stable-vector rewrite would change native behavior. This correction
also supersedes the older statement that the body is absent from saved
analysis: the current guarded prototype contains the full 362-byte function.

## Bounded next packets

| Packet | Exact owned functions | Readiness and required contract |
| --- | --- | --- |
| `native_occlusion_query_device_reset2` | `B5FE20` 27 bytes; `B5FE60` 33 bytes | Completed on main `f698fb9`; final audit pins original/main real-D3D9 fixture and strict Win32 validation. Borrow actual owner and the address of current renderer publication; reuse actual `B1FEF0`, call real COM directly into owner `+10`. No old-pointer cleanup or HRESULT policy. |
| `native_texture_device_reset4` | `B3DD30` 86; `B3DD90` 362; `B33F10` 1; `B33F20` 3 bytes | Bounded implementation is ready after explicit scratch/profile API freeze. Required surface operations are actual full providers. Preserve current signed level loops, argument/output alias, seeded reused surface cell, and the separate 16-byte pool table. |
| `native_renderer_vertex_layout_binding1` | `B23F20` 226 bytes | Ready for profile `D62AF4` and existing actual hardware-owner context. Current zero slot `BD30E0` rereads deleting slot `B60770`; actual lifetime, `B5FF00` getter, Win32 atomics and optional guard providers exist. Preserve the device table captured before getter and device receiver reloaded afterward. |
| `native_renderer_index_binding1` | `B24B00` 236 bytes | Source prerequisites exist for `D61DE0` and physical `D61E10/D61E58`, but its raw current-profile adapter must be fixed before declaring the packet ready. The `B48DC0` naked tailcall requires callable relocated tables; original numeric tokens are not callable host addresses. |

These address sets are disjoint from the three discovery leases. Texture
callbacks can progress independently of binding work. Restore `B23B10`
becomes the next route implementation after its raw providers are integrated
and the listed profile/actual-storage API is frozen. It must keep every
current-field load and original loop rule described above.

`B24710` texture binding still needs its complete old-owner terminal domain:
2D and cube lifetimes are present, while volume `B3F430/B3EB70` is unresolved.
The cube/volume borrowed getters `B3CF90/B3D0C0` are fresh four-byte `+10`
leaves but have no separate source entries. `B24840` vertex binding also needs
full `B23710` intrusive assignment and logical-vertex destruction, whose
`B62010 +4C` terminal remains unresolved. These are prerequisites, not ready
full binding packets.

General `B241C0` is a separate 664-byte prerequisite. It visits 26 actual
owner cells, calls real InterlockedDecrement, dispatches each current zero
slot, and clears only after return. Its null branches preserve the original
owner word. The private constructor fragment proves those cells null only
for its sole constructor caller; `B262C0` supplies no such proof. General reset
visits 16 sampler banks, preserving the extra four owners and gamma at cache
`+1938`. Closing the terminal profiles and current-field behavior is required;
`D3D9StateCache::invalidate` is not that implementation.

The remaining vertex and general cache barriers continue to block complete
unbind/release composition and therefore the broader `B29670` device recreate
prerequisite. This discovery supplies concrete next work without claiming
that reconstructing the smaller callbacks completes device recreation.

## Primary integration

The primary revalidated 52 source/evidence/artifact pins, 32 current
source files and all 85 live-Ghidra/installed-PE spans (5,392 bytes). This
remains dependency discovery, without a new runtime claim. Query reset2 is
complete on main. Actual vertex-layout binding1 is assigned to
`system_fog_owner_discovery`; texture reset4 is assigned to
`particle_clock_lifetime` with explicit device/output and owner-seeded surface
scratch cells. Index binding retains its current-profile adapter prerequisite.
The complete three renderer routes remain open until their listed actual
bindings, lifetime paths and cache-release dependencies are implemented.

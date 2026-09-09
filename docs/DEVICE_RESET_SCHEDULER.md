# Device reset scheduler

`00b2abd0` is a renderer-ECX routine with no stack arguments and ordinary
`RET`; it preserves EBX/ESI/EDI/EBP and installs an SEH frame. It has a complete
589-byte body through `00b2ae1c`. This audit distinguishes its own state writes
from effects of called helpers, especially the still-separate device recreation
fallback `00b29670`.

## State and entry gates

| Location | Observed role in this routine |
|---|---|
| Global `0108d4c8` | Recorded render thread ID, compared against `GetCurrentThreadId` |
| Global byte `0108d4b8` | Pending reset request |
| Global byte `0108d4b9` | Enables cooperative-level polling; cleared alongside pending after reset success |
| Global DWORD `0108d4c4` | Consecutive/persistent lost polling counter; exact reset sites below |
| Renderer `+1d8a` | Lost/unavailable byte |
| Renderer `+1d8b` | General resource-ready byte, changed by release/restore helpers |
| Renderer `+1d8c` | Dynamic-buffer-ready byte, changed by paired buffer helpers |
| Renderer `+1d90` | Draw-inhibit/countdown field; written to 2 after successful Reset |
| Renderer `+1a10` | Device pointer |
| Renderer `+1a28` | Stored presentation parameters |
| Platform singleton `0109cf04`, `+41` / `+30` | Active byte / HWND used for focus gating |

The scheduler enters the optional renderer guard before checking thread
identity. If the current thread differs, it returns without polling/resetting.
The guard remains held across polling, callbacks, COM Reset, fallback calls,
and the sleeps described below. Nested release helpers use the same optional
guard infrastructure. The global guard mode is checked again before leaving;
concurrent mode changes remain outside the established safe contract.

If both global bytes `0108d4b8` and `0108d4b9` are zero, the routine returns.
It initializes a local cooperative result to zero, then calls
`device->TestCooperativeLevel()` **only if `0108d4b9` is nonzero**.

## Cooperative-level decision table

| Poll result / state | Exact next behavior |
|---|---|
| Polling disabled, pending set | Keep local result zero; proceed to active/focus gate |
| `D3DERR_DEVICELOST` (`88760868`) | Increment lost counter modulo DWORD, mark renderer lost, and take the lost branch below; no Reset attempt |
| `D3DERR_DEVICENOTRESET` (`88760869`) | Proceed to active/focus gate regardless of pending byte |
| Any other result, pending clear | Return without Reset or fallback |
| Any other result, pending set | Proceed to active/focus gate; only exact zero enters Reset, other values enter fallback decision |

The lost branch at `00b2ac7d..00b2acc6` increments the counter, stores it, and
sets renderer `+1d8a=1`. Its threshold is **unsigned greater than 10** (`JBE`
skips recovery), so the eleventh qualifying observation triggers eligibility.
Recovery additionally requires platform active byte nonzero and
`GetFocus()==platform.HWND`. Only then does this branch set the counter to zero
and call `00b29670`. It immediately leaves the scheduler afterward, without
executing its common sleep. Lost observations lacking focus still increment
the counter. This routine does not reset the counter merely because a later
poll/Reset succeeds; only the stated recovery site writes zero here. Effects
inside `00b29670` are a separate matter.

All paths admitted to the normal reset/fallback stage first require platform
active and exact focus equality. This is `GetFocus`, not foreground-window
identity. The platform pointer is loaded before the focus call, and its HWND
is read afterward. Failure exits without the common sleep or global-flag clear.

## Reset attempt, success, and failure

At `00b2ad14`, the scheduler accepts only local cooperative result
`88760869` or **exactly zero** for a Reset attempt. Other results go directly
to the fallback decision without the pre-reset state transitions.

The attempt sequence is:

1. Set renderer lost `+1d8a=1`; `Sleep(100)`.
2. Call dynamic-buffer release `00b237d0`.
3. Call general resource release `00b262c0`.
4. `Sleep(100)`.
5. Reload platform singleton and call HWND getter `00bec230`; store its result
   at presentation parameter `+1a44` (`hDeviceWindow`). This is a fresh HWND,
   not necessarily the earlier focus-gate value.
6. Call `device->Reset(&renderer.presentation_parameters)` at `00b2ad6b`.
   The live stored parameter structure is passed directly, so API mutations
   remain in it; there is no temporary-copy rollback.

Reset success is tested using `CMP EAX,0; JNZ`, **not** `SUCCEEDED(hr)`.
Only exact `D3D_OK` follows the success sequence:

1. Clear renderer lost byte; set `+1d90=2`.
2. Call resource/default-surface restore `00b23b10`.
3. Call dynamic-buffer restore `00b1fd90`.
4. Cached render state 161 (`MULTISAMPLEANTIALIAS`) receives whether stored
   presentation `MultiSampleType` at `+1a38` is nonzero.
5. Call default-state initializer `00b26170`.
6. Call renderer virtual `+f0` with the cached gamma float at `+196c`, loaded
   and stored through x87 into the stack argument.
7. Clear both globals `0108d4b8` and `0108d4b9`.
8. `Sleep(100)`, then leave the optional guard and return.

There are no HRESULT checks or rollback branches around the restore/default/
gamma helpers. In particular, the scheduler clears pending/polling flags after
those calls without independently proving that every restored resource exists.
Typed error propagation is useful but must be identified as an interface
extension rather than native transactional behavior.

A nonzero Reset return, or an unexpected cooperative result admitted by a
pending request, calls immediate gate helper `00b20c50` on the current platform
singleton. This repeats active-byte and focus-HWND checks. If true it calls
`00b29670`; if false it skips fallback. Both then perform the common
`Sleep(100)`. The scheduler itself does not clear pending/polling or restore
lost/readiness in this failure branch. For an actual failed Reset, preceding
release helpers normally left readiness bytes zero and lost one; any further
changes made by fallback cannot be inferred from this scheduler.

Thus a complete Reset attempt has three 100 ms sleeps here. A direct unexpected-
cooperative-result fallback has one. Wrong-thread, no-work, missing-focus and
DEVICELOST branches have none. These sleeps are not license to replace the
logic with a fixed periodic reset loop.

## Immediate helper evidence and integration boundary

`00bec230` is exactly `MOV EAX,[ECX+30h]; RET`: borrowed platform HWND getter.
`00b20c50` checks byte `+41`, calls GetFocus only if active, compares against
`+30`, and returns boolean one or zero. Neither helper has an internal guard.
The import table independently identifies `00ce223c=GetCurrentThreadId`,
`00ce2310=GetFocus`, and `00ce2230=Sleep`.

The nearest still-missing, bounded implementation pair is dynamic-buffer
readiness around existing buffer wrappers:

- `00b237d0`: optional guard; skip when `+1d8c=0`; otherwise clear it before
  callbacks, release/null the vertex COM buffer at owner `+1974` first and
  index COM buffer at owner `+1978` second. Preserve wrapper identity and
  metadata. Each nonnull COM pointer gets a balanced AddRef/Release pair before
  the owned Release; both wrappers are assumed present. No lost-byte gate.
- `00b1fd90`: no internal guard; skip when buffers are ready or renderer lost.
  Otherwise set ready one before calling vertex wrapper `+20(device)`, then
  index wrapper `+20(device)`. The device pointer is loaded separately for
  each callback. No HRESULT rollback is present.

Current `VertexBufferBinding` / `IndexBufferBinding` and recreation helpers
supply much of this pair's operation. Preserve the early readiness writes,
vertex-before-index order, and explicit error differences. This pair can close
a concrete scheduler dependency without duplicating default-surface work or
inventing a full renderer object.

The scheduler itself still needs a coherent renderer/platform owner, exact
surface/resource release and restore traversal, cached gamma behavior, and the
`00b29670` recreation fallback. It should not be marked reconstructed by
injecting no-op callbacks for those branches. A pure decision projection can
document gates, but cannot replace the device/lifecycle work required by the
actual startup-to-game objective.

## Byte evidence and uncertainties

Each export/memory batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Full bodies below
matched installed PE bytes exactly; exported scheduler assembly includes all
branches and its final return, with no truncated free/no-return continuation:

| Complete body | Bytes | SHA-256 |
|---|---:|---|
| `00b2abd0..00b2ae1c` | 589 | `923113874aa3e3eca6eb973e7ce7b7f157d10fb53c3d4acfc4d16cee80b8567a` |
| `00b20c50..00b20c6e` | 31 | `ea7ac658dc0fa3d2ac7b95e845cb22f1eae18f27fdcc172b5ae961929c3a810a` |
| `00bec230..00bec233` | 4 | `c8cc0b90c656bd6a43ec1c5684c1a494142d2fce4803153d9b1a0944cad7e281` |
| `00b237d0..00b238c2` | 243 | `92b213ddab7130d8ac41cc441d12d7392b628d8b8c68202db471f69bf9fad71c` |
| `00b1fd90..00b1fdd5` | 70 | `bbd6b07b5ee4ea6764bab28df153cad3383743b11aad75deee1eb67e6a64fa81` |

The original field/global names are descriptive hypotheses. This audit does
not establish every writer of those globals, exceptional callback behavior,
concurrent mutation semantics, all recreation-fallback effects, or runtime
fullscreen/lost-device correctness. It changes only this document, not code,
shared metadata, Ghidra annotations, or test/build state.

# Native surface ownership and the next concrete dependency

The smallest ready implementation is the three-function lazy resource-support
owner at `0108FEDC`: getter `00B3E730`, constructor `00B61D50`, and deleting
destructor `00B61D60`. It can use the existing concrete singleton lifetime
domain and actual CRT allocator. Full surface construction/destruction needs
that owner, the recovered surface pool, pooled strings and actual renderer
surface-list removal. A COM-only terminal adapter cannot supply those actions.

This discovery changes only this document and
[`native_surface_owner_next.json`](../reports/native_surface_owner_next.json).
The checkout is based on `55505bb`; surface-pool dependency `ae51f49` is a
separate completed commit. The saved target is `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Seventy-two bounded ranges matched live Ghidra
and the installed PE, including fourteen rechecked entries from the earlier
fifty-span group discovery. Names below describe behavior; original source
symbols and the resource-support class's original purpose remain unknown.

## Actual surface storage and profile

The surface object occupies `0x34` bytes inside a `0x38` raw pool slot.
`0108DB00` owns that slot, and its trailing DWORD at `+34` is the current slab
ID. Surface construction and destruction leave that DWORD untouched.

| Offset | Native storage |
| --- | --- |
| `00` | Surface profile `00D619A0` |
| `04` | Intrusive count, initialized to one |
| `08`, `0C` | Raw scalar words initialized to `3F800000` / float one |
| `10`, `14` | Native string length and pooled buffer pointer |
| `18` | Format |
| `1C`, `20` | Width and height |
| `24` | Multisample type |
| `28` | Wrapper flags |
| `2C` | Owned `IDirect3DSurface9` reference |
| `30` | Exact recreation-kind byte |
| `31..33` | Constructor-unwritten bytes |
| `34..37` | Pool slab ID, outside the surface object |

Both shadow color and depth wrappers use `00D619A0`. Its relevant virtuals are
`+00 -> 00BD30E0` (call current deleting virtual with flag one),
`+04 -> 00B3F5B0`, `+14 -> 00B3CC80` (initialize binding),
`+3C -> 00B3D510` (release for reset), and `+40 -> 00B3D550` (recreate).
The report records all seventeen table entries. Format/size/multisample getters
are direct field reads. Other entries include name access, LockRect, UnlockRect
and `D3DXSaveSurfaceToFileA`; the copy-operation entry is outside this packet.

## Complete construction, COM ownership and cleanup

`00B3F630..00B3F7AD` receives ECX = raw owner and three stack arguments:
surface pointer, flags and recreation-kind byte. It returns the same owner in
EAX and uses `RET 0C`. No second surface constructor was found among the five
direct creation paths. Adjacent `00B3F5D0` constructs a twelve-byte diagnostic
record containing a borrowed pointer and native string, not a surface.

Construction installs base `00CEB130`, count one and the two scalar words,
then the surface profile and empty name. It stores null COM `+2C`, the exact
input byte at `+30`, and flags at `+28`. For a nonnull input it performs a
balanced AddRef/Release pair on that captured input before and after calling
the binding initializer. These pairs remain observable COM calls.

`00B3CC80..00B3CCE3`, ECX owner and stack COM pointer, `RET 4`, stores the
incoming pointer directly at `+2C` without releasing a previous value. For
nonnull input it AddRefs that input, reloads the current stored `+2C` pointer
and invokes GetDesc through it, then copies format, dimensions and multisample
from an uninitialized stack descriptor. It ignores HRESULT and preserves flags.
Null input clears `+18`, `+1C`, `+20`, `+24` and `+28`, while retaining `+30`.

The remaining constructor creates a seven-character pooled string `Surface`,
copies it into a temporary twelve-byte diagnostic record containing the
captured input pointer, invokes the real `00B3E730` singleton getter, and frees
the record's string before the first temporary string. That record does not
AddRef/Release its pointer. The object's name at `+10` remains empty. Finally,
the constructor reloads flags and increments `0108DAFC` if bit `10` or `100`
is set. Consequently a null input also clears the flags that would trigger this
counter, and callbacks can affect the final flags load.

Constructor FuncInfo `00DF77BC` has four unwind states. They destroy, in reverse
order, the diagnostic record string, first temporary string, object name, and
base. Its base leaf goes through the five-byte thunk `00B3D4F0 -> 00BD30F0`.
There is no COM release or pool return in this constructor unwind map.
The outer creation routines have separate raw-slot cleanup leaves that call
`00B3DCC0`, which returns the unchanged `+34` slot ID through the canonical pool.
The report includes those maps for 2D, cube, registered target, offscreen and
default-surface factories. They do not establish COM rollback on constructor
failure; automatic cleanup must not invent it.

`00B3F4E0..00B3F587` performs this order:

1. Install the surface profile and load current renderer global `00F8D394`.
2. Call `00B27D60`, which removes the first matching borrowed pointer from the
   renderer's actual `+1B0C/+1B10/+1B14` array via `00B25630`.
3. Invoke the real resource-support getter.
4. Reload COM `+2C`, Release it if nonnull, then clear `+2C` after the callback.
5. Reload flags and decrement `0108DAFC` for bit `10` or `100`.
6. Return the name buffer using its captured pointer and length plus one,
   without clearing the native name words; call the reference base destructor.

Its two-state EH map cleans the name and base, with no extra COM cleanup.
`00B3F5B0..00B3F5CF` calls this complete destructor, then returns the same raw
slot through `00B3D860` with ECX = `0108DB00` only when deletion flags bit zero
is set. It returns the original pointer in EAX with `RET 4`, even after slot
return. There is no ordinary surface-object free. An exception from the ordinary
destructor prevents execution of the subsequent pool return.

## Parents, shadow groups and reset responsibilities

`00A8FF30` creates the shadow color 2D texture with flags `10` and depth 2D
texture with flags `100`, then calls each texture's virtual `+30(0,0)`.
It publishes creator-owned texture references at target `+10/+18` and returned
surface references at `+14/+1C`. These textures use profile `00D61948`.

`00B3FD80` takes two stack words and uses `RET 8`; only the first, mip level,
is consumed. Its pseudocode omits the unused second argument. A cache hit
increments the actual wrapper count before returning it. A miss obtains a
temporary COM surface, allocates from `0108DB00`, constructs the surface with
the texture's flags and low byte `((flags >> 8) & FFFFFF01)`, then drops the
temporary COM reference. If flags bit zero is clear it also increments the
wrapper count for the cache and appends an actual `(mip, wrapper)` record at
texture `+40/+44/+48`. The returned creator reference belongs to the caller.
Both observed shadow flags therefore give separate target and cache references.

Cube accessor `00B3FE90..00B3FF24` consumes `(mip, face)` with `RET 8`, calls
GetCubeMapSurface with face then mip, constructs the same surface profile with
kind zero, releases the temporary COM surface and returns the creator reference.
It does not append a 2D cache record. The 2D/cube native paths ignore the getter
HRESULT; no typed fallback for invalid output is established.

Global target getters `00A8FDA0/00A8FDC0` return borrowed `+14/+1C` pointers.
Group assignments `00B1FAB0/00B1FB00` retain the actual surface count before
releasing a previous wrapper. The group's separate cached COM references have
their own lifetime and are not refreshed by those assignments. Five groups
created by the shadow owner each add their own surface references.

Target destruction `00A8FFF0` releases color surface, color texture, depth
surface, then depth texture, clearing each field after its callback. A 2D texture
destructor releases its retained source stream, renderer associations and COM
texture before dropping cached surface-wrapper references. Cached-wrapper zero
release can therefore reach the complete surface destructor and pool return.
No field or call in the surface wrapper retains the native parent texture;
the cache/target/group owners and their separate COM references supply the
observed retention. No additional parent edge should be fabricated.

Reset keeps wrapper identities and intrusive counts alive:

- `00B3D510` captures `+2C` for the balanced AddRef/Release pair, then reloads
  `+2C` for the final Release and clears it after return. Metadata is preserved.
- `00B3D550` uses current `+30` to choose CreateRenderTarget (lockable false)
  or CreateDepthStencilSurface (discard true), quality zero and null shared
  handle, writing directly to `+2C`. It adds no nonempty guard and does not
  interpret HRESULT.
- Parent texture `00B3DD30` invokes cached-wrapper virtual `+3C` before releasing
  its COM texture. `00B3DD90` creates the texture, obtains each cached mip and
  invokes the existing wrapper's virtual `+14`, then releases the getter COM
  reference. Cached surfaces are not reconstructed or added to the renderer
  surface list during these operations.
- Default capture `00B238D0` uses `(flags=0, kind=0)` for both color and depth,
  retains wrappers in renderer `+197C/+198C` and drops creator/getter references.
  Reset restores these exact wrappers through GetRenderTarget/GetDepthStencilSurface
  and virtual `+14`; the generic recreation-kind branch does not restore defaults.
- The registered render-target factory appends a borrowed wrapper pointer to
  the renderer list. Construction itself does not append it. Release/restore
  of that list use virtual `+3C/+40` with retained raw cursors and freshly loaded
  base/count endpoints. Stable storage/lifetimes are required by current typed
  projections; a snapshot traversal would alter native behavior.

## Reuse boundaries

`D3D9SurfacePool` from `ae51f49` supplies the actual slot layout, current IDs,
allocation, returns, compaction and canonical binding. `NativeString` and the
actual pooled storage can supply the eight-byte string operations, with explicit
native destruction preserving stale name words. The existing concrete lifetime
domain supplies manager lookup, real section storage and shutdown registration.

`D3D9SurfaceBinding` is a projection of fields `+18..30`, not an intrusive owner.
Its initializer rejects nonempty state, zero-initializes the descriptor and
rolls back AddRef on GetDesc failure. It also calls GetDesc on its captured
input rather than the native post-AddRef field reload. Its reset helper reloads
the field between the initial AddRef and Release, unlike the native captured
pair. Its general release clears all metadata, unlike the native destructor.
Its recreate helper adds a nonempty guard. These are useful supported-domain
COM helpers, not full native callback/failure behavior.

`D3D9SurfaceRegistry` stores binding pointers in a vector and substitutes its
capacity bookkeeping; it is not the actual raw renderer array or native wrapper
identity. `D3D9ResetTexture2D` borrows level bindings, and `D3D9FrameTargets`
uses shared binding owners. Neither provides the actual count, scalar deletion,
surface pool return or parent cache ownership. A later full surface packet needs
an actual raw renderer-list binding/removal implementation and native field
operations; unresolved reset or destruction must not be replaced with no-ops.

## Ready follow-up packet

Proposed files: `include/bsp/native_resource_support.hpp`,
`src/native_resource_support.cpp`, `docs/NATIVE_RESOURCE_SUPPORT.md`, and
`reports/native_resource_support_audit.json`. Own only `00B3E730`, `00B61D50`
and `00B61D60`; EH/guard/lifetime helpers are supporting evidence.

The API should expose actual eight-byte `NativeResourceSupportStorage`, take a
reference to the real published pointer representing `0108FEDC`, and accept the
same `SingletonLifetimeDomain` used by other registered owners. It must retain
the actual manager section and explicit counter, perform the cold-path recheck,
allocate eight CRT bytes, publish after construction, perform the second manager
lookup and registration, unlock, and return the live published pointer. A warm
lookup performs no manager work.

Constructor `00B61D50` writes only vtable `00D62B64` and returns the owner.
The four bytes at `+04` remain untouched; their role is unknown. Deleting
destructor `00B61D60` unconditionally clears the published pointer, writes base
profile `00CE3818`, frees through the actual CRT boundary for flags bit zero,
then returns the original pointer. Its omitted post-free `ADD ESP,4` at
`00B61D80` is part of the complete function. It does not unregister itself from
the manager. Primary shutdown dispatch must bind that real deleting action.

Getter EH state one frees the just-allocated object and proceeds to state zero,
which decrements/unlocks the captured section through `00411EE0`. Once published,
registration runs in state zero, so a registration failure unlocks without
rolling back or freeing the published owner. The class's original purpose is
provisional, but allocation, publication, registration and destruction are
concrete. One focused cold/warm/shutdown fixture with real allocation and the
actual shared lifetime domain is sufficient for this packet; no surface, COM,
renderer reset or independent singleton registry belongs in it.

Discovery verified source and native bytes only. No C++ implementation, build,
native runtime fixture, Ghidra mutation, ledger change or game validation was
performed in this packet.

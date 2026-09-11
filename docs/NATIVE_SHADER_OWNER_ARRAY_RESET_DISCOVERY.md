# Native shader-owner array reset discovery

The complete unload pair `00B188A0` / `00B24E20` is ready for bounded native
source implementation using the existing 2D, cube and volume owner providers.
The reload pair `00B19000` / `00B24DD0` is not ready: its actual Lua override
and renderer resource-loading routes remain incomplete. A callback for either
route would leave the requested native lifecycle unresolved.

This read-only discovery is pinned to source commit
`1085a30c28970d7256c579040d2d15a6b9e7774a`. Every live read used guarded
`python tools/bsp.py ghidra`, which verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base before dispatch.
The configured project is `C:/Users/sqz269/bsp.gpr`. The companion
`reports/native_shader_owner_array_reset_discovery.json` contains exact bytes
and SHA-256 hashes for 23 installed-PE-matched code, profile and EH spans, plus
18 current source file hashes and Git blob IDs. No Ghidra writes, source edits,
build, native differential fixture, device reset or game validation occurred.
These descriptive operation names are hypotheses; this is not a binary ABI
replacement or a claim that all native object profiles have been reconstructed.

## Actual caller storage and ABI

All four primary bodies receive their object in ECX, take no stack arguments,
and end in plain `RET`. EAX has no stable semantic result. Ghidra's current
zero-argument prototypes and fastcall pseudocode do not describe an EDX input.
The assembly proves the receiver and the direct child call's ECX setup.

The renderer's profile `00D5F0A8` contains `00B24DD0` at `+120h`
(`00D5F1C8`) and `00B24E20` at `+124h` (`00D5F1CC`). Current direct-call
queries found no callers for either parent; their table references were read
directly. The bounded implementation therefore accepts the actual renderer
receiver, without inventing a device-reset scheduler.

| Storage | Established accesses |
| --- | --- |
| Renderer `+1A98h` | Embedded effect registry; constructor stores profile `00D5F074` at `00B325A8`. |
| Renderer `+1A9Ch` / `+1AA0h` | Current record base/count, zeroed at `00B32590` / `00B32596`. |
| Record stride `2Ch`, record `+28h` | Actual shader-owner pointer, passed directly as ECX with no retain or null check. |
| Owner `+0Ch + index*4` | Actual retained texture pointer cell. |
| Owner `+38h` | Texture high-water short; unload preserves it. |
| Owner `+3Ch + index*8`, `+94h` | Native eight-byte registered names and name high-water short, used by reload. |

Both parent loops first read count, then base, and compute an end using DWORD
`count*2Ch + base`. They test cursor/end equality, not signed count positivity.
Each iteration captures `[old_cursor+28h]` for its direct child. After the child
returns it reads current count, then current base, computes a new end, advances
the **old cursor** by `2Ch`, and repeats until equality. Neither routine writes
the registry header, allocates records, clears the registry or rebases its
cursor after reentrant mutation. No capacity model or copied vector is needed.
All reached raw addresses must remain valid at the original accesses; these
are not mutation-safe container traversals.

## Complete unload behavior and available terminal closure

`00B188A0..00B188F4` has no EH frame and no guard. Its initial unsigned
16-bit comparison skips exactly a zero texture count. Index starts at zero.
At each iteration it sign-extends the current `+38h` short to a DWORD,
compares the index **unsigned**, and stores the low short of `index+1` if the
index is at least that bound. It then loads the slot once.

For a nonnull captured texture, it performs `InterlockedDecrement(texture+4)`.
Exactly zero invokes the texture's current table slot zero with ECX texture
and no stack arguments. The owner slot is set to zero only after that dispatch
returns; even a nonzero decrement result is followed by the zero store. An
exception from terminal destruction propagates before that zero store. There
is no retry, rollback, early null publication or enclosing cleanup. Null slots
skip both the decrement and the zero store.

It then reads current `+38h`, increments the unsigned index, and compares it
against the sign-extended short as an unsigned DWORD. Do not replace these
reads with one cached count or drop the apparently redundant count-growth
store: callbacks can change the raw owner. Negative high-water shorts become
large unsigned bounds; the native body supplies no capacity repair. The usual
eleven-slot owner layout does not establish native bounds safety.

The routine leaves the texture high-water count, registered names, name count,
error texture at `+98h`, and secondary object slots untouched. This is the
texture-unload phase; it does not replace owner destruction or the broader
`00B187A0` cleanup that the older owner discovery describes.

The complete terminal route already used by current
`src/native_renderer_texture_binding.cpp` supplies a bounded implementation:

| Current actual texture profile | Slot zero | Fresh slot `+4h` | Established source terminal |
| --- | --- | --- | --- |
| `00D61948` | `00BD30E0` | `00B3F590` | `delete_native_texture_2d_00b3f590`, `native_texture_2d_owner.hpp/.cpp` |
| `00D61870` | `00BD30E0` | `00B3F410` | `delete_native_cube_texture_00b3f410`, `native_cube_texture_owner.hpp/.cpp` |
| `00D618B0` | `00BD30E0` | `00B3F430` | `delete_native_volume_texture_00b3f430`, `native_volume_texture_owner.hpp/.cpp` |

The full `00BD30E0..00BD30EE` body tests its receiver, reloads the current
owner table, and calls table `+4h` with deleting flag 1. It does not decrement
again. The first slot-zero read and the later slot-4 read must remain separate.
The existing binding implementation uses original numeric profile/call tokens
to select these actual source providers, without converting native code words
into callable host pointers. Its helper is file-local; there is no exported
generic texture-release API at this source base.

The new pair can borrow `NativeRendererTextureBindingContext` unchanged: it
already supplies all three actual owner contexts and actual profile words.
Those providers borrow the application's real renderer publication, optional
synchronization, string pool, canonical texture/surface pools, retained-memory
owners, counters and support singleton. Their published domain requirements
remain in force. This is source-ready for the three admitted profiles, not
proof that an arbitrary table encountered elsewhere is supported. Neither a
new shader-owner constructor nor registry allocation is a dependency of these
two complete borrowed-storage operations.

## Reload boundary, current providers and corrected string contract

`00B19000..00B191CE` walks current name count `+94h` in ascending unsigned
index order using the same signed-short-to-unsigned-bound pattern. It
constructs a zero selected-name header and calls `00B1BC70` with ECX equal to
the **current** override-manager publication `00F8D434`, output native-string
storage, and the actual registered-name header. A nonempty override takes
priority; otherwise it copies the current registered name. Copying uses full
`0041DD40` with preserve 1 and then the current destination length, following
the actual post-allocation source/header reads. `00BF7680` is the game's
overlap-aware memcpy implementation; a future port must not introduce a C++
non-overlap precondition silently.

An empty selected name skips loading and leaves an existing texture slot
unchanged. A nonempty selection reloads renderer publication `00F8D394` and
calls its current table `+64h` with the native string header and flag 0.
Current profile `00D5F0A8` resolves this to named-but-unported `00B319B0`.
After that returns, the helper grows the texture high-water short if needed,
captures the old slot, and compares it with the returned texture. On identity
change it publishes the new pointer, retains nonnull new, then decrements and
possibly destroys old. It finally releases the temporary returned reference,
including on pointer identity. A null returned pointer clears an old slot;
an empty selected string does not. Both zero transitions require the same
current-profile terminal closure as unload. It never rolls back a published
slot when a subsequent operation throws.

`00B1BC70..00B1BD1D` receives ECX manager, stack output header/name header,
returns the output header in EAX and uses `RET 8`. It obtains a Lua globals
object from manager `+4h` through `00B67980`, indexes it through `00B68100`,
destroys the globals temporary, tests nil through `00B65FB0`, constructs the
output string, then destroys the lookup object. The native `14h` LuaObject
tracking and cleanup remain material dependencies.

Two details refine the older `SHADER_OWNER_TEXTURES.md` wording:

* `00B68100` passes the registered header's data pointer as a **C string**,
  substituting address `0108FF2C` only when data is null. Stored length is not
  the Lua key length. The lookup goes through full `00B67800`.
* Nil passes literal address `00F8D438` to `0041E870`, whose current ledger and
  assembly identify a **C-string constructor**, despite its historical name
  `BSP_NativeString_Assign`. It zeroes the output header and scans the pointed
  characters; `00F8D438` must not be modeled as an eight-byte string header.
  Non-nil uses `00B685C0`: only a bound kind-2 LuaObject with exact Lua type 4
  uses `lua_tolstring(..., NULL)`; otherwise it constructs from the C string
  at `00CE3A0C`. Both fallback locations begin with zero in the saved image.
  This does not prove the runtime bytes at `00F8D438` immutable or always empty.

Existing `NativeStringStorage` / `PooledStringStorage` and actual-header
resize/destruction provide the shared-pool string boundary. They do not
implement the native override manager or its LuaObject storage.
`GuiLua51Host` explicitly uses host registry handles instead of the original
`14h` tracking layout and has protected host behavior; passing that host as a
replacement callback would not close the native routine.

| Named or existing dependency | Current status at the pinned source base |
| --- | --- |
| `0041DD40`, `0041DD20`, actual `00419CC0` / `00BD1510` string-pool route | Actual-header operations and actual shared-pool provider are available; retain the existing supported domain. |
| `00B1BC70`; `00B67980`, `00B67800`, `00B67700`, `00B65FB0`, `00B68100`, `00B685C0` | Native override/output/LuaObject family incomplete; some names and typed Lua behaviors exist. |
| `BSP_Renderer_LoadTextureByName` `00B319B0` | Complete body known, no complete source provider. Optional guard, copied lowercase name, actual renderer `+1A74h`, `00B30B40(name,flags,0,1)`. |
| `00B30B40` | Complete resource-manager cache/alias/resolve/load route unported, despite existing native record and alias helpers. |
| `BSP_TextureManager_ResolveName` `00B31C20`, fallback initialization `00B31BD0` | Actual cache/fallback resolution still incomplete. |
| `BSP_TextureManager_LoadFromFile` `00B2C2D0` | Existing `load_retained_texture_2d_00b2c2d0_fragment` covers a typed successful 2D route. It does not provide the whole actual loader/VFS/cache/exception/other-image-type route. |
| `BSP_VFS_ResolveExistingName` `00BDF4C0` | Existing `resolve_existing_resource_00bdf4c0_fragment` is explicitly partial; naming is not full provider closure. |

## Exception evidence

Unload and both parent loops have no EH frame. Reload's handler `00CBC5F0`
loads FH3 info `00DF4898`, with two unwind states and no catch map. The map at
`00DF4888` is `0 -> -1` using `00CBC5E0`, then `1 -> 0` using `00CBC5E8`.
Those actions destroy selected and override native strings respectively via
full `0041DD20`, at EH-frame offsets `-20h` and `-18h`. State 0 begins after
the selected header is zeroed; state 1 begins only after override construction
returns. Normal cleanup lowers to state 0 before releasing the override and
to -1 before releasing selected. A failed override construction therefore
does not attempt to destroy the unconstructed override. Texture mutations
have no separate compensating cleanup. Original FH3 second-exception behavior
must be preserved if this family is later implemented.

The override helper's handler `00CBC8E9` names info `00DF4CF0`, with four
states and no catch map. At `00DF4CD0` the transitions are: state 0 to -1 via
`00CBC8D0` (output destruction guarded by bit 0 at EH-frame `-38h`), state 1
to 0 via `00CBC8C0` (globals at `-20h`), state 2 to 1 via `00CBC8C8` (lookup
at `-34h`), and state 3 to 0 via the same lookup action. Normal code arms the
output bit only after successful construction and lowers to state 0 before
the final lookup destructor. This is an output-construction contract, not
plain assignment to a synthetic string. Exact handler/map/action bytes are
in the report. No x87 or suspect no-return body was encountered in this packet.

## Concrete next packets and ownership

| Packet | Function ownership | Files and integration boundary |
| --- | --- | --- |
| `native_shader_owner_texture_release2` | `00B188A0`, `00B24E20` | Ready. Worker owns new `include/bsp/native_shader_owner_texture_release.hpp`, matching `src/` file, `docs/NATIVE_SHADER_OWNER_TEXTURE_RELEASE.md`, and `reports/native_shader_owner_texture_release.json`. Borrow actual owner/renderer pointers and existing `NativeRendererTextureBindingContext`; compose complete terminals in a local helper. Primary integrates CMake and sharded names/reconstruction records after review. |
| `native_shader_owner_override_discovery` | `00B1BC70`, `00B68100`, `00B685C0`; read-only dependency scope `00B67980`, `00B67800`, `00B67700`, `00B65FB0`, actual manager `00F8D434` | Independent discovery, not yet an implementation packet. Worker owns only a new override discovery doc/report; first establish actual `14h` LuaObject/provider/lifetime contracts and mutable fallback bytes. Coordinate leases before claiming the shared Lua addresses. |
| `native_renderer_texture_loader_closure_discovery` | `00B319B0`, `00B30B40`, `00B31C20`, `00B31BD0`, `00B2C2D0`; read-only VFS boundary `00BDF4C0` | Independent discovery, not yet an implementation packet. Worker owns only a new loader-closure doc/report. Review existing alias/record and actual owner providers, then split the full manager and VFS/decoder work by explicit address ownership. |
| `native_shader_owner_texture_reload2` | `00B19000`, `00B24DD0` | Blocked until override and full renderer loader providers close. Reserve new reload header/source/doc/report when ready; do not add service callbacks or populate synthetic owner records to make the outer loop compile. |

For the ready pair, compile MSVC Win32 with the existing build and inspect the
small complete generated bodies. Reuse the current native owner fixtures when
available. If a new differential case is justified, one focused case should
prove terminal destruction before slot clearing and current-count reread,
plus the parent old-cursor/current-end ordering; no broad suite is requested.
That later validation is not performed or claimed by this discovery.

## Complete primary body hashes

End addresses are exclusive. These saved Ghidra bytes matched the configured
installed executable byte-for-byte on 2026-09-11; whole-image identity is not
implied.

| Start | End | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `00B188A0` | `00B188F4` | 84 | `dbcc3306bdbd177feeec8cb6075ededabf0018b27d3b8f3860095105b7c3a565` |
| `00B19000` | `00B191CE` | 462 | `efac483a85ca420e0c670bd25547e6678b8ce87779d1490f34ab2211f4899c56` |
| `00B24DD0` | `00B24E11` | 65 | `546c5e92b87cea6a70b1ecf786b9a6f721a51ae8e4e043eef7ee854cb622152b` |
| `00B24E20` | `00B24E61` | 65 | `1ce8fa23f78500b6ea52082a257209562fc5596929488a1f3be84be3f7d3467d` |

# Native 2D texture owner: next concrete dependencies

Read-only discovery from main `040201c`. The next ready implementation packet is
the actual `0108DB38` texture pool, listed below. The complete `00D61948` texture
owner is not ready: its source-stream zero dispatch and renderer resource-container
removal must remain concrete. A COM-only destructor cannot stand in for it.

The audit contains 41 complete function extents, three caller fragments and ten
exception maps: 83 code/data spans match both the current Ghidra program and the
installed PE. Every live query verifies `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Only this document and its JSON report are changed;
there is no C++ implementation, Ghidra mutation, build or runtime test in this packet.

## Concrete owner and constructor contracts

The texture owner is 50h bytes in a 54h pool slot. Its actual intrusive count is
at +04h, independent of its COM texture count and of every cached surface count.
The trailing pool slab ID at +50h is outside the texture and must survive both
constructors, destruction and reset.

| Offset | Actual field |
| --- | --- |
| +00h, +04h | Profile `00D61948`, native intrusive count |
| +08h, +0Ch | Owned NativeString length and buffer |
| +10h | Owned IDirect3DTexture9 pointer |
| +14h | GetLevelCount result |
| +18h, +1Ch | Actual level-zero format, flags |
| +20h | Serial from actual shared DWORD `0108D6E8` |
| +24h | Initially zero; loaded-file caller stores low source size |
| +28h, +2Ch | Actual GetLevelDesc(0) width and height |
| +30h | Initially zero; later role is outside this owner packet |
| +34h, +38h | Saved requested dimensions, distinct from actual descriptor |
| +3Ch | Initially zero; loaded-file caller stores requested mip count |
| +40h, +44h, +48h | Actual cached-surface data, signed count and capacity |
| +4Ch | Retained native source-stream owner |
| Slot +50h | Current slab ID owned by the actual texture pool |

`00B3F7B0..00B3F92F` is the unnamed constructor: ECX owner, stack COM/saved
width/saved height/flags, EAX owner, RET 10h. It calls `00B33FC0`, installs the
texture profile, initializes its own fields and **adds one COM reference** through
the current +10h field. It reloads that field for GetLevelCount, but GetLevelDesc
uses the captured input COM pointer. It copies the actual descriptor and saved
dimensions separately. Two real pooled diagnostic strings contain the verified
16-character literal `handmade texture`; the stack diagnostic COM word is borrowed.

`00B3F930..00B3FA81` is the loaded/named constructor: ECX owner, stack native
name/COM/saved width/saved height/flags, EAX owner, RET 14h. Its named base chain
`00B34230 -> 00B34120` copies the real pooled owner name and consumes the shared
serial once. It **adopts the input COM reference**; its two AddRef/Release pairs
are balanced diagnostics on the captured COM pointer. The file-loader region
`00B2C5D4..00B2C648` confirms adoption of D3DX's successful output and does not drop
that COM reference after construction. A separate diagnostic record copies the
native input name before consulting actual resource support `00B3E730`.

Both constructors ignore descriptor HRESULT and store descriptor output. They
test their original stack flags for bit 10h when incrementing `0108DAF8` after
diagnostic cleanup. This differs from the surface constructor, which reloads its
owner flags after callbacks. Neither constructor itself retains +4Ch or registers
the texture in the renderer array. All bytes +00h..+4Fh are initialized on the
normal constructor path; only the pool-owned +50h..+53h remain outside those writes.

The unnamed factory calls `00B3F2B0`, constructs the texture, appends a **borrowed**
pointer to actual renderer +1B00h, and drops its temporary COM reference after a
balanced pair (`00B2A201..00B2A32B`). No intrusive retain accompanies that array
append. The loaded-file factory instead assigns the source through `00B23640`
at `00B2C624`; later `00B2C833` drops the local source reference. The named resource
container and the renderer's reset pointer array are different domains.

## Complete destruction and callback domains

`00B3F2E0..00B3F40E` is the complete 302-byte destructor, including the native
tail after the array free. In order, it:

1. Installs `00D61948`, decrements the actual +4Ch stream count and invokes its
   current virtual zero only at zero; clears +4Ch after that callback.
2. Captures current renderer `00F8D394` and its virtual +6Ch slot, then passes the
   texture's native name at +08h. It reloads the renderer publication separately
   for `00B27D40`, which removes the first borrowed texture pointer at +1B00h.
3. Captures current COM +10h for an AddRef/Release pair, calls actual resource
   support, then reloads +10h for final Release and clears it after the callback.
4. Iterates cached records with a signed index and live +44h bound. Every
   iteration reloads +40h, captures that record's surface pointer and the address
   of its pointer slot, decrements the actual surface count, dispatches current
   virtual zero at zero, then clears the **captured slot address** after callback.
5. Reloads current texture flags after all those callbacks and conditionally
   decrements `0108DAF8`; resizes the record array to zero, frees its storage, then
   calls `00B33F50` to return the current name and install the refcounted base.

The post-free tail at `00B3F3E9` cleans the stack, and `00B3F3F6` calls the base
destructor. The array data pointer and capacity remain stale after free. Native
retained cursors/slot addresses must not be replaced by a snapshot or an updated
container lookup after a callback.

`00B3F590..00B3F5B0` takes ECX owner and stack flags, returns the original owner
in EAX, and uses RET 4. It runs the complete destructor first, then returns the
slot through `00B3D8D0` on canonical pool `0108DB38` only for flag bit zero.
It does not ordinary-free the 50h texture owner, and it does not return its slot
if destruction throws.

The renderer notification is real work. Primary renderer profile `00D5F0A8`
slot +6Ch contains `00B32250`, a currently unrecognized Ghidra function with the
complete extent `[00B32250,00B32340)`. It observes the optional guard gate,
creates and lowercases a real temporary name, and passes the **original input
name pointer** to `00B31DC0` on renderer +1A74h. That container scans 2Ch records
and nested alias lists, calls a current resource virtual +0Ch for accounting,
copies a final record through `00B30510` when needed and destroys the old final
record through `00B2F990`. Those nested ownership operations are not reconstructed
by a borrowed pointer-vector erase. They remain an explicit full-owner blocker.

## Actual cached surface ownership and reset

Each cache record is eight bytes: mip DWORD followed by a retained surface-owner
pointer. `00B3D9B0..00B3DA18` reserves signed capacity, with minimum one, copies
raw records, frees the old array, then publishes data and capacity. It adds no
surface references. `00B3DA20..00B3DA70` zero-initializes new records and changes
the signed count; shrinking does not release owners. `00B3EC40..00B3EC57` resizes
to zero and frees the array. The caller must perform the actual owner releases.

`00B3FD80..00B3FE88` takes ECX texture and **two** stack words: captured mip and
an unused word, RET 8. A cache search captures its data and count. A hit increments
the stored surface's actual +04h count before testing that pointer for null; a
null stored surface is not a safe cache miss. A miss calls GetSurfaceLevel into
the original mip argument's stack word, obtains a real surface-pool slot, and
calls the now-complete native surface constructor with current texture flags and
a kind byte derived from flags bit 8. It drops the COM getter reference afterward.

When the reloaded flags have bit zero clear, it increments the new surface count
again, grows the actual record array when full, and appends the key/pointer. The
caller and cache therefore own separate references. With bit zero set, the caller
owns the creator reference and the surface is not cached. The texture never
substitutes a COM-only level wrapper for this actual surface owner.

`00B3DD30..00B3DD86` calls each current cached surface's virtual +3Ch, now concrete
`00B3D510`, then performs the captured texture COM pair and a reloaded final
Release. It retains the same surface wrappers, cache records and source stream.

`00B3DD90..00B3DEFA` recreates the COM texture with the stack-passed device and
current metadata, publishes/retains new COM before releasing old, and drops the
getter reference even for an identity match. It then gets each cached mip COM
surface, reloads the cache to select its existing wrapper, calls wrapper virtual
+14h (`00B3CC80`), and drops the getter reference. It does not allocate replacement
wrappers or modify their intrusive counts.

The raw HRESULT boundary is material: FD80's surface output overwrites the
incoming mip word; DD90's CreateTexture output overwrites the incoming device
word. DD90's subsequent GetSurfaceLevel output uses the first pushed ECX word,
initially the texture owner address, and later the preceding getter value. These
are not zero-initialized safe outputs. The native calls ignore HRESULT. The reset
pool switch also falls back to the incoming device word for an unsupported low
flag nibble. Existing `D3D9ResetTexture2D` intentionally supplies safer typed
boundaries and borrowed stable levels; it cannot substitute for exact native
storage/callback behavior in the complete owner.

## Retained source and backing

The loaded texture retains the same native stream wrapper, including its cursor.
`00B23640` receives destination slot in ECX and source slot in EDX, publishes and
retains the new object before decrementing the old one, and skips identity matches.
The ordinary successful count sequence is local wrapper 1, texture assignment 2,
then local release 1. Backing has a separate reference count.

The concrete source profile is `00D642C0`: 14h bytes with count +04h, retained
backing +08h, end +0Ch and cursor +10h. `00BEF6D0` allocates that wrapper and retains
the supplied backing; its zero route is `00BD30E0 -> 00BB8F90 -> 00BEF9C0` followed
by ordinary wrapper free for deleting flag bit zero. Its destructor releases the
current backing at zero, clears +08h after callback and preserves end/cursor.

Backing profile `00D15AD8` is 10h bytes: count +04h, allocated data +08h and stored
signed length +0Ch. `008D43C0` uses array allocation for a positive requested size,
or one byte with stored length zero for a nonpositive size; it increments actual
`0109DB98` and adds the original signed request to `0109DB9C`. `008D4440` and
complete scalar deleter `008D4470..008D44B2` free data, decrement the object counter,
subtract stored length, then perform base/optional storage deletion. Fields are
not cleared. Negative request accounting is consequently not the same as clamping
the original request to zero.

Retained recreation `00B3E190..00B3E1ED` obtains size through current stream virtual
+30h and then reloads texture +4Ch for `00BEF610` data, independently of cursor.
It passes the real bytes to D3DX and writes directly to texture +10h. Current
`MemoryStream`/`D3D9RetainedTexture2D` share backing through `shared_ptr` and guard
fully initialized data; their public headers explicitly omit the original
allocator, counters and intrusive ABI. Keep that useful typed path separate from
the required actual stream/backing ownership chain.

## Exception maps

The report records complete leaves, handlers and FuncInfo for ten maps. Texture
destruction has states 1 -> 0 -> -1: cache-array storage cleanup at `00CBEE98`,
then logical base at `00CBEE90`. It does not continue the surface-release loop,
release COM again or release the source again during unwind.

Unnamed construction has four states: diagnostic record, temporary string,
cache-array storage, logical base. Named construction has three: diagnostic
record, cache-array storage, logical base. Neither map releases acquired COM.
FD80's one-state map returns only the raw surface slot through `00B3DCC0` if the
surface constructor throws; it does not Release GetSurfaceLevel's COM output.
That state is cleared before normal getter Release. A later reserve failure is
outside that pool-slot unwind state and happens after the intended cache retain.
These are static exception-map findings, not injected native exception results.

## Dispatchable next packet

Packet ID: `native_d3d9_texture2d_pool`. Own exactly these eleven entries:

| Entry | End exclusive | Action |
| --- | --- | --- |
| `00B3EE80` | `00B3EF53` | Construct actual 38h pool |
| `00B3EF60` | `00B3F090` | Allocate raw slot; complete 304-byte body |
| `00B3D1E0` | `00B3D220` | Initialize AC4h slab |
| `00B3D8D0` | `00B3D935` | Return raw slot; complete 101-byte body |
| `00B3E510` | `00B3E5B0` | Trim empty slabs and rewrite all moved IDs |
| `00B3E430` | `00B3E4BA` | Free slabs/table, drain actual lock, unlink |
| `00B3D3B0` | `00B3D3BE` | Table unwind helper, including post-free POP |
| `00B3F2B0` | `00B3F2BA` | Canonical allocation thunk |
| `00B3DCD0` | `00B3DCDC` | ECX-slot return thunk |
| `00CD7B60` | `00CD7B76` | Static initializer and atexit registration |
| `00CE0CA0` | `00CE0CAA` | Static destructor |

Owned files: `include/bsp/d3d9_texture2d_pool.hpp`,
`src/d3d9_texture2d_pool.cpp`, `docs/D3D9_TEXTURE2D_POOL.md`, and
`reports/d3d9_texture2d_pool_audit.json`. Primary retains CMake/Ghidra/ledger edits.

Use the actual shared `AllocatorListDomain` over `00E188B4`, the existing real CRT
allocation seam, and actual Win32 critical-section storage, as in the completed
surface pool. There is no second registry/head. Canonical owner `0108DB38` is 38h
bytes, profile `00D61940`, virtual zero `00B3E510`. A slab contains 32 slots of
54h bytes, 32 WORD free indices at +A80h, WORD free count +AC0h and two unwritten
bytes, for AC4h bytes total. Every slot's +50h DWORD is the current slab index.
Initial pointer capacity is 32; growth publishes `2 * capacity + 2` before
allocation. Trim rewrites all 32 IDs in a moved slab, including occupied slots.
The constructor's three-state map `00DF7688`/FuncInfo `00DF76A0` unwinds table,
actual critical section and shared allocator base. Allocate has no lock-unwind
map; destruction drains a positive actual recursion counter before deleting it.

This packet requires no texture constructor, COM object, source stream, resource
container or cached-surface substitute. Use one focused native comparison for
this geometry, table growth, moved IDs, return/reuse and destruction, plus the
required strict Win32 build. The existing surface-pool fixture is a useful
template; the actual 304/101-byte function bodies must not be inferred from its
different geometry.

Independent later prerequisites are the three cache-array entries
`00B3D9B0/00B3DA20/00B3EC40`, the five logical-base entries
`00B33FC0/00B34120/00B34230/00B33F50/00B34010`, and actual backing/stream lifetime
with the nine entries listed in the report. Recheck leases before assigning them.
The full texture owner still requires actual renderer +6Ch/container behavior.

Endpoint notation only: older surface prose used `00B3F7AD` as its last included
byte. The existing surface JSON evidence correctly uses the exclusive endpoint
`00B3F7AE` for all three bytes of RET 0Ch at `00B3F7AB`. No existing native-byte
evidence is corrected or discarded by this discovery.

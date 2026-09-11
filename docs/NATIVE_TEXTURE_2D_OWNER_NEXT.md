# Native loaded 2D texture owner: implementation dependencies

Read-only discovery from `c77199e`, owning the analysis of `00B3F930`,
`00B3F2E0`, `00B3F590` and `00B32250`. The complete three-entry texture owner
is conditional on four concrete packets below. All four dependency packets are
ready independently. The notification packet is the recommended next assignment.
The prior [discovery](NATIVE_TEXTURE2D_OWNER_NEXT.md) predates the integrated
texture pool, native surface owner, string operations and resource-container
removal. Those implementations are available in this checkout.

The report records 76 code/data spans freshly read through target-guarded Ghidra
CLI queries and matched to the installed executable, including complete native
tails and exception records. The selected project is `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Four allocation/free entries are explicitly
five-byte boundary prefixes, not complete allocator reconstructions. No C++,
tests, exports, Ghidra annotations, ledgers or shared build metadata change here.

## The three owned texture entries

| Entry | End exclusive | Original ABI |
| --- | --- | --- |
| `00B3F930` | `00B3FA81` | ECX raw owner; stack native name, COM texture, saved width, saved height, flags; EAX owner; RET 14h |
| `00B3F2E0` | `00B3F40E` | ECX owner; RET; no stable semantic EAX result |
| `00B3F590` | `00B3F5B0` | ECX owner; stack flags; EAX original address; RET 4 |

The actual owner occupies 50h bytes of an actual `D3D9Texture2DPool` slot.
Slot +50h is the pool's slab ID and is preserved. Important owner fields are
intrusive count +04h, native name +08h/+0Ch, COM texture +10h, actual level count
+14h, actual format +18h, flags +1Ch, shared serial +20h, saved dimensions
+34h/+38h, cache data/count/capacity +40h/+44h/+48h, and retained stream +4Ch.
The cache contains eight-byte records: mip DWORD and actual surface-owner pointer.
Its counts are signed in the observed comparisons; address arithmetic is Win32
DWORD arithmetic. Neither the reset array nor the cache is a host owner vector.

`00B3F930` calls the actual named base `00B34230 -> 00B34120`. The base copies
the native name, adopts COM without AddRef, stores flags, then captures and
increments actual shared serial `0108D6E8`. The outer owner installs `00D61948`,
zeros +24h/+3Ch and all three cache words, and clears +4Ch. It calls
GetLevelCount on current owner +10h. The two balanced AddRef/Release pairs and
GetLevelDesc(0) use captured input COM, rereading that COM object's table for
each call. Descriptor HRESULT is ignored and its stack output is not cleared.
Actual width/height/format and saved input dimensions are different fields.

The final diagnostic record contains borrowed input COM plus a separate real
native string copy. Normal string release uses the pointer captured after its
resize and the current temporary length; unwind uses the current record fields.
Actual resource support `00B3E730` runs before that release. Tracking tests the
original stack flags bit 10h after diagnostic cleanup. No constructor registers
the owner in the renderer or retains its source. A safe descriptor initialization,
COM-only wrapper, copied name, or new support singleton would change this contract.

The complete destructor, including the tail hidden by the cached `_free`
no-return analysis, performs these operations in order:

1. Install `00D61948`; capture nonnull +4Ch; InterlockedDecrement its actual +04h;
   at zero invoke the captured stream's current table slot zero; clear owner +4Ch
   only after the decrement/terminal call returns. End and cursor belong to the
   stream and are not reset by this texture owner.
2. Capture current renderer `00F8D394`, capture its table and the address of slot
   +6Ch, call full `00B33E40` to obtain owner +08h, then load that captured slot's
   current entry and call it on the captured renderer. The proven primary table
   `00D5F0A8` has `00B32250` there. Reload renderer publication separately for
   full `00B27D40 -> 00B25580`, removing the first borrowed pointer at +1B00h.
3. Capture owner +10h for a balanced COM pair; call actual resource support;
   reload owner +10h for its final Release and clear only after that call.
4. Starting at signed index zero, reload the cache data each iteration and use
   the current signed count at the loop bound. Capture both surface pointer and
   its slot address. Decrement actual surface +04h, dispatch current zero at
   zero, and clear the captured slot after return, even if a callback changed
   owner +40h. A null stored pointer skips decrement and clear.
5. Read current flags and possibly decrement actual `0108DAF8`. Disarm the cache
   unwind state before `00B3DA20(0)`, reload data and free it through `00BF6989`.
   Disarm the base state before `00B33F50`. Data/capacity and native name fields
   remain stale after their frees; the final profile becomes `00CEB130`.

`00B3F590` calls the complete destructor, then tests the low byte of the original
flags. Bit zero alone returns the slot through `00B3D8D0` on canonical pool
`0108DB38`; the result remains the original address. No slot return occurs after
a throwing destructor. The actual pool API is already available.

## Ready packet 1: renderer texture-name notification

Own exactly `00B32250..00B32340` and four new files:
`include/bsp/native_renderer_texture_name_notification.hpp`,
`src/native_renderer_texture_name_notification.cpp`,
`docs/NATIVE_RENDERER_TEXTURE_NAME_NOTIFICATION.md`, and
`reports/native_renderer_texture_name_notification_audit.json`.
Suggested ID: `native_renderer_texture_name_notification`. Native ABI is ECX
actual renderer, stack actual eight-byte name header, RET 4, no semantic result.
There is still no Ghidra function definition here; preserve that old metadata
and let the integrator create/annotate the complete function after implementation.

The receiver is captured in EBP. When entry-time mode `0108D6DC` is nonzero,
the function captures the **current global** renderer separately into its guard,
calls `00B33AD0`, and saves only AL. It arms guard unwind only after enter returns.
It then initializes a real temporary native name, resizes/copies from the input,
and captures the temporary data in EDI after resize. String unwind is armed only
after that copy completes. It lowercases the temporary through full `004BCC00`,
but calls full `00B31DC0` with the **original input name**, on receiver +1A74h.
The temporary's allocation, lowercase stores and cleanup remain observable.

Before normal cleanup it disarms string unwind. Normal release uses captured EDI
and the current temporary length + 1. Exception cleanup uses `0041DD20`, therefore
reads the current temporary pointer and length instead. After normal name release,
the function tests current mode, disarms guard unwind, and, if enabled, invokes
`00B33B00` with the saved renderer and saved stack word. It never recaptures the
global renderer for that leave. A skipped entry leaves the guard uninitialized;
use the existing admitted guard domain, without inventing an initialized fallback.

Dependencies already available: `NativeRendererSynchronizationGlobals`, actual
guard enter/leave/unwind, actual native string resize/lowercase/destroy, shared
`SizedStoragePool`, `SingletonLifetimeCallbacks`, and complete actual
`remove_native_render_resource_by_alias_00b31dc0`. Supply its borrowed immutable
`NativeRenderResourceAccountingTables`; match-time profile requirements remain
`D61948/D61870/D618B0` with verified +0Ch entries. These are resource accounting
requirements, separate from the receiver's captured table slot +6Ch.

## Ready packet 2: retained memory owners

Suggested ID: `native_retained_memory_owners`; four new files with stem
`native_retained_memory_owners` under `include/bsp`, `src`, `docs` (uppercase)
and `reports` (`_audit.json`). Own exactly these seven entries:

| Entry | End exclusive | Action |
| --- | --- | --- |
| `008D43C0` | `008D4437` | Construct actual 10h backing; ECX allocation, stack signed request, EAX owner, RET 4 |
| `008D4440` | `008D446D` | Backing destruction, including complete post-free counter/base tail |
| `008D4470` | `008D44B2` | Inlined backing destruction and optional ordinary free; EAX original address, RET 4 |
| `00BEF6D0` | `00BEF74C` | ECX actual backing; allocate/return new actual 14h wrapper; RET |
| `00BEF9C0` | `00BEFA36` | Actual wrapper destruction; ECX owner, RET |
| `00BB8F90` | `00BB8FAE` | Complete wrapper destruction and optional ordinary free; EAX original address, RET 4 |
| `00B23640` | `00B2367B` | ECX destination slot, EDX source slot; EAX destination; RET |

The loaded-file caller at `00B2C39F` invokes full `00BEF750`, captures its new
wrapper, and later assigns it to texture +4Ch at `00B2C624`. The conversion has
two nonnull routes, both ending in `00BEF6D0`: share the input memory backing,
or construct backing through `008D43C0`. The latter temporarily owns backing
count 1, wrapper creation increments to 2, and conversion drops it to 1.
Conversion itself has arbitrary source type-query/seek/read dispatch and is
evidence only, not included in this seven-entry implementation packet.

`00BEF6D0` ordinary-allocates 14h, installs `00D642C0` and count 1, and zeros
backing/end/cursor. It still reads and conditionally releases the current backing
slot before assignment. It publishes captured input backing, increments its
actual count, then reads captured backing +08h data and +0Ch length to set cursor
and wrapped end. There is no allocation-null safety branch that returns safely:
the null path later dereferences the null wrapper. No EH cleanup is registered.

Current stream zero profile must map `D642C0[0]=BD30E0`, current deleting slot
`[4]=BB8F90`; the full route destroys via `BEF9C0` before ordinary wrapper free.
The destructor captures backing, decrements actual count, calls current zero only
at zero, and clears wrapper +08h after return. It installs stream base `D5C104`
then refcounted base `CEB130`; end/cursor and count are preserved.

For the admitted backing domain, `D15AD8[0]=BD30E0` and current `[4]=8D4470`.
Constructor size is signed: positive request allocates that many array bytes and
stores it; nonpositive request allocates one byte and stores length zero. After
allocation it publishes data, increments actual DWORD `0109DB98`, then adds the
**original signed request** to `0109DB9C`, modulo 2^32. Destruction captures data
before installing `D15AD8`, ordinary-frees that data, decrements the current object
counter, reads current stored length after free, subtracts it, and installs base.
Neither data nor length is cleared. Its scalar deleting wrapper contains that
same body inline; it does not call `008D4440`.

`00B23640` captures source once and old destination once. Identity skips all work.
Otherwise publish new, retain new if nonnull, then decrement captured old and
dispatch its current zero at zero. There is no rollback or subsequent clear.
Do not provide generic terminal callbacks or assume any arbitrary caller-supplied
backing is `D15AD8`: borrow current actual tables and state supported profile
preconditions at each zero dispatch. Shared memory conversion does not itself
validate the supplied backing's profile. Existing `MemoryStream` shared_ptr state
does not satisfy these storage, counter, allocator or terminal contracts.

## Ready packets 3 and 4: base/name lifetime and cache storage

`native_logical_texture_named_base` owns `00B34120..00B341D0`,
`00B34230..00B34255`, `00B33F50..00B33FB7`, `00B34010..00B34015`,
`00B33E40..00B33E44` and diagnostic cleanup `00B3F4C0..00B3F4DE`.
Use four new files with that stem, uppercase documentation, and `_audit.json`.
These compose the existing actual string storage/pool and full seven-byte
`BD30F0` base action. No COM release belongs in the base destructor or diagnostic
cleanup. The base destructor returns the captured current name buffer without
clearing its header. Shared serial is consumed after successful name copying;
the wrapper changes the profile to `D5F228` without another serial increment.
The diagnostic helper reads its string at record +04h/+08h and ignores borrowed
COM at +00h. The arbitrary source and receiver header alias effects are native.

`native_texture_surface_cache_storage` owns `00B3D9B0..00B3DA18`,
`00B3DA20..00B3DA70` and `00B3EC40..00B3EC57`.
Use four new files with that stem, uppercase documentation, and `_audit.json`.
Reserve clamps signed requested capacity to at least one; it allocates wrapped
capacity * 8 bytes only if current signed capacity is smaller. Copy uses individual
DWORD stores, reloading data and count, with a null destination-slot skip. It
frees current old data before publishing the captured new data and capacity.
Resize reloads count after reserve, zeroes each new record through current data,
and shrinks using repeated current-count decrements before the final assignment.
Destroy resizes to zero and frees current data; data/capacity remain stale. None
of these helpers retains or releases a surface, and none registers EH cleanup.

Full getter `00B3FD80..00B3FE88` separately proves cached owner production through
actual surface pool and `00B3F630`. Cache hit retains the stored pointer before
checking null. A miss ignores GetSurfaceLevel HRESULT; the output overwrites the
incoming mip stack word. It constructs `D619A0` with current flags and low kind
byte `(flags >> 8) & 1`, drops getter COM, and conditionally adds a separate cache
reference. Cached surface zero dispatch is therefore the supported current
`D619A0 -> BD30E0 -> B3F5B0 -> B3F4E0 -> actual surface pool` chain. The surface
owner API is integrated. Getter reconstruction is a later single-entry packet
after cache-storage integration; it is not needed to implement the three owner
entries when their actual cache producer domain is supplied.

## Exception state and implementation acceptance

The audit decodes eight complete FuncInfo/unwind-map sets and their leaves.
No native exception execution occurred in this discovery.

| Owner | Unwind actions, highest state first |
| --- | --- |
| `B3F930` | 2: diagnostic `B3F4C0`; 1: cache `B3EC40`; 0: base via `B34010` |
| `B3F2E0` | 1: cache `B3EC40`; 0: base via `B34010` |
| `B32250` | 1: current temporary name `41DD20`; 0: saved guard `B21110` |
| `B34120` | 1: owner name `41DD20`; 0: refcounted base via `A81880` |
| `B33F50` | 0: refcounted base via `A81880` |
| `8D43C0` | 0: refcounted base |
| `BEF9C0` | 0: stream/refcounted base via `BB86E0` |
| `B3FD80` | 0: raw surface-slot return via `B3DCC0` |

Constructor state 1 is armed before GetLevelCount; diagnostic state 2 is armed
only after the diagnostic copy. A copy failure does not gain a later string
cleanup. Texture destruction arms state 1 before releasing source; it disarms
cache cleanup before normal resize/free and base cleanup before normal base.
No map retries source/COM/surface release or returns a texture pool slot. Native
FH3 behavior, including a second exception during unwind, is a separate fixture
boundary and must not be replaced by an assumed cleanup policy.

After the four packets are integrated, assign the three owned texture entries
with four new `native_texture_2d_owner` files and compose actual context storage:
renderer publication/synchronization, current verified tables, retained-memory
counters/owners, existing `NativeSurfaceOwnerStorage` and context, canonical
`D3D9Texture2DPool`, actual resource support/lifetime, shared string pool, shared
serial and tracking counter. Preserve all 54h slot preimages outside native writes.
Freeze those domains before one focused original-caller fixture. Normal and
unwind paths should execute complete original owner/dependency instructions, with
allocation/Win32/COM boundaries observed explicitly; unsupported profiles should
remain outside the stated domain. Build and fixture evidence do not establish
binary ABI compatibility, GPU behavior or game validation.

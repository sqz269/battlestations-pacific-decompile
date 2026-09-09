# Texture loader file and memory stream handoff

Read-only analysis on 2026-09-09 of `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Every Ghidra batch verified that target. Exports
and additional Capstone disassembly are in ignored
`exports/bsp/owner_textures/vfs/`. No functions were created or renamed by this
batch. Several short virtual methods are not defined as Ghidra functions yet;
their assembly below comes from verified saved-program bytes.

This resolves the concrete **physical-directory** implementation reachable from
the game's file manager. It does not establish which mounted provider wins for
every installed asset or implement archives, full mounting, or native pooling.

## Dispatch from texture loading

`00b2c2d0` copies the requested native string and calls global `0109ceec`, virtual
`+4`, with `(name, 2)`. It calls stream virtual `+18h` to check validity. On an
invalid result it invokes virtual `+4` with flag 1. On success it calls
`00bef750`, releases the original stream through InterlockedDecrement/virtual
slot zero, then uses the converted stream's virtual `+30h` size and `00bef610`
byte pointer for D3DX. It releases the converted stream after processing.

The normal startup path at `0073d637` constructs the file manager using
`00beda60`. Its chain is `00beda60 -> 00be1dc0 -> 00bda6f0`; the last publishes
the instance in `0109ceec`. `00beda60` installs vtable `00d68d04`, whose first
seven DWORDs are:

| Slot | Function |
| --- | --- |
| `+00h` | `00bedac0` |
| `+04h` | `00bdf310` open resource |
| `+08h` | `00bdd440` |
| `+0Ch` | `00bdd520` |
| `+10h` | `00bdc8b0` |
| `+14h` | `00bd91f0` |
| `+18h` | `00bdb040` select provider factory |

The base table `00d685b4` has the same six non-destructor targets. Do not use the
earlier constructor's `00d683e4` as a seven-entry file manager table: adjacent
unrelated data follows that base table.

## Name resolution before opening

`00bdf310` has ECX manager, stack native-string pointer and flags, EAX stream,
`RET 8`. It copies the name, calls the already named resource-path normalizer,
then alias replacement `00bdca80`, then mount traversal `00bdd0a0`.

Alias replacement scans 10h-byte `{from-string, to-string}` records at manager
`+94h`, count `+98h`. It requires equal lengths and case-insensitive C-string
equality, copies the first matching replacement, and returns. It does not
recursively resolve another alias.

Mount traversal walks the native tree rooted at manager `+40h`. A mount's empty
virtual prefix matches any name; a nonempty prefix must be shorter than the
name, compare equal to its initial substring, and be followed by `/`. The
callback receives the remaining path after that slash (or the whole name for
an empty prefix). Traversal stops at the first callback reporting success.
Comparator, insertion order, complete mount construction, and tie priority were
not recovered, so this is not a claim that disk files override archive files.

The open callback table is `00d6838c`; `+4 = 00bda690`, `+8 = 00bd9040`.
`00bda690` has ECX callback, stack mount record and suffix string, `RET 8`.
It calls the provider pointer at mount record `+8`, virtual `+8`, passing suffix
and the original open flags. It writes the returned pointer to callback `+4`;
on success it copies the mount's `+0Ch` byte to callback `+0Ch`.
`00bd9040` simply returns whether callback `+4` is non-null. Conditional logging
in the callback depends on manager `+20h`, mount `+0Ch`, and flags bit zero.

Flags bit zero has substantial behavior in `00bdf310`: on a failed open, a set
bit returns null quietly, while a clear bit logs the original name and invokes
manager callback `+90h`. With texture flags 2, bit zero is clear. The function
still returns a null pointer if that callback returns; texture loading itself
does not guard null before the virtual validity call. A host adapter should
explicitly handle failure without claiming identical native error behavior.
Successful clear-bit opens also call tracking helper `00bde9c0`, increment
manager `+28h`, and add the stream's virtual `+2Ch` result to `+2Ch` statistics.
Tracking/error callbacks are dependencies, not optional native behavior proven
irrelevant by this batch.

## Concrete physical-directory provider

`00beda60` registers factory singleton `00bed990` through `00be0660`. Its primary
table is `00d68cfc`, with factory method `+4 = 00bf4df0`. The manager's
`00bdb040` walks its factory list and returns the first non-null factory result.

`00bf4df0` accepts a nonempty system path ending in `\\`. It allocates from a
pool and constructs `00bf4d30`, with a boolean selected from its second string
argument. The constructor's diagnostic string identifies
`cPhysicalDirectoryX86`; its table is `00d69168`. Archive factories registered
elsewhere in startup were not traversed.

Physical provider virtual `+8 = 00bf4ba0` has ECX provider, stack suffix-string
pointer and flags, EAX stream or null, `RET 8`. It obtains a file stream from
pool singleton `00bf42a0` via `00bf3770`, builds the physical path using provider
virtual `+1Ch = 00bf3970`, then calls thunk `00bf5590 -> 00bf52a0` with path and
flags. If stream virtual `+18h` is false, it releases the stream and returns
null. Pool allocation either reinitializes a stored pointer or allocates 20h
bytes and calls constructor `00bf50d0`.

`00bf3970` has ECX provider, stack output string and suffix string, EAX output,
`RET 8`. It calls string-combination helper `004261a0` with the provider's native
root string at `+8`, then changes `/` to `\\` in the result starting at the root
string's length. Thus it preserves the root prefix bytes; it does not blindly
rewrite every slash in the complete path. Complete combination alias behavior
and root preparation remain separate dependencies.

## Physical file stream

`00bf50d0` initializes vtable `00d691b0`, reference count `+4 = 1`, handle
`+8 = INVALID_HANDLE_VALUE`, 64-bit position `+10h = 0`, and 64-bit size
`+18h = 0`. Offset `+0Ch` is not initialized by this constructor. The relevant
virtual methods are:

| Slot | Routine | Native interface and behavior |
| --- | --- | --- |
| `+00h` | `00bf55a0` | Final release target; pool recycling not recovered here |
| `+04h` | `00bf5090` | Deleting/destruction route, not reconstructed here |
| `+0Ch` | `00bf4ff0` | Type query over IDs at `0109dc30..0109dc38` |
| `+18h` | `00bf5020` | ECX stream, plain RET, AL = handle != -1 |
| `+1Ch` | `00bf4f20` | ECX stream, stack signed 64-bit distance and DWORD origin, RET 0Ch; SetFilePointerEx writes cached position |
| `+24h` | `00bf5030` | ECX stream, stack destination, DWORD requested bytes, optional actual-count pointer, RET 0Ch |
| `+30h` | `00bf4f90` | ECX stream, plain RET, EDX:EAX = cached 64-bit size |

`00bf52a0` has ECX stream, stack path-string pointer and flags, `RET 8`. For the
texture flag value 2 it calls `CreateFileA` with GENERIC_READ (`80000000h`),
FILE_SHARE_READ (1), OPEN_EXISTING (3), null security/template and attributes 0.
On success it stores `GetFileSizeEx` at `+18h`; finally it zeros position
`+10h/+14h`. It logs failed opens. Additional creation/write modes and directory
creation exist in the same body but are not needed for this read-only route.

`00bf5030` calls synchronous `ReadFile` with a local actual count initially
zero. On failure it passes manager `+18h` to `00bd9e30` (error handling unresolved).
It advances the cached 64-bit position by the actual count, writes that count
through the optional third pointer if non-null, and returns actual count in
EAX. It does not loop to fill the requested buffer. Handle lifetime and pool
recycling need reconstruction before treating these methods as a full stream.

## Conversion to a memory stream

`00bef750` has ECX input stream, EAX a newly owned memory-stream wrapper, plain
RET. It returns zero for a null input. It first queries input virtual `+0Ch`
with type ID stored at `0109dba0`.

If the type query succeeds, it calls `00bef6d0` with the existing backing object
at input `+8`. This creates a **new cursor starting at the backing beginning**;
it does not preserve the input cursor and does not copy the bytes.

Otherwise it calls input seek virtual `+1Ch` with three zero DWORDs, queries
64-bit size via `+30h`, and allocates a backing object using **only EAX, the low
32 bits of size**. It calls input virtual `+24h` exactly once with destination,
that low DWORD count, and null count-output pointer. It neither checks a short
read nor verifies the ignored high size DWORD. Finally it wraps the backing
and releases its temporary reference. Do not silently reinterpret this as
robust multi-read loading of arbitrary-size assets.

Backing constructor `008d43c0` has ECX 10h-byte destination, stack signed length,
EAX destination, RET 4. It sets table `00d15ad8`, reference count 1, data pointer
at `+8`, and length at `+0Ch`. Positive lengths allocate that many bytes; values
below 1 set stored length zero and allocate one byte. Contents are not cleared.
Global counters `0109db98/0109db9c` are adjusted, including the original requested
length. Allocator/destructor and exceptional sizes remain unported.

`00bef6d0` has ECX backing, EAX wrapper, plain RET. It allocates 14h bytes with
table `00d642c0`, retains the backing at wrapper `+8`, sets cursor `+10h` to
backing data and end `+0Ch` to data plus length. Allocation failure is not safely
handled before subsequent dereferences in the native body.

For that memory stream, `+18h = 00bef4c0` always returns true;
`+30h = 00bef600` returns `end - backing.data` sign-extended into EDX:EAX.
`00bef610` returns `wrapper.backing.data`, independent of cursor. These exact
semantics explain the D3DX pointer/size pair without treating an arbitrary
stream as contiguous storage. The backing survives the original stream release
because the new wrapper retains it. Full memory-stream destruction/read/seek
semantics remain separate dependencies.

## Actionable next implementation

For the smallest faithful installed-file route, reconstruct the physical stream
constructor/open/valid/seek/read/size methods as a unit with explicit ownership,
then the memory-wrapper/backing lifetime and `00bef750` conversion. The current
diagnostic DDS reader can supply assets, but it is not evidence that native VFS
mounts, aliases, archive selection, tracking callbacks, and pool reuse work.

The short `00bf5020`, `00bf4f90`, `00bef610`, and `00bef600` routines are complete
bounded candidates once their object models exist. A larger justified next unit
is the flag-2 physical-file open/read path with one installed asset comparison;
do not invent archive stubs to claim a complete file manager. Preserve the native
32-bit conversion limit and single-read evidence as explicit boundaries.

## Body verification

The following full body ranges were independently compared with installed PE
bytes and saved Ghidra bytes. End addresses are exclusive. All comparisons
matched; this is static evidence, not runtime or archive validation.

| Start | End | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `00bdf310` | `00bdf4b5` | 421 | `e354c379eeae3b582f71333f00c627d34b1d19e8c2c835c861922bda2cd7e748` |
| `00bda690` | `00bda6f0` | 96 | `f61f3853c5e54193f80f903c6bfde68962fb3e01bd3dd58ff805a6b3553a48df` |
| `00bf3970` | `00bf39b1` | 65 | `83322b019ce0e47cdec0a1fbce1d04a3d9593917cb2064655650fb18ef603a7b` |
| `00bf4ba0` | `00bf4c67` | 199 | `416d4e8a211ca26833ffe57980ceb468aa3f3f10cfebc98977b34f3133ff45a9` |
| `00bf50d0` | `00bf50fb` | 43 | `9f0a1e587ebcc5e8f5a5447de34adc30be4a0d7bc3ffdcbb605769ed4eb28716` |
| `00bf52a0` | `00bf54ed` | 589 | `c861932ca72ee94f3f4895b4d540754537c94c9716e97ef0ec130d6f5093687c` |
| `00bf5020` | `00bf502a` | 10 | `0a032cb89a32866d6ebcd61d725e5a5cf50356630344759f0d564fd74cd57636` |
| `00bf5030` | `00bf5084` | 84 | `d0b1d2fd5e3863f9e82af8f94b038bb84c2fc189c683fd5f3a31fd92397ad48d` |
| `00bf4f20` | `00bf4f40` | 32 | `956d15aa55fd38f9f349ef3c873b984bd3fae3e02c78e78e949743f929dbb27c` |
| `00bf4f90` | `00bf4f97` | 7 | `72499fbb5b630d058b85fc2bb7b6d50c4febbb4fc23621cf4bf05b073999a9dd` |
| `00bef750` | `00bef840` | 240 | `696afedd7e769bad800086544926b2ea51da1cf17ed5606fc16497b8a90b80e5` |
| `00bef6d0` | `00bef74c` | 124 | `f62bfa31edee50d201af2c0e89c98e96db82cd967e805f8ab9841dd9bfe53595` |
| `00bef610` | `00bef617` | 7 | `92c5e2972d2442d6d750d027a5da0e7ee862ca52fef58c2af20331258d8191ef` |
| `00bef600` | `00bef60b` | 11 | `3868b670bd048da71266ed078741a94b1706b40c90a35cdc4a51c6ecad47cd36` |
| `008d43c0` | `008d4437` | 119 | `76e462d92e841f92697d60c23f8a0dcf2a9f9a098bd5fd5fb9901a19bc69115a` |

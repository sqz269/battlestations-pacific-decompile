# Native memory stream and backing ownership

Read-only recovery from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Project and program were checked before every
analysis batch. Evidence is in ignored `exports/bsp/owner_textures/vfs/`.
This handoff identifies the next physical-file-to-texture dependency; no memory
stream implementation, Ghidra mutation or build was performed in this batch.

## Object model and virtual methods

The wrapper allocated by `00bef6d0` is 14h bytes:

| Offset | Contents |
| --- | --- |
| `+00h` | Vtable `00d642c0` |
| `+04h` | Interlocked reference count initialized to 1 |
| `+08h` | Retained backing-object pointer |
| `+0Ch` | End pointer, backing data plus backing length |
| `+10h` | Independent cursor pointer |

Backing constructor `008d43c0` initializes a 10h-byte object with table
`00d15ad8`, reference count `+4 = 1`, data pointer `+8`, and signed length
`+0Ch`. For a positive requested signed length it allocates that many bytes and
stores the length. For any value below 1 it stores length zero and allocates one
byte. Contents are not cleared. Global `0109db98` increments by one, and
`0109db9c` increments by the original requested length, including its raw
negative value. This is not a growable byte vector.

The established initial memory-stream virtual slots are:

| Slot | Routine | Behavior |
| --- | --- | --- |
| `+00h` | `00bd30e0` | If non-null, invoke virtual `+4` with destruction flag 1 |
| `+04h` | `00bb8f90` | Wrapper deleting destructor |
| `+08h` | `00bef4a0` | EAX = type ID stored at `0109dba0` |
| `+0Ch` | `00bb8f60` | AL = supplied ID equals one of three DWORDs at `0109dba0..0109dba8`; RET 4 |
| `+10h` | `006f9d20` | EAX = 0; RET 8; interface purpose not established |
| `+14h` | `00bef4b0` | EAX = global `0109dbac`; interface purpose not established |
| `+18h` | `00bef4c0` | AL = 1, no backing/cursor check |
| `+1Ch` | `00bef540` | Seek using low 32 bits only, no bounds checks |
| `+20h` | `00bef580` | Sign-extended 32-bit cursor offset in EDX:EAX |
| `+24h` | `00bef590` | Read using unsigned remaining-length clamp |
| `+28h` | `00bef5f0` | Plain RET 0Ch, no effects; likely write interface, not proven from caller |
| `+2Ch` | `00be41a0` | Call virtual `+30h`; optional stack pointer receives EDX; EAX remains low size, RET 4 |
| `+30h` | `00bef600` | Sign-extended 32-bit end offset in EDX:EAX |

All listed routines use ECX as their object receiver except `006f9d20`, which
does not inspect the receiver. Methods following these slots were not needed
for the texture load and are not claimed recovered.

## Seek and bounds

`00bef540`: ECX wrapper, stack low offset DWORD, high offset DWORD, origin DWORD,
RET 0Ch. The high DWORD is ignored. Origin zero uses backing data, origin one
uses the current cursor, and **every other origin** uses end. The low offset is
added with 32-bit wrapping and stored as the new cursor. There is no signed
range check, no [begin,end] clamp, and no successful BOOL result contract.
Negative offsets work by their two's-complement low bits; out-of-range seeks
can make subsequent reads invalid. Do not replace this with physical-stream
`SetFilePointerEx` semantics or silently clamp it in a claimed native port.

`00bef580`: ECX wrapper, EDX:EAX result, plain RET. It computes cursor minus
backing data in 32 bits, then CDQ sign-extends that result.
`00bef600` does the same for end minus backing data. These methods do not return
an unsigned 64-bit pointer difference or backing length directly.

`00bef610`: ECX wrapper, EAX byte pointer, plain RET. It returns
`wrapper.backing.data`, regardless of cursor. This is the pointer paired with
virtual size by the D3DX texture loader.

## Read behavior

`00bef590`: ECX wrapper; stack destination pointer, unsigned requested byte
count, optional actual-count pointer; RET 0Ch. It computes unsigned remaining
bytes as `(end-cursor) mod 2^32` and clamps requested count with unsigned
CMOVNC. For a final count of four it performs a single DWORD load/store; for
two it performs a WORD load/store; every other count, including zero, calls
`memcpy`. It then advances the cursor by the final count and writes that count
through the optional output pointer when non-null.

There is **no returned byte-count contract** here: EAX is overwritten with the
optional output pointer before the return. Physical stream `00bf5030` returning
actual bytes in EAX must not be generalized to this implementation.

If the cursor is beyond end, subtraction wraps and can authorize an invalid
read. If it is before the backing start, remaining length includes that invalid
prefix. A typed implementation needs explicit supported preconditions or an
explicit host failure boundary; the native routine is not bounds-safe merely
because it clamps requested size. Overlap is not memmove: preserve the special
two/four-byte cases and do not assume arbitrary overlapping buffers are safe.

## Construction and stream conversion

`00bef6d0`: ECX non-null backing, EAX newly owned wrapper, plain RET. Allocate
14h bytes; initialize count 1 and backing/end/cursor zero; release an existing
backing if present (the new object's field is zero); store supplied backing;
InterlockedIncrement(backing+4); set cursor to data and end to data+length.
It does not retain the input wrapper or preserve any input cursor. Native
allocation failure leads to dereferences of a null wrapper, not a recoverable
null-return path. No allocator substitution is claimed by this handoff.

`00bef750`: ECX source stream, EAX newly owned memory wrapper, plain RET:

1. Null source returns null.
2. Query source virtual `+0Ch` using the ID at `0109dba0`. On success, pass
   source `+8` to `00bef6d0`. This shares backing and resets the new cursor to
   its beginning. The source's own cursor remains unchanged.
3. Otherwise seek source to `(offset low=0, high=0, origin=0)`, query its 64-bit
   size, and allocate a backing using only EAX (the low 32 bits as signed size).
   EDX is saved on the stack but is not used to allocate or read.
4. Request exactly that original low-DWORD size from source virtual `+24h`,
   with backing data and a null actual-count pointer. Do not check the source's
   returned result, actual count, or short-read condition.
5. Create a wrapper through `00bef6d0`, then release the local backing reference.
   The newly returned wrapper holds the surviving reference.

For ordinary positive files below 2 GiB, step 4 requests the allocated length.
For low-DWORD values whose sign bit is set, the backing constructor allocates
one byte and stores zero length, while the read still receives the original
large unsigned count. This unsupported native path must not be exposed as safe
arbitrary-file loading. Short reads may leave uninitialized backing bytes;
the wrapper still exposes the full requested length. The physical file owner
must remain alive until conversion finishes; after conversion, the new backing
is independent of that file handle.

## Destruction and exact free ordering

Wrapper final reference invokes `00bd30e0`, which calls deleting destructor
virtual `+4` with flag 1. `00bb8f90` has ECX wrapper, stack destruction flags,
RET 4: call `00bef9c0`, then free wrapper storage if flags bit zero is set.

`00bef9c0` has ECX wrapper, no stack arguments, plain RET. It reinstalls
`00d642c0`, reads backing `+8`, decrements the backing reference count if
non-null, and invokes backing virtual slot zero only when the result is zero.
Only **after** any backing destructor returns does it clear wrapper `+8`. It
then sets stream-base table `00d5c104` and calls base destructor `00bd30f0`,
which sets table `00ceb130`. End/cursor fields are not cleared.

Backing table `00d15ad8` has slot zero `00bd30e0` and deleting destructor slot
`+4 = 008d4470`. Thus backing's last reference similarly invokes it with flag 1.
`008d4470` has ECX backing, stack destruction flags, EAX original storage,
RET 4. Its complete order is:

1. Set table `00d15ad8` and free the data pointer at `+8`.
2. Decrement global object counter `0109db98` and subtract stored backing
   length `+0Ch` from byte counter `0109db9c`.
3. Call base destructor `00bd30f0`.
4. If flags bit zero is set, free the backing-object storage; return its old
   address in EAX.

The saved Ghidra function export **incorrectly stops at 008d4482**, after the
first call to free, due to the callee's no-return analysis. Raw disassembly and
installed-byte comparison establish the real end at `008d44b2`, including the
second conditional free and RET 4. The full body hash below uses the corrected
66-byte range. No function-body repair or annotation change was performed.
Allocator/free instrumentation, global counter thread safety, and exceptional
allocation behavior remain dependencies; raw counters are not a host lifetime
implementation.

## Next implementation boundary

A real next unit can model retained backing plus independent cursors, implement
the small read/seek/size/data methods, and convert the already reconstructed
physical reader into this shared backing using the native positive-size path.
The existing installed DDS comparison can verify that resulting bytes and
length reach D3DX unchanged, and a second wrapper can verify sharing survives
the first wrapper's release. This does not require a new test framework.
Keep native invalid-seek/negative-size behavior outside explicitly supported
host preconditions; do not claim ABI compatibility, robust arbitrary-size
loading, archive decompression, or zero-filled short reads.

## Verified complete body ranges

Saved Ghidra bytes and installed PE bytes matched for every body below. The
byte lengths include final RET operands; ends are exclusive. Detailed values
are also in ignored `memory_stream_evidence.json` beside the exports.

| Start | End | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `00bef540` | `00bef57d` | 61 | `daf89af22b34c985f1a54071d85eb03e08045470122e29fb766ecbd04657eac0` |
| `00bef580` | `00bef58b` | 11 | `1a708b39f19bc2f6f4e716cdb74bc8e5f82f2418f1862155773fbc46c6c8c688` |
| `00bef590` | `00bef5e6` | 86 | `e250c1e0947817d753a5132cdd34703080ede478fb5a1ddf6ec8cdfaf9361e11` |
| `00bef5f0` | `00bef5f3` | 3 | `dbd1a662565b07272069f88faadf4327acaf51b7d14f236c84c0a9679d07cef3` |
| `00bef600` | `00bef60b` | 11 | `3868b670bd048da71266ed078741a94b1706b40c90a35cdc4a51c6ecad47cd36` |
| `00bef610` | `00bef617` | 7 | `92c5e2972d2442d6d750d027a5da0e7ee862ca52fef58c2af20331258d8191ef` |
| `00bef6d0` | `00bef74c` | 124 | `f62bfa31edee50d201af2c0e89c98e96db82cd967e805f8ab9841dd9bfe53595` |
| `00bef750` | `00bef840` | 240 | `696afedd7e769bad800086544926b2ea51da1cf17ed5606fc16497b8a90b80e5` |
| `008d43c0` | `008d4437` | 119 | `76e462d92e841f92697d60c23f8a0dcf2a9f9a098bd5fd5fb9901a19bc69115a` |
| `008d4470` | `008d44b2` | 66 | `6f300372d3019149192f16df5af69c965b98ebb5efbe2dfd80868afdfc68b70a` |
| `00bb8f90` | `00bb8fae` | 30 | `a88682c86cbbdd6d0c0c2bdf811ab62d50d05d2504bfa098ea7615c160e23438` |
| `00bef9c0` | `00befa36` | 118 | `0e4fabcd08941fce4451eb93b3bbda3644238984e0d2a3b5048ca7bb0e7ff9ec` |

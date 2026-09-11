# XLive pipe capacity and framing

This packet reconstructs 52 complete native bodies as typed C++ in
`xlive_pipe_framing.cpp`. Descriptive names and PIPEIPC provenance are hypotheses.
These interfaces are not binary replacements. The report records every original
ABI, complete assembly, live/disk byte comparison and inclusive terminal address.
No Ghidra changes, endpoint operation or game validation occurred.

## Capacity and callbacks

The public `00A5DF96`/`00A5DFCE` wrappers are stdcall, three arguments, `RET 0Ch`.
They reject null/-1 owner before null output. Their thiscall helpers `00A5F204`
and `00A5F230` require a protocol object, then accept unsigned payload capacities
through `3B8h`, writing `payload + 48h`. Failure preserves the output. This argument
is a byte count; the older IPC interface's `channel` spelling is misleading.

The public encoder `00A5E055` and decoder `00A5E09E` are stdcall with five DWORD
slots (`RET 14h`). They validate owner, require mode zero, then reject a null
callback with nonnull context. They add no buffer or size-pointer validation.
The bodies `00A5F6C0`/`00A5F7C5` use ECX=owner and four stack arguments
(`RET 10h`), capture owner.protocol before callbacks, and return signed EAX.

Encoding rejects capacity below 72. Its stdcall callback receives frame+72, a
pointer to a local payload capacity, and context. A negative callback result exits
unchanged. Otherwise the local byte count is compared against the **current**
outer capacity minus 72 using unsigned subtraction, including wraparound.
The encoder never writes that outer capacity. A positive callback result is
discarded; successful framing returns zero. A null callback sets only the local
payload count to zero. Header generation follows the callback and copies 18
DWORDs forward. It does not imply payload encryption or a reduced frame length.

Decoding rejects fewer than 72 bytes. Its stdcall callback receives frame+72,
bytes-72, and context **before header validation or protocol mutation**. Negative
callback results exit; positive results are discarded. Callback replacement of
owner.protocol does not change the protocol object already captured by either body.

## Encoded values and state changes

The canonical protocol is A8h: provider+0, encoded40 seed+4, encoded72 pending+30h,
encoded40 key+7Ch. Each encoded object starts with its actual lock pointer. The
frame host borrows that storage and the shared immutable table view; it allocates
no shadow protocol or replacement OVERLAPPED.

`A5F25C` and `A5F2A4` invert the negative test of packed signed32/signed64 results.
Equality, ordering, numeric bigint meaning and cryptographic semantics are
unproven. Source names therefore describe the operation without assuming those
meanings. For the true key predicate, encode transforms pending with seed, then
the F8B9A0 global, and copies it to the frame. Decode applies those transformations
in the opposite order, compares against a locked temporary copied from the header,
and returns E_UNEXPECTED when the wide predicate is true.

For the false key predicate, encode transforms F8B858 into a temporary wide object,
combines F8B9A0, copies its 72 bytes, then invokes its deleting lock destructor
with flags1. The temporary's native bytes are unwritten before the first combine
reads them. The typed host requires an explicit defined 72-byte stack preimage;
zero-fill is not a recovered initialization. It obtains that preimage before
acquiring ownership of the temporary lock, an explicit host adaptation.

For the false decode predicate, the first 40 header bytes replace the seed, then
`A5F020` transforms it with F8BBA0. Native code sets last error to zero and calls
`CryptGenRandom(provider,8,local64)`. A false result exits only if GetLastError is
signed negative, returning that raw value. Zero/positive errors continue consuming
the local64 preimage. The adapter marks all bytes defined on success and preserves
existing definedness on failure. If any consumed byte remains unknown, the typed
body throws an explicit guard **after the earlier seed writes**, rather than
inventing random output. With defined output it updates pending through `A5EE78`,
then key through `A5EF36`. There is no rollback or added HRESULT normalization.

Value wrappers take the global acquisition section, enter all input/output locks
in native order, leave the acquisition section, perform their table transforms,
then leave object locks in the same order. Repeated aliases enter and leave
repeatedly; lock pointers are reloaded on each virtual dispatch. `A5EE78` takes
only its output lock. These native functions have GS epilogues, not C++ exception
unwind cleanup; required host/lock operations must not throw during a locked
transform. No invented unwind behavior was added.

## Tables and direct dependencies

The existing protocol packet supplies locks, `A5FE80`/`A5FBC3`, and one authenticated
table view covering D25D9C through exclusive D56EB0, 200980 bytes, SHA-256
`f3456122844489dc39ceee393fc4ecfaddf789449bddf89b434b2cd55fe5c77c`.
This packet adds `A5FA70` (seed bits and encoded bytes to 2-bit output) and
`A5FD15` (encoded pairs to packed bits), plus the exact constant/row wrappers.
Signed16 row indices, signed minimum lengths, x86 wrapping displacement, in-place
aliases and the first-iteration special case are preserved. `A5FD15` clears
`output_size/8 - 1` bytes before its loop, even for an empty signed minimum.
The generic interfaces require valid bounded native buffers; the framing callers
use lengths40/72 and displacement0. `A60551` processes only 40 bytes of a 72-byte
initialized temporary, leaving its final32 bytes intact.

Required shared globals are one acquisition section F8B778, wide objects F8B858
and F8B9A0, narrow objects F8BBA0 and F8BADC, and raw72 F8B8F8. The latter is read
directly, with no skipped lock pointer. Their recovered initialization belongs
to the separate globals packet. The Win32 frame system forwards real
SetLastError/GetLastError/CryptGenRandom and existing lock construction. The
`ReconstructedXLivePipeFramingHost` supplies the four framing overrides; real
transport open/close and I/O remain abstract for integrator composition.

## Validation and limits

All52 functions' complete assembly spans matched the saved live program and
installed PE; no missing starts or export flow gaps were found. Strict Win32
C++17 `/W4 /WX /fp:strict` build and both existing CTests passed, as did eight
native seed comparisons. One ignored synchronous fixture compares both new
generic loops against relocated native bytes, including displacement0/1,
differing lengths and in-place buffers. Only documented table operands and the
two native memset calls are relocated; no native pipe or SDK body is executed.

The same fixture uses authenticated original global constants and canonical key
initialization to exercise both framing branches. It checks callback ordering,
negative status propagation, current capacity reload, unchanged outer capacity,
captured protocol pointer, aliased lock order and temporary destruction. It also
checks raw negative RNG errors, the unknown-output guard with prior seed writes
retained, known preimages on nonnegative failure, and successful recorded output.
The OS frame host itself is build/link checked; real CryptGenRandom and endpoints
are not exercised. Table authentication uses the existing local SHA-256 API.
Actual stack preimages, shared global lifetime, caller buffer lifetime and live
transport integration remain explicit runtime contracts.

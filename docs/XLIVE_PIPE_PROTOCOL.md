# XLive pipe protocol lifecycle and initialization

`ReconstructedXLivePipeProtocolHost` implements the transport's two protocol
operations with the recovered allocation, locking, table transforms, CryptoAPI
calls, and teardown. Twenty-two functions are reconstructed in
`src/xlive_pipe_protocol.cpp`; `reports/xlive_pipe_protocol.json` records each
native ABI, final instruction and evidence boundary. Descriptive names remain
hypotheses. No protocol success, random output, global object, or allocator
preimage is fabricated.

## State and required bindings

The canonical protocol prefix is `XLivePipeProtocolNativeState`, size `A8h`.
Its members are provider `+00`, lock plus 40-byte encoded value `+04`, lock plus
72-byte encoded value `+30`, and lock plus 40-byte encoded value `+7C`.
`XLivePipeEncodedValue` and `XLivePipeEncodedWideValue` expose these shared
objects for framing. The native constructor `A5F336` allocates/stores **only**
the three lock pointers, in offset order `+04`, `+30`, `+7C`.

All value bytes retain the allocation preimage. The initialization transforms
read the destination's previous `+08..+2F` and `+80..+A7` bytes while replacing
them. The entire 72-byte pending value survives unchanged. The concrete Win32
system adapter requires `XLivePipeProtocolPreimageHost` to supply an explicit
defined `A8h` allocation image, copied into its real allocation before native
construction. This is a projection binding, not a native callback or a claim
that an observed allocator returns zero bytes. An unavailable preimage remains
an integration boundary. The provider must not manufacture padding/defaults.

`XLivePipeProtocolGlobals` borrows the single acquisition critical section at
`F8B778` and the initialized encoded values `F8B8CC` and `F8BADC`. It allocates
no substitutes. Successful transforms require valid source/destination locks
and payloads. Native null lock allocation is not converted to a success object
or a new HRESULT error path. The pending lock may be null and is conditionally
destroyed, exactly as the native member cleanup specifies.

Static initialization is separate, unreconstructed work in this packet:

| True entry | Native operation | Final instruction |
| --- | --- | --- |
| `CD6E16` | ECX=`F8B774`; `A5FA27`; register `CE099A` with `BF6FF5` | `CD6E2B RET`, length 1 |
| `CD6E62` | ECX=`F8B8CC`, data=`E12B58`; `A60015`; register `CE09C6` | `CD6E7C RET`, length 1 |
| `CD6E98` | ECX=`F8BADC`, data=`E12BA8`; `A5FFCF`; register `CE09E8` | `CD6EB2 RET`, length 1 |

The acquisition section is at `+4` inside the global lock object `F8B774`.
Raw BSS values are not valid substitutes for these initialized globals. The
table/view/allocator dependencies have concrete contracts so a later owner
binding can supply the actual shared state without changing protocol behavior.

## Lifecycle and ordering

`A5F416` receives ECX=pointer to the transport protocol slot. It requests an
`A8h` allocation, constructs the three locks, then calls `SetLastError(0)` and
`CryptAcquireContextW(&provider, null, null, PROV_RSA_FULL, F0000040h)`.
Acquire failure reads the last error and forcibly clears the provider output,
even if the failed API wrote a value. Acquire success calls `SetLastError(0)`
then `CryptGenRandom(provider, 4, &seed)`. A successful randomness call must
define all four output bytes. A failed call never consumes its output.

Either API failure destroys the context: release a nonzero provider, destroy
members, free allocation, set the local context pointer null, then publish
null to the transport slot. Last-error zero is normalized to `507h` on the
acquire branch, but no error code is returned or stored by this function;
the source retains its observable API reads and identical branch outcome.
Random failure also cleans up when `GetLastError()==0`. On success, the seeded
transform precedes the key transform, and only then is the context published.
Null allocation publishes null without API calls. Throwing allocation or lock
operations propagate without an invented rollback/unwind scope.

`A5F1D8` reloads and conditionally destroys locks at `+7C`, `+30`, `+04`, each
through virtual slot zero with flag 1; it does not clear those pointers or
payloads. `A5F371` dereferences the owner slot once, releases a nonzero provider,
destroys those members and frees the context. It does not clear the owner's
slot. The public transport host takes the already-loaded context value because
the native callee never writes the slot.

The `1Ch` lock object contains a vtable and actual `CRITICAL_SECTION` at `+4`.
`A5FA5A/A5FA27` allocate and initialize it; `A5F939/A5F944` enter and leave;
`A5F91E` deletes the section and changes native vtables; `A5FA3E` performs
destruction, frees only when flags bit 0 is set, and returns the old pointer.
The source uses normal C++ virtual/destructor transitions rather than writing
original executable vtable addresses. It is not a binary-compatible object.

## Transform evidence and immutable tables

`A5ED9F` and `A5EECB` first enter the global acquisition section, enter source
then destination value locks, and leave the global section. They perform the
two table operations, then reload/leave source and destination locks in that
order. Recursive aliases are retained. No RAII changes the native unwind order.
Their stack temporaries initially copy 40 bytes from `D25D9C`/`D26010` via
`A5EBC3/A5ED17`. Fixed wrappers `A60123/A6014E/A60259/A60284` select the
original signed16 row tables and use native widths 40 and displacement zero.

`A5FBC3` and `A5FE80` are full generic table-driven transform bodies, with
native `__stdcall` nine arguments and `RET 24h`. They choose the signed minimum
of three native widths, retain two unsigned recurrence states, sign-extend row
indices, and emit the two selected high bits per output byte. The seed variant
uses zero input bits for iterations 0..7, then little-endian seed bits. The pair
variant uses the low two bits of both input bytes. Output is at index zero for
the first iteration and `index-displacement` thereafter, with the distinct
first-iteration recurrence reset when displacement is nonzero. Reads precede
each write, preserving in-place aliases. Buffer validity and sufficient seed
bits follow the native caller contract; a width is not necessarily a seed byte
count. No big-integer or named cipher interpretation is asserted.

The single borrowed table view covers VA `D25D9C` through exclusive `D56EB0`,
RVA `925D9C` through exclusive `956EB0`, exactly **200980 bytes**. Framing's
highest row is `D56E20` with 72 signed16 entries; including it prevents duplicate
table ownership. SHA-256 is
`f3456122844489dc39ceee393fc4ecfaddf789449bddf89b434b2cd55fe5c77c`.
The view validates length/hash with actual CryptoAPI SHA-256 before use and
bounds-checks address lookups. Its backing bytes must remain immutable/alive.
The source table block is `.rdata`, raw file offset 9592220, from the installed
12223752-byte executable (image base `400000`) whose SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report lists exact subtable ranges and the original-image binding contract.

The prior `PIPEIPC_*` names are analyst `block_pipe_ipc` tags, not recovered
vendor symbols. Actual CryptoAPI/critical-section calls are standard Win32
boundaries. The executable's table recurrences and wrappers are custom behavior
without established library provenance; they were reconstructed directly.
No standard cryptographic implementation was copied or invented to replace
those recurrences.

## Verification and Ghidra repairs

Strict MSVC Win32 Release build, both existing CTests and all eight native seed
comparisons passed. One ignored synchronous fixture compared both generic loops
and composed initialization outputs against original machine code. Only their
nine absolute table operands per body were relocated to the retained original
table bytes. It covered aliases, displacement, minimum widths, initialized
output, pending-byte preservation, lock order, actual recursive Win32 lock
lifetime, acquire/random failures, null allocation, and table hash rejection.
Protocol CryptoAPI outcomes were scripted; table validation used real CryptoAPI.
No pipe, worker thread, network endpoint, DLL or game runtime was started.

Read-only Ghidra analysis used the existing BSP project/program; final live
count was 63063. Missing virtual entries `A5F939` and `A5F944` have verified
bare RETs at `A5F943` and `A5F94E`. Three free calls hide normal tails at
`A5F4D4..D6`, `A5F394`, and `A5FA53`. The report provides exact parent-only
repair commands and separate unreconstructed startup-entry candidates. No
Ghidra writes or interior-block function definitions were performed.

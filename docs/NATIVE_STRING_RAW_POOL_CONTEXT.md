# Native string operations through the actual raw pool

Addresses: 0041dd20, 0041dd40

The existing native header operations now have overloads taking
`NativeStringRawPoolContext`, which borrows the application's pool publication
01090AA8, small-return gate01090AA4 and raw manager publication01090AA0.
These extend two already reconstructed bodies; they add no native body count.

The earlier `NativeStringStorage` interface declares release `noexcept`.
Its actual-pool adapter calls the lazy getter inside that boundary, so an
escaping getter exception terminates instead of reaching an original caller's
cleanup. BeginFrame's record destructor has a native cleanup state that needs
that exception to propagate. The new overloads call the complete current
00419CC0, BD1120 and BD1510 providers directly, with no semantic callback or
intervening `noexcept` declaration. Existing host overloads keep their contract.

| Body | Original ABI | Coverage |
|---|---|---|
| 0041DD20..0041DD3C | ECX actual8h header, no stack slots, RET, no result | Complete existing body, raw-pool composition added |
| 0041DD40..0041DDE3 | ECX actual8h header, two DWORD slots, preserve low byte, RET8, no result | Complete existing body, raw-pool composition added |

Destruction captures nonnull data and wrapping length+1 before resolving the
current getter. It never changes the header. Resize keeps the complete native
schedule: equal length touches no pointer or pool; zero length releases before
clearing pointer then length; a changed nonzero length allocates before fresh
preserve-copy reads, then captures the current old header before another getter
and return. Only a successful return publishes the new pointer, length and
terminator. Getter calls remain required for large blocks and gated small
returns, even when BD1510 subsequently avoids reading the pool. No rollback is
introduced when a getter throws after allocation.

The shared internal schedule preserves the existing host copy policy. The raw
overload uses `memmove` for nonzero preserve copies: native BF7680 compares
destination with source and source+count at BF7694..BF769A, then selects a
backward path that executes STD/REP MOVSD/CLD at BF785F..BF7862. It is an existing
library dependency, not newly ported CRT code. The source omits zero-byte copies
and requires valid address ranges.

The BeginFrame worker independently reviewed both overloads against the full
listings and its actual record callers. The companion report pins the complete
29-byte and164-byte bodies, overlap evidence, source hashes and nine call rows.
The initial Win32 build and parent exception fixture are recorded separately;
no full parent fixture pass is implied by the interface review. The source
contexts are new C++ interfaces, with no general native ABI, FH3/SEH, concurrent
mutation or gameplay claim.

The initial combined Win32 build and both existing CTests passed. All nine
numeric report call rows passed. Both original signatures and complete Ghidra
bodies were saved and read back, with existing comments preserved. The parent
exception fixture remains pending and is a separate validation boundary.

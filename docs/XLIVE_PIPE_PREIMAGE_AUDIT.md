# Pipe preimage audit

Protocol and temporary preimages are necessary for byte-faithful execution of
the recovered bodies. They change generated frame bytes. Requiring every bit of
each encoded preimage byte overstates the mathematical dependency: only bits0/1
reach the table transforms. This does **not** justify choosing zero low bits.
Fully initialized C++ byte storage remains necessary even when only two bits of
its value are observable to the native operation.

This audit changes no shared implementation. Names, predicate interpretations
and PIPEIPC authorship remain hypotheses. The proposed production capture is an
explicit MSVC Win32 boundary, not an original-game trace or an ABI replacement.

## Native dataflow

`A5F336` writes only the three lock pointers at context+4/+30/+7C. The seed40
payload at +8, pending72 at +34, and key40 at +80 retain allocator bytes. Provider
and lock DWORDs are overwritten before their recovered consumers use them.
`A5F416` initializes seed and key through transforms that read their output
preimages; pending is still unwritten when the constructor returns.

The first read of an encoded input byte in `A5FE80` masks it with3: right input
at A5FEC3 and left at A5FECF. The other shared transforms and packed comparisons
also consume encoded inputs through low-two-bit masks. Their outputs are masked
to2 bits before stores. Thus encoded preimage bits2..7 cannot reach these recovered
frame outputs. All low-two-bit positions remain conservatively required; a bit
that is insensitive around one chosen baseline is not proven globally dead.

`A5F6C0` allocates/stores a temporary lock at A5F778/A5F77D, then calls A5EE0D.
Only the pointer at local-50 is written. Its72 payload bytes at local-4C are read
by A601D5's in-place combine before the header copy. The later transform does not
erase all dependence on those bytes. Pending's heap preimage likewise reaches
A6022B after decode's random64 seed transform, and then subsequent frame headers.

Decode's initial header-to-seed copy overwrites the previous40-byte seed payload,
so that particular path does not need its *old* heap seed preimage. This does not
make the constructor's seed initialization or other seed-consuming paths dead.
The eight raw CryptGenRandom output/preimage bytes are a separate representation:
A5FA70 reads their64 individual bits over iterations8..71. The encoded-byte
low-two-bit rule does not apply to those eight raw bytes.

## Original-machine-code comparisons

The ignored fixture relocates only the authenticated table operands in original
A5FE80/A5FBC3/A5FA70 and the table operands plus two CRT memset calls in A5FD15.
It composes the exact recovered call-site transform chains and immutable row
arguments. The native frame wrapper itself is not executed. No predicate result
is forced, no SDK body is loaded, and no endpoint or thread is created.

Inputs use the actual initialized globals from fixed block E12AC8..E12D00,
SHA-256 `20436ee57ad0c179e4cde3ffd80ea75b53339dea4d500cda75769177176eae21`.
Canonical key initialization gives the false key predicate; the recovered key
update gives true. Variations are every single bit at every byte against a zero
comparison baseline, all256 constant-byte patterns, and256 mixed preimages.
The zero baseline is solely an experiment reference, not a production binding.

| Varied preimage / observed output | Cases | Different byte outputs |
| --- | ---: | ---: |
| Heap seed40 / initialized seed40 | 832 | 494 |
| Heap key40 / initialized key40 | 832 | 496 |
| Temporary72 / initial frame header72 | 1088 | 527 |
| Heap pending72 / initialized pending72 | 1088 | 527 |
| Heap pending72 / subsequent header72 | 1088 | 527 |
| Temporary72 / header-derived seed40 | 1088 | 495 |
| Heap seed40 / subsequent header72 | 832 | 493 |

Every high-six-bit flip is insensitive, consistent with the universal input
masks. Low-bit flips and mixed preimages change outputs. Across these samples,
the native wide comparison predicate against each baseline stays unchanged for
the varied headers/pending states, and the key comparison is also unchanged for
key-initializer variants. This is sampled predicate agreement, not a proof of
protocol equivalence for every preimage. Applying the narrow predicate to varied
seed representations does change results; its semantic suitability for seed
objects is unproven and is not treated as a frame-validation result. No equality
or cryptographic meaning is inferred from any of these predicates.

## Concrete capture contract proposed for production

Use a small, noinline MSVC x86 leaf that receives valid, owned source storage,
a distinct destination, and an exact byte count. Its body uses only register
moves and `REP MOVSB`; the compiler saves/restores ESI/EDI. It makes no API or
allocation calls, touches no TLS, and therefore preserves LastError without an
extra GetLastError/SetLastError pair. Win32's normal clear-direction-flag ABI is
required. This explicit compiler-extension boundary observes machine bytes and
writes every destination byte, avoiding an ordinary C++ read/mask of an
indeterminate source value. Subsequent C++ copies use the fully written result.

The local proof compiled with `/std:c++17 /O2 /W4 /WX /fp:strict`. Its assembly
confirms the capture leaf has no calls or extra memory stores. It also confirms:

- `new XLivePipeProtocolNativeState` calls operator new and then capture, with
  no payload initialization between them. Both relevant native-shaped types
  are trivially default constructible; their arrays have no member initializer.
- `XLivePipeEncodedWideValue temporary;` reserves storage and writes only its
  lock before capture. Pending/temporary payloads are **not already zeroed**.
- Placement-default-construction over known fixture patterns preserves all
  payload bytes. Actual new allocation and actual temporary capture retain
  LastError sentinels. The fixture patterns are not used by production capture.

The narrow shared changes proposed for integrator review are:

1. Change the protocol preimage provider to receive the actual newly allocated
   context address. Capture immediately after `new T` and before the first lock
   allocation, then copy the captured representation back to define C++ storage.
   Its existing no-argument provider cannot identify that allocation. A complete
   allocation-host override is another valid way to bind the same exact address.
   Hold the new allocation in RAII until capture/copy finishes: the existing
   allocator leaks it if its preimage provider throws. No locks exist at this
   point, so exception cleanup only frees the new context storage.
2. Change the frame temporary provider to receive the actual temporary payload
   address. Declare `Wide temporary;`, create/store its lock in native order,
   capture its72 payload bytes, copy them back, then enter A5EE0D. Do not capture
   unrelated helper-local storage and label it the frame temporary's preimage.
   A custom capture-provider exception must destroy the newly created temporary
   lock before propagating. Keep this added capture-boundary cleanup distinct
   from the native transforms, which have no C++ exception unwind cleanup.
3. For the real random API host, use an actual uninitialized8-byte local output
   buffer. Call CryptGenRandom into that buffer, immediately capture its actual
   bytes with the same leaf, copy to the typed output and mark all bytes known.
   The leaf preserves CryptGenRandom's LastError for the existing caller check.
   On failure, the result includes actual unwritten remainder/partial API writes.
   It is not the wrapper's currently zero-initialized backing array. Recording
   hosts may still return unknown bytes and exercise the existing explicit guard.

Capture all8 bits when convenient; that reports actual observed bytes and avoids
invented padding. If a future optimized representation retains only low2 bits,
it must still write fully initialized C++ bytes and explicitly justify the
unobserved upper-bit choice. Raw random64 still requires all64 known bits.

Do not use `new T{}`, `new T()`, `Wide temporary{}` or explicit zeroing before
capture: they replace the preimage. Capture must not allocate, acquire locks,
invoke callbacks or change the order of the original allocation/lock/CryptoAPI
calls. Valid owned ranges are required; invalid memory is not a new successful
fallback. The captured values vary with this process's allocator, stack layout,
compiler and execution history. They support honest current-process execution,
without claiming byte parity with an unrecorded original-game run.

Both local audit/capture fixtures passed. Shared source, repository build inputs,
Ghidra state and installation files remain unchanged by this audit.

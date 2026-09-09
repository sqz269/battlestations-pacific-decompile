# Bounded raw-inflate stream implementation

`include/bsp/inflate_stream.hpp` and `src/inflate_stream.cpp` implement a new
typed projection of the verified game wrapper. The supported operations are
buffered read, forward seek and rewind within the current output block.
The adapter retains an explicit seekable source and uses the existing
`bsp_zlib121` dependency. It does not parse MPKG entries or replace native
game functions.

Evidence is in `INFLATE_STREAM_READ_SEEK.md` and
`reports/inflate_stream_read_seek_audit.json`. The complete wrapper range
`00bbbd80..00bbc3ff` matched the original executable with SHA-256
`6170337200869ee5c3fdbb44b91ebfabc8bdbb8f43e87b0c138277d7fc7b2ddb`.
The source follows constructor `00bbc1d0`, refill `00bbbf00`, read
`00bbc140`, seek `00bbc060`, position `00bbbe50` and size `00bbbdd0`.
The existing reviewed analysis supplied the implementation contract; integration
also records the bounded C++ result in the corresponding Ghidra comments.

## Source and public API

`InflateSource` supplies two `noexcept` virtual methods: seek to a DWORD
absolute source offset, and read up to a DWORD count while reporting actual
bytes through a reference. Read returns a host success flag. The source must
write within the requested destination range and report actual even on
failure. Positive short reads are accepted. Ownership is a
`std::shared_ptr<InflateSource>` retained until destruction; another owner
must not move that source's active cursor while this adapter is decoding.
An existing `MemoryStream` may be wrapped through its public methods and
actual retained owner. No native fields are inferred from that host class.

`InflateStreamDescriptor` is exactly three DWORDs: source offset, compressed
size and decoded size. `InflateStream` is move-only and keeps zlib and its
buffers behind an out-of-line pimpl. No zlib header or native register ABI
appears in the public header.

`open_00bbc1d0_fragment` initializes raw inflate with window bits `-15`, then
seeks the retained source to the descriptor offset. Both buffers start empty;
logical position is zero. Input capacity is `0x4000`; output capacity is
`0x10000`, matching the constructor's constants. Allocation, initialization
and source-seek failures return explicit statuses and leave the adapter
uninitialized. A failed source seek may itself have changed the source cursor.
An already open adapter returns `already_initialized` without changing state.
Move assignment from a new default instance can dispose an existing adapter.

The original constructor takes ECX object and four stack arguments, with
RET10h; its two capacity-like arguments are ignored. Native read takes ECX
object and destination/requested/optional-actual on the stack, RET0Ch, and
leaves the actual pointer in EAX. Native seek takes a low/high offset and
origin, RET0Ch, with no stable result. The host status returns and pimpl
are new interfaces, not native ABI-compatible replacements.

## Successful read and seek behavior

Refill sets zlib output to the entire output buffer. When input is exhausted,
it requests `min(16384, compressed_remaining)` bytes from source, subtracts
the actual successful read and supplies those bytes to zlib. Positive short
reads are consumed and followed by further source reads as needed.

The exact native flush selection is preserved:

```text
compressed_remaining == 0 && decoded_remaining <= avail_out
    ? Z_FINISH : Z_SYNC_FLUSH
```

The condition concerns compressed bytes still to read from source, excluding
already buffered `avail_in`. Refill stops when output fills or inflate returns
a nonzero status, and publishes any output prefix. Decoded production and
logical consumption remain separate counters. On successful zlib calls the
produced output extent equals the native `total_out` delta. The host uses the
actual output extent so a prefix is also accounted for when stock zlib 1.2.1
reports a late window-allocation failure before updating `total_out`.

Read drains unsigned `min(request_remaining, buffered_output)` amounts,
advances logical position and repeats. The optional actual pointer is always
written, including on host errors. Request zero is an initialized no-op that
reports zero and returns `ok`, even if a prior failure is latched. A nonzero
read requires a valid destination. An exact fulfilled request returns `ok`
unless a failure is already known; a request larger than the remaining
declared decoded extent returns `end_of_stream` with its actual prefix.

Seeking discards the high DWORD of the signed 64-bit offset, adds its low
DWORD modulo 32 bits to the selected base, and compares targets unsigned.
Origins 0, 1 and every other value select start, current position and declared
decoded size, respectively. Forward seek drains buffers without copying and
refills as necessary. A target beyond EOF leaves position at the decoded end
and returns `end_of_stream`; it is not silently reported as reached. Rewind
within the current output block adjusts its cursor and logical position while
preserving inflater/source state. Position and size return zero-extended
DWORD values through the host's `uint64_t` interface.

The current buffer remains available for rewind even after its final byte was
read. Refilling replaces it. Seeking farther backward returns
`unsupported_backward_seek` before altering the source, buffers, decoder or
position. This bounds the supported domain around the verified native defect:
native `00bbbe10` resets inflater/source/counters but preserves stale input
and output buffers. The host does not invoke or silently repair that reset.

## Host failure behavior

| Status | Meaning and resulting state |
|---|---|
| `not_initialized` | Operation requires a successful open. Read actual is zero. |
| `already_initialized` | Reopening is rejected without changing the existing stream. |
| `invalid_argument` | Null source on open or null destination for a positive read. The stream remains usable after an invalid read. |
| `allocation_failed` | Pimpl allocation or initial zlib allocation failed. |
| `source_seek_failed` | Initial source seek failed; adapter remains unopened. |
| `source_read_failed` | Source returned false. Bytes from that failed source call are discarded, even if it reported a positive actual count. Previously decoded/copyable prefix remains available. |
| `invalid_source_count` | Source reported more bytes than requested. No invalid count is fed to zlib. |
| `decoder_error` | Inflate returned a terminal status other than stream end or buffer error. `decoder_status()` exposes the underlying integer result. |
| `decoded_size_mismatch` | Stream end arrived before the declared decoded count, or inflate produced more than that count. Overproduction publishes only the prefix within the declared extent. |
| `no_progress` | A source read returned zero before the declared compressed end, or refill cannot publish bytes/progress. The native indefinite retry is terminated. |
| `unsupported_backward_seek` | Target lies before the retained output block; all state is unchanged. |

Refill failures latch. Any output already produced within the declared size
remains drainable. Nonzero reads and seeks report the known failure even if
they satisfy the immediate request using that prefix. If a request reaches
past the prefix, its failure result includes all bytes already copied or
skipped; no new source read/inflate call occurs after the prefix drains.
Callers can inspect position or actual count to distinguish progress from
failure. A failed read never discards bytes already delivered to the caller.

`Z_BUF_ERROR` with output is not automatically fatal: stock 1.2.1 uses it for
unfinished `Z_FINISH` even after progress. The block is published and the next
decode attempt waits until it drains, matching native successful behavior.
An empty buffer error becomes `no_progress`. Likewise, a zero source read is
a terminal host outcome rather than a busy retry. These guards intentionally
do not reproduce native failure behavior, unsigned count underflow or hangs.

The declared decoded size remains the normal EOF boundary. When it reaches
zero, the adapter does not inflate extra data solely to demand a final marker
or consume all declared compressed bytes. A zero decoded size is immediately
EOF without decoding. Thus a truncated input that already emitted its entire
declared decoded count is not necessarily rejected. This preserves the native
successful boundary; the adapter is not an independent archive integrity
validator and performs no CRC check.

## Integration and focused verification

`bsp_core` now includes the adapter. MSVC Win32 compilation and both existing
CTests pass. `src/inflate_stream_probe.cpp` adds one scenario to the existing
D3D9 probe, with no new test target. Three explicitly framed stored-DEFLATE
blocks contain 150,123 deterministic bytes; their independent plain payload
is the byte-comparison oracle. The source starts after a 13-byte prefix and
returns at most 1,021 bytes per read, requiring 148 positive short reads.

The fixture passes reads across 64KiB output boundaries, current-block rewind,
forward skip across the next boundary, rejected seek-to-zero with unchanged
position, ignored offset high DWORD, short EOF actual count, and source lifetime
through adapter destruction. It exercises stock zlib through the wrapper;
compressed-Huffman blocks, malformed/error inputs and real archive entries are
not covered. See [integration evidence](../reports/parallel_implementation_validation.json)
and [probe output](../reports/parallel_implementation_probe.txt).
No native differential execution, game archive fixture or gameplay validation
is claimed. The other implementation worker independently reviewed this source
against the assembly-backed contract without finding an actionable defect.

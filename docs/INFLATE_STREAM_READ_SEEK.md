# Raw-DEFLATE stream read and seek contract

This is an assembly-backed contract for the game wrapper at `00bbc1d0`,
including read `00bbc140` and seek `00bbc060`. No C++ implementation or game
validation is claimed. Descriptive names below are proposals, not recovered
symbols. This batch made no Ghidra edits and preserved library identities.

Integration follow-up: the primary agent verified the selected spans again,
created the missing read/seek definitions, and saved five game-wrapper names
and evidence comments for construction, refill, reset, seek and read. Previous
comments and imported library/compiler names were preserved. No C++ was added.

The smallest coherent implementation is a retained, buffered raw-inflate
reader with forward seek and rewind within its current output block. Native
rewind beyond that block has a confirmed stale-buffer defect; silently
implementing a clean restart would change the observed contract. Native
empty-refill failure can also loop indefinitely. A host implementation needs
explicitly documented failure outcomes for those cases.

## Evidence and scope

`tools.ghidra_export.Client`, using `config/target.json`, verified project `bsp`,
program `/battlestationspacific.exe`, x86 LE32 and base `00400000` before each
analysis/export batch. Project file is `C:/Users/sqz269/bsp.gpr`. Current disk
executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

These complete raw spans matched the installed PE. The two small extensions
were coordinated with the parent agent. Other callees were not newly analyzed.

| Range, inclusive | Bytes | SHA-256 |
|---|---:|---|
| `00bbbd80..00bbc3ff` | 1664 | `6170337200869ee5c3fdbb44b91ebfabc8bdbb8f43e87b0c138277d7fc7b2ddb` |
| `00d643f8..00d6443f` | 72 | `7f5e91ce63c733791c027fd304756d76cc82747331245d383d5bef85e0a0b42f` |
| `00bc9650..00bc969f` | 80 | `b9c5c15dfd6a63307cbba5f4501724909450407f64d79a67de0cc4fc741ccc87` |

Ignored artifacts are in `exports/bsp/parallel_inflate/`, including target
identity, original/saved byte comparisons, raw Capstone disassembly and
available decompilations. Read and seek were undefined at export time;
`decompile_function` returned `No function found` for both. Their full bodies
and returns are established from raw assembly. The destructor decompilation
stops incorrectly at `_free`; raw bytes establish the continuation to
`00bbc3d9`. The tracked machine-readable audit is
`reports/inflate_stream_read_seek_audit.json`.

## Native layout and ABI

These offsets come from constructor and method instructions, independently of
the repository's `MemoryStream` class. That class is a separate host projection.

| Wrapper offset | Observed role |
|---|---|
| `+00` | Primary vtable `00d64400` |
| `+04` | Intrusive reference count, initialized to 1 |
| `+09` | Byte initialized to 1; virtual `+18` returns it in AL |
| `+0c` | Retained source stream |
| `+10/+14/+18` | Three copied DWORDs: source offset, compressed count, decoded count |
| `+1c/+20` | Input/output buffer descriptor pointers |
| `+24` | Logical decoded position, DWORD |
| `+28` | Allocated `0x38`-byte `z_stream` |
| `+2c` | Compressed bytes still to read from source, excluding already buffered input |
| `+30` | Decoded bytes still to produce, excluding already buffered output |

Each 16-byte buffer descriptor is `{base, data_end, allocation_end, cursor}`
at offsets `+0/+4/+8/+c`. Both start empty with `data_end == cursor == base`.
Input allocation is `0x4000` bytes; output allocation is `0x10000` bytes.
The constructor consumes four stack slots and returns `this` in EAX, `RET 10h`.
Only stack arguments 1 and 2, source and descriptor pointer, are read; the
two apparent capacity slots are ignored in favor of immediate constants.
`00bb8be0`'s cached caller additionally allocates `0x34` bytes for the wrapper;
that caller was prior evidence, not part of this batch's byte verification.

| Address | Native input, return and role |
|---|---|
| `00bbbde0` | ECX wrapper, no stack args, RET: seek source to descriptor offset and restore counters, without zlib reset |
| `00bbbe10` | ECX wrapper, no stack args, RET: reset inflater, then the same source/counter reset |
| `00bbbe50` | ECX wrapper, RET: position as zero-extended `EDX:EAX` |
| `00bbbdd0` | ECX wrapper, RET: declared decoded length as zero-extended `EDX:EAX` |
| `00bbbe60` | No consumed inputs, RET: allocate/init raw z_stream, EAX pointer |
| `00bbbea0` | ECX buffer, DWORD count on stack, RET4: set end=base+count and cursor=base, unchecked |
| `00bbbec0` | ECX buffer, pointer on stack, RET4: assign cursor, unchecked |
| `00bbbed0` | ECX buffer, destination/count on stack, RET8: copy unsigned min(count,end-cursor), advance cursor, EAX copied count |
| `00bbbf00` | ECX wrapper, no stack args, RET: refill output; no stable result/status contract |
| `00bbc010` | ECX wrapper, RET: AL false only if output empty and decoded-remaining zero; otherwise refill if empty, then AL true without checking refill result |
| `00bbc030` | ECX wrapper, origin on stack, RET4: zero-extended base 0/current/length for origin 0/1/other |
| `00bbc060` | ECX wrapper, offset-low/offset-high/origin on stack, RET0Ch: seek; high DWORD ignored; no stable return value |
| `00bbc140` | ECX wrapper, destination/unsigned count/optional DWORD actual-pointer on stack, RET0Ch: read; epilogue leaves EAX equal to actual-pointer, not bytes read or success |
| `00bbc1c0` | RET0Ch with no body, primary virtual `+28`; exact source-level name remains provisional |
| `00bbc1d0` | ECX object, source/three-DWORD descriptor/two ignored slots, RET10h, EAX object |
| `00bbc320` | ECX wrapper, no stack args, RET: non-deleting destruction |
| `00bbc3e0` | ECX wrapper, flags on stack, RET4, EAX object: preserve existing `CG_scalar_deleting_dtor_00bbc3e0` name |

The verified primary table maps `+1c -> 00bbc060`, `+20 -> 00bbbe50`,
`+24 -> 00bbc140`, `+28 -> 00bbc1c0`, `+30 -> 00bbbdd0`, and
`+4 -> 00bbc3e0`. Standalone buffer helpers and `00bbc010/00bbc030` have the
same operations that read/seek inline; read/seek do not call those helpers.
The table's other slots remain outside this contract.

The game calls `inflateInit2_` with ECX stream, EDX `-15`, then version pointer
and stream size on the stack. The verified version literal is `1.2.1`.
`inflate` receives ECX stream and EDX flush; `inflateReset` receives ECX stream.
Stock linked zlib uses its normal C ABI, so these are evidence for a new typed
interface, not permission to call stock functions through native registers.

## Refill and read

`00bbbf00` sets `next_out=output.base` and `avail_out=output.capacity` on
every invocation. It does not append or compact output. Its callers first
check that output is empty.

While output space remains, if input `end == cursor` and compressed-remaining
is nonzero, it asks source virtual `+24` to read
`min(input.capacity, compressed_remaining)` into input.base, supplying a
DWORD actual-count pointer. It ignores any source return value. It resets
input.cursor to base, sets input.end to base+actual, subtracts actual from
compressed-remaining, and sets `next_in/avail_in` from that actual count.
A positive short read is accepted and the remainder can be requested later.

The flush decision at `00bbbf95..00bbbfb6` is exactly:

```text
flush = compressed_remaining == 0 && decoded_remaining <= z.avail_out
      ? Z_FINISH (4) : Z_SYNC_FLUSH (2)
```

It records `total_out`, calls inflate, updates input.cursor from `next_in`,
and subtracts the unsigned change in `total_out` from decoded-remaining.
It stops on `Z_STREAM_END` or any other nonzero result, or when output is
full. It then sets output.end to allocation_end minus `avail_out` and
output.cursor to base. There is no retained zlib status or EOF flag.
A degenerate zero-capacity input request returns before publishing output;
the normal constructor's fixed nonzero capacity excludes that branch.

Read `00bbc140..00bbc1b5` follows this contract:

```text
remaining_request = requested
destination_cursor = destination
while remaining_request != 0:
    if output.end == output.cursor:
        if decoded_remaining == 0: break
        refill()
    count = unsigned_min(remaining_request, output.end - output.cursor)
    memcpy(destination_cursor, output.cursor, count)
    output.cursor += count
    destination_cursor += count
    logical_position += count
    remaining_request -= count
if actual_pointer != null:
    *actual_pointer = uint32(destination_cursor - destination)
```

The compiler repurposes the incoming request stack slot as destination_cursor
at `00bbc14e`; requested is held in EBP. Thus `00bbc1a9..00bbc1b1` reports
bytes copied, not requested minus a pointer or a decompressor total. A zero
request makes no refill or copy and writes zero when actual_pointer exists.
The actual count is written only at normal return. There is no destination
validation. Output is drained even if decoded-remaining is already zero;
that counter tracks production, not the consumer position.

For a valid stream with matching descriptor counts, normal short reads happen
only when the request exceeds the remaining decoded extent. The source may
return smaller positive chunks without making the wrapper itself return
early. The reader does not independently verify compressed exhaustion or
`Z_STREAM_END` when the declared decoded length has been supplied.

## Seek and reset

Seek takes a nominal 64-bit offset and an origin. Entry `ESP+8`, the offset's
high DWORD, is never used. The target is unsigned modulo-32-bit
`low_offset + (origin == 0 ? 0 : origin == 1 ? position : declared_length)`.
All ordering and buffer bounds checks use unsigned comparisons.

Equal target is a no-op. For a lower target, compute the candidate native
pointer `output.cursor + target - position`. If it lies inclusively between
output.base and output.end, set cursor to it and position to target. This
rewind reuses the current output block and preserves source, zlib and
remaining-production counters.

If that candidate is outside the buffer, seek calls `00bbbe10` at `00bbc0c1`.
The reset calls stock `inflateReset`, seeks source to `(descriptor.offset,0,0)`,
sets position=0 and restores both remaining counts. It ignores reset/seek
results. **It does not change either descriptor's end or cursor.** The freshly
verified `inflateReset` body also does not clear `next_in`, `avail_in`,
`next_out` or `avail_out`. It clears totals/message and internal decode state.
This matches local unmodified zlib 1.2.1 `inflate.c:103..121`.

After reset, target zero returns immediately. A positive target enters the
forward loop using whatever output bytes are still marked unread. Thus the
normal relation between logical position and buffered output has been lost.
For example, after reading 70,000 bytes with a second output block covering
decoded offsets 65,536 onward, seeking to zero cannot fit in that block.
Reset changes position to zero but preserves output.cursor at original offset
70,000. The next read starts there. This is a symbolic consequence of the
verified instructions, not a runtime fixture result. If input bytes remain,
they also survive reset and can be decoded as if they began a fresh stream.

Forward seek loops over output blocks. When output is empty and
decoded-remaining is zero, it returns without reaching the requested target.
Otherwise it refills if needed, skips min(target-position,unread output),
and increments position. When skipping a whole block it sets cursor=end;
for a partial block it bounds-checks the candidate cursor but increments
position even if that pointer check fails. Valid descriptors make that check
pass. There is no explicit target-versus-length clamp and no achieved-position
or success return; on a valid stream, seeking beyond end stops at the decoded
end. The caller must use the position method to learn where it stopped.

## Failure and progress boundaries

| Condition | Observed consequence |
|---|---|
| Source returns zero before declared compressed end | Refill still calls inflate. Empty output plus nonzero decoded-remaining makes read/seek retry; persistent zero reads can loop forever. |
| Source fails without writing actual count | No fallback count is initialized before each call. The first count slot is seeded by entry `PUSH ECX` with the wrapper address; later calls retain the previous value. Native count/pointer arithmetic can become invalid. |
| Source reports count above request/capacity | No validation before end-pointer update and unsigned subtraction. |
| zlib error or early `Z_STREAM_END` | Refill publishes any produced prefix and discards status. Once that prefix drains, a nonzero decoded-remaining can make outer read/seek retry indefinitely. |
| `Z_BUF_ERROR` | Refill stops even when output was produced. It is not synonymous with no progress: stock 1.2.1 returns it for unfinished `Z_FINISH`. |
| Descriptor decoded count too small | Output capacity is not clamped to decoded-remaining. Excess production can underflow the DWORD counter; the wrapper does not enforce the declared length. |
| Descriptor decoded count zero | Read reports zero without inflating, even when compressed input exists. |
| Allocation/init/source-seek failure | Native construction/reset ignore several failures and can dereference invalid state. Successful host allocation is a required projection precondition, not native failure parity. |
| Backward seek beyond current output block | Restored counters coexist with stale input/output; this is not a correct arbitrary replay implementation. |

Destruction releases the source first, calls inflateEnd, frees z_stream,
then input backing/descriptor, then output backing/descriptor, then base
cleanup `00bd30f0`. The deleting wrapper frees the object when flags bit 0
is set. These continuations are present after `_free` despite Ghidra's
truncated pseudocode.

## Next coherent implementation and annotation handoff

Implement a new typed raw-inflate adapter that owns a retained seekable source,
the explicit descriptor, the fixed buffers and a stock zlib stream. Keep
compressed-read, decoded-produced and decoded-consumed counts distinct.
Preserve positive short reads, flush selection, partial-output publication,
optional actual-count reporting, modulo-low-DWORD target calculation and
forward/in-block seek behavior. A whole decoded vector bypasses these methods.

Expose host outcomes for allocation/source failure, invalid source counts,
terminal inflate errors, declared-size overproduction and empty refill with
no recoverable progress. Preserve already copied bytes in the actual count.
Document those guards as host safety behavior; do not claim native error
equivalence. Do not treat every `Z_BUF_ERROR` as fatal when it produced bytes.
Reject out-of-block backward seek before changing state in the first fragment;
a later correctly restarting API must explicitly document its departure from
the verified native reset. This gives a concrete, usable read/seek subset
without intentionally copying a hang or silently fixing native behavior.

No additional source/callee reconstruction is needed for that bounded adapter.
MPKG transformation, entry parsing/provider semantics and archive fixtures are
still the separate `ARCHIVE_PROVIDER_ENTRY.md` dependency. `MemoryStream` can
be a typed source only through its public interface and actual retained owner;
its host fields do not establish this native layout.

The JSON audit proposes game-only names and evidence comments for the parent
to review and apply. Undefined read/seek methods need function definitions
with the full verified bodies before naming. Preserve existing comments and
old names/definitions in the ledger, retain `inflate*` library names and both
existing compiler-generated destructor labels, save the project, and refresh
affected exports after any accepted annotations.

This batch is byte-verified and assembly-analyzed only. It adds no C++ or
tests, so no build was required. It does not establish reconstructed,
build-tested, fixture-tested, ABI-compatible or game-validated stream behavior.

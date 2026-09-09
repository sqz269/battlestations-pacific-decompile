# MPKG transformed bytes, directory entries and entry sources

Addresses: `00bb87a0`, `00bb8850`, `00bb8a90`, `00bb8be0`, `00bb8d60`, `00bb8e00`, `00bb9090`, `00bb9330`, `00bb9520`, `00bb95b0`, `00bb9700`, `00bb9920`, `00bef840`, `00bf1000`, `00bf1130`, `00bf11c0`, `00bf1240`

The existing `InflateStream` can consume a compressed MPKG entry directly
from the archive's **transformed backing**, using a lazily resolved local
data offset and the compressed/decoded counts retained from its directory
record. That is one of three distinct native entry routes. Small stored
entries are copied into new memory; large stored entries reopen the original
logical path and return a reader that performs neither transformation nor an
entry-length clamp. Those routes must not be silently combined.

The audit below establishes static contracts; the implementation follow-up
is described near the end of this document. The audit made no Ghidra mutations
or game validation. Existing analysis names and CRT/library tags were preserved.
Descriptive field names are hypotheses tied to exact byte offsets and uses;
no external ZIP specification replaces the observed implementation.

## Evidence and corrections to the earlier handoff

Every live batch used `tools.ghidra_export.Client` with `config/target.json`
and verified project `bsp`, `/battlestationspacific.exe`, x86 LE32, base
`00400000`. The original executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The parent approved the constructor tail and the exact source helper/data
extensions. No analysis overlapped the separate physical async packet.

| Verified span | Bytes | SHA-256 |
|---|---:|---|
| `00bb8700..00bb9bff` | 5376 | `47f9f4b3df812fb5dbabc77507eabfde62b7bfe5c481de69eabf233ec2bb2941` |
| `00bb9920..00bb9c0a`, complete constructor | 747 | `ae56080e3ddaa652e2eb8a0453e85bb08fa03043bae163f9957d554915d2502b` |
| `00e144f0..00e14708`, transform table | 537 | `26abd3e2995fdbacecb6f352b5c0eb20ff20c9a7ada6312ba619c9d4c6fa6183` |
| `00bef840..00bef8e7`, memory copy helper | 168 | `87cb643f48b4a214ae3c002c0cc2c854a41ca3ca9510cf4555de886564bf88f3` |
| `00bf1130..00bf11bd`, adopting source constructor | 142 | `7c87769b698e9a5c940abe24502923ad2d061cd04c03c90310a063ac81b4af66` |

`reports/mpkg_entry_loading_audit.json` includes per-function hashes and
additional vtable/read/destructor comparisons. Raw bytes, Capstone listings,
current decompilations and target checks are in ignored
`exports/bsp/parallel_mpkg_entry/`.

The earlier `ARCHIVE_PROVIDER_ENTRY.md` handoff is refined in four places:

- The final transform block uses the **global output index modulo the final
  block length**, not an index relative to that block. A generic block reversal
  is not equivalent.
- `00bef840` produces a new backing by copying one source read. The small
  stored path is not a shared view of the transformed backing.
- The directory parser seeks to the stored directory offset without applying
  the computed prefix adjustment. Entry local-header offsets do add it.
- Large stored reads directly delegate to the reopened source without decoding
  or clamping to their declared entry extent.

## Transform and archive state

Constructor `00bb9920` takes ECX state and one stack path-string argument,
returns state in EAX, and ends with RET4 at `00bb9c08`. It copies the path into
state `+4/+8`, clears its entry vector, opens the path through VFS virtual `+4`
with flags 2, converts that source to memory through `00bef750`, and releases
the original returned source reference. The memory source's size low DWORD
is used as a signed loop bound; the size high DWORD is not consumed.

It allocates a second backing of that low-DWORD size, then performs the
following mapping for positive signed lengths `N <= INT32_MAX`. All indices
below are integers; `K` is the verified 537-byte table at `00e144f0`:

```text
B = 753  (0x2f1)
Q = N / B
for i in [0, N):
    block = i / B
    L = (block == Q) ? N % B : B
    source_index = block * B + L - 1 - (i % L)
    decoded[i] = encoded[source_index] XOR K[source_index % 537]
```

The decisive unsigned DIV uses EDI, the global output index, at
`00bb9a49..00bb9a57`. For `N=758`, the five final output bytes have source
indices `754,753,757,756,755`, not `757,756,755,754,753`. This is an arithmetic
example derived from instructions, not an installed archive fixture. For a
positive exact multiple of 753, no visited index has `block == Q`, so the
zero remainder is never used as a divisor. Nonpositive signed sizes skip the
transform after allocation; they do not establish a safe native empty/huge
archive contract. There is no signature-based bypass for already decoded input.

The original memory wrapper is destroyed through virtual `+4` with flag 1.
The transformed backing is wrapped through `00bef6d0`, stored at state `+0c`,
and its temporary backing reference is released. The cached implementations
of `008d43c0`, `00bef6d0` and `00bef750` establish the existing backing/wrapper
roles; those helpers were not newly identity-compared in this packet.

| State offset | Observed role |
|---|---|
| `+04/+08` | Retained original logical path string length/data |
| `+0c` | Transformed memory stream |
| `+10` | Low-DWORD transformed archive length, assigned by scanner |
| `+14` | Entry count, zero-extended from EOCD word at `+0a` |
| `+18` | `eocd_offset - directory_offset - directory_size`, DWORD arithmetic |
| `+1c` | EOCD offset, initially `ffffffff` |
| `+20/+24` | Directory size / directory offset from EOCD |
| `+28/+2c/+30` | Ordered entry array pointer / count / capacity |

## End marker and directory byte cursor

`00bb87a0` reads the final `min(archive_length,65535)` bytes in one unchecked
source read. If the tail length `T` is greater than four, it tests signature
`50 4b 05 06` at offsets `T-4` down through **1** within that tail. Offset zero
is excluded by the loop condition; a four-byte tail is not scanned. It chooses
the nearest matching signature to the end, without validating the following
fields or comment extent. It stores `archive_length - distance_from_end` in
state `+1c`. On no match, the initial `ffffffff` remains.

Constructor then seeks to that offset and makes these reads, all with null
actual-count pointers and unchecked results:

| Offset from signature | Bytes | Use |
|---|---:|---|
| `00` | 4 | Read and discarded |
| `04/06/08` | 2 each | Read and discarded |
| `0a` | 2 | Total entry count to state `+14` |
| `0c` | 4 | Directory size to state `+20` |
| `10` | 4 | Directory offset to state `+24` |
| `14` | 2 | Read and discarded |

Little-endian scalar assembly may look like byte swapping in pseudocode,
but it reconstructs low-to-high bytes without changing the stored value.
No missing-signature, multi-disk, size, comment or EOF validation appears.

`00bb9700` is ECX state, no stack arguments, RET. It seeks transformed source
to **state+24**, allocates `state+10 - state+24` bytes, and reads that amount
once. Actual read count is stored in the temporary cursor object but not used
to guard parsing. It initializes cursor to allocation base and calls
`00bb95b0` exactly state+14 times. It reads through the archive tail rather
than restricting allocation/iteration to directory size. Crucially, it does
not add state+18 to the directory seek. A future parser must not change that
to `eocd_offset - directory_size` as an assumed ZIP correction.

The scratch reader has base at `+0`, reported count at `+4`, current pointer at
`+8`. `00bb8850` accepts reader/header/kind as **three stack arguments, RET0Ch**;
ECX is not an input. It reads 46 fixed bytes when kind=0, and 30 when kind is
nonzero. It merely reads the first DWORD; it never checks a header signature.
There are no reader bounds checks.

| Field role | Central input offset (kind 0) | Local input offset (kind 1) | Parsed header offset |
|---|---:|---:|---:|
| Signature value, unchecked | `00` | `00` | `04` |
| Creator/version field, ignored by entry projection | `04` | absent | `08` |
| Required-version field | `06` | `04` | `0a` |
| Flags | `08` | `06` | `0c` |
| Method | `0a` | `08` | `0e` |
| Combined time/date value | `0c` | `0a` | `10` |
| CRC-like value | `10` | `0e` | `14` |
| Compressed byte count | `14` | `12` | `18` |
| Decoded byte count | `18` | `16` | `1c` |
| Name byte count | `1c` | `1a` | `20` |
| Extra byte count | `1e` | `1c` | `22` |
| Comment byte count | `20` | absent | `24` |
| Disk field | `22` | absent | `26` |
| Internal attributes | `24` | absent | `28` |
| External attributes | `26` | absent | `2c` |
| Relative local-header offset | `2a` | absent | `30` |

Header `+0` records the supplied kind. The names of ignored fields are
ZIP-compatible interpretations; their widths/offsets and absence of decisions
based on them are the observed facts. No ZIP64, encryption, method whitelist
or data-descriptor rule is introduced by this table.

## Directory record and lazy local offset

`00bb95b0` is ECX state, one stack reader, RET4. It reads a kind-0 fixed header,
then `00bb9090` copies exactly its name length into an owned native string,
advancing the reader. It adds extra length plus comment length to the reader
cursor and appends a 36-byte entry using `00bb9520/00bb9330`. No normalization,
case folding, duplicate elimination or special directory-name branch occurs.
The append preserves directory order and owns a copy of the name.

| Entry offset | Stored value |
|---|---|
| `+00/+04` | Name byte length/data pointer |
| `+08` | state prefix adjustment + header relative local-header offset |
| `+0c` | Resolved-offset flag, initially byte zero |
| `+10` | Provisional data offset: entry+8 + 30 + central name length + central extra length |
| `+14` | Method word from central header |
| `+18/+1c` | Compressed / decoded byte counts from central header |
| `+20` | Retained CRC-like DWORD; not checked by the observed read path |

The provisional offset is not trusted on first open because the flag is zero.
`00bb8a90` takes ECX state and one stack entry, RET4, EAX data offset. If the
flag is already set, it returns entry+10 without reading. Otherwise it seeks
the transformed stream to entry+8, allocates/reads exactly 30 bytes, parses a
kind-1 header and caches:

```text
entry.data_offset = entry.local_header_offset + 30
                  + local_header.name_length + local_header.extra_length
entry.offset_resolved = true
```

It uses the **local** lengths on first resolution and does not cross-check
local versus central method, flags, counts, name or signature. Its raw return
at `00bb8b2b` restores the cached offset to EAX after `_free`; the decompiler's
`extraout_EAX` and truncated tail are misleading.

`00bb8d60` takes ECX state and stack name/flags, RET8, EAX stream or null.
Flags bit0 rejects immediately. It scans entries in insertion order, requires
equal stored name lengths, then uses `_stricmp` on their bytes, with the native
zero-length special case. The first equal entry wins, including duplicates.
It performs no path cleanup or independent normalization. On a match it calls
`00bb8be0` with entry and flags. The latter consumes two stack slots (RET8)
but does not use flags. `00bb8e00` uses the same lookup and returns AL bool,
with one stack name and RET4.

## Three entry opening routes

All routes first resolve the local data offset through `00bb8a90`.

| Condition | Source and operation | Returned ownership |
|---|---|---|
| Method word nonzero | Transformed memory stream; descriptor `{entry+10, entry+18, entry+1c}` passed to raw inflater `00bbc1d0`, then `00bef750` converts the inflater to memory | Newly buffered decoded entry, independent of archive backing after conversion |
| Method zero and decoded size `<=0x40000` | Transformed memory stream; `00bef840(offset64, length64)` performs a seek and one copy read | New memory backing, not a shared subrange |
| Method zero and decoded size `>0x40000` | Reopen stored original logical path through VFS, flags2; `00bf1130(source, offset64, length64)` adopts that source | Source-owning reader over the reopened stream; no transform in constructor/read |

Nonzero method is not restricted to value 8: the observed branch attempts raw
inflate for every nonzero word. It ignores flags and performs no CRC check.
The already reconstructed inflater uses fixed buffers and declared decoded
length as its normal EOF boundary; its host error handling is documented in
`INFLATE_STREAM_IMPLEMENTATION.md`. The native compressed entry is converted
to memory before return, so the live inflater does not escape this opening
function. Independent host cursors over retained transformed backing can
safely serve the same successful sequential behavior.

`00bef840` takes ECX source and offset-low/high, length-low/high on the stack,
RET10h, EAX memory wrapper. It consumes both offset words, but only the low
length DWORD. It allocates backing through `008d43c0`, reads exactly the
requested low length once with no actual-count output, wraps through
`00bef6d0`, and releases the temporary backing. Allocation/read failures and
uninitialized tails are not checked natively. The apparent `unaff_retaddr`
length in its decompilation is an ABI artifact resolved by assembly.

`00bf1130` takes ECX object and source, offset-low/high, length-low/high,
RET14h, EAX object. The 40-byte reader stores source at `+8`, start at
`+10/+14`, end=start+length at `+18/+1c`, and current absolute position at
`+20/+24`. It seeks source to start and stores it **without AddRef**.
Destructor `00bf11c0` decrements/releases source, then calls base cleanup;
`00bf1240` is the scalar-deleting wrapper. This confirms transfer of the
reopen result's ownership on the normal construction path.

Verified vtable `00d68db0` has read `+24 -> 00bf1000`. That method takes
destination/requested/optional-actual on the stack, RET0Ch. It forwards the
requested count unchanged to source virtual+24, receives actual through a
temporary stack slot, increments current absolute position using ADD/ADC,
and reports actual to the caller when supplied. EAX also contains that actual
count at return. It does **not** compare against stored end, seek on each
read, transform bytes or check source failure. The temporary actual slot is
the incoming request slot, so a source that fails to write it leaves the
requested count as the apparent actual count.

Thus the large route must retain the distinction between a transformed-memory
source and a reopened logical source. If reopening returns the same encoded
bytes used initially, its results can differ from reading transformed memory
at that offset. The absence of an installed archive fixture prevents deciding
whether real packages avoid that combination or another provider changes the
source. Substituting transformed bytes would change the observed route.

## Implemented owning index and materialization

`include/bsp/mpkg_archive.hpp` and `src/mpkg_archive.cpp` provide an owning
`MpkgArchive`. `load_00bb9920_fragment` consumes the entire fully initialized
backing of a supplied `MemoryStream`, preserving its cursor. It retains new
transformed backing, EOCD/directory metadata, ordered entries and an optional
`MpkgReopenSource` callback. `plan_entry_00bb8d60_fragment` performs the
length-gated case-insensitive lookup and lazy local-length resolution,
producing a scalar snapshot:

```text
MpkgEntryReadPlan {
    route: raw_inflate_from_decoded | copy_from_decoded | reopen_original_range
    source_offset, compressed_size, decoded_size
    method
}
```

The archive retains backing rather than putting ownership in the scalar plan.
`open_entry_00bb8d60_fragment` materializes the selected route into independent
`MemoryStream` backing using the real byte-copy helper `00befa40`. The first
route supplies `InflateStreamDescriptor` and a cloned memory source with an
independent cursor to the existing inflater. Inflate failures report actual
prefix count, host status and zlib result in the error string while preserving
the output stream. The small stored route copies the transformed extent.

The large route requires an explicit original-path reopen callback returning
a newly owned `InflateSource` with its own cursor. Missing/null callbacks fail
before entry-buffer allocation. The adapter seeks that source and reads only
the declared count, accepting positive short reads until complete and rejecting
zero progress, failures and reported counts above the request. It never falls
back to transformed backing. This bound and complete buffering are added host
behavior: native large opening returns a live unbounded source reader.
The reopened source is released after materialization. Callback and ordinary
allocation exceptions propagate; guarded format/source errors return false.
The subsequent [mount/reopening audit](MPKG_MOUNT_INTEGRATION.md) clarifies
that native code reopens the original logical path through the current VFS.
It does not pin the original physical provider or immutable bytes. A mount
adapter must bind this callback to current aliases/providers. The synthetic
fixture supplies the original encoded vector as that callback's chosen source.

The bounded parser rejects incomplete supplied backing, missing scan
signature, invalid signed-size domain, out-of-bounds fixed/name/extra/comment
reads and out-of-bounds resolved transformed entry extents. Those are host
failure outcomes in place of unchecked memory access. It preserves the
native marker scan endpoint, directory seek, global-index remainder, record
order, lazy resolution and method routing. It adds no unobserved signature
checks, local/central consistency checks, method whitelists or automatic prefix
correction. A future stricter archive validator would be a separate policy.

Name comparison uses bounded `_strnicmp` after exact stored-length equality,
which retains native `_stricmp` stopping behavior, including embedded NUL,
without reading beyond the supplied string view. CRT locale behavior remains;
no path normalization or code-page conversion is added. Plans and output
streams stay unchanged on guarded failure. A failed payload-extent check can
leave the successfully resolved local offset cached, as native resolution
caches before later read operations.

Materialization requires decoded size 1..INT32_MAX because the existing copy
helper does not support empty backing. Zero-size entry metadata remains
indexable, findable and plannable. Large original-source extents are bounded
by requests/read counts on that separate source rather than its transformed
backing's length. Failed reopening can affect an external cursor but preserves
the output stream. Parent integration owns CMake/probes, VFS wiring and build
verification. Native malformed-input, allocation, concurrency and live large
reader behavior remain unsupported.

The integrated minimal synthetic probe builds decoded local headers, payloads,
directory records and an end record, then writes encoded bytes by inverse
permutation `encoded[j] = decoded[i] XOR K[j % 537]`. A nonzero final remainder
with nonzero phase, differing local/central extra lengths, duplicate names and
a raw-DEFLATE entry exercise the main contract in one fabricated archive.
A large stored record reopens the same original encoded fixture and compares
its encoded subrange, which differs from the transformed payload, to verify source identity. Generated fixtures are
not installed-game archive evidence.

## Validation boundary

A recursive read-only `rg --files --hidden --no-ignore` search of the installed
game root, excluding `.git`, found no `.mpkg`, `.zip` or `.pak` files. The
command returned no matches and no errors; its arguments/result are saved in
`installed_archive_search.json`. This is current local enumeration, not proof
that archives cannot exist inside other providers or another installation.

All listed raw spans matched the original PE. Complete raw continuations
were checked after misleading `_free` no-return annotations in the marker
scanner, local offset resolver and directory loader. `00bf1000` was undefined
in Ghidra and was inspected directly from raw assembly. The audit itself
decoded no archive fixture and needed no build. Source has subsequently been
reconstructed as the bounded host implementation above. MSVC Win32, both
existing CTests and the full D3D9 probe now pass. The one synthetic archive
exercises all three routes, including a 637-byte dynamic-Huffman stream that
expands to 159,744 bytes, and independently framed directory/transform quirks.
See [fixture scope](MPKG_FIXTURE.md) and
[integration results](../reports/parallel_entry_validation.json); no native differential,
ABI-compatible or game-validated MPKG loading is claimed.

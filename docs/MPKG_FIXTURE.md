# One synthetic MPKG archive fixture

Addresses: `00bb9920`, `00bb8d60`, `00befa40`

`probe_mpkg_archive()` in `src/mpkg_archive_probe.cpp` supplies one synthetic
archive scenario to the existing D3D9 probe. It checks the three buffered host
entry routes, ordered case-insensitive lookup, lazy local lengths, the native
directory/prefix distinction, the final transform block, and usable outputs
after archive and explicit original-source owners are destroyed. It creates no installed archive and
does not use an installed game file as fixture input.

The framing follows the independently audited byte offsets in
[MPKG_ENTRY_LOADING.md](MPKG_ENTRY_LOADING.md). Neither fixture generation nor
the expected payloads call or read the archive implementation. Local and
central headers are assembled directly into a decoded byte vector; an inverse
of the audited transform creates its encoded source. The key is copied from
`reports/mpkg_entry_loading_audit.json`, address `00e144f0`, 537 bytes, SHA-256
`26abd3e2995fdbacecb6f352b5c0eb20ff20c9a7ada6312ba619c9d4c6fa6183`.

## Framing and independent payloads

The directory contains these four ordered records. Names are deliberately
different from their local names; the first record also has local extra length
5 versus central extra length 1. The first plan therefore must replace its
provisional central-length offset with local data offset 84.

| Central name | Local header offset | Local data offset | Method | Source bytes | Decoded bytes / expected output |
|---|---:|---:|---:|---:|---|
| `Case.TXT` | 29 | 84 | 0 | 26 | ASCII `first small decoded entry` plus LF |
| `cASE.tXT` | 110 | 149 | 0 | 23 | ASCII `duplicate must not win` plus LF; not selected |
| `Packed.TXT` | 172 | 208 | 8 | 637 | 159,744 known ASCII bytes |
| `Large.BIN` | 845 | 892 | 0 | 262,181 | Original **encoded** source bytes `[892,263073)` |

Queries use `cAsE.TxT`, `pACKED.txt`, and `lARGE.bin`, with flags 2. The first
query matches both equal-length duplicate names but must return the first
record's independent 26-byte oracle. Plans must report the expected route,
source offset, method and compressed/decoded counts before opening.
Opening thus uses an already resolved local offset; direct opening with an
unresolved offset is not independently exercised by this scenario.

The large entry's decoded framing payload is the deterministic byte pattern
`(i * 17 + 31) & 255`. Its expected opened output is instead copied directly
from the completed original encoded fixture at the known local data offset.
The fixture explicitly verifies these two byte sequences differ. The reopen
callback returns a new `OriginalEncodedSource` with its own cursor over that
same original encoded vector. It records source identity, source size, reopen
and seek/read counts, requested bytes and actual bytes. It does not substitute
transformed backing or invent an actual count.

The compressed payload was generated once with Python 3.13.11 (Anaconda),
zlib compile version 1.2.13 and runtime version 1.3.1:

```python
phrase = b"MPKG independent raw-DEFLATE fixture: directory offsets and stream ownership.\n"
plain = phrase * 2048
c = zlib.compressobj(9, zlib.DEFLATED, -15, 8, zlib.Z_DEFAULT_STRATEGY)
compressed = c.compress(plain) + c.flush()
```

The fixed blob embedded in C++ is 637 bytes. Its first block has BFINAL 1 and
BTYPE 2 (dynamic Huffman). The output oracle repeats the literal 78-byte
phrase; it is not obtained by running the archive implementation or its
inflater. The 159,744-byte output crosses the inflater's 64-KiB output boundary.

- Compressed SHA-256: `707aac721ff2608596dc6553c4ed96ae9d3bbc2a93a889508164bb02e7fdd514`.
- Plain SHA-256: `4a61cc7deb97c823c6a5fc5e94366450a94d25b3f6a5b7f50757eaf822bc91fb`.

## Prefix and final-block sensitivity

The fixture has 263,567 bytes. Central records begin at direct offset 263,073
and occupy 223 bytes. Another 249 padding bytes precede EOCD at 263,545. EOCD
declares central size 443, including 220 padding bytes, so:

```text
263545 - 263073 - 443 = 29
```

The resulting 29-byte adjustment applies only to relative local-header
offsets. Seeking the directory at `EOCD - central_size` would incorrectly
start 29 bytes into the first directory header. Padding therefore does not
cancel the prefix check.

The last transform block is 17 bytes, with its global starting index modulo
17 equal to 16. It contains the consumed EOCD count, central size and offset.
An independent Python byte-framing calculation found that a naive final-block
reversal would produce entry count 47,872 and directory offset 1,027. The
partial-block check thus affects parsing rather than only unused tail bytes.
The inverse encoder assigns each encoded position exactly once in that
calculation.

Independent framing identities, computed without the archive implementation:

- Decoded fixture SHA-256: `b716c3303cee9efa17d3a7869bf47d6397f52e203f94aa53ea9e039de57fe200`.
- Encoded fixture SHA-256: `a7de489986d4ab83d2d4492d38d75a0f060c234fbd18ffc61f3fcff14ee15868`.
- Large encoded output SHA-256: `b93b2a64659363f4854b74523a6379a8fefd5f9cb50dfcd467910568c60dd63f`.

## Ownership and validation boundary

The encoded `MemoryStream` begins at cursor 7, which archive loading must
preserve. The archive and all original/reopened source owners leave scope
before output validation. Weak references then establish destruction of the
original vector and reopened source. Each output must remain fully initialized
at cursor zero, report its exact expected size, and read back the independent
oracle with its actual count. Seven extra requested bytes remain untouched
after the memory stream's normal EOF clamp.

The embedded key/blob identity and framing were checked against the audit and
independent Python generator, and another worker reviewed the expectations.
Integration into the existing probe now passes MSVC Win32 compilation, both
existing CTests and execution of this scenario: four entries, all three routes,
one original-source reopen and 262,181 requested/actual large-entry bytes.
See [integration results](../reports/parallel_entry_validation.json).
No additional test target or malformed-input suite is added. The lifetime
observations do not distinguish a copied transformed backing from a retained
shared view; source inspection establishes use of the independent-copy helper.

This is a bounded synthetic host fixture. It does not establish native
differential execution, real installed MPKG loading, VFS provider registration,
game rendering, native source/allocator ABI, malformed-input equivalence or
native large-reader behavior beyond its declared range. The host archive
materializes all three outputs; native large entries retain an unclamped reader,
as separately documented in the entry-loading audit.

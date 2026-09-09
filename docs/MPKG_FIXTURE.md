# One synthetic MPKG archive fixture

Addresses: `00bb9920`, `00bb8d60`, `00befa40`, `0073cb10`, `0073d881`, `0073d888`, `00bdb040`, `00bdb120`, `00be1890`, `00be80b0`, `00be6480`, `00bb97b0`

`probe_mpkg_archive()` in `src/mpkg_archive_probe.cpp` supplies one synthetic
archive scenario to the existing D3D9 probe. It checks the three buffered host
entry routes, ordered case-insensitive lookup, lazy local lengths, the native
directory/prefix distinction, the final transform block, and usable outputs
after archive and explicit original-source owners are destroyed. The same probe
also frames a small nested archive to exercise real factory registration and
the two fresh package scans. It creates no installed archive and
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

The framing helper now accepts a vector of records for reuse by the nested
scan portion. The original four-record call still supplies exactly the fields
above, including the explicit 159,744-byte decoded count for the compressed
entry. An independent Python framing calculation using the preserved key/blob
confirmed the same original size, offsets and hashes after this refactor; it
did not execute the C++ generator.

Independent framing identities, computed without the archive implementation:

- Decoded fixture SHA-256: `b716c3303cee9efa17d3a7869bf47d6397f52e203f94aa53ea9e039de57fe200`.
- Encoded fixture SHA-256: `a7de489986d4ab83d2d4492d38d75a0f060c234fbd18ffc61f3fcff14ee15868`.
- Large encoded output SHA-256: `b93b2a64659363f4854b74523a6379a8fefd5f9cb50dfcd467910568c60dd63f`.

## Two fresh scans through the concrete manager

`package_scan_fixture()` adds real nested encoded bytes within this existing
probe. The 770-byte inner archive contains one stored `marker.txt` with the
independent ASCII oracle `opened through the second freshly mounted package`
plus LF. The 1,523-byte outer archive contains those **encoded inner bytes**
in one stored entry named `./inner.mpkg`. Both local data offsets are 65.
The outer entry is below `0x40000`, so opening it uses the decoded small-copy
route; this avoids substituting decoded outer bytes for an encoded inner file.

Independent framing SHA-256 identities:

- Inner encoded: `3f3bd127b0cb1110eacc4485fcfc298897d1b2abe192621ca547d4b5fb6cbbb9`.
- Outer encoded: `d1db2f5ea1efc9512cb74cc1f94b0c1bfce4791e2d51ad8e776505250a5a927e`.

Two `VfsProviderManager` instances share one actual `VfsProviderFactories`.
Mounting `filestore` and `FILESTORE` verifies shared provider/stream-store
identity, its empty native system name, and overwrite of its shared device ID
from supplied fixture value 3 to 7. The empty system-name query finds it;
the factory word `filestore` does not. A peer-only physical provider checks
the trailing-backslash factory gate, copied system name, supplied device ID 8
and ownership byte 1. Its synthetic root is never opened or enumerated; factory
construction does not check filesystem existence. These IDs are fixture inputs,
not claims about actual startup device assignments.

The main manager's FileStore receives three names, all backed by owned memory
streams initially at cursor 7:

| Stored logical name | Bytes | Intended outcome |
|---|---|---|
| `./patch2.mpkg` | Valid encoded outer archive | Construct and mount MPKG |
| `./bad.mpkg` | ASCII `not an MPKG archive` plus LF | Report the host malformed-input guard |
| `.mpkg` | Valid outer bytes | Decline at the native factory's length-greater-than-five gate |

FileStore's path normalization preserves `./`, so its `.` query matches these
names and manager opening can use their unchanged logical spelling. The
scanner callbacks invoke actual manager enumeration, system-name lookup, and
factory construction/registration. The expected processing order is:

| Pass | Ordered names and dispositions |
|---|---|
| First | `./bad.mpkg` failed; `./patch2.mpkg` mounted; `.mpkg` declined |
| Second | `./inner.mpkg` mounted; `./bad.mpkg` failed; `./patch2.mpkg` already mounted; `.mpkg` declined |

The first list remains fixed while its outer mount is added. The second query
sees the outer provider before FileStore and discovers `./inner.mpkg`; it skips
the outer by unchanged provider system name. The callback trace expects exactly
two queries, seven lookups and six mount requests. Every computed priority is
1000, including `./patch2.mpkg`: `patch` does not begin the whole name. Actual
mount calls must receive prefix `.`, ownership zero and device ID -1.

The startup helper deliberately returns false for these recorded failures, but
both passes finish. Successful outer/inner registrations survive; declines and
failures preserve registration count and the caller's output provider pointer.
The fixture checks copied MPKG system names, case-insensitive lookup of the
outer spelling, rejection of basename-only `inner.mpkg`, and unchanged retained
source cursors. It opens `marker.txt` through the current manager and verifies
its copied output after both managers leave scope. Per-entry logs give pass,
logical name, priority, mount arguments and disposition; skipped entries are
included in those logs but do not issue a mount request.

This checks FileStore ordering and cross-provider encounter order for these
specific names. The nested archives each have only one directory record, so
the new portion does not independently exercise multi-entry archive enumeration
order or duplicate spelling within that enumeration. The malformed archive
outcome is an explicit host guard, not native malformed-input equivalence.
Live alias changes, recursive reopening and expired-context errors remain
covered by the earlier mounted adapter portion of the fixture; this new manager
portion does not independently repeat its large-entry reopening checks.

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

The coordinated MSVC Win32 build and both existing CTests passed in
[startup_manager_build.txt](../reports/startup_manager_build.txt). The full
[startup_manager_probe.txt](../reports/startup_manager_probe.txt) records all
seven scanner dispositions above, outer/inner sizes 1,523/770, exactly two
queries, seven lookups and six mount requests, with shared identity and
nested discovery/failure continuation checks equal to 1. The original scenario
still reports 263,567 bytes, four entries, all three routes and its successful
262,181-byte original-source read. These are executed host fixture results;
the independent SHA-256 framing calculation remains a separate check.

This is a bounded synthetic host fixture. It does not establish native
differential execution, real installed MPKG loading, native provider ABI,
game rendering, native source/allocator ABI, malformed-input equivalence or
native large-reader behavior beyond its declared range. The host archive
materializes all three outputs; native large entries retain an unclamped reader,
as separately documented in the entry-loading audit.

# CC12 physical write: independent primary review

**PASS** for the bounded raw ABI fragment in Source packet
`776117627d7cc04a1adee8ac279d271e7543683b`. This review independently parses and
rehashes retained offline evidence. It neither rebuilds nor executes the probe,
queries Original addresses, or changes Source/runtime/Ghidra state. The machine
receipt is [cc12_physical_write_primary_review.json](../reports/cc12_physical_write_primary_review.json).

The complete `00BF4F50..00BF4F86` body is 55 bytes and 21 instructions.
Original, normal production COFF section 49, and the unique linked body at
`20001030` agree except the sole four-byte operand at offset 26: COFF DIR32
`__imp__WriteFile@20`, Original IAT `00CE2290`, linked IAT `2001F024`.
Both PE imports identify `KERNEL32.dll!WriteFile`; the map identifies the genuine
kernel32 import provider. The retained probe checks its actual loaded IAT against
`GetProcAddress`, the complete loaded code, and the unmodified Original RX copy
before the first entry call, then checks the same bodies and IAT cells afterward.

The caller's requested DWORD at entry ESP+8 is the actual API count destination.
The raw entry does not initialize it, test the API BOOL, retry, or call a failure
callback. After the API it reads that DWORD into EAX, performs ADD/ADC on cache
words +10/+14, restores ESI, and writes the optional count through returned ECX.
RET0Ch, optional-output order, and the carry-preserving intervening MOV match.
EDX is API-clobbered and differs between the successful pairs; no EDX or EFLAGS
equivalence is accepted.

The genuine current Source constructor is exactly 43 bytes/11 instructions,
without relocation or normalization, at the unique linked address `20001000`.
It supplies six separately reset and checked guarded backings, retaining literal
Original profile values. No fabricated Source class or function table is used.
There are three fresh Source/Original pairs and six independently created real
files, with distinct simultaneously live handles in each pair:

| Case | Retained observation |
|---|---|
| Null output | Count 8; cache `ffffffff:fffffffc -> 00000000:00000004`; OS cursor `5 -> 13`. |
| Output aliases cache low | Count 5; cache `12345678:fffffffe -> 12345679:00000005`; OS cursor `3 -> 8`. The intermediate low value 3 is inferred from exact instructions, not separately observed. |
| Invalid backing HANDLE | Requested 7 becomes count 0/error 6; cache `89abcdef:76543210`, control-file cursor 11, and all file bytes stay unchanged. Both actual backing HANDLEs are `ffffffff`; printed handle columns identify valid control files. |

Probe inspection confirms checks of the full 64-byte backing and guards, payload
and count guards, actual caller requested stack slot, ECX against its own optional
pointer, ESP cleanup, EBX/ESI/EDI/EBP, file length/cursor/full contents, immediate
LastError observations, and deletion on close. The retained `run01` is the sole
execution family; there is no historical fixture replay. Static proof timestamp
`2026-10-08T10:48:21.946178+00:00` precedes probe start
`2026-10-08T10:48:21.948711+00:00`.

All 479 manifest pre/copy/post rows agree with 308 independently rehashed frozen
files. The complete 22,700-byte object has 93 sections, 52 code sections, 3,627
code bytes, and 135 code relocations. Its exact bytes occur in one archive member
at offset 57,957,096 among 1,889 members. The frozen 209,408-byte executable and
whole 73,558,886-byte archive also match their pins. All six frozen production
sources match the packet commit after Git text newline normalization; physical
artifact hash comparisons remain byte-exact.

The consumed-header and searched-library sets reproduce the compiler/linker logs:
164 headers and 19 libraries have retained physical copies. The driver records
6,030 header and 550 library candidates, leaving 5,866 and 531 unused respectively.
Unused candidate hash dictionaries are transient, so this review accepts their
retained counts/recipe evidence and does not claim independently archived hashes
or physical copies for every candidate. Tools, backends, selected inputs, genuine
Windows SDK WriteFile declaration, normal build recipes, object/archive/member,
and frozen executable identities were reviewed.

The Win32 build and three existing CTests passed in the retained evidence. The
initial zero-archive link failed with ten unresolved externals and remains
preserved; the successful link uses genuine core/Lua/zlib archives and two explicit
objects, with no probe stubs or adapters. The embedded manifest is `asInvoker`.
Two independent audit assertion mistakes (Git CRLF text and the existing Python
cache directory) were corrected; all failed audit logs remain in the unique
ignored review family. They did not rerun the production fixture.

This accepts one raw ABI fragment and zero new Original functions. It does not
admit the physical-stream class/table/lifetime, raw substreams, Source read
`BF5030`, ordinary linked providers, broad alias domains, asynchronous/partial
I/O, or game behavior. Later root build/member checks are separate from this
frozen worker review. Full independent parser, pin receipts, and retained log
copies are under `local/cc12_physical_write_primary_review_20261008a/` in the
review worktree; the authoritative worker frozen family is named in the JSON.

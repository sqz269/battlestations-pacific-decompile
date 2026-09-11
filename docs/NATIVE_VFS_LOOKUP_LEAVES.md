# Native VFS lookup leaves

This packet reconstructs eight complete native functions, 605 bytes, against actual caller-supplied FileStore, MPKG and MSAR storage. These are prerequisites for further VFS lookup consumers. They do not complete a cache loader, general VFS traversal, archive construction, or stream ownership. Names are descriptive hypotheses. The C++ interfaces are not binary ABI replacements and have not been game validated.

| Address / complete bytes | Original ABI | Reconstructed behavior |
| --- | --- | --- |
| BE5C00 / 62 | ECX FileStore; stack name; RET4; AL Boolean | Primary tree contains |
| BE5C40 / 62 | ECX FileStore; stack name; RET4; AL Boolean | Same contains query at provider slot +18; does not rewrite name |
| BE6040 / 80 | ECX unused; stack output, name; RET8; EAX output | FileStore slot +1C name copy |
| BB5540 / 80 | ECX unused; stack output, name; RET8; EAX output | Shared package/base slot +1C name copy |
| BB8E00 / 108 | ECX archive state; stack name; RET4; AL Boolean | Ordered MPKG row lookup |
| BB8E80 / 8 | ECX provider; stack name; tail jump inherits RET4 | Load state pointer at +14, tail-call BB8E00 |
| BBA650 / 182 | ECX provider; stack name; RET4; EAX index | MSAR row index or FFFFFFFF |
| BBA710 / 23 | ECX provider; stack name; RET4; AL Boolean | BBA650 result differs from FFFFFFFF |

The current profiles are D689E8 (FileStore: +10 BE5C00, +18 BE5C40, +1C BE6040), D64390 (MPKG: +10 BB8E80, +1C BB5540), D643C4 (MSAR: +10 BBA710, +1C BB5540), and D641A0 (base: +1C BB5540). All eleven words of these tables are pinned. D69168 is pinned to distinguish its already-reconstructed physical methods; this packet does not dispatch or replace them. FileStore constructor BE7FA0, MPKG constructor BB9CB0 and MSAR constructor BBB5B0 establish the respective storage relationships. This does not establish normal startup reachability of the latent MSAR profile.

FileStore passes its actual primary tree at provider+14 to the existing full BE5A50 finder, including BE54D0 lower-bound and 443D00 actual-header comparison. Both contains leaves read the returned owner, capture current tree+4 head, then validate the owner through the established returning CRT boundary. They compare the current returned node with that captured head. This differs from the date leaf BE5C80, which captures its head before zeroing date output and finding. Neither contains leaf modifies the name or tree.

The MPKG provider stores a pointer at +14 to a separate 34h archive state. BB9CB0 allocates that state and calls BB9920 before publishing it. BB8E00 reads the state's array pointer at +28 and signed count at +2C; records have stride 24h with a length/data header at +0/+4. A nonpositive count returns false before reading the query or array. Each iteration rereads the current array and both lengths. Equal zero lengths match without reading either data pointer. Other equal lengths call current CRT `_stricmp(query, row)`. Misses advance the DWORD offset/index and compare against the current signed count. No sorting, path conversion, parsed-state projection or row snapshot is introduced.

MSAR has an inline vector: begin/end/capacity at provider+1C/+20/+24, with stride 18h. The capacity is not read. BBA650 computes `(int32)(end - begin) / 24` using DWORD subtraction and signed truncation toward zero, then compares the index as unsigned. A null begin skips the end read and ends the search. Each row performs the count calculation twice: the first controls the loop, the second controls CRT validation. If that handler returns, execution rereads begin and continues into the row. Row length precedes query length; equal zero lengths match without data reads, otherwise the call order is `_stricmp(row, query)`. The first matching row wins. This source preserves current reads and the returning-handler continuation; stable-storage comparison cases do not exercise a concurrent bounds change between those reads.

BE6040 and BB5540 share the already-complete actual-header copy semantics of 426060. They compare header identity, zero output length and pointer before taking the identity branch, and abandon any preexisting output storage. A distinct source resizes through the actual 419CC0 pool, then rereads the source length and current destination length/source data/destination data before copying. ECX is unused: these methods do not read a provider root. There is no initial-copy cleanup scope to add if allocation fails. The exported API requires `ActualNativeStringPoolStorage`; it does not accept the separate `SizedStoragePool` alias-list interface.

Reached source dependencies are the complete raw tree routines in `native_vfs_date_leaf_providers.cpp`, raw 426060 in `native_pooled_string_substring.cpp`, 41DD40 in `native_string.cpp`, and the actual pool getter/storage/lifetime implementations. Native CRT calls remain the established host `_stricmp`, `memcpy`, and returning invalid-parameter boundaries. Inherited limits include the raw string helper's omission of a zero-byte `memcpy`, its C++ exception interface rather than original SEH, and the actual storage interface's `noexcept` release boundary. Invalid raw pointers, impossible allocation sizes, invalid unterminated nonempty names and unsynchronized concurrent mutation do not gain validation or a portable C++ promise here. Zero-length headers can retain unusable data pointers because native lookup does not read them.

The focused ignored probe links the actual `bsp_core.lib` and executes unchanged original code for all eight functions plus the three FileStore search dependencies: eleven bodies, 871 bytes. Relative calls share one relocation delta; only external CRT and actual resize call targets are bridged. It compares returned values and the full 2048-byte record arena. Copies also compare the actual pool prefix through offset 8AD484 at identical addresses, preserving the live OS critical section. Cases cover case-insensitive hits, misses, zero headers with unusable data, empty storage, signed MPKG counts, MSAR truncated row count, duplicate-first behavior, recorded high-bit lengths, identity copies, empty copies, and abandonment of an unusable old output. This validates composition with the actual resize/pool implementation; it does not independently execute original pool code or native exception unwinding.

`reports/native_vfs_lookup_leaves_audit.json` records fresh guarded original spans, current source and build artifacts, native profiles, the strict Win32 build, two existing CTests, eight seed checks, and the focused probe result. At worker handoff, shared Ghidra metadata and main source registration were unchanged. The primary subsequently registered all eight entries and completed the validation below.

The original consumer investigation remains bounded and incomplete. B1A4F0 is pinned as a complete 1338-byte native consumer but is not reconstructed here. Its current virtual +4/+8/+C resource routes and BECCD0 message-pump policy need concrete dependency closure. Its direct append B1A3C0 requires 4DA180 reserve, 4D6F70 record copy and 4D45A0 destruction, including hidden post-free publication/cleanup paths obscured by no-return annotations. Existing B305F0/B30B40 cache analogues do not remove these requirements. The renderer pollers B21F70/B22030 retain the callback dependencies documented in the prior renderer reload discovery. Existing A84740 sound-cache and B31090 effect-cache interfaces remain qualified host/provider domains.

The next ownership prerequisite should bind the actual pool through the existing alias chain: 4D48A0 copy-list -> 4D26A0 insertion -> 4CE6F0 node allocation -> 44BCB0 string copy, plus 4D05E0 clear and 4D0A10 destruction. Current APIs expose `SizedStoragePool&`; 4CE780 count growth, 4C3020 sentinel allocation, CRT allocation/free, returning iterator validation and native exception order must remain intact during any coordinated interface work. Then the concrete B1A3C0/4DA180/4D6F70/4D45A0 record closure can be recovered without substituting a pool or resource callback. No renderer, node or getter files were claimed for that future packet.

These leaves also prepare the separate BDD440 membership visitor (D68398, provider +10) and BDD600 name-probe visitor (D683E8, provider +18). The current BDD0A0 source is explicitly qualified to the D683B0 date visitor. Extending those visitor domains requires their own full callback, stop, temporary ownership and dispatch packet; no general traversal closure is claimed by this leaf implementation.


## Primary main-library integration

Strict MSVC Win32 compilation, both existing CTests and eight fresh seeds
passed. The primary verified 47 worker pins and 22 fresh guarded spans,
3,117 bytes including all 605 owned bytes. The unchanged fixture linked the
actual main archive and passed 50 original comparisons: 42 lookups and eight
copies over eleven original bodies totaling 871 bytes. All 102,400 arena bytes
and 72,786,976 actual owning-pool prefix bytes across the eight copy pairs
matched, as did returned pointers and values. Pool/string/CRT bridges retain
the existing rebuilt-provider boundary; original provider internals are not
independently replayed.

Seven exact archive members and 203 complete COFF sections were verified
against linked bytes: 14,619 bytes and 626 relocations. All eight owned entries
are retained separately. No runtime code postimage or native exception test
was added. Three missing saved functions (BE5C40, BE6040 and BBA710) were
defined from their complete bytes; previous names/comments were preserved,
new descriptive names applied where needed, evidence saved and exports
refreshed. Other visitor domains remain separate packets.

The primary library SHA256 is `51b97fe1c5170423d42db1cf27bc0ad0e6476626593ed722a4a6ecb4c10c16a0`. The read-only bundle is
`local/vfs_lookup_leaves_primary/`, seal `79601243c10696ba45ec44bbb5a7f33e33bf70ee1ccf5c028cfeeac1120a63e9`.
Evidence is recorded in `reports/native_vfs_lookup_leaves_audit.json`.

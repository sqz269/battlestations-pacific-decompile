# Native entity intrusive unlink Source (CC12)

Two complete raw Source entries now preserve Native `00903F30` and `00924710`.
Each emits **83 bytes / 29 instructions**, exactly equal to its whole installed
Native body, with **zero relocations**. One normal Win32 build passed all three
existing checks. The complete new object appears exactly once in `bsp_core.lib`.
This is a raw helper implementation; the actual World/entity owner remains
**unready**, and no production caller or game link inclusion was added.

The worker awards **zero new Original-function ledger credit**. `00903F30`
already has the semantic `chain_unlink_00903f30` reconstruction; Root will
deduplicate/enrich that record and check `00924710` before adding any record.
Two new raw Source definitions do not imply two newly reconstructed Original
functions. Existing Ghidra names remain hypotheses and were not edited.

## Source and physical calling contract

Files are `include/bsp/native_entity_intrusive_unlink.hpp` and
`src/native_entity_intrusive_unlink.cpp`. The definitions use MSVC Win32 naked
inline assembly and an explicit fastcall facade: ECX is the actual **0x0C / 12-byte**
header, EDX is an unused padding formal, and the actual entity pointer occupies
one stack word at entry ESP+4. RET4 consumes that word; EAX returns the same
entity. ECX and EBX/EBP/ESI/EDI survive; EDX is volatile.

| Source entry | Native extent | Entity links | Source section | SHA256 |
| --- | --- | --- | ---: | --- |
| `unlink_native_entity_world_chain_00903f30` | `[00903F30,00903F83)` | previous+34, next+38 | 4 | `1eb60de632316963b95f9385620d97fe53bb5188cd20ac3cc593f988489e4dc4` |
| `unlink_native_entity_sibling_chain_00924710` | `[00924710,00924763)` | previous+40, next+44 | 3 | `5662da33dd4d146824cdadff4440b6b95f0c20d14d1293674b9bff17bfbe6d31` |

The actual header is `first+0,last+4,count+8`. It is distinct from the embedded
World category-root `count,head,tail` layout. The caller supplies actual physical
storage and lifetime for every access reached. No vector/handle projection,
membership check, World cast, null/bounds guard, allocation or callback is added.

If both entity links are zero, the routine skips mutation **only when signed
count > 1**. Count 1, zero and negative values follow the mutation path. It
updates the prior next link or header first, reloads the actual entity next
link, updates that node's previous link or header last, clears entity **next
before previous**, then performs `ADD DWORD [ECX+8],-1`. This preserves the
Native DWORD wrap and final ADD flags, including carry; replacing it with DEC
would change the contract. The skip path retains `CMP count,1` flags. Exact
reload/overlap order is preserved. No instruction touches x87, SSE or DF.

This matches the static Native register/stack schedule; no packet entry was
executed and no drop-in executable replacement, Native SEH or hardware-fault
integration is claimed.

## One normal build and frozen consumed inputs

The baseline is `4cf096d4617d3f79ab3066597d7b9554346808fb`. Root owns the single registration in
`CMakeLists.txt`, staging commit `4cf096d4617d3f79ab3066597d7b9554346808fb`:
`target_sources(bsp_core PRIVATE src/native_entity_intrusive_unlink.cpp)`.
It was merged before pre-build capture; `cmake/startup.cmake` was untouched.
The generated project contains exactly one corresponding compilation item.

Pre-build capture ended `2026-10-08T22:38:19.562337+00:00`. Every captured physical input
was rehashed before the single `scripts/build.ps1` invocation, which ran from
`2026-10-08T22:38:56.858560+00:00` to `2026-10-08T22:39:29.491715+00:00` and returned 0. Source and
the registration recipe were unchanged afterwards. No build retry occurred.

The new translation unit has **14 actual inputs**: its Cpp compilation key,
the new Hpp and 12 standard/compiler headers recorded by MSVC. Every input was
physically retained before compilation and independently matched afterwards.
The actual configured compiler/frontend/code generator/librarian/linker and
normal-build orchestration tools, command/read/write records, generated
project, recipes and logs are retained. Tool binaries are physical provenance,
not a loaded-module trace. The existing `CL.write` record groups six core TUs
under one pipe-delimited batch key; the new object's write is checked in that
batch while input closure remains specific to this new TU.

`reconstructed_math`, `native_math_differential` and `tool_tests` all passed.
These are the normal script's three existing checks; no test was added. No
ad hoc probe or game was launched, and neither new Source/Native packet entry
was invoked. The retained normal log includes unrelated target rebuilds and
their existing linker warning; no complete input-admission claim is made for
every unrelated target in that build.

## Complete COFF and archive provenance

The actual object is **1073 bytes**, SHA256
`375265be3c3543c9bd1bf74ec7e00b810fd4c0251c9066676379418c6c52f506`. All 5 COFF sections
are retained. Its only two nonempty executable sections contain precisely
**166 bytes / 58 instructions**. Every byte and every instruction start matches
the corresponding complete Native extent, independently recovered from the
current unchanged installed PE and the accepted readiness family. Neither
section has a relocation, external data operand or CALL.

The whole current core archive is 73670972 bytes, SHA256
`b84e7d1c9c28e4a4bca3a2155dc2cbea8416735543ba20dff6b900c77bde0eda`. The unique member is
`bsp_core.dir\Release\native_entity_intrusive_unlink.obj`, with payload file offset
63742364. Its complete
1073-byte payload equals the actual object, not merely a symbol,
section number or folded code range. A whole-archive symbol-definition scan
finds each new function defined once by that same physical member. The positive
mapping is Native extent to full Source section to complete object to exact
archive member; no relocation target mapping is needed because there are none.

The current Source scan finds only the two declarations and two definitions.
The normal game map contains neither symbol nor the new member. Therefore this
packet explicitly has **no game-linked consumer and no runtime integration**.
It does not replace the semantic handle helper or add a World constructor,
entity publication, attachment, placement, class callback or destructor caller.
Whole `004CB030` ownership/lifetime remains the production frontier.

## Validation records and retained family

Two evidence-reader assertions were corrected after the successful build:
the first assumed a single-file `CL.write` key, and the second assumed `CL.read`
repeated its Cpp key among header rows. Both failure records and their fixes
are retained; they changed only the verifier. The third verification passed.
Source and build outputs were unchanged and no build was repeated.

The first ZIP inventory check also exposed a manifest-filter error: a nested
accepted-readiness `MANIFEST.json` was omitted from the inventory although it
was present in the ZIP. Its CRC passed. The failed ZIP and failure receipt are
retained separately; the final inventory excludes only its own outer manifest.

Evidence: `local/cc12_native_entity_intrusive_unlink_source_evidence/`. ZIP: `J:\PROG\battlestations-pacific-decompile-cc12_wake_append_source_ast\local\cc12_native_entity_intrusive_unlink_source_evidence.zip`,
126217913 bytes, SHA256 `4f48ded34799e0de881d108c34b69883e21e65ae38233f2e260747450e9caae4`. Its manifest covers
8940 files, with 8941 ZIP entries. Every ZIP
payload hash and CRC passed. The report and this document are generated outside
the sealed family to avoid a checksum cycle. Root handles independent Source
admission, ledger deduplication, Ghidra annotation and main integration.

# Native game table context owner

Packet `cc12_game_native_table_context_owner_source` adds
`bsp::game::GameNativeGameTables`, a stable Source owner for one retained
`NativeGameTablesContext`. This closes the table-storage dependency identified in
the [application-context audit](CC12_GAME_APPLICATION_CONSTRUCTION_CONTEXT_READINESS.md).
It does not instantiate a game or complete that application context.

The private storage is exactly 23 unit rows of 16 bytes, aligned to four bytes,
one real volatile DWORD append count initially zero, and 12 by 97 DWORD rank
output. Each context field references that owner's actual storage or the existing
authored `native_unit_conversion_definitions()` and
`native_gunnery_preference_words()` providers. The owner retains the context
itself; its count is a reference to the same mutable cell used by the builder.

Construction requires an explicit `uint32_t unit_frame_word_preimage`, without a
default. That complete value is retained for the existing builder, which preserves
its upper 24 bits and authors only the flag byte. Zero-initializing table storage
does not choose the opaque frame word. Supplying this Source input does not prove
the natural Native caller's stack preimage or ABI.

The owner is neither copyable nor movable. `borrow_construction_context()` returns
the stored context once, provided its actual append count is still zero. A prior
borrow or a nonzero count throws `std::logic_error`. There is no reset, reissue,
builder call or implicit retry in either construction or borrowing. The caller
must retain the owner through every consumer and any failed construction's
retained context/operation; destruction does not perform game cleanup.

After borrowing, raw references and context copies can still call the existing
builder more than once. The owner does not add an ABI overflow guard, intercept
raw calls, or make a second append safe. The original builder's fresh-count and
single-append requirements remain. Separate owner instances also remain separate
domains; future application composition must select and retain its one actual
owner rather than fabricate sibling storage.

The existing constructor call sites are unchanged: `004DDFA1` invokes
`008D9150`, and `004DE075` invokes `00727BD0`. The rank builder still clears each
97-word row before processing that row's authored values, including the existing
row-zero ID 97 cross-row write before the following row's clear. This packet adds
no arithmetic, native ABI, register, x87, EH, Ghidra or ledger reconstruction.

The Native symbols `00F88A50`/`00F88BC0` and `00E092C8`/`00E19BF8` identify the
established roles represented by these Source cells. They do not mean that this
new C++ object resides at original executable addresses. Authored literal pointer
identities continue to belong to the C++ image.

The normal Win32 Release build passed at coordinated commit
`94e38c65ebfc4bbfffc1e027c2ab96da7742a266` on 2026-10-08. All three existing CTests
passed: `reconstructed_math`, `native_math_differential` and `tool_tests`. The
gunnery row-clear/cross-row checks in `tests/math_tests.cpp` remain unchanged.
These checks do not exercise this new owner's borrow method or qualify current
Native/application behavior. No new test, probe or game entry execution ran.

The first build attempt at `5ec6014e` stopped during CMake configuration: the
integrator's registration referenced `bsp_game` before its deferred creation.
No Source compilation or tests ran in that attempt. The integrator corrected
only that registration to defer `target_sources`; the unchanged owner Source was
then built with a separately frozen input set. Both attempt logs and recipes are
retained. The successful attempt ran from 22:53:21 to 22:54:03 UTC.

Complete new COFF evidence retains the 124-byte constructor, 69-byte borrow method,
their compiled code/data targets, both unique authored-table/gunnery members of
the actual `bsp_core.lib`, and 98 actual compiler-read input files. Four runtime
libraries, three unique CRT members and five linked-PE import tuples are retained
as build-time bindings. They do not establish a loaded runtime DLL or execution.
The current PE map has no public entries for this owner or its two authored-data
getters, consistent with removal of unused COMDATs. This is emitted-object and
build evidence; it does not qualify a linked or instantiated application owner.

The whole R114 tested archive dated `2026-09-17T23:32:25.572801+00:00` was retained;
its archive SHA-256 and all 594 member sizes/hashes plus ZIP CRC passed. Those
historical table fixtures support the existing builders, not a new execution of
this owner. Current frozen Source and normal-build artifacts are separated under
`local/cc12_game_native_table_context_owner_source_20261008a/`. The current build
archive contains 6,985 whole files; every member size/hash and ZIP CRC passed.
The [packet report](../reports/cc12_game_native_table_context_owner_source.json)
indexes the complete archives, compiler inputs, machine receipts and final seal.

Full application context, raw GlobalConfig ownership, canonical sound-string
binding, lifetime dispatch, unwind behavior, startup and gameplay remain open.

## Independent primary review

Root separately built the unchanged Source and the corrected registrations once;
all three existing checks passed. Its complete current objects contain 2 raw
unlink functions, 14 table-owner/compiler functions, 9 authored-unit-table
functions and 19 gunnery functions. Every executable section, data section and
ordered physical COFF relocation/symbol index was retained. Both authored
getter operands resolve to their actual definitions in unique whole current
core members. The two complete raw bodies again match all 166 Native bytes
and 58 instruction starts with no relocations.

All 14 raw and 94 table-owner Cpp/header inputs matched physical precompile
images. Each TU additionally tracked Windows SORTDEFAULT.NLS and TZRES.DLL;
these four records were captured after compilation, so their precompile
identity and complete compiler filesystem reproducibility are not claimed.
The worker and primary object hashes differ where paths or metadata differ.
The normal game map still excludes the new entries; no new entry, owner or
Native ABI was executed. No additional tests were added.

Primary receipt: `local/cc12_two_owner_modules_primary_review/receipt.json`,
SHA256 `3044ee6ed2741ad8afef493248a82084ef1fc9a86c65b51857661c270bf8f991`.
Root independently rehashed all 8,940 raw and 12,514 table evidence payloads;
all 8,941 raw ZIP payload hashes and CRCs passed. The table ZIP CRC/member
checks and historical R114 fixtures remain the separately retained worker
proofs; they are not a new primary fixture execution. Rehash receipt SHA256
is `f1b644f8a9430a8f56ddaa438b23c782f017f939e45406631bf1117dd8af1ffe`.

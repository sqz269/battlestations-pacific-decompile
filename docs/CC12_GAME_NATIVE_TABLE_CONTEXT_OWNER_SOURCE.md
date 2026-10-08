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

Validation is pending the integrator-owned CMake registration and normal Win32
build. No new test, probe or game execution is authorized for this packet. The
existing three CTest checks will be used; the gunnery row-clear/cross-row checks
in `tests/math_tests.cpp` remain unchanged. Those existing checks do not exercise
this new owner's borrow method or qualify current Native/application behavior.

The whole R114 tested archive dated `2026-09-17T23:32:25.572801+00:00` was retained;
its archive SHA-256 and all 594 member sizes/hashes plus ZIP CRC passed. Those
historical table fixtures support the existing builders, not a new execution of
this owner. Current frozen Source and later normal-build artifacts are separated
under `local/cc12_game_native_table_context_owner_source_20261008a/` and indexed in
the [packet report](../reports/cc12_game_native_table_context_owner_source.json).

Full application context, raw GlobalConfig ownership, canonical sound-string
binding, lifetime dispatch, unwind behavior, startup and gameplay remain open.

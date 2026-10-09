# Unit/rank constructor context readiness

Audited Source base: `33c0a0518ff8dddedaa088bfef35d440b4a55aa7`.
This packet changes evidence only and adds no Original credit.

`GameNativeGameTables` already is the smallest genuine unit/rank constructor
context provider. `NativeGameConstructionContext` has one
`NativeGameTablesContext& tables` field, not separate unit and rank contexts.
Another provider class would duplicate the existing owner. The remaining
application work is retaining an instance, supplying the explicit native local
word preimage and borrowing its context into the complete game construction
context. That composition is not performed by this audit.

## Existing owned storage and services

| Requirement | Current provider | Remaining obligation |
| --- | --- | --- |
| Unit payload | Owned, 4-byte-aligned `23*16` byte array | Retain it through every consumer and retained failed operation. |
| Unit count | Owned volatile DWORD initialized to zero | Preserve the single native append; do not reset or replay. |
| Opaque unit local word | Required constructor argument, copied into the context | Obtain an explicit caller input; no default zero or invented native stack value. |
| Unit source definitions | Reference to static C++ `native_unit_conversion_definitions()` | Preserve the C++ literal pointer domain; it is not native image pointer identity. |
| Gunnery preferences | Pointer to static C++ `native_gunnery_preference_words()` with `12*97` authored DWORDs | Keep the verified authored values; no importer or mapped mutable original globals are needed. |
| Rank output | Owned `12*97` DWORD array | Retain actual output storage through its consumers. |
| Stable context | Owned `NativeGameTablesContext`, with copy/move deleted on its owner | Borrow once and keep the provider alive. |
| Constructor services | Existing `NativeGameConstructionCalls::call_008d9150` and `call_00727bd0` concrete defaults | Retain the surrounding game construction call service; the table owner itself does not own it. |

There are no missing dynamic raw container headers in this table domain. Unit
storage is a flat 16-byte-row buffer plus a separate count; preferences and
rank output are flat DWORD arrays. No allocation call service or profile/string
header owner is required to prepare this context.

The owner constructor only initializes its storage and reference-bearing
context from the required preimage and static C++ data accessors. It calls no
builder, reset, import, SDK function or game entry. Its one-borrow accessor
checks `context_borrowed_ || unit_count_ != 0`, throws on either condition, and
marks the context issued before returning the same stored reference. A failed
game operation does not permit reissue. That guard does not prevent direct raw
calls or context copies from replaying the append; caller discipline remains
necessary after the first borrow.

## Existing behavior and native boundaries

The unit call is already composed at `004DDFA1` inside `004DDB90`; the rank
call is already composed at `004DE075`. Their concrete call methods forward
the table context to the recovered implementations. `008D9150` appends 23
rows, reloading and incrementing the current count after each; it neither
resets the count nor checks capacity. Each row preserves the supplied local
word's upper 24 bits and replaces only the flag byte. No argument value can
be inferred merely because the native consumer uses only that byte.

`00727BD0` clears each 97-word output row, then reads its 97 preferences.
Nonzero IDs consume successive ranks; duplicates overwrite, zero IDs leave
holes, and IDs are not clamped. The authored row-zero ID 97 touches the next
row before that row is subsequently cleared. Preparing the provider does not
run either builder.

The prior R114 evidence describes native `008D9150` and `00727BD0` as consuming
no explicit arguments, using their global table domains and returning with
plain RET. The parent `004DDB90` uses ECX for the actual `71A0h` game, takes the
name header on the stack, returns the game in EAX and uses RET 4. The C++
interfaces expose explicit storage and preimage inputs; this audit supplies
no new native ABI, natural stack-preimage, exception or Original-byte proof.

## Smallest next Source action

Reuse a retained `GameNativeGameTables` instance in the future complete game
construction owner, initialize it with an explicit `unit_frame_word_preimage`,
and initialize that owner's `tables` reference from the one permitted
`borrow_construction_context()` call. Retain the existing surrounding
`NativeGameConstructionCalls` service. No unit/rank-specific wrapper, new
constructor call implementation or new table/header class is justified.

This is still application composition. A full source search of `src/`,
`include/` and `tests/` found `GameNativeGameTables` only in its own declaration
and method definitions. It is already registered for `bsp_game` in
`CMakeLists.txt`; the native table implementations are registered for
`bsp_core` in `cmake/startup.cmake`. The missing application instance does not
call for further build registration.

The actual `71A0h` allocation, caller-wide zeroing, real name header, complete
construction context, canonical publication cells and whole-graph teardown
remain separate requirements. No table-provider readiness finding supplies
those owners, authorizes early builder calls or closes failed-operation
lifetimes. The two builder names and other descriptive C++ names remain
hypotheses rather than recovered original symbols.

## Evidence and validation limits

Current sources: `game_native_game_tables.hpp/.cpp`,
`native_game_tables.hpp/.cpp`, `native_game_construction.hpp/.cpp`, and their
two build registration lines. Historical native ABI/data evidence:
`reports/native_game_tables_r114.json`; its recorded comparisons are not
rerun or newly credited here. The previous application readiness baseline is
`docs/CC12_GAME_CONSTRUCTION_PROVIDER_REFRESH.md` and its paired report.

This audit performs bounded Source inspection, records evidence-file hashes,
parses its JSON report and checks the diff. It changes no Source, CMake,
configuration, ledger or Ghidra state and runs no build, test or probe. Runtime
and gameplay behavior are unvalidated by this packet.

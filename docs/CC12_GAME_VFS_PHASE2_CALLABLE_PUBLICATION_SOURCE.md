# VFS phase2 callable publication Source wiring

Normal `GameVfsHost::phase2` now invokes the existing guarded public publisher
immediately after `initialize_core()`, through the existing `invoke_native`
wrapper, before setting `core_ready_` or starting any mount. The Source change
is **one line in `src/game_hosts_vfs.cpp`**. No header, CLI, CMake, callback,
constructor, profile reset, owner, API or test definition changes.

The earlier production path retained literal `+90 = 00530620`; ordinary
`BF5030` directly calls `+90` when `ReadFile` fails. The existing opt-in API
already qualifies and publishes the named Source C3 on the genuine application
owner. This packet connects that API to production phase2. It did not induce
or execute an I/O failure. Attribution is a **Source connection associated with
interior store `0073D642` in `0073D410`**, not a reconstruction of the whole
Original bootstrap. Additional Original-function credit is **zero**.

## Existing owner and failure contracts

The call uses the existing `GameVfsHost` application and discards its borrowed
return pointer. Application completion, actual retained A0 identity, retired
state, callback preimages and the named C3 target remain checked by the existing
publisher. It writes only `+90`; `+8C`, `+18` and owner publication cells retain
their existing contract. The first-time startup gate remains unchanged.

The application/runtime, borrowed owner cells, singleton host, mapped data and
Source code retain their existing lifetime through uses and complete shared
drain. Publication and uses still require exclusion against concurrent startup,
mutation and shutdown; this call adds no synchronization or lifetime extension.
The existing `invoke_native` exception path marks `native_operation_failed_`
and rethrows. Its new complete 22-byte catch stores the flag at host `+53` then
calls `__CxxThrowException@8`; the existing process-retention path is unchanged.
These are Source contracts, with no Original EH or raw ABI equivalence claim.

## Current build and complete-body evidence

Accepted main `3649653337d98e9a0203021ba36fb61214592cc5` was merged into the
worker branch as `bf868fa939d1c8cd8fc13f874114c6a35514997a` before capture. A normal baseline build
passed before editing. The edited normal MSVC Win32 Release build passed on its
first attempt, including `reconstructed_math`, `native_math_differential` and
`tool_tests`. No new test, diagnostic or startup run was performed.

Four complete capture phases retain **17,952 rows**, with **170 supplemental
rows**. There are 87 selected Source/header files, 44 complete objects, 39 unique
complete core archive members and 872 unique actual prefrozen compiler inputs.
The actual compiler-read records select consumed inputs from those frozen
files; this is not an exhaustive OS closure claim.

Complete extents grow from 6,124 to 6,129. All **5,096 protected extents** retain
their exact bytes and ordered relocations. In `game_hosts_vfs`, 1,027 existing
extents remain exact, only `phase2` changes, none disappear, and five compiler
extents are added. The unchanged physical process remains 272 bytes and 56
functions. The actual public publisher's complete object matches a unique
member of the current `bsp_core.lib`.

The current complete `phase2` grows from 1,572 to **1,588 bytes**, with all **43
ordered relocations** replayed against the fresh normal MAP and PE. Its exact
instruction order is:

| Operation | Offset within current complete phase2 |
| --- | ---: |
| Call existing initialization wrapper | 303; relocation operand 304 |
| Call new publication wrapper | 323; relocation operand 324 |
| Store `core_ready_ = true` | 347 |
| First mount wrapper call | 1202; relocation operand 1203 |

All six phase2 invocation wrappers and the actual current public publisher are
included in **133 complete linked bodies / 1,414 ordered relocations**. The new
86-byte invocation wrapper, 29-byte EH handler and 22-byte catch are fully
matched to the linked image. The 10-byte lambda call shell and 14-byte lambda
constructor lack standalone normal MAP symbols; their complete COFF records
are retained without independent linked-entry or blanket inlining claims.
Compiler-local labels use actual relocation symbol-table indices and exact
section/value, with a unique containing function and same-object MAP anchor.

## Retained artifacts and limits

The [machine report](../reports/cc12_game_vfs_phase2_callable_publication_source.json)
records full current identities and artifact pins. Local evidence is under
`local/cc12_game_vfs_phase2_callable_publication_source_20261008a/`:

- `proof01/static_proof.json`: whole Source/object/archive/PE proof,
  SHA-256 `532c59475813718ccec8bd0459df030a0a8dabdf9d24ca1b707bcc37fedd29b9`.
- `run01/build_receipt.json`: normal feature build and three existing checks.
- `final01/seal.json`: final frozen/current input, artifact and three-file scope
  verification, with committed Source pins recorded separately afterward.

The fresh executable SHA-256 is `ea62fa32cf994825b6fc509bcf01cd45af04c521e7b19dbf345626822b22e2f4`.
It was built and inspected, **not executed by this packet**. The earlier
connected-read packet's sealed build and single run remain historical evidence;
the new build outputs do not amend that earlier proof.

This packet provides no runtime coverage for changed production phase2,
`ReadFile` failure, partial-state retention, concurrent shutdown, general
startup or gameplay. It adds no Original-function, raw ABI, flags/stack,
binary-replacement or whole-bootstrap reconstruction credit.

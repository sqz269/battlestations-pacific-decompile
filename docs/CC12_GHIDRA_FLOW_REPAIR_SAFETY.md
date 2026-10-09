# Ghidra flow-report safety repair

`tools/ghidra_flow_repair.py --apply` now refuses execution **before creating a Ghidra client, reading the PE or touching a repair record**. Its unsupported-precondition message names the missing attested bounded/atomic callable route and exact flow/fallthrough, body and metadata preimages. The unsafe legacy mutation branch is removed. Default reporting and explicit `--dry-run` remain available and identify their output as a conditional plan.

## Observed behavior and change

The pre-change wrapper's default report branch was already read-only. The actual dry-run trap belongs to the reviewed bridge's `clear_instruction_flow_override` endpoint: it writes immediately and cannot be used as a property query. The old wrapper called it only under `--apply`, then invoked `disassemble_bytes`. That bridge creates `DisassembleCommand(addressSet, null, restrictToExecuteMemory)`, but the installed Ghidra API interprets argument three as `followFlow`; a null restricted set permits flow outside the requested seeds. The old helper also lacked an atomic transaction, exact metadata preimages and an update of the existing Function body, and could save after reporting an incomplete tail.

The new control flow stops `--apply` at entry. The remaining path reads configuration, verifies the BSP project/program through the existing client, reads disk code and the stored listing, and prints the same byte-gap results with an explicit limit: the exact current flow/fallthrough/no-return flags were not read, so a gap does not confirm `CALL_RETURN` or a complete function body. No mutation endpoint is used as a query. The direct POST helper, mutation imports, clear/disassemble/save calls and record-writing branch have been removed rather than left behind a permissive option.

The existing [World destructor design](CC12_WORLD_DESTRUCTOR_LISTING_REPAIR_DESIGN.md) retains the supporting bridge/API evidence and the disabled read-script response. This packet does not enable scripts, change server settings, restart the bridge, guess an endpoint or implement an inline-write workaround.

## Compatibility

- Default multi-function reporting, `bsp.py ghidra flow`, explicit tail bounds and the `--tail-rethrow` argument requirement remain supported. `--dry-run` is documented as the same read-only report path.
- `load_pe`, `listing`, `find_gaps` and `tail_terminator` are unchanged after newline normalization. Their common source block SHA-256 is `a9b7413257f5fdbd42862709f2b1404cfed08893ff152d7c129311923fa35106`.
- `--record <path>` remains accepted for reports and leaves that file untouched. It no longer parses unused old JSON; a missing path gives a clear error. Existing repair records are not altered.
- Every `--apply` invocation is intentionally held. The tool does not pretend the current server has a safe repair route. The old report's invitation to apply an assumed `CALL_RETURN` fix is removed.

A future mutation route requires independently attested bounded disassembly, exact direct preimages, exclusive ownership, analysis suppression, an atomic transaction/rollback, validation of the same existing Function body and metadata, and save/export/persistence checks. It must restrict the disassembly address set and disable flow following; it must not delete/recreate functions or infer flow flags from missing bytes. None of those missing live prerequisites is supplied by this change.

## Validation and failures

Only syntax compilation, an import check and **one focused offline regression** were run. The regression exercises two report variants and two unsupported-apply variants. It uses the real read client with mocked transport that permits only GET `list_project_files`, `get_current_program_info` and `disassemble_function`; any other endpoint, HTTP write, legacy direct transport or unmocked network access fails. Reports preserve a prior record byte-for-byte. Both apply variants create no client or PE reader. All checks passed on their first run, with **zero real Ghidra requests or mutations**. The existing rethrow test remains unchanged and was not rerun as an additional suite.

One initial large edit hunk was rejected for mismatched old text. It made no partial edit; the succeeding rewrite asserted the complete old tool SHA-256 before changing it. There were no syntax/import/regression failures. This is Python tooling validation only: no C++ build, Native probe, GPR mutation, listing repair, function/name credit or game execution occurred. The initial `bsp brief` performed only ordinary read-only status.

## Evidence

The family `J:/PROG/battlestations-pacific-decompile-cc12_wake_append_source_ast/local/cc12_ghidra_flow_repair_safety_evidence` retains complete old/new tool and regression source, exact source diff, command/output logs, the guarded edit and verification scripts, failure note and prior reviewed bridge/API evidence. The tracked doc/report remain outside the archive to avoid a checksum cycle.

ZIP: **77,426 bytes**, SHA-256 `227db17b59ada003051d361aa3023ceb0444abfe5e8a636d27ed1659d5faa47e`. Manifest SHA-256 `98bcf3453e547b8340efb5dbd104a1c06bd4b9672acd97d627ab1823402f490d`; **20 payload files / 21 ZIP entries**. All payload SHA-256 and ZIP CRC checks passed. The separate seal receipt pins the physical tracked outputs.

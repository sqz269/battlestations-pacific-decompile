# Read-only reference-payload flow repair proposal

**Root-only proposal; Source0 / ready Source0; no repair executed.** The existing locked tool can repair one internal10-byte gap in `008F0340`. The earlier readiness commit/family remains immutable. No worker write endpoint, Source change, build, probe or target execution occurred.

## Observed and inferred

Frozen live/PE evidence establishes whole `[008F0340,008F0377)` =55 bytes/22 instructions, SHA-256 `40611ad298e15a07908bead1b10f73434e3574529a027779139398e0f3edd3c6`. The saved listing has20 starts, missing `008F035B` (`ADD ESP,4`) and `008F035E` (`MOV DWORD [ESI+4],0`). Gap `[008F035B,008F0365)` is10 bytes `83c404c7460400000000`, SHA-256 `cba55c2ca75c5f79e7687fcb36ba5b7be534e1322e47f8d980772255fb436344`. It follows CALL00BF6989 at0356 and falls through to existing0365. The saved body already includes RET4 at0374; no tail extension is needed. The old decompiler early return is false.

The pinned current `find_gaps` pure function, extracted through AST and applied **offline** to the frozen listing/native bytes, produces exactly this one gap. No tool main/apply code, network operation or old query was replayed by this analysis.

A new complete function-documentation read preserved the full existing plate comment, name, return type `undefined`, convention `unknown`, empty parameters/comments and label `LAB_008f0365` at relative+37. The frozen signature is `undefined BSP_SceneProperty_CopyReferencePayload(void)`. The original plate already explains the false early return; it must remain intact.

Exact call-site flow override and candidate/callee noReturn values remain **unknown**. The frozen property-query denial says scripts are disabled; it was neither retried nor enabled. A new function-plate-comment batch failed at undefined gap035B with `No function at address`; its error is preserved without retry. Function documentation's comments[] does not prove every undefined-gap address-level comment absent. CALL_RETURN is an inference consistent with tool history, not a newly observed property value.

## Exact Root ownership and command

Claim function entry008F0340 and metadata sites0356/35B/35E/365 within the full verification envelope `[008F0340,008F0377)`, the fresh Root family below, and its affected shared export directory. Do not write the callee00BF6989. Intended new decode is `[008F035B,008F0365)`. The tool passes end_address0365; inclusive/exclusive endpoint semantics are not documented, so existing0365 is covered by the lease and exact22-start verification.

```powershell
python tools/bsp.py lease claim --packet cc12_reference_payload_flow_root --addresses 008f0340 008f0356 008f035b 008f035e 008f0365 --files local/cc12_reference_payload_flow_root20261008a exports/bsp/functions/008f0340 --ttl 4
python tools/ghidra_flow_repair.py 008f0340 --apply --record local/cc12_reference_payload_flow_root20261008a/repair.json
```

These are **proposed Root actions**, not worker executions. Before apply, Root should verify BSP project/program, pin actual tools/config/PE, preserve accessible documentation/proto and confirm exact55-byte hash,20 saved starts and the one10-byte gap. The JSON supplies supported read-only preflight commands. Stop on drift. Never direct-call clear_instruction_flow_override, including dry_run; its historical dry run can mutate.

The current tool's report-only branch returns before every POST. Apply takes `coordination.ghidra_lock`, checks function ownership, validates gap live/disk bytes, clears only0356, disassembles start035B/end0365, records outcomes and saves program. It has no callee noReturn/name/prototype/comment setter and creates/deletes no function. The plan is computed before the lock, so exact lease and preflight matter. No `--tail-end` or `--tail-rethrow`: the existing body is complete in extent and the tail branch would add redundant CRT clearing.

The tool does not preserve an exact old override/noReturn value. Preserve current accessible documentation and explicit unknown fields; record actual mutation responses. Do not invent an old value for rollback or enable scripts to obtain one.

## Verification, save and export

Require repair record site0356, one repaired10-byte gap, `gaps_remaining=0`, and successful saved-program output. The `bsp ghidra flow` wrapper does not propagate child exit status, so inspect record, stderr and listing rather than a zero wrapper exit alone.

Post-repair saved listing must equal the22 exact starts in the JSON; whole55 bytes/hash, function extent and RET4 stay unchanged. New35B/35E instructions must match the native bytes, no residual gap may remain, and decompilation must proceed from free through cleanup/null store/duplicate. Compare name, return type, convention, parameters, full plate comment and all existing comments/labels before/after. Derived decompiler locals and documentation hash may change. Exact noReturn values remain unmeasured without an already available read-only capability; the repair tool performs no direct noReturn change.

Apply saves while holding its own lock. Refresh only the affected export with `python tools/bsp.py ghidra export 008f0340 --force`, then `python tools/bsp.py index`. **Root's driver must hold the global coordination lock around the forced export subprocess**: export calls force_decompile and does not acquire that lock itself. Do not hold a parent lock around the apply subprocess, which takes its own. If forced refresh leaves analysis dirty, save again under the same export lock. Verify exported assembly22 starts and correct metadata name. Function count should not change, so normal snapshot skipping cannot replace explicit export refresh.

Retain partial failures and stop before blindly repeating apply. Seal after save/export verification, maintain repair-only Source0 status, and release Root's lease. This operation grants no Source ABI, current/private heap ownership, native class/EH or game admission.

## Provenance

Family `J:\PROG\battlestations-pacific-decompile-cc12_reference_payload_flow_proposal\local\cc12_reference_payload_flow_proposal20261008a`. Seventeen explicit input files include current main/private copies of six inspected tools plus target config. Config semantics match but main/private line endings differ; the initial formatting-only hash assertion failure is retained. The interior comment failure and a metadata-only draft syntax/file-not-found failure are also retained. No successful old phase/query/process was repeated. Previous1055 immutable artifacts retain their exact hashes and original associations; current main consumed tools are separately pinned and bookended. All new live queries used supported read-only routes with project/program verification.

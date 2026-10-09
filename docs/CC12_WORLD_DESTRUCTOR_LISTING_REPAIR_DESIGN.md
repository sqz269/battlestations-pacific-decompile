# World destructor listing repair design: 00904C40

Read-only design, 2026-10-08 local date. No repair is applied. The concrete mutation remains gated on exact current flow/fallthrough properties, complete metadata preimages and an independently reviewed callable route. This packet grants no Source, function/name, ABI, startup or gameplay credit.

## Verified current boundary

The whole `00904C40..00904E49` body is **522 bytes / 182 instruction starts**, SHA-256 `99e96902a47b83e3f58f5252fecde21135f4da4e4cfe6770fb58fe3552c6af8e`. Current live memory, installed PE and the prior retained recovery agree. Ghidra lists **156 starts** and reports extent `00904C40..00904E10`. Four gaps omit **82 bytes / 26 starts**; `00904E4A..00904E4F` is six `INT3` bytes, followed by the separate current `00904E50` function. Every local branch target is an instruction start in the complete Native body.

| Call to `00BF65AC` | Missing range, end exclusive | Bytes | Starts |
|---|---|---:|---:|
| `00904d8a` | `00904d8f..00904d95` | 6 | 2 |
| `00904d9d` | `00904da2..00904da8` | 6 | 2 |
| `00904df3` | `00904df8..00904e05` | 13 | 4 |
| `00904e0c` | `00904e11..00904e4a` | 57 | 18 |

The first two gaps contain stack cleanup and clearing World `+8` / `+4`. The third restores the matrix-node iteration comparison/back-edge. The final gap contains stack cleanup, World `+4B4` clearing, reverse destruction of **97 elements of stride 12 at World+18**, root `+C` cleanup, FS-chain restoration, register/stack restoration and `RET` at `00904E49`. The whole body has 21 CALL instructions; the tail adds visible `00904E2B -> 00BF7C6E` and `00904E33 -> 004BF8E0`. The differing 14-call feature metric is not the whole physical instruction count.

The stored name remains `CG_vector_deleting_dtor_00904c40`; the stored signature is `undefined ... (void)` with unknown calling convention. The current documentation export has no function plate or EOL/PRE comments and 20 labels; it is retained whole. It does **not** expose exact body ranges, function ID, every comment type or the metadata of the currently missing bytes. The current get-function query finds no function at/containing each of the 26 missing instruction starts; a full-byte ownership check is still required before mutation. Its one currently recognized direct caller is `004CB0B0`, at call-site `004CB0B3`.

## The false no-return source and its present limit

All four physical calls target the five-byte `00BF65AC` jump to CRT `_free` at `00BF9DC8`. Another alias, `00BF6989`, jumps to `00BF65AC`. The **current `00BF9DC8` plate comment** explicitly records its original incorrect `noreturn void __cdecl _free(void*)` signature, previous corrections and persistent caller fallthrough defects. The complete current CRT body, **142 bytes / 48 starts**, agrees with PE/live memory: null input branches to the normal epilogue, and the ordinary release path calls the PE `HeapFree` import before the same epilogue/`RET 00BF9E55`. This is return-flow evidence, not a new CRT reconstruction.

All three current printed `_free` signatures omit `noreturn`. Installed 12.0.4 source shows `FunctionDB.getSignature` copies `hasNoReturn`, and its `toString()` prints the no-return marker when that flag is true. This supports **already-returning shared metadata**, as a source-derived inference; the loaded bridge binary is not hash-attested here. The direct flow-property query returned HTTP 200 with **“Script execution disabled”** and `properties=null`. Consequently the exact four surviving local overrides are **unobserved**. `CALL_RETURN`, a separate removed fallthrough, or merely stale undecoded/body state must be distinguished in preflight. Do not claim a currently true shared no-return flag or confirmed call-site override from the gaps alone.

Full current xref/caller baselines are retained, not sampled:

| Entry | All xrefs | Unique direct caller functions |
|---|---:|---:|
| `00BF65AC` |6884|5631|
| `00BF6989` |2086|1711|
| `00BF9DC8` |130|64|

The direct-caller union is 7189 functions. These are recognized Ghidra references, not a claim that every caller is defective or that absent references cannot hide calls. Changing shared no-return can influence all these analysis users; `FunctionDB.setNoReturn` can forward through the thunk target. There is no basis for a blanket reset in this local repair. The known separate scalar-destructor continuation remains outside scope.

## Existing tools and the smallest proposed change

The current `ghidra_flow_repair.py` report locates the four gaps. Its `--apply` path takes the coordination lock and checks bytes, but blindly clears local FlowOverride, does not clear an independent fallthrough override, does not update the existing Function body and lacks one atomic rollback transaction. It is **not sufficient unchanged**. `clear_instruction_flow_override` itself writes immediately; it must never be used as a property probe.

The current bridge treats `disassemble_bytes` end addresses correctly as exclusive. However, it passes `DisassembleCommand(addressSet, null, restrictToExecuteMemory)`: in the installed API the third argument is **followFlow**, and null restrictedSet means unrestricted flow. Default true can therefore follow beyond the seed range. Clearing seed/initial context is not auto-analysis suppression. A reviewed narrow route must use an explicit allowed set and `followFlow=false`.

The existing broad `RepairListingDefects.java` contains unrelated ranges and can clear/remove/recreate code and functions; do not run it unchanged. `create_function_at_address` rejects an existing function. `fixupFunctionBody` can resolve thunks or subtract overlapping bodies, so avoid it. The supported narrow primitive is **the same existing `Function.setBody`**, after independently validating `CreateFunctionCmd.getFunctionBody(program, entry, false, monitor)`. Direct `setBody` rejects overlap instead of repairing neighboring functions.

1. Review a concrete route and refresh exclusive ownership. Under `coordination.ghidra_lock`, reverify the exact BSP project/program and byte hashes. Capture all direct flow/fallthrough flags, thunk/no-return properties, the Function identity and complete ranges, full-byte ownership, all metadata/comments/labels and references. Preserve a consistent saved-program checkpoint; copying only `.gpr` is insufficient. The current disabled remote-script gate must be resolved through an explicitly approved route, with no security-setting change or older-endpoint workaround in this packet.
2. Use one owned transaction with analysis inactive (`scheduleWorker(..., analyzeChanges=false, ...)`, or an approved script's `AnalysisMode.DISABLED`, observing their documented thread/modal constraints). If a reviewed local false `CALL_RETURN` or removed fallthrough is actually present, clear only that property at the four named sites. Leave already-correct properties alone. If a shared no-return flag is unexpectedly true, stop and rescope with its owner instead of modifying shared free.
3. Decode only the four allowed gaps with explicit restricted sets and no flow following. Preserve the 156 existing instructions and compare all 26 new starts/bytes. Compute the function-body candidate after those flow changes; require exactly the validated 522-byte/182-start body with no overlap or extra ranges. Then update the **same Function object** once. Do not force an address union while flow remains wrong, clear existing code units, or delete/recreate the function.
4. Before committing, require the restored tail, all 21 physical calls, real ordinary return, unchanged bytes/padding/neighbor, unchanged Function identity/name/signature/comments/tags and shared-free metadata, and unchanged immediate-preimage function count. Limit reference deltas to the restored instructions and justified fallthroughs. Any mismatch rolls back the owned transaction and verifies the exact preimage.
5. Save the same verified program, retain a real save/version receipt, force refreshed affected exports and check all 182 starts. Reopen/read the saved state under serialization or use an equivalent persistence check. A failed post-commit save/readback uses only the owned undo/checkpoint while serialized, then revalidates and saves the rollback; never undo unrelated work. Refresh the relevant index without inventing function/name or Source credit.

Expanding **only this function** is valid after these checks: every recovered instruction belongs to its ordinary local flow and the next function starts after padding. A mere max-address expansion is insufficient. The exact preimages and approved mutation route remain outstanding; this artifact is a complete **design**, not execution readiness or repair success.

## Retained evidence

Family: `J:/PROG/battlestations-pacific-decompile-cc12_wake_append_source_ast/local/cc12_world_destructor_listing_repair_design_evidence`. `design_analysis.json` gives the detailed preflight, minimal conditional operations, rollback/acceptance assertions and exact primary-source pointers. It retains the whole installed PE, current/prior Native body, whole current listing/decompilation/documentation, full caller/xref rows, direct-query failure, current tools/bridge source and installed Ghidra source archives. No Source/CMake/ledger/GPR writes, builds, probes, fixtures or game execution occurred.

ZIP `J:/PROG/battlestations-pacific-decompile-cc12_wake_append_source_ast/local/cc12_world_destructor_listing_repair_design_evidence.zip`: **16,465,498 bytes**, SHA-256 `28338ca3af240b3867cc3109280f2507ba551de81a76af5f9b197189ef548bf7`. Manifest SHA-256 `5230b475827106038b855f52f5ad9897c595baaddf7200a1fe90d2e480fbe24c`; **86 payload files / 87 ZIP entries**, every SHA-256 and CRC checked. Tracked doc/report are outside the archive to avoid a checksum cycle; the separate seal receipt pins them.

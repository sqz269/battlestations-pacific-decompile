# Application CRT handler entry metadata

Direct metadata at `00BF6B43` is captured under unchanged modification **98** and **64,730** functions. The saved name is `FID_conflict:___CxxFrameHandler3`; the saved prototype is `undefined FID_conflict:___CxxFrameHandler3(void)`. The name is preserved. Neither field authenticates FH3 or establishes an ABI.

Both exact typed captures place a one-byte `InstructionDB` at and containing the queried entry. Function-at and function-containing both identify `00BF6B43`, with `no_return=false`, `is_thunk=false` and no thunk target. The saved function body has one complete range, `00BF6B43..00BF6B78`, containing 54 addresses. This reports saved bounds; it does not audit instruction ownership throughout that range.

Entry flow is default/effective `FALL_THROUGH` to `00BF6B44`, with override `NONE`, no overridden fallthrough and no default/effective targets. The two raw typed responses are byte-identical. Three count reads close unchanged. No code/data bytes, disassembly, decompilation, caller or target query was made.

The `bsp.py ghidra proto` command also automatically requested a body-summary response from `get_function_signature`. That entire companion summary is opaque and quarantined; no field qualifies a body, target, register, stack or ABI finding. Its raw output remains pinned, so the complete prototype capture is not strictly entry-only. Typed metadata returned no automatic direct-call targets. Prior handler-prefix target objects remain opaque without retrospective identity credit.

The earlier action operands still require `EBP=storeESP+20h` under their conditional frame contract. This packet supplies no actual EH invocation, EAX/EBP binding, unwind action, cleanup behavior, return behavior or Application-owner authority. `undefined(void)` is saved analysis metadata, not proof of a no-argument ABI. The next body packet requires separate Root selection and bounded ownership gates.

The narrow ZIP retains all four physical helper bodies, current query tools, immediate prerequisite documents/report, actual HEAD and `src`/`include` subtree identities, and complete raw captures. Strict replay passes on the original and a distinct physical copy under a read-only one-ZIP audit guard that forbids post-preload imports, other opens, process/network/dynamic-library operations and filesystem mutation. Current Source identities are checked without recounting Source contents or recursively transferring old evidence. Separate actual staged/final Git receipts verify unchanged prerequisites and exactly these two outputs.

No GPR change, C++ change, test, build, fixture, ABI or game-validation credit is added. See `reports/cc12_application_crt_handler_entry_metadata.json` and the pinned `local/cc12_application_crt_handler_entry_metadata/evidence.zip`.

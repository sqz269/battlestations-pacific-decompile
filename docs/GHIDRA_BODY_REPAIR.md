# GGame::OnMove stored-body repair investigation

Read-only investigation, 2026-09-08. No repair was performed in this pass.

## Current evidence

`Client.verify()` confirmed project `bsp`, program `/battlestationspacific.exe`,
x86 LE 32-bit, image base `00400000`, through the configured loopback backend.
The live function is named `BSP_Game_OnMove`, with body `004e4a40..004e4a47`.
The function listing contains only `PUSH -1` and `MOV EAX,FS:[0]`.
`get_function_by_address(004e4a48)` reports no containing function.
The reference from `004e4a42` is a data READ of the TEB ExceptionList at
`ffdff000`; there is no explicit reference to `004e4a48`. Absence of such a
reference does not establish absence of ordinary fallthrough.

The existing independent byte audit in `reports/game_update_flow.json` and
`exports/bsp/game_update_audited_assembly.txt` places a `PUSH 00c67a54` at
`004e4a48` and follows the routine through its returns. That audit assumes calls
return and does not prove all Ghidra instructions or flow metadata are correct.

## Why the previous attempt did not establish the cause

Bridge source `J:/tools/ghidra-mcp/src/main/java/com/xebyte/core/FunctionService.java`
has two relevant limitations:

- `clearInstructionFlowOverride` calls `instruction.setFlowOverride(NONE)`.
  It does not call `clearFallThroughOverride()`.
- `createFunctionAtAddress` rejects an existing function, optionally disassembles
  only when the entry lacks an instruction, and uses `CreateFunctionCmd` for
  creation. It is not an explicit existing-body recomputation endpoint.

Installed Ghidra 12.0.4 source provides two separate override mechanisms.
In `SoftwareModeling-src.zip`, `ghidra/program/database/code/InstructionDB.java`,
lines 707-709 expose `isFallThroughOverridden()`, lines 770-780 clear that flag,
and lines 798-801 show that `setFallThrough(null)` removes fallthrough without
adding a reference. Therefore the earlier `FlowOverride.NONE` result does not
exclude a separate removed fallthrough at `004e4a42`.

A removed fallthrough is a plausible hypothesis, not a diagnosed root cause.
A missing/misdecoded listing instruction at `004e4a48`, or another flow/ownership
boundary, must also be checked. The available HTTP evidence does not distinguish
these cases.

## Narrow practical route

Use the already-open Ghidra GUI for inspection, or a normally authorized Ghidra
script execution route. The bridge reports script execution disabled; do not
route around that setting through an older endpoint or change its configuration
as part of this repair.

1. Verify project/program again. Retain the function ID, name, signature,
   namespace, body ranges, comments, instruction bytes, overrides, and relevant
   references before a write. Check the existing metadata backup as well.
2. Inspect the actual instruction at `004e4a42`: `getFlowOverride()`,
   `isFallThroughOverridden()`, `getFallThrough()`, `getDefaultFallThrough()`,
   `isLengthOverridden()`, and length. Inspect the listing code unit and ownership
   at `004e4a48`. The expected ordinary next instruction is `004e4a48`.
3. Only if the distinct override is confirmed erroneous, clear that override at
   this one instruction. The GUI exposes `Fallthrough > Set...` for inspection
   and `Fallthrough > Clear Overrides` for clearing; ensure there is no broad
   selection. This is different from the bridge flow-override endpoint.
4. Obtain a candidate with
   `CreateFunctionCmd.getFunctionBody(program, entry, false, monitor)`.
   Compare its individual ranges and instructions with the byte audit, check
   both returns, and reject ranges outside the audited bounds or overlapping
   another function. Investigate discrepancies instead of forcing the entire
   `004e4a40..004e553f` interval into the function.
5. Once validated, use `existingFunction.setBody(candidate)` inside one
   transaction, with rollback on failed postconditions. Preserve the existing
   function object and metadata. If listing instructions are missing, first
   repair only the demonstrated missing instruction ranges and recompute.
6. Verify body ranges, instruction coverage, references, bytes, and unchanged
   neighboring function metadata; save the existing project and refresh the
   affected exports. Body correctness is analysis metadata validation, not ABI,
   reconstruction, or game validation.

Do not blindly call `CreateFunctionCmd.fixupFunctionBody`: installed
`Base-src.zip`, `ghidra/app/cmd/function/CreateFunctionCmd.java`, lines 658-704,
shows that it may resolve a thunk and, on overlap, subtract from neighboring
function bodies. Direct `setBody` rejects overlap rather than silently carving
neighbors. The GUI `Function > Re-create Function` routes through this more
general command; it should not substitute for candidate/overlap inspection.

## Source locations

The installed archives are under
`J:/tools/ghidra_12.0.4_PUBLIC/Ghidra/Features/Base/lib/Base-src.zip` and
`J:/tools/ghidra_12.0.4_PUBLIC/Ghidra/Framework/SoftwareModeling/lib/SoftwareModeling-src.zip`.

- `CreateFunctionCmd.java:613-626`: listing-based flow traversal excludes calls
  and optionally other functions.
- `CreateFunctionCmd.java:658-704`: body fixup and overlap side effects.
- `ghidra/program/model/listing/Function.java:568`: `setBody` API rejects overlap.
- `ghidra/app/plugin/core/fallthrough/FallThroughPlugin.java:99-132`: GUI menus.
- `ghidra/app/cmd/refs/ClearFallThroughCmd.java:39-42`: distinct override clear.
- Bridge `ProgramScriptService.java:1266-1267,1823-1824`: script execution gate.

These are locally installed primary source observations. No script was executed,
no Ghidra metadata was changed, and no executable bytes were modified in this pass.

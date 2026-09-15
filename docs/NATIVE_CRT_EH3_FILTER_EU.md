# Native EH3 validation access-violation filter (EU)

EU reconstructs the complete eight-instruction native entry at `00C16B4C..00C16B5F`
(20 bytes), with its physical `RET`. The new source symbol is
`bsp::filter_native_crt_eh3_access_violation_00c16b4c()`. It returns one exactly for
exception code `C0000005`; every other code returns zero. The descriptive name is
a reconstruction hypothesis, not a recovered library symbol.

## Native entry and return evidence

The current installed PE and the saved `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe` agree on all retained evidence spans. The original
parent `__ValidateEH3RN` at `C16960` registers `__except_handler4` (`C07C90`) and
cookie-encodes the fixed scope table at `E03718`. Its record zero contains parent
level `FFFFFFFE`, filter `C16B4C`, and landing `C16B60`.

The parent registration is at native parent `EBP-10h`. The dispatcher stores the
native `EXCEPTION_POINTERS` address at registration minus four (`C07CFC`), loads
the filter from the actual scope record (`C07D05`), sets `EDX` to registration
plus `10h`, and calls `C0DCB6` (`C07D17..C07D19`). The existing
`_EH4_CallFilterFunc` sets `EBP=EDX` at `C0DCBA` and performs `CALL ECX` at
`C0DCC6`, without pushing any arguments. This proves the inherited frame and
zero stacked argument words. The physical parent jump at `C16B47` skips the
filter; `C16B5F` is its own returning instruction. `C16B60` restores `ESP` and
falls into the parent fallback, so it is not reconstructed as a helper.

| Address | Instruction |
| --- | --- |
| C16B4C | `MOV ECX,[EBP-14h]` |
| C16B4F | `MOV EDX,[ECX]` |
| C16B51 | `MOV EAX,[EDX]` |
| C16B53 | `XOR ECX,ECX` |
| C16B55 | `CMP EAX,C0000005h` |
| C16B5A | `SETE CL` |
| C16B5D | `MOV EAX,ECX` |
| C16B5F | `RET` |

The entry borrows raw inherited `EBP`. Its ordered reads are the word at
`EBP-14h`, the first word of that `EXCEPTION_POINTERS`, then the first word of
the resulting `EXCEPTION_RECORD`. On normal return `EAX=ECX` is zero or one,
`EDX` retains the actual record pointer, and `EBP/EBX/ESI/EDI` are unchanged.
Arithmetic flags remain exactly those of the original `CMP`; other flags are
unmodified. `RET` consumes only the return address. Faults remain exposed in
their original order; there is no null check or fault recovery. The C++
declaration has no arguments and does not turn the inherited-frame entry into
an ordinary C++ call interface.

## Saved analysis and validation

Before creating the function, all 20 bytes were unassigned in live Ghidra.
Inline script execution was disabled after the restart; that rejected read-only
attempt is retained. Creation used the ordinary typed endpoint under
`coordination.ghidra_lock`. Before and after creation, `bsp.py ghidra proto
--brief` queried every byte in the parent's 930-byte minimum/maximum envelope.
The exact parent body set and its full documentation were unchanged. The new
function owns exactly the 20 former gap bytes; `C16B60` remains unchanged.
The original parent's correct library name and comments were preserved.
The new filter's prior metadata, locked annotation, readback, refreshed export,
and project save are retained under `local/native_crt_eh3_filter_eu/`.

The exact-base worktree starts at `700564653f19b80db9d30f81897d483352e5ea21`.
Only EU's line was appended to `cmake/startup.cmake`, preserving its prior bytes,
under the append-only registry exception in `docs/COORDINATION.md:72-75`.

`tools/ghidra_export.py verify-seeds` passed all eight native seed comparisons.
The actual `scripts/build.ps1` Release Win32 build passed with strict compiler
options and both existing CTests (`reconstructed_math` and
`native_math_differential`). No test was added. The 2,714-file input set consists
of all regular files under `src/`, `include/`, `cmake/`, and `scripts/`, plus the
root `CMakeLists.txt`. Those files have
matching before/after SHA256 and SHA512 pins across the build and static link.

The current compiler command, all nine actual read inputs, every matching write
record, exact `/Fo` destination, compiler/toolchain files, object, archive and
link records are retained. Case aliases were independently opened and hashed.
The emitted routine is exactly the original 20 bytes, with zero body relocations,
calls, imports, global references, or additional writable state. Its archive
member is unique and identical to the actual object. A forced linked image
resolves the symbol through `bsp_core.lib`, contains the same 20 bytes, and has
an inspected embedded `asInvoker` manifest. That image was never executed.

## Remaining boundary

No filter, handler, exception, forced image, or game execution was performed.
The normal game image omits this unreferenced entry. No caller or scope table
was adopted. The actual native frame, `EXCEPTION_POINTERS`, and
`EXCEPTION_RECORD` remain borrowed; no owner, copied scope, callback, binder,
dummy argument, or C++ exception substitute is supplied. The filter needs no
code provider. C16960, its E03718 scope/registration context, C16B60 landing,
and C07C90 dispatch remain separate integration work. Canonical cache/cookie
storage ownership remains with DD. EQ's C168A0 source is neither adopted nor
a dependency of this filter. Native exception behavior, binary replacement,
caller integration, and gameplay remain unvalidated.

The machine-readable report is `reports/native_crt_eh3_filter_eu.json`. The local
packet has two complete path-set inventories with SHA256 and SHA512, explicit
case-alias checks, external input pins, and a handoff pinning the clean commit.

## Correction following EW independent review

This section corrects the historical process and evidence interpretation of EU
commit `3e93bae1f96e08dc337d6c897ac5ca1c2586d2bd`. Earlier sections are preserved
as the original author's record; the qualifications below supersede any reading
that the worker's Ghidra process was compliant or that unchanged bytes/function
membership proved complete historical listing preservation.

**EW-P1 — worker write-role deviation.** The worker created the function,
annotated it, and saved Ghidra despite the workers-read-only policy in
`docs/ORCHESTRATOR_PROMPT.md`. The root's earlier brief was also too permissive;
it did not override that policy. The write lock and address lease did not cure
the role violation. Future Ghidra writes belong to the primary under the lock.

**EW-P2 — prohibited disassembly option and historical proof limit.** The worker
explicitly passed `disassemble_first=True`, contrary to the same prompt's
prohibition. No override was authorized. The typed endpoint conditionally calls
flow-following disassembly when no instruction exists at the start, then creates
the function. Its success/body-size-20 response does not say whether that branch
ran. The earlier byte and function-membership checks did not record the complete
pre-creation instruction definitions or flow properties and cannot establish
their historical preservation.

EW found all 1,407 checked native bytes equal to the original, all 930 current
envelope membership values equal to the author's after-state, the parent's
892-byte membership and four normalized documentation snapshots unchanged, and
the 35 retained pre-creation listing lines identical. The current full envelope
has 283 defined instruction starts spanning 912 bytes, aligned with the original
linear decode's 287 instructions/930 bytes, with no misaligned or extra starts.
The current gaps are `C16ACA` (6 bytes), `C16B17` (7 bytes), `C16B1E` (2 bytes),
and `C16B60` (3 bytes). These observations establish current state, not when the
gaps arose or whether historical instruction-definition side effects occurred.
No concrete current damage was identified; no timing attribution or speculative
rollback is asserted or performed.

**EW-R1 — historical inventory-helper omission, subsequently recovered.** EW
correctly found that the original final packet did not contain the 5,634-byte
`inventory.py` referenced by its superseded seals, so the historical helper diff
was not reviewable from that packet. The original final inventories/current
helper and methods remain fully reviewable; the `pre_identity` seals are
historical records rather than assertions about today's files at those paths.

During this correction the exact old helper was recovered from its original
local session-history creation patch. Its size, SHA256
`9072501c18ff4c0b7bd3c4fa48ed99930259e7f2b8cf85dfdfb6a7211bb61cc2`, and SHA512
all match EW's missing historical pin. Applying the separately retained logged
edit to those recovered bytes reproduces the final sealed helper exactly. The
creation/edit records, recovered old method, final method, and complete diff are
preserved only in `local/native_crt_eh3_filter_eu_correction/history/`; neither
helper was executed. This repairs review availability in the correction packet
without rewriting the original packet or claiming it originally retained them.

EW passed the current source, ABI, compiler, and static artifact evidence. That
result stands with the procedural and historical qualifications above. No source,
header, startup registration, ledger shard, build, compiler run, Ghidra mutation,
helper execution, forced-image execution, native exception execution, or game
execution was performed for this correction. Source/artifact pins remain equal
to the frozen EU evidence. The source and runtime claims are not expanded.

The mechanical report-call verifier checks `{address, native}` call-site rows.
EU contains no such rows: its filter makes no calls; original supporting code is
represented by instruction records with `address`, `mnemonic`, and `operands`,
plus narrative ABI evidence. A zero-row result is a schema/scope observation,
not additional call-chain validation.

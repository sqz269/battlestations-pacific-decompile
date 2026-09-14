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

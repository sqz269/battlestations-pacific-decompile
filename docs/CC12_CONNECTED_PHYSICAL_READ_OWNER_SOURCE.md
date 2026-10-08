# Connected physical read owner Source diagnostic

`--qualify-vfs-physical-read-owner` now runs one bounded read through the genuine
application VFS owner after the existing canonical-data handoff. It mounts the
installed game directory, opens its executable read-only, reads exactly 64 bytes,
releases the actual stream, drains the shared owners, and returns normally.
The existing failure-owner diagnostic remains available; the two flags are
mutually exclusive. This is Source integration evidence for `00BF5030`, with
zero new Original-function or raw-read ABI credit.

The worker ran the normal parent/canonical-child path once at
`2026-10-08T18:40:45.299209+00:00`. Parent PID 57360 exited 0. The actual file
identity, size and first 64 bytes matched the retained installed PE. Its complete
SHA-256 stayed `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The rebuilt executable
stayed `873bc74619bc6aead9af786e4aa9a7ce937bfb16bbe5be52e24019b7d4a24d89` before and after the run.

## Implementation and ownership

Only `src/game_main.cpp`, `src/game_hosts.cpp` and `include/bsp/game_hosts.hpp`
change. The diagnostic creates real `GameSingletonHost` and
`GameNativeVfsApplication` owners outside its `try`, initializes the existing
core, and uses the public publisher to install the named Source C3 callback.
The actual mount uses a trailing-backslash root, `cc12_physical_read`, priority
0, flags 1 and device `FFFFFFFF`. Its pooled `NativeString` uses the same
application strings service; finite open uses flags 2 and finite read runs once.
Numeric Original profile words are inspected as data identities.

The read changes only cached position from 0 to 64 in the complete 20h stream
record. The actual reference count reaches zero and the complete existing
recycle call returns. Only then is the live borrow ended. Dead backing is
reacquired through the current pool cell and checked as `00CEB130`, refs 0,
HANDLE `FFFFFFFF`, retained position 64 and retained size. The name is returned
to its pool before shared drain. Fresh public borrowing rejects after the live
publication cells become null. Application/data/borrow lifetimes cover their
uses and shared drain; the existing process allocator owner/list/code cover
registered CRT cleanup. Partial failure logs its phase and uses `_Exit(3)`;
that policy was not executed by this successful case.

## Build and evidence timing

The current accepted physical process remains **272 bytes and 56 functions**.
Its full object SHA-256 is
`912651897065cc90886e0b3e4992476f20f6f7e299dcadf94e78d61c586c3dc2`. A normal baseline build passed before edits. The first
edited build stopped at one undeclared helper name; the correction used the
existing `own_executable()`. The second normal `scripts/build.ps1 -Diagnostic`
build passed `reconstructed_math`, `native_math_differential` and `tool_tests`.
No Source changed after that successful build or the run.

Five complete capture phases retain 22,403 rows, plus 170 supplemental input
rows. The selected path has 43 complete objects, 39 unique complete archive
members, 85 Source/header files and 871 unique prefrozen consumed compiler
inputs. Historic captures are verified against their own phases, with current
artifacts checked against `after_build`; the incomplete first optional-PDB
capture is retained but excluded. This is not an exhaustive OS closure claim.

Before execution, the preserved static gate checked 1,833 protected complete
extents with ordered relocations, the unchanged earlier diagnostic, 45 linked
production bodies/282 relocations, and 75 changed or new CLI bodies. The new
4,981-byte diagnostic body/147 relocations was fully replayed. Four other mapped
EH bodies were explicitly not fully replayed at that point; 61 further owned
COFF extents have no standalone normal MAP symbol.

**After execution**, the additional read-only audit replayed all 124 unique
linked bodies/1,345 relocations, including those four EH bodies/190 relocations.
All 19 compiler-local operands now use actual relocation symbol-table indices,
exact COFF section/value, and a unique containing function's same-object MAP
anchor. Direct-object origins use their final token. The first static gate and
failed audit drivers remain unchanged. This later proof is not represented as
having existed before the run. Eight removed compiler names retain all full
byte/relocation correspondences; only three have one match, and no semantic
equivalence is inferred for the five unmatched names.

The independent runtime audit reconstructed **37 complete records** and checked
six full named loaded code bodies against the retained fresh PE. It confirmed
the actual provider/root, six owner graph stages, registrations 6 to 8, real
HANDLE file identity, one exact 64-byte read, same-pool release, null publications
and retired-borrow rejection. No native object record occurs after shared drain.

## Artifacts and limits

The machine-readable report is
[`reports/cc12_connected_physical_read_owner_source.json`](../reports/cc12_connected_physical_read_owner_source.json).
The local evidence family is
`local/cc12_connected_physical_read_owner_source_20261008a/`:

- `proof01/static_gate.json`: preserved pre-execution gate,
  SHA-256 `152f3a2dd9f9448a1b17488f313773867b6844380929184eb9a397c42ce1125a`.
- `diagnostic01/execution_receipt.json` and `diagnostic.log`: sole worker run;
  log SHA-256 `94a765842bf4dd3220b2223989e848f300991f428fad6fffc868d5455c5c0920`.
- `audit01/runtime_audit.json`: independent captured runtime audit,
  SHA-256 `b33786b03824fa57e74e0bc5840d6aeac8bbf0656029a0604a7029a6176eb671`.
- `postlink05/additional_link_audit.json`: explicitly post-run whole-body audit,
  SHA-256 `e3d69fb70256ceb9b161d3a8a38bddd90b52b6d063ef52afd9b53911a843e63c`.
- `final01/seal.json`: final frozen/current artifact and exact five-file scope
  verification, with report and Source pins.

The embedded manager mount header, provider/root and actual mount/open route
are recorded; internal mount-tree nodes are not separately logged. The existing
Source discards `CloseHandle`'s BOOL, so the evidence is completed release-path
return and invalidated HANDLE storage, not an observed close result. The CRT
callback is not separately instrumented: registration 0 and normal process
exit are observed. No raw notifier, Original function, new probe, new target or
new test was introduced or executed. No failure-case, concurrent-use, raw ABI,
general startup, rendering or gameplay claim follows from this bounded case.
The supported input size/prefix and provider body lengths are intentionally
pinned; a changed build requires a fresh qualification gate.

## Independent primary acceptance

The integrator rehashed all 22,573 historic frozen rows and retained 4,494 current physical inputs before compiling. All 913 actually consumed selected compiler inputs were present in that freeze. Three edited Source files changed from CRLF to LF during Git checkout; all other bytes match, and the actual integrator files are separately retained. Previously accepted Type6/wake changes and current build recipes/artifacts are identified explicitly rather than called worker-byte-equal. The normal Win32 build passed all three existing checks.

All 43 current selected objects and their 5,096 complete function extents match the worker graph with exact bytes, ordered relocations and explicit compiler-name bijections. Each of the 39 Core objects appears exactly once as a full current archive member. Independent replay verified all 124 linked bodies and 1,345 relocations against the fresh executable and map **before** the integrator execution, including every compiler-local operand by its actual COFF symbol index and containing function. This timing differs from the preserved worker post-run extension.

The integrator then executed one normal parent/canonical-child diagnostic at `2026-10-08T19:25:51.120602+00:00`, parent PID 115092, exit 0. Its independent audit verified all 37 complete records and six complete loaded Source bodies. The actual installed-file handle identity and 64 read bytes matched; only stream position changed from 0 to 64 before release. Reference-zero release returned the dead backing to the same live pool, shared drain completed, and a subsequent borrow rejected. The installed executable and the tested rebuilt executable retained their hashes. No Source changed after the current build/run.

Primary receipt: `local/cc12_connected_physical_read_owner_source_primary_review/receipt.json`, SHA-256 `5d48e91f26c04e3e271439cd0eff156400a1be75f6ccea2299d0a4159900eb17`. Static gate: `e0930e7e49b41dd4f6ef3066f04f338c26d42d919bc6b0f26c6d19d7be91cd7b`; runtime audit: `294063b8e91c8b96b11a6cfcefa81cd8db8dadd22041fe19b8e1b19ed2943081`. Worker and integrator each ran once: two historic/current parent executions total, no retries. Retained review-driver failures are evidence-format/history handling errors and caused no extra executable run or Source change. This establishes the bounded Source read/ownership path; zero new Original-function credit, no raw ABI or failed-read execution, no separately instrumented CRT callback, and no general startup or gameplay proof.

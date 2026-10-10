# CC12 raw-context wide-header destructor

This packet adds only the `NativeStringRawPoolContext&` overload of
`destroy_native_wide_string_header_00436430`. It permits the actual raw pool
getter to propagate a C++ exception, matching the existing raw narrow-header
destructor's provider route. The `NativeStringStorage&` overload remains
`noexcept` and unchanged. No caller is switched to the new overload.

The actual eight-byte header retains its existing contract: DWORD length at
`+0`, buffer at `+4`. The new overload reads the buffer first. Null skips the
length read, getter and return helper. Otherwise it captures the current length
and computes unsigned, wrapping `2*length+2` before calling the actual
`00419CC0` provider with the supplied `01090AA8` pool and `01090AA0` raw-manager
publication cells. The returned owner, captured buffer and size are then passed
to the existing `00BD1510` provider with the supplied live `01090AA4` gate.
Neither header field is written or reread after the getter.

The recovered native `00436430..0043644E` function uses the header in `ECX` and
returns with `RET`; see [the existing wide-header evidence](NATIVE_WIDE_STRING.md).
The accepted [node-owner evidence](CC12_LOCALE_MAP_NODE_OWNER_READINESS.md)
also records the inline wide stage's pointer/size capture, getter-before-return
ordering, and native `(block, size, unused=1)` packet. This new normalized C++
overload reuses the existing return provider, whose Source API omits that unused
native argument. It does not reproduce the original calling convention, native
stack packet, or EH/SEH frame ABI. No new Native queries or Ghidra writes were
performed for this Source-only packet.

## Build and compiled evidence

The normal `scripts/build.ps1` completed in the worker worktree. Its three
existing checks passed: `reconstructed_math`, `native_math_differential`, and
`tool_tests`. No new behavioral test, probe, or game run was added.

The new overload is a complete 57-byte COFF section with 26 decoded x86
instructions. The pointer test branches at `+0Ch` to `+36h`, bypassing both the
length load at `+0Eh` and both providers. `LEA EDI,[EAX*2+2]` at `+15h` captures
the wrapping byte count before the getter call at `+21h`. The REL32 operands at
`+22h` and `+2Dh` name the raw-manager `00419CC0` overload and `00BD1510` return
provider, respectively. `EBX` and `EDI` retain the buffer and size across the
getter. The complete decode and relocations are in the report and evidence ZIP.

Compiler dependency logs identify three actual consumers of the changed header.
All their complete objects and sections are retained before and after the build,
alongside the complete narrow-string object:

| Object | Existing function sections preserved byte-for-byte, including relocations | New function sections |
| --- | ---: | ---: |
| `native_wide_string.obj` | 7 | 1 |
| `native_online_notification_dispatch.obj` | 50 | 0 |
| `native_online_signin_ui.obj` | 220 | 0 |
| `native_string.obj` | 45 | 0 |

The narrow-string whole object is byte-identical. The other whole objects differ;
the table claims preservation of their existing function sections, not identical
object metadata. In addition, the old wide storage overload (34 bytes), narrow
storage overload (28 bytes), and raw narrow overload (51 bytes) each have exact
bytes and relocations across the frozen Root, worker-before, and worker-after
cohorts. Complete COFF parsing retains all 15 zero-raw-pointer sections across
the selected cohorts with empty file payloads and their declared storage sizes.

## Source and artifact qualification

The copied Root baseline is explicitly
`Source3352-particle-model-typed-views`: 3,352 source/build inputs plus four
artifacts. All 3,356 frozen pins matched live Root at the pre-edit capture.
The worker's pre-edit Git commit was
`d6f514825b71062f6c36a50f622f44e3330a0376`; its 3,352 canonical blobs all matched
the worker's current bytes. Against frozen Root, 3,345 worker source inputs were
byte-exact and all 3,352 were LF-equivalent. The seven differences were line
endings and are retained as distinct byte cohorts.

After the build, exactly the wide header and implementation differ from worker
before/Git. The complete post-build compiler-input cohort is retained: 3,350
inputs match worker-before exactly; 3,343 match frozen Root exactly, and 3,350
are LF-equivalent to frozen Root. This is a new mixed source epoch, not an
unchanged or fully rebuilt copy of the frozen Root epoch. The before artifacts
were captured as existing artifacts; their provenance is not upgraded to a new
before-build claim. All four current worker artifacts and all three complete
Core archive indices are retained (1,988 object members each).

An actual before/after census of `bsp_game.dir/Release` found 80 worker objects,
all byte-identical. This is only that worker directory census. It neither
borrows Root's App census nor proves every retained directory entry belongs to
the current generated project, nor establishes runtime behavior.

## Portable review and remaining boundaries

`local/cc12_native_wide_string_raw_context/evidence.zip` retains 13,648 pinned
files. It is 115,112,273 bytes with SHA-256
`b9746ba00e9cee16dc0df8d98152c893cd2ce9ca2dad4bbd782bc4cd4417f9f7`.
The archive includes frozen/current/Git Source cohorts, whole Core archives,
complete selected objects, dependency logs, build results, and the capture and
ZIP-only review scripts. A physical copy under `physical_relocation` passed
the same review using only the copied ZIP and scripts extracted from it.
The review checks retained bytes, source differences, archive membership,
complete section/symbol/relocation parsing, the positive new overload, and
preservation of old function sections. It does not invoke a compiler, Native
query, or executable.

This closes only the raw-wide Source helper gap. Raw node cleanup, map/base/
locale lifetime composition, caller activation, injected exception behavior,
native ABI parity and game validation remain separate. Root review and its own
build are still required before integration. This packet makes no ledger edit.

Machine-readable details: [report](../reports/cc12_native_wide_string_raw_context.json).

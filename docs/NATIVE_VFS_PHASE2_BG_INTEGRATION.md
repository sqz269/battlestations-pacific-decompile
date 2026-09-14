# Native VFS phase 2 integration (BE/BF/BG)

`GameNativeVfsRuntime` now retains the actual enumeration and package-scan
contexts through the shared raw manager drain. Its phase-2 entry performs the
three loose mounts, two fresh package scans, and all 78 original resource-search
registrations before the later MPAK factory/cache tail. Provider dispatch uses
the captured native table entry and concrete physical, FileStore, MPKG and MPAK
implementations. No projected provider tree is seeded by the graph fixture.

The batch registers all eight new translation units in `cmake/startup.cmake`.
That file's integration lease was released once source registration and the
combined build finished, so other orchestrators can register their work.

## Review corrections and native evidence

- The 14,197-byte `00738360` sequence reloads `0109CEEC` after both temporary
  headers and before **each** registration. Its source now accepts the actual
  volatile publication reference. All 78 load sites and 156 literal operands
  were checked; registration 54 uses native `pfv`, while registration 53 uses
  `pfx`. The source had incorrectly used `pfx` for both.
- `0073CB10` had a saved body ending after `_free` at `0073CDFB`. Its 22-byte
  return tail is restored through `0073CE15`. `00BE1130` likewise omitted 24
  epilogue bytes after `00BE11F8`; its complete body is 229 bytes and 80
  instructions through the `RET 8` ending at `00BE1214`.
- The reported exclusive ends of `00557A90` and `00BDB120` cut through their
  final return instructions. The corrected complete envelopes are respectively
  159 bytes ending at `00557B2E`, and 190 bytes ending at `00BDB1DD`.
- Final primary evidence covers 16 complete live/disk envelopes, 5,831 saved
  instruction owners and 799 internal direct CALLs. Across worker and primary
  reports, 1,701 checked rows represent 810 unique direct CALL sites. The
  preliminary 5,822-instruction count is retained with an explicit correction.
- Sixteen names, native ABI declarations and evidence comments were saved and
  re-exported. Prior names/comments, labels and relevant propagated local
  names/storage were retained. Only proven returning call-site overrides were
  repaired; callee no-return flags were not changed. `__thiscall` declarations
  exclude the implicit ECX parameter, and explicit stack argument bytes were
  independently checked against native return cleanup. The MPKG thunk inherits
  its target's cleanup.

The receipts are `reports/native_vfs_phase2_bg_annotations.json`,
`reports/native_vfs_phase2_bg_body_repair.json` and
`reports/native_vfs_phase2_bg_flow_repair.json`. The primary validation report
preserves prior incorrect ranges alongside the corrected evidence.

## Exact source and installed-assets check

The combined source revision is
`dcff4d91d2709a7124bfd39826a12256b5f3b07a`: 2,493 code/config/build-input files,
strict MSVC Win32 Release build and both existing CTests passed. The resulting
core archive SHA-256 is
`a38213fa1488744a7f413276543108d4cf467ab083f1ed90dd9019856c66a517`.
All eight original seed spans match disk. Exact source, artifact, fixture and
log hashes are pinned in `reports/native_vfs_phase2_bg_validation.json`.

The existing manifested graph probe was extended in ignored `local/`. It
constructs the raw manager, performs both scans, and verifies three mounts,
six prepended search groups, all 13 extension-prefix pairs, and these actual
stored group counts:

| Group, in list order | Extensions | Directories |
| --- | ---: | ---: |
| mpaks | 1 | 6 |
| events | 1 | 1 |
| sound | 1 | 11 |
| fshaders | 1 | 7 |
| shaderfx | 1 | 10 |
| textures | 2 | 23 |

After the archive-factory tail, it verifies four factories, reads the installed
657-byte `scripts/fundamentals.lua` byte-for-byte, and drains the same raw
manager and retained owners. No permanent test suite was added. The inspected
installation contains no discovered MPKG/MPAK archives: archive payload
enumeration, populated archive reads and duplicate diagnostics were not
runtime-exercised by this fixture.

## Remaining executable and fidelity work

`GameVfsHost` and its consumers still require a coordinated move to this raw
manager. `BDD0A0` source specializes the enumeration visitor profile; other
profiles remain separate implementations. Physical enumeration's one-byte
append helper getter timing, original FH3/CRT identity, malformed/huge-length
domains and drop-in ABI compatibility remain outside the verified contract.

The explicit `0109CEF0` empty input is supported by the original PE's `.data`
zero-fill region and Ghidra's initial zero byte. It does not claim the original
global's fixed address or runtime immutability.

The graph probe requires four numeric read-only bands to be free. The separate
`docs/NATIVE_DATA_STARTUP_RESERVATION_BG.md` packet identifies a secondary heap
occupying `D10000` before executable TLS in 11/160 final sample starts. A
suspended launcher reserved all four bands in 120/120 measured starts and
preserved unrelated allocations during injected rollback. Production ownership
handoff to the native-data mapper is still required; finite probe success is
not a reliability guarantee. No original game process or gameplay was tested.

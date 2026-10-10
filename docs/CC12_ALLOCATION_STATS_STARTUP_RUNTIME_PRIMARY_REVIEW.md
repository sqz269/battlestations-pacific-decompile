# Allocation statistics startup runtime: primary review

The accepted Source3348 executable completed the genuine allocation-statistics
constructor and reached the same previously observed FMOD failure. Startup and
gameplay remain unvalidated: the captured run created no window or device and
presented no frames. This packet changes no C++ and does not rerun the executable.

Worker `ebda6fa0244c66bcdc23d4dfa0f553b4e4211d18` recorded exactly one
reconstructed Source execution, PID 50296, at 2026-10-10 13:25:52 UTC. It exited
with code 1 after 0.226 seconds, within the three-frame/30-second limit. The
hidden launch and 16 process-specific window scans observed no top-level windows;
this is not visual validation or proof against an unobserved transient window.
The original executable was not run or changed.

The selected immutable executable is 5,099,520 bytes, SHA-256
`0d2959feb6b426d74907e495b0c48dd9d184ffb30789873c92120c7936d99bc5`.
Only its path, the log path and the copied personal-options root differ from the
prior accepted startup command. The copied options, explicit DLLs and other
captured live inputs were unchanged after execution.

## What the run establishes

Log line 57 follows the genuine `00BE2900` constructor, the single AB0 null
comparison and the successful handover state. The accepted Source allocator
returns a nonnull allocation or throws; it cannot reach this marker through a
null-allocation skip. Lines 58, 59 and 62 subsequently reach clock, resolver and
VFS continuations. These accepted Source statements and the reached markers
support normal Source constructor completion. No live receiver, publication
cell or manager-slot instrumentation was added. Native register ABI, FH3 and
hardware-fault behavior remain outside this evidence.

Lines 84–87 reproduce `_FMOD_EventSystem_Init@20` result 61, sound creation
result 78, length retrieval result 37, then the existing raw-length output guard
for `sound/gui/error.fsb` (2,688 bytes, mode 2634, no returned bank). The guard
stops before reading an unwritten length output. Line 88 records zero windows,
devices, presented frames and completed loops. This does not identify the
underlying Windows/audio cause or establish corrupt bank data. No SDK probe,
output selection, OS repair or original-game execution was performed.

## Independent retained-evidence review

The primary copied the complete worker ZIP, verifier, runtime files, document
and report. The unchanged verifier passed against the copied ZIP, reading its
retained bytes without another runtime execution. The ZIP is 61,375,209 bytes,
SHA-256 `83a8e7ef4579bc26f660ccdc312846eeadc426068ff8179eb946fc24d51c4e2f`,
with 3,469 payloads totalling 213,852,900 uncompressed bytes.

The primary independently compared all 3,352 accepted Source/artifact pins
between its current files, its immutable Source3348 freeze and the worker ZIP.
All matched. It independently checked 3,348 complete Git blob preimages and
their LF-normalized equality to the accepted Source. The retained complete Root
manifest and artifact receipt equal the original accepted receipts. The prior
normal MSVC Win32 build and three existing checks passed; they were not rerun
for this evidence-only packet. The replay rechecks 1,057 complete selected Core
member payloads and 70 complete App objects. The 320 positive Core definitions
remain the prior complete archive receipt, not a newly executed experiment.

The portable packet does not capture the complete OS/module/file-I/O/package
environment or recursively rerun older nested evidence bundles. Its absolute
historical paths remain provenance labels. The full primary receipt is
`reports/cc12_allocation_stats_startup_runtime_primary_review.json`; complete
local evidence is under `local/cc12_stats_startup_Source_primary/runtime/`.
The overall reconstruction goal remains active.

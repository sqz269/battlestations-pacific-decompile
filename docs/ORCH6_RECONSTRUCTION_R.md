# Orchestrator 6 reconstruction batch R

Addresses: 00925C40, 00925C90, 00696330, 00696340, 00693550, 00693560, 0042B970, 00522E90, 00695F90, 00CD3910, 00CD3940, 00CDF4A0, 00CDF4B0, 00CCD6A0, 00926C80.

R integrates the observer notification wrappers, the core dispatch schedule and the actual pending entity list owner wrappers. Application startup now publishes the actual observer dispatch owner through the existing raw singleton manager. Names remain hypotheses, with original ABI and evidence boundaries in the individual reports.

| Change | Verified scope |
| --- | --- |
| [Event wrappers](OBSERVER_EVENT_PRODUCER.md) preserve the original endpoint and producer-specific getter | Five native/source pairs, 184 original bytes, 913 ordered words per side, zero mismatches |
| [Dispatch schedule](OBSERVER_DISPATCH_SCHEDULE.md) supports nested dispatch and callback mutation over the actual shared vector | Ten original spans, 1485 bytes, 88 matched canonical words; source-only exception behavior recorded separately |
| [Pending owners](NATIVE_PENDING_ENTITY_OWNERS.md) explicitly initialize and destroy the two raw lists | Four native/source comparisons, 290 original bytes, allocation failure and real CRT shutdown checked |
| [Application observer lifetime](GAME_OBSERVER_RUNTIME.md) publishes through the actual application manager | The process log records publication and teardown; the unused observer lock stays lazy |
| [Destroy recursion](UNIT_DAMAGE_AND_DEATH.md) uses the argument low byte and refreshes the parent cause | Independent native listing review and required build checks |

MSVC Win32 Release and both existing CTests passed at `61223cd893778d1eb27ba0872e4affd78f8916d1`. The executable SHA256 is `5cd31e110f74fcbd3d64d848567bafd9ba6788fb401cbb0f3a689a9a85455a8f`. The 120-frame USN01 process run completed with 18,557 finite trajectory rows, 241 unchanged Airfield2 samples, 2,400 avoidance queries, 1,080 cruise reads, 10,080 generic ticks and 420 valid world registration nodes. It also verified actual observer dispatch publication and raw-manager teardown. Exact commands and counters are in `reports/orch6_reconstruction_r.json`.

Review removed copied STL helper bodies from the dispatch implementation; canonical vector operations are reused after byte/provider equivalence checks. Original helper bytes remain in the ignored differential oracle. Event-wrapper comparisons use the same reviewed schedule on both sides; the independent core fixture supplies its separate evidence.

The process run does not establish execution of observer event delivery, pending-list population or the corrected destroy branch. Original exceptions, concurrency, gameplay parity and binary replacement compatibility remain unproved. The local evidence archive retains 688 hash-verified artifacts before worker worktree cleanup.

Two secondary-pointer adjustment thunks and two exception-handler entries were defined, saved and exported for S. Two free-call continuations were decoded, but their instructions remain outside Ghidra's stored function bodies; skipped padding was left unchanged. See `reports/observer_endpoint_function_definitions.json` and `reports/observed_endpoint_flow_repair.json`.

## Follow-up packets

S recovers the actual unit observer prefix stores and stable unit aliases, primary observed-endpoint lifetime, and process storage with real pending-list CRT callbacks. Real unit callback providers and producer/drain bindings remain necessary before runtime event delivery can be claimed.

## Compiler artifact provenance correction from S

R retained Hostx64/x86 compiler/linker binaries, while its fixture command used `vcvars32.bat`. S verifies that this selects Hostx86/x86 and retains those resolved tools and scripts. R lacked hashes for the selected fixture compiler/linker binaries; its immutable manifest is unchanged. This correction does not alter independently retained source, object, library, executable or result hashes. See `reports/game_pending_entity_runtime.json` for paths, hashes and the toolchain log.

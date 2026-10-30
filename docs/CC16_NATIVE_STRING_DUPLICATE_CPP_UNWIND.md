# One actual Source duplicate allocation-failure check

The existing raw `duplicate_native_string_00438e40` admission covers successful/null calls and leaves failure transport through its naked frame unproved. The hidden-entity record constructor inherits that gap. A new normal-copy fixture would duplicate existing evidence, so this packet checks one concrete missing path using the actual current Source helper and allocator. It changes no production source, public contract, caller, guard or failure policy.

One owned Win32 probe at Source head `03ef58daf98d9dff844e8a4039be830868d429b7` ran once and passed. The recorded process was PID64572, creation FILETIME134378080556956158, and the tool observed terminal exit0. This is execution of the compiled Source helper, not the original executable, a relocated Original copy, a game, or a DLL playback test.

## Actual failure and caller

The input is a genuinely committed16MiB readable buffer, filled with nonzero bytes and a final NUL. The probe creates a private unnamed/noninherited job, assigns only itself and verifies that its sole member is its actual PID. After input construction it reads private commit and sets only that job's per-process limit to private commit plus4MiB. It does not exhaust machine memory or modify another process/job.

It installs the real CRT `_set_new_handler` hook. The hook only records bytes/count and returns0; it is not an allocation replacement or a synthetic throwing game callback. The real `singleton_lifetime_allocate` calls actual malloc, then `_callnewh`, then throws its own `std::bad_alloc`. The ordinary caller invokes the actual raw helper inside a try block containing one independent observation marker, with a typed bad_alloc catch and separate catch-all. Four volatile stack canaries remain live across that call.

The observed malloc request/new-handler size was exactly16,777,216 bytes, with one handler call. The typed bad_alloc catch ran, the catch-all and normal-return branch did not, and the marker was constructed/destroyed once. All four canaries and the three checked input endpoints were intact. The recorded private commit was17,813,504 bytes and the job limit22,007,808 bytes.

After observation, the limit-reset API reported success; the handler setter returned the expected prior no-retry hook. Input-release and handle-close APIs also reported success, and the process exited. No post-restore limit/handler readback or pre-exit job detachment was recorded. These API acknowledgements are not represented as those additional observations. Input endpoint checks are not a complete post-run input hash; four canaries are not whole-stack or register-ABI proof.

## Physical inputs and immutable receipt

The i386 probe has an embedded manifest and links the helper from the actual common `bsp_core` object. Static verification checks the entire57-byte function:49 non-CALL bytes match the original; the two actual relative calls reach the genuine size adapter and current CRT memcpy. Final PE bytes match the object after its actual relocations. There is no numeric Original-code call or copied Original executable fixture.

The emitted ordinary caller has maxState3, try0..1/catchHigh2, state1 before the raw call and marker action1->0. The typed std::bad_alloc handler, catch-all, volatile stores and post-catch reads are physical emitted code. This checks the stated caller's Source cleanup, not every callee-saved register, arbitrary private-frame aliases or Original FH3.

Twelve immutable files are pinned by `local/raw_duplicate_cpp_unwind/case_manifest.json`, SHA256 `d0f3b598e4b96a29985e4c47e863c47b9574e9cc437014fe14ba23e08e3a6b4e`. They include Source/command/object/listing/map/manifest-embedded executable, build log, defining Core objects, preflight, run output and initial report. The initial report's SHA256 is `6d72455b2604754ce9ff0464413ae015aeb82c20c0b30a4df6ca27b8bc488d26`. Later review metadata does not overwrite this frozen report. Independent review `ac6d45fd269abc12e2457e8603d73d2ce9df0ffa` passes; the primary read its entire58-line document and verified20 retained path digests. Its nine complete selected Source/EH sections (1,836 bytes) and105 relocations match the frozen objects and PE.

## Strict claim and remaining gates

This establishes only the frozen caller/compiler/input and actual no-retry allocation-failure path in the observed Source environment. Loaded CRT DLL identities/versions were not captured, so it is not a portable frozen-runtime attestation. Handler retry/reentry/arbitrary throws, other Source callers, full hidden-record construction/cleanup, ownership, current publications and whole-family runtime admission remain separate.

The original private CRT/heap/types/terminal policy, register and private-frame ABI, FH3/SEH/hardware faults and Game/GUI activation are not proved. The existing public header and previous qualified general admission remain unchanged. No new repository test suite/framework, automatic rollback, terminal policy or Source implementation is added. Unique Original/vendor/duplicate-body, ABI, full-game runtime and activation credit are zero. The narrow Source execution is recorded explicitly, rather than being called static-only evidence.

Machine-readable evidence: `reports/cc16_native_string_duplicate_cpp_unwind.json`.

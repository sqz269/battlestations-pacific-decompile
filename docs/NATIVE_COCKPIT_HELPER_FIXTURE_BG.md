# Focused cockpit helper terminal fixture

Addresses: 00B3C5C0, 00B3C6C0. Descriptive source names are hypotheses.

One original/source pair passed at x87 CW027F. This extends the existing BF camera
lifecycle fixture without expanding its broader checkpoint suite. Each path has
15 matching camera checkpoints (16,740 camera bytes), equal 36-byte normalized
helper pre-free observations, and 8,880 mapped code bytes unchanged after execution.

The fixture constructs an actual raw camera through the established BF camera,
viewport, renderer, node and native pool providers, including the existing viewport
replacement exercise. Its actual count is still one when handed to the helper.
The source uses the canonical `NativeCameraReference` binding for that same camera.
The helper occupies actual 24h storage from the existing BF681B/free observation
domain. Its already-constructed setup has profile D61854, count one, camera+0C,
zero +08/+18, and distinct +10/+14/+1C/+20 values preserved across destruction.
This setup does not execute or reconstruct B3C800.

The fixture drives the actual helper+04 zero transition. The original side reloads
the current D61854 table and invokes BD30E0, which reads current slot04 and pushes
flag1. It executes original B3C6C0 and B3C5C0, including the repaired ADD ESP,4 at
B3C6D5. The source uses `NativeCockpitHelperReference` over actual+04 and its real
owner implementation, compiled explicitly in WorkerSource mode.

| Routine or transfer | Evidence and measured coverage |
| --- | --- |
| B3C5C0..B3C649 | 138-byte complete original body loaded; normal camera-nonnull, +08-zero, +18-zero route executed. |
| B3C6C0..B3C6DD | 30-byte complete original body loaded; flags1 terminal route executed. ECX helper, DWORD flags, RET4. |
| B3C5F2 -> B6DFA0 | ECX current camera+0C, no stack arguments. Retained camera cleanup observation sees helper camera member still set. |
| B6E007 -> current camera slot18 | B6F310 logical release; actual camera count one becomes zero and current slot0 dispatch reaches BD30E0. |
| B3C633 -> BD30F0 | Helper stamps CEB130 after camera teardown and member clear. |
| B3C6C3 -> B3C5C0 | Captured ECX helper, no stack arguments. |
| B3C6D0 -> BF65AC | PUSH captured ESI helper; ADD ESP,4 at B3C6D5. Exactly one actual24h free. |

At helper pre-free observation both paths have CEB130, actual count zero, and
cleared camera+0C. The four untouched words match their setup values. The actual
camera native pool already has all 32 slots free and its raw name has returned
once. The source camera companion is retired after camera cleanup, and the helper
companion retirement callback occurs after the actual helper free and deletes both
separately allocated helper host companions. Post-release code observes only
external flags and camera state; no actual helper bytes are accessed after free.

No production allocator, provider, CMake, header, source, test or ledger file was
changed by this packet. The old BF archive remains SHA256
`8806f59c804d3c5918b5a2e7dc2d9f2ab8ef0cb18327a6f88aa9d49cd02de13f`.
The strict build used Win32 MSVC `/W4 /WX /fp:strict /MD /O2 /MANIFEST:EMBED`.
Selected src/include, worker helper inputs, fixture, recipe and the three current
libraries were hashed before and after the successful run. Baseline libraries did
not yet contain the new helper TU. Four report call rows passed live validation.

Revised capture: `C:/Users/sqz269/bsp-bg-cockpit-lifetime/worker_capture_v2.zip`.
Archive SHA256: `6d6967c7840648ead7a68b609472969470efdddeb26132853d17edef84a276f1`.
The initial `worker_capture.zip` remains unchanged at SHA256
`26202022b3b3f8bb97ecfc479a82dc078c5d9f9344d675bfb27a592c96817ba7`.
That initial control observed retirement ordering with surviving host companions
and used worker include precedence. Revision 2 executes deletion of both helper
companions and uses only selected root headers, all covered by its input pins.
The adjacent `archive_manifest.json` enumerates contents, including original span
bytes, live disassembly and byte comparison, source input ZIP, library copies,
build recipe, executable, logs, and both snapshot files. The 176 new helper/table
bytes match guarded live Ghidra and the installed PE with SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Analysis was read only against `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, bridge8089.

Worker replay:

```powershell
pwsh -NoProfile -File C:/Users/sqz269/bsp-bg-cockpit-lifetime/run.ps1 -Repo J:/PROG/battlestations-pacific-decompile-orch3-20260910 -WorkerSource -WorkerRepo J:/PROG/battlestations-pacific-decompile-orch3-cockpit-fixture-bg
```

After central registration and build, omit both worker options for the integration
replay. Default mode compiles only the fixture and links the selected root's three
current libraries. Preserve the sealed ZIP before replaying the working directory.

Coverage excludes nonzero helper+08/+18, the unknown retained18 producer, original
B3C800 construction, native FH3/SEH exceptions, and a native releasing caller above
the fixture-driven count transition. Source failure handling was reviewed only.
The camera field is never a substitute model node; renderer bindings and actual
raw string-pool providers are inherited unchanged from BF. This is bounded fixture evidence, not
drop-in ABI compatibility or game validation.

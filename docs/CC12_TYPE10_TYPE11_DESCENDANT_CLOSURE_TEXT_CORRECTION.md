# Type10/11 descendant-closure TEXT correction

The existing 540-second inner Python timeout can end the direct child before
the 600-second PowerShell wait expires, leaving the old tree-kill branch
unreached. This packet proposes an explicit, independently pinned Job Object
supervisor that cleans up its job even after the direct child exits. It remains
**Source=0, unadopted and unexecuted**. Passive review does not prove runtime
cleanup. CPython documents that `run` kills and waits for its direct child on
timeout; that alone is not a descendant-closure observation.
[CPython subprocess](https://docs.python.org/3.13/library/subprocess.html)

Packet `cc12_type1011_descendant_closure_TEXT_correction` owns zero addresses
and only this document plus
`reports/cc12_type10_type11_descendant_closure_TEXT_correction.json`.
The worker started from Main `c99da7ed0d346209a66a2a4a6b861d748b56fe07`.

## Explicit added execution layer

The proposed route is PowerShell → new `external_job_supervisor.py` → unchanged
`external_python_driver.py` → unchanged selected program. The original child
argument vector, cwd, Python executable, isolated flags, and exact root Python
environment are retained. Existing compiler/reader subprocesses remain under
the original recipe/bootstrap checks and inherit the job through their parent.

Two new concrete Root fields, `external_job_supervisor` and
`external_job_policy`, remain null alongside all eleven previous null fields.
Root must independently pin the supervisor in `selected_code` and select its
policy. The template preserves every previous value, including the old held
dependency snapshot, and adds an explanatory correction record. It grants no
acceptance, receipt, materialization, or execution admission.

The fresh ignored family is `local/t1011jobTEXT` in the worker worktree. Final
inputs are identified by `FINAL_UNADOPTED_TEXT_INPUTS_v2.json`:

| New or changed input | Bytes | SHA-256 |
| --- | ---: | --- |
| `candidate_TEXT_v4/external_job_supervisor.py.txt` | 18910 | `2fe6ffb82fae33f55d3631f348e5efc08eca0a42dd0d46d23014e5206c552ccd` |
| `candidate_TEXT_v4/dependency_bootstrap.py.txt` | 15638 | `5f366715e70485ff2c2936df0a9f2dcef6dc60fa350e1543a977d53c90f9a8f9` |
| `candidate_TEXT_v4/external_preinterpreter_guard.ps1.txt` | 17775 | `e3a74da3e7cd4ab5727bc308464e702749461d5a3e438439732d49e9f0025e38` |
| `proposed_job_policy_UNADOPTED.json` | 490 | `b117b7541f0eb4e93a0fed835cf3e5c59198286f8feaad34368bf7e214268796` |
| `proposed_dependency_selection_template_v5.json` | 8959335 | `49b9a88b2a6a7be0f0b183d2fd67d88909d4dfb8ec282cdf7ebfea87bcbe6c6e` |

The other five existing payloads are byte-identical. All eight final payload
pins are in the report; `.txt` files were never imported or executed.

## Proposed lifetime mechanism

The supervisor creates an anonymous, non-inheritable job, sets only
`JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`, and verifies those limits. It supplies
`PROC_THREAD_ATTRIBUTE_JOB_LIST` during `CreateProcessW`, also requesting
`CREATE_SUSPENDED`, Unicode ENV, extended startup information and no window.
It checks job membership before resuming the initial thread. Creation-time
assignment avoids the orphan interval between separately creating a suspended
process and assigning it to a job.
[Microsoft's creation-time job assignment explanation](https://devblogs.microsoft.com/oldnewthing/20230209-00/?p=107812)

Only duplicates of the existing stdin/stdout/stderr handles are inherited;
the job handle is excluded. Both breakaway flags are absent. The supervisor
retains its handle until cleanup. Ordinary `CreateProcess` descendants remain
associated; existing external services and independently brokered/WMI processes
are outside this process-tree claim.
[Job Objects](https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects),
[startup attributes](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-updateprocthreadattribute)

Every exit path after job creation requests job termination, including normal
completion and early nonzero exit. Cleanup then polls basic accounting for zero
active processes and requires the root process handle to be signaled before
closing handles. Termination alone is insufficient. A failed pre-resume
membership check additionally attempts direct cleanup of the unresumed child.
[TerminateJobObject](https://learn.microsoft.com/en-us/windows/win32/api/jobapi2/nf-jobapi2-terminatejobobject),
[job accounting](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_basic_accounting_information),
[nested-job accounting](https://learn.microsoft.com/en-us/windows/win32/procthread/nested-jobs)

The separate policy proposes a 560-second supervisor run deadline and a
20-second cleanup budget, inside the unchanged 600-second outer watchdog;
the original 540-second inner timeout remains. Setup, filesystem operations
and postflight can still exhaust the outer watchdog. Abrupt supervisor death
uses last-handle-close termination as a fallback, but **a missing or invalid
closure record never establishes closure or permits advancement**. A normal
zero exit that leaves active descendants is cleaned up and held unsuccessful.

The exclusive job record binds the selection, dispatch attempt, supervisor,
policy, environment pin, original argv and supervisor PID. It records the root
result, termination attempt, final accounting, handle closure and postflight.
PowerShell requires the matching zero-active closure in both its final
advancement gate and `successful_outer_result`. Existing output-pipe, outer
timeout, exit-code, content, environment and postflight checks remain.

## Passive preservation evidence

The prior `local/t1011hostEnvTEXT` seal was reverified with all 13,047 artifact
hashes and exact 13,048-file membership. Its 2,616,714-byte seal SHA-256 remains
`843709f896d31e6a9213464ce8c34b710413d8c07dfca90e896bcd094b607b8f`.
Fresh fixed copies cover the final seven payloads, template, final-input record,
private profiles and all 119 materialization files. All 119 files, including
the Source/header/probe/case inputs, remain byte-identical.

All 340 original named Python guards remain; the modified bootstrap adds one.
Only its `initialize` and `verify_snapshot` functions change. Its inner timeout
wrapper, runtime description, environment checker and mapped-Capstone guard
are AST-identical. Recipe compile/link/launch functions and all four programs
are unchanged. PowerShell retains all 36 original `Require` calls and 11
`Check` calls, in order; totals become 47 and 15. Original mandatory parameters,
host/membership functions, root ENV/cwd, child argv, 600-second wait and fallback
kill text remain. Python AST and PowerShell parsing passed.

Before optional own analysis imports, 2,102 files and 3,588 membership records
across 16 scopes were freshly frozen. Six SDK headers were separately frozen
as data. Own stdlib-only review modeled seven x64 layouts from those headers
and inspected 18 Win32 bindings. This is not ABI or API execution proof.
Pre/post/final checks verified 2,239 original/frozen pairs, all previous sealed
artifacts, scope membership and unchanged private ENV. Primary API sources are
recorded in `primary_contract_sources.json`.

The first own finalizer stopped because its basename check mistook the copied
prior seal for the new root seal. Its source and partial outputs remain;
`finalize_own2.ps1` uses the exact root path. Final conclusions are in
`own_final_conclusions2.json`; the corrected finalizer passed.

The new family is sealed at **2,294 artifacts plus seal, 2,295 exact files**.
The seal is 428,873 bytes, SHA-256
`3b4b64201752fcd96204b05c15685c77e0fbaf68d60ef70594b6dfeb81aeb418`.
All artifact hashes and membership were independently reverified after sealing.

## Remaining Root and runtime checks

Root must select a fresh complete dependency closure, review/adopt the added
route and pins, and supply its original required receipts. Old content and
membership holds are not refreshed or waived here. Runtime review must cover
Windows x64/nested-job compatibility, valid inherited standard handles, exact
argv/ENV/cwd, normal completion, early nonzero exit, inner timeout, supervisor
deadline, setup failure, and the supervisor-death/missing-record hold.

Selected code imports, proposed API calls/process launches, Add-Type compilation,
compiler/Native/provider/original-PE/target execution, and prior-runtime replay
are all zero. There are no Main/Root writes, C++ changes, tests, or runtime,
ABI, game-validation or Source-admission claims.

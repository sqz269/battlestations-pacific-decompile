# Type10/11 owned-job closure TEXT peer review

This independent, read-only review found no confirmed defect in the proposed
process ownership, cleanup, or admission control flow. It supports continuing
the TEXT review only. **Source_credit=0; UNADOPTED; no Root admission, API/ABI
validation, descendant-closure observation, fixture result, or game result.**

Reviewed worker commit: `049e1f4417dd762f08a72d5f999dff4b9a2c57ac`.
Peer branch: `agent/cc12_type1011_job_peer`, based on
`29aeaf6f7aa4cbf45e6a3b32ff3c65d80b347399`. The peer owns zero addresses and
only this document and `reports/cc12_type10_type11_job_closure_TEXT_peer.json`.
The author's ignored inputs remain in
`J:/PROG/battlestations-pacific-decompile-cc12_type5_complete_guard_materialization_TEXT/local/t1011jobTEXT`.

## Independently checked inputs

The peer rehashed all 2,294 seal entries and enumerated exact recursive
membership: 2,295 files, no missing, extra, or changed files. The 428,873-byte
seal has SHA-256
`3b4b64201752fcd96204b05c15685c77e0fbaf68d60ef70594b6dfeb81aeb418`.
`FINAL_UNADOPTED_TEXT_INPUTS_v2.json` is 4,306 bytes with SHA-256
`1d7228181155a45c9d3637dbb208472d37fe2c1e4597b720a7c35bb35cf7f621`.
All eight referenced payload pins, the policy pin, and the template pin match.

Comparison against the seven fixed previous payloads confirms that only the
bootstrap and PowerShell text changed. All 131 original/frozen copy pairs
match their pins; both materialization trees contain the same 119 files and
every corresponding file is byte-identical. The peer did not repeat the
author's separate full 13,047-artifact previous-family audit.

Passive Python AST comparison confines changes to bootstrap `initialize` and
`verify_snapshot`. All 340 original `need`/`insist`/`check`/`require` call ASTs
remain; the prior payloads now contain 341. Seven final Python payloads parse.
PowerShell parses with zero errors and retains all 36 original `Require` calls
and 11 original `Check` calls in order; the new totals are 47 and 15.
These checks parse text and read bytes; they do not execute selected functions.

## Process ownership and exit paths

References below are line numbers in the sealed `candidate_TEXT_v4` payloads.

| TEXT evidence | Peer assessment |
| --- | --- |
| Supervisor 129-137 | Anonymous job; default noninheritable handle is checked; exact `KILL_ON_JOB_CLOSE` limit is set and queried before child creation. |
| Supervisor 139-160 | Three inheritable duplicates of existing standard handles are the entire `HANDLE_LIST`; the job handle is excluded. Both attribute arrays remain live until list deletion. |
| Supervisor 150-165 | `JOB_LIST` assigns the job during `CreateProcessW`; `CREATE_SUSPENDED` is also requested. Membership and one active process are checked before `ResumeThread`, which must return the initial suspend count of one. |
| Supervisor 179-203 | Every handled exit after successful job creation attempts `TerminateJobObject`, including zero and nonzero root exits. Cleanup polls accounting for zero, then requires the retained root handle to be signaled. Failed membership additionally attempts direct cleanup of the unresumed root. |
| Supervisor 204-232 | Handles and attributes are released; cleanup failures or missing zero accounting cannot produce `descendants_closed=true`. Residual descendants before termination make an otherwise zero root exit unsuccessful. Failure records may be written without closure evidence; such records are not positive closure observations. |
| PowerShell 46-55, 131-143 | Record identity includes selection, attempt, supervisor, policy, environment, label, PID and original argv. Both the stored success expression and final advancement gate require matching zero-active closure and successful supervision. Missing, malformed or unsuccessful records hold the attempt. |

Microsoft documents creation-time job assignment and the inherited-handle
requirements for these attributes. Attribute value storage must remain alive
until list deletion. The candidate's retained arrays and explicit handle list
match those requirements in text.
[UpdateProcThreadAttribute](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-updateprocthreadattribute)

Normal `CreateProcess` descendants remain within the claimed job hierarchy when
breakaway is prohibited. Accounting and termination include child jobs. Existing
services and independently brokered processes remain outside this claim.
[Nested Jobs](https://learn.microsoft.com/en-us/windows/win32/procthread/nested-jobs)

## API layouts, handles and arguments

The peer compared all 18 ctypes bindings with Windows SDK declarations. Frozen
headers cover 16; live SDK 10.0.26100.0 `jobapi.h` and `processenv.h` provide
`IsProcessInJob` and `GetStdHandle`. No signature-width or field-order mismatch
was found. Using a `STARTUPINFOEX` pointer for the startup argument supplies the
required prefix at the same address; the extended-startup flag and extended
`cb` are present. Pointer-sized attributes, handles and `SIZE_T` use pointer
width; `BOOL`, `DWORD` and `UINT` use four bytes.

The following sizes are independent arithmetic models under Windows SDK/MSVC
default packing, not measurements of a selected interpreter or compiler:

| ctypes structure | x64 bytes | x86 model bytes |
| --- | ---: | ---: |
| BASIC_LIMIT | 64 | 48 |
| IO_COUNTERS | 48 | 48 |
| EXTENDED_LIMIT | 144 | 112 |
| ACCOUNTING | 48 | 48 |
| STARTUPINFO | 104 | 68 |
| STARTUPINFOEX | 112 | 72 |
| PROCESS_INFORMATION | 24 | 16 |

The candidate explicitly rejects a 32-bit supervisor before loading its Win32
bindings. Its x64 size vector agrees with this model. Critical x64 offsets
agree as well: accounting `ActiveProcesses=40`, startup standard handles at
80/88/96, extended attribute pointer at 104, process IDs at 16/20. This does
not prevent the original recipe from building Win32 targets.

The three standard-handle duplicates remain open through child cleanup. Process,
thread and duplicate handles are closed before the job handle; failures are
recorded and hold success. Invalid inherited stdin/stdout/stderr fails setup.
The caller still collects both output pipes and treats a pipe wait failure as
a hold. Actual .NET/CPython handle inheritance and pipe closure remain untested.

The explicit application path, cwd, root environment and original argument
vector are retained. The quoting loop doubles backslashes before embedded
quotes and closing quotes, including empty arguments; no static escaping defect
was found. Actual argument round trips, including Unicode and trailing
backslashes, remain a runtime check.

## Runtime gaps and admission boundary

No confirmed TEXT blocker was found. The following checks still require an
authorized runtime route and cannot be inferred from the author's or peer's
passive results:

1. Validate selected x64 interpreter field offsets, signatures, return/error
   behavior, actual host-job compatibility and atomic assignment before resume.
2. Observe normal zero exit, early nonzero exit and zero exit with a lingering
   ordinary descendant. The last must clean up and remain unsuccessful.
3. Exercise the original 540-second inner timeout, the proposed 560-second
   supervisor deadline and 20-second cleanup budget, and the unchanged
   600-second outer wait. The supervisor deadline starts after resume; setup,
   hashing and postflight are outside it. The nominal 20-second margin is not
   proof that the supervisor always finishes before the outer watchdog.
4. Exercise setup/API failure, failed membership, termination/accounting/handle
   errors, and delayed root signaling. Missing observation must remain a hold.
5. Kill the supervisor during setup, suspended creation, execution and cleanup.
   The noninherited last-handle fallback should terminate its job, while a
   missing/invalid closure record must still block advancement. Test the outer
   watchdog and incomplete record/pipe paths separately.
6. Verify actual child argv/ENV/cwd, valid standard handles, output propagation,
   EOF and final closure-record identity. Do not extend ordinary-descendant
   evidence to independently brokered processes.

The recorded `terminated` counter is `TotalTerminatedProcesses`, which Microsoft
defines as terminations caused by limit violations. It is not an independent
count of processes killed by explicit `TerminateJobObject`. Admission correctly
depends on successful termination request, observed zero active processes and
root signaling rather than that counter.
[Job accounting fields](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_basic_accounting_information)

All 13 concrete Root selection fields remain null; both adoption flags remain
false. The old held dependency snapshot is retained and has not been refreshed
by this peer. Root must independently select/adopt complete current dependencies,
supervisor, policy, code and environment pins and provide its required receipts.

Selected payload imports/execution, Win32 API calls, compiler/Add-Type execution,
Native queries, fixture/game execution and Root writes were all zero. No C++
change or new test was made. The peer's two review files are its only repository
content changes.

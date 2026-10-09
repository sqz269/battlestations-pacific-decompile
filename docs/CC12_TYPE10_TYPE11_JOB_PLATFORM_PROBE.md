# Type10/11 OWN Job platform probe

Two bounded OWN platform cases passed on this machine: creation-time suspended
Job assignment followed by root/descendant cleanup, and supervisor death with
last-job-handle termination observed through retained child process handles.
This is **Source_credit=0 platform API/ABI evidence only**. The selected
supervisor, bootstrap, driver, fixture recipe and fixture probe were never
imported or executed. No selected-route admission, Root receipt, materialization,
original timeout-policy result, provider/Native/PE execution or game result is
established.

The worker fast-forwarded its existing isolated branch to Main
`17daf67dd022ce20f8914c94f9f1e9c2001ea8b1`, then claimed zero addresses and only
this document and `reports/cc12_type10_type11_job_platform_probe.json`.
The platform was Windows version 10.0, build 26200, with Anaconda CPython
3.13.11, 64-bit AMD64. The declarations used were SDK 10.0.26100.0.

## Retained OWN inputs and outcomes

Both ignored namespaces are in this worker worktree's `local/` directory.
Each retains the complete source, a frozen `.DATA` copy, passive AST review,
SDK declarations, invocation, full private environment, runtime identities,
outputs, observations, result and seal. Source was reviewed and pinned as DATA
before either invocation. No source from the author's immutable namespace was
copied into or imported by the probe.

| OWN namespace | Outcome | Artifacts / exact files | Seal SHA-256 |
| --- | --- | ---: | --- |
| `cc12_type1011_job_platform_probe_20261009_0802` | Exit 1; own count/bookend assumptions failed; retained | 35 / 36 | `d021fe973c3a5268c6a241ac19bc7943b56bd7a5d1b571fbbe8e639bcb4b2d9f` |
| `cc12_type1011_job_platform_probe_20261009_0802_v2` | Exit 0; both cases and exact bookend passed | 36 / 37 | `0316fc9d5dacb52271d10c3f0c8dbad7111143bfcea44362f4e92b861885b638` |

Every artifact hash and exact recursive membership in both seals was separately
reverified after sealing: zero missing, extra or changed files. The final source
SHA-256 is `2b5fa901d8e174c727063d030c5e0f619de3e981024b5f37a847ee114e5f36df`.
The successful controller recorded about 0.658 seconds for its two cases and
runtime bookends. This is a single observed run, not a timing guarantee.

The first attempt assumed that only one process would remain after the benign
root exited. Actual accounting reported two active processes, although explicit
cleanup still reached zero. Its first use of UTF-16 encoding and failure
formatting also loaded four stdlib modules after the bookend. The fresh second
attempt preloaded those known modules and replaced the unsupported count
assumption with bounded owned-job PID-list observation. It kept the same two
behavioral cases. The initial SDK `shared/winnt.h` lookup error and correction
to `um/winnt.h` are also retained; all required copies existed before execution.

## Concrete platform observations

| Check | Actual observation |
| --- | --- |
| Atomic creation | Anonymous Job; job handle noninheritable; queried limits exactly `0x2000`; `JOB_LIST` and three inheritable standard-handle duplicates in `HANDLE_LIST`; Job handle excluded. |
| Suspended membership | Both creations reported membership before resume, accounting `active=1,total=1`, unsignaled root handle and `ResumeThread` previous suspend count 1. |
| Normal root exit with residual descendant | Root PID 65388 exited 0. Descendant PID 110908 remained alive and was verified in the exact owned Job. Accounting was `active=2,total=4`. |
| Residual member identities | The bounded Job list contained the descendant Python and `conhost.exe` PID 64196. Both were verified members with unsignaled retained/query handles. |
| Explicit cleanup | `TerminateJobObject` succeeded; accounting became `active=0,total=4`; Job PID list was empty. The original root handle was signaled, and the retained descendant handle signaled with exit code `0xC1010001`. No cleanup error was recorded. |
| Supervisor death | Before death the owned Job contained two Python processes and two `conhost.exe` members. Controller retained Python handles for root PID 104068 and descendant PID 85040; both were unsignaled before killing only its exact supervisor process handle. |
| Last-handle observation | Supervisor termination code was `0xC1010002`. Both retained Python handles became signaled about 5.8 ms after that request, well before their authored 15-second sleep lifetimes. The supervisor's natural-finally record was absent. Both child exit codes were 0. |
| Standard handles and arguments | Private stdin contents, stdout/stderr markers and pipe EOF passed. Root argv preserved an empty argument, whitespace, embedded quotes, trailing/backslash-before-quote cases and BMP/non-BMP Unicode. Both root and descendant ENV and cwd matched their private profile. |

The observed `conhost.exe` members explain why the number of explicitly launched
Python payloads is not a safe assumption for `ActiveProcesses`. The explicit
cleanup case observed zero while the original root and descendant process
handles were still retained. `TotalTerminatedProcesses` remained zero; the
probe correctly treats it as a limit-violation counter rather than the count
of explicit job terminations.

The death case intentionally did not duplicate or inherit the Job handle. After
supervisor death there was no Job handle from which to query zero accounting.
The positive observation in that case is that the two retained Python process
handles signaled. The conhost identities were observed before death, but their
handles were not retained through death. This fallback result is not a selected
positive closure record. Its zero child exit codes also do not imply successful
execution of a selected route.

## Actual layouts and identity evidence

All 27 used ctypes signatures were independently compared with retained SDK
declarations. Pointer/handle/`SIZE_T` widths, 32-bit `BOOL`/`DWORD`/`UINT`, return
types, buffer pointers and the extended-startup prefix match. The Job PID-list
query used bounded storage and verified each opened member against the owned
Job before reading its process-image identity.

| OWN ctypes structure | Actual x64 size | Actual alignment |
| --- | ---: | ---: |
| Limits | 64 | 8 |
| Counters | 48 | 8 |
| Extended | 144 | 8 |
| Accounting | 48 | 8 |
| Startup | 104 | 8 |
| StartupEx | 112 | 8 |
| ProcessInfo | 24 | 8 |

Every field offset is retained in the report. Critical offsets are accounting
`active=40`, standard handles 80/88/96, extended attribute pointer 104, and
process IDs 16/20. These are measurements of OWN definitions in this actual
interpreter; they do not evaluate the selected supervisor's classes.

Before and after the successful run, the exact snapshot matched: source and
Python executable pins, 62 Python module file origins, 31 native module file
pins, 27 API function origins, 12 frozen SDK files and the controller ENV hash.
API function addresses were resolved to their actual mapped module origins;
the relevant files were `kernel32.dll` and `KernelBase.dll`. The executable
SHA-256 was `ec0ea8d6907787b76dcf8524aaa93e52e167ceee62fa8778e182ea637a3dbc1d`.
Live original SDK headers also matched the pre-execution frozen data after the
run. These are disk file pins at mapped origins, not mapped-memory byte proof.

Full controller ENV (82 keys), child ENV (8 keys), and actual child observations
are retained privately in ignored files. Their values were not printed or
committed. The report contains counts and pins, not their full values.

## Scope and remaining selected-route work

The OWN cases used anonymous Jobs, private Python children, explicit pipe/file
handles and `subprocess.list2cmdline`. They exercised no author-selected
function, custom argv encoder or PowerShell/.NET process setup. All process
waits and record/pipe deadlines were finite; cleanup targeted only owned Jobs
and the retained supervisor handle. No broad process kill, compiler, shared
Ghidra mutation, agent spawn or tracked `src/` change occurred.

The original 540/560/600-second route was not exercised. Selected-route checks
remain for actual Root dependency/code/environment selection and receipts,
PowerShell inheritance/postflight/record gates, the selected custom argv encoder,
timeouts, early nonzero exit, API/setup/cleanup failures, partial/malformed
records and the outer watchdog. Other Windows versions, interpreters and host
Job restrictions remain untested. These platform results grant no selected
admission or Source credit.

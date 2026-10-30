# Original coverage guard: feasibility and safety gates

Status: no safe coverage launch mode is enabled. The initial suspended-root
DynamoRIO attach candidate failed a private fixture before client initialization
or fixture entry. No original game was launched. The ordinary guarded
baseline/histogram path remains unchanged.

## Tool provenance checked locally

- Package: `DynamoRIO-Windows-11.91.20749`, under the coverage worktree's
  ignored `local/tools/dynamorio/`.
- `bin32/drrun.exe -version`: `11.91.20749 -- build 0`.
- Local `bin32/drrun.exe` SHA-256:
  `112433d9fe45c8e7bbd0ee4f61f4d7ff2de8f6ee76554e2421805dbe0cbdf166`.
- Inspected `bin32/drinjectlib.dll` SHA-256:
  `173c8d46926c9b7a05c71fd7521526baf2878e668e84b80e02388c4e64a1fc9e`.
- Downloaded archive SHA-256 supplied by the primary, matched to the official
  release asset digest:
  `9ef2d8f990a6acb4d303385fec0105bd45e54d36e92269494ac8d45e966c3844`.

The local `include/dr_inject.h` declares Windows
`dr_inject_process_attach(pid, data, app_name)`. Both 32-bit launch tools expose
`-attach`; their help explicitly labels attach experimental. This establishes
an API candidate, not successful attachment to a never-resumed process.
The official [injection API](https://dynamorio.org/dr__inject_8h.html) documents
the attach and injection interfaces.

## Why a stock drrun prefix is insufficient

The current guard creates the exact copied retail image suspended, verifies
its restricted token, assigns its owned job, and only inherits protected
read/list source handles. Its Windows debugger owns the original-entry IAT
redirect and rejects every additional executable before continuation.

Changing the root command to `drrun -- retail.exe` breaks the root-image
identity and child policy. Retaining the debugger would also retain a second
tracing/control mechanism, while removing it would remove the current entry
redirect. A job notification arriving after process creation is not an adequate
replacement for the pre-execution child gate. No such prefix should reach the
original game.

## Initial candidate topology: rejected by the bootstrap check

1. The guard creates the **retail root itself**, suspended, with the existing
   `medium-write-locked` token, private environment/cwd, source AccessCheck
   audit, all no-delete source opens and explicit protected handle list.
   Keep every independent parent handle until the owned job is empty.
2. The proposed guard would use no `DEBUG_PROCESS`/`DEBUG_ONLY_THIS_PROCESS`, no entry `INT3`, no motion
   breakpoints and no debug-event loop for this opt-in path. DynamoRIO is its
   only tracing mechanism.
3. Before any root resume, assign a kill-on-close job with
   `JOB_OBJECT_LIMIT_ACTIVE_PROCESS` and `ActiveProcessLimit=1`, without
   breakaway. This is the candidate kernel replacement for the current
   root-only debugger gate. Its actual child-before-user-code behavior must
   pass the fixture below. See Microsoft's
   [job-limit contract](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_basic_limit_information).
4. Run the 32-bit attach/configuration helper in its own owned job, with no
   inherited source/job/token handles. It may address only the already-created
   root PID. All DR configuration and outputs must stay under private `local/`.
   Both processes receive `DYNAMORIO_CONFIGDIR` pointing there; use PID-specific,
   non-global configuration and remove it after the owned processes exit.
   DynamoRIO documents this directory override and separates one-time execution
   from persistent/global configuration in
   [Windows deployment](https://dynamorio.org/page_deploy.html).
5. Load a small redirect/entry-gate client alongside `drcov`. Enumerate already
   loaded modules and watch later module loads. Wrap
   `SHGetSpecialFolderPathA`; for `CSIDL_PERSONAL` with flag bits masked, safely
   copy the already-prepared private ANSI Documents path and return TRUE.
   Leave other CSIDL calls on the original path. The supported
   [`drwrap_skip_call`](https://dynamorio.org/group__drwrap.html) API requires the
   x86 stdcall argument size, here 16 bytes. Do not weaken application-state
   preservation with fast-clean-call shortcuts.
6. The client must gate execution of the main module's original entry on a
   successful redirect installation and report the expected PID/module/entry
   and private Documents target. A bounded attach/ready failure must terminate
   the owned job while source handles remain held. Never resume natively to
   "help" an unready tracer initialize.
7. Retain Normal priority, DRAIN/timeout checks, owned-process-only termination,
   crash/forced-kill distinction, protected-file inventory/verify and source
   handle identity/lifetime receipts. Verify token, no-debugger state and
   inherited handle coverage after DR initialization. No disk EXE or source ACL
   changes are part of this design.

The installed xlive `CloseHandle(HMODULE)` workaround is debugger-specific.
This candidate has no debugger, so it must test the ordinary failure return
without installing a blanket invalid-handle exception handler.

## Private bootstrap result

The manifested Win32 `entry_probe.exe` has a custom entry that writes only a
private marker. It was created suspended with the actual flags9 token/default
DACL, protected dummy read/list handles and a kill-on-close, one-process job.
The attach helper used a separate owned job and private configuration directory.
A minimal compiled DR client would record client initialization and an executed
clean call at the fixture's entry. It does not yet redirect Documents or collect
drcov data; this case tests the earlier bootstrap dependency first.

Two attempts returned helper exit 1, with neither client nor application-entry
marker present. The second helper log reports `Failed to create process` with
an invalid/uninitialized image-name string, without a usable numeric error.
The parent never resumed the root after that failure. Owned processes were
terminated and dummy hashes remained unchanged. Evidence is ignored output in
the coverage-guard worktree's `local/dr-bootstrap-probe/`: `result.json`,
`result-second.json`, and `stage-second/injector.log`.

The matching release's
[`core/win32/injector.c`](https://github.com/DynamoRIO/dynamorio/blob/cronbuild-11.91.20749/core/win32/injector.c)
also contradicts the proposed purely debugger-free attach assumption:
`dr_inject_process_attach` calls `DebugActiveProcess` and handles debug events.
Its `create_attach_thread` then requires a remote `kernel32.dll` and `SleepEx`.
Those are loader-phase dependencies for a never-resumed root. This source
inspection explains why public attach is not an established pre-loader handoff;
the CLI output does not independently identify which internal return failed.
Do not infer a specific Win32 error from the malformed message.

This rejects the attempted topology, not every possible DR integration.
No native resume was used to get past the failure. A guarded startup admission
stub or custom process-creation adapter would be a new design with additional
ABI/ordering fixtures; neither exists here.

## Remaining private fixture gates before any original run

Use one manifested Win32 fixture, private dummy protected files/directories,
and the exact candidate token/default DACL. No original handles or game are
needed for feasibility.

- Attach to a never-resumed, already-job-assigned root. Prove its entry does
  not execute early and that the redirect/client-ready handshake precedes it.
  If attach requires running the original entry unguarded, stop this approach.
- Run the redirect client and drcov together. Check Personal and flag-bit
  results, non-Personal fallback, x86 stack/callee-saved behavior and actual
  drcov blocks from the fixture's expected module.
- Check SDK startup only in the private fixture: DirectInput and FMOD return
  results must be recorded under the same token/tracer/environment.
- Check every inherited source-handle flag against an input handle manifest,
  including a 69,700-handle case. A count alone cannot prove DR retained each
  particular handle. After parent copies are closed in a separate lifetime
  test, the child copies must still prevent delete/rename/remove-directory.
  Keep the production parent's independent copies throughout the real run.
- Verify other mutation rights still fail, private writes succeed, source
  fixture hashes stay unchanged, and no handle collides with `00400000`.
- Attempt an extra executable whose first action writes a marker. The kernel
  job gate must prevent that marker and any user-mode child execution, without
  relying on an active debugger.
- Exercise missing client, failed redirect/attach and timeout cleanup. Source
  handles must outlive every owned target, and helper cleanup must affect only
  owned processes. Configuration/log artifacts must remain private.

## Decision and fallbacks

The immediate blocker is the failed pre-loader attach protocol and absence of
a proved entry redirect, not an observed DR/retail incompatibility.
The public attach API alone does not establish initial suspend-count behavior
or client readiness ordering. No safety relaxation is an acceptable experiment.

PIN was examined next. Intel's current
[download page](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-binary-instrumentation-tool-downloads.html)
offers Windows Pin 4.4. Its official ZIP endpoint returned HTTP 403 Access
Denied to the host download, and no installed `pin.exe` was found in the checked
tool/download locations. No unofficial mirror was substituted and no PIN target
was run. The official
[`-pid` documentation](https://software.intel.com/sites/landingpage/pintool/docs/99850/Pin/doc/html/group__CL__OPTIONS.html)
describes attachment to an already-running process; it does not establish this
guard's pre-entry protocol.

TinyInst was then inspected from official source, commit
`9b564702c01481825e4a1629d1701cf49d3d091f`.
[`Windows/debugger.cpp`](https://github.com/googleprojectzero/TinyInst/blob/9b564702c01481825e4a1629d1701cf49d3d091f/Windows/debugger.cpp)
provides `Attach`, `OnProcessCreated`, `OnEntrypoint` and `OnException`.
Stock attach explicitly assumes the entry has already run and skips installing
its startup entry breakpoint. Stock `Run` creates the target itself, so neither
stock path preserves the current guard automatically. A custom single-debugger
adapter could instead attach to the guard-created root and explicitly establish
the original-entry hook before releasing its initial thread. It would also have
to carry over the exact xlive exception filter, source-handle/job lifecycle and
all failed-gate cleanup. TinyInst would replace the Python debugger, not run
beside it. No such adapter or TinyInst target run was attempted here.

## CREATE/inject API assessment

The public Windows header exposes `dr_inject_process_create(app, argv, &data)`
and getters for the resulting PID/process handle. It has no creator callback,
token/extended-startup parameter or API adopting a caller's suspended
`PROCESS_INFORMATION`. The shipped x86 DLL's export table agrees. The same
release's `injector.c` uses plain `STARTUPINFO` and
`CreateProcessW(..., TRUE, CREATE_SUSPENDED, NULL, NULL, ...)`.

| Required property | Public create/inject path |
| --- | --- |
| Initial suspension | Provided by `dr_inject_process_create`. |
| Exact restricted primary token | Can be inherited from a creator already running with that primary token; there is no explicit token argument. |
| Explicit protected source `HANDLE_LIST` | Not provided. The creator passes `TRUE` for general inheritance, with no `STARTUPINFOEX` argument. |
| Job assigned before any resume | Possible using the public process-handle getter before injection/run. |
| Adopt the root already created by the guard | No supported public API. Do not forge the private opaque `data` structure. |
| Pre-OEP Documents admission | Still requires a separately proved client or startup gate; create/inject alone supplies none. |

Thus the unmodified public interface is not a concrete implementation of the
existing explicit-handle-list guarantee. A creator whose only inheritable
handles happen to be source handles is not a replacement for that guarantee.
No CREATE/inject dummy run was started after this boundary was established.

Two further designs are possible, but neither is implemented or proved:

1. **Reviewed DR source extension.** Add a narrow adoption/creator API in the
   injection frontend, accepting duplicated suspended-process/thread handles
   and the checked image identity. The existing guard can then create the root
   with its exact token/handle list/job first. The library initializes its own
   injection state; callers never reproduce its private layout. Define handle
   ownership and explicitly disable attach/debug-key paths before using the
   normal injection/run APIs. This is a DR source patch, not a stock SDK feature.
2. **Owned creator import shim.** The shipped 32-bit `drinjectlib.dll` imports
   `KERNEL32!CreateProcessW` at IAT RVA `0003415C`. An owned helper could replace
   only that in-memory import with a strict creator that calls
   `CreateProcessAsUserW` using the explicit `STARTUPINFOEX` source list, assigns
   the root job before returning, and returns the real PI to the library.
   This avoids changing the DR source/disk DLL, but is unsupported interception,
   not a public callback. It adds ABI, argument-filtering, inheritance, helper
   job and failure-cleanup obligations and needs separate review/fixtures.

The default CREATE route can use non-debugger map injection, unlike the failed
attach route. It also consults the image's IFEO debugger configuration and can
unset/restore a configured debugger key. Any future design must reject that
branch **before** calling a routine that could change it; an after-the-fact
`dr_inject_using_debug_key` check is too late. Do not modify system/debugger
configuration to make injection work.

These are interface/security boundaries, not a claim that all DR topologies
are impossible. Production guard and wrapper code remain unchanged. Only a
reviewed, concrete creation bridge should proceed to the existing private
entry-admission, redirect, SDK, inherited-handle, child-policy and cleanup gates.

## Bounded owned-creator fixture

The creator-import design was subsequently approved for a **private dummy
fixture only**. `tools/dr_create_guard_probe.py` builds a manifested Win32
`dr_helper_probe.exe`, a byte-identical `dr_root_probe.exe` copy and the small
entry-marker DR client. It uses the existing flags `0x9` restricted-token and
default-DACL factory. Production guard/wrapper files are unchanged.

The helper changes only its loaded `drinjectlib.dll` CreateProcessW import at
RVA `0003415C`; it restores the pointer immediately after the single SDK create
call. The SDK DLL on disk is hash checked and unchanged. The shim rejects any
other call/image/arguments/flags, checks the expected dummy image SHA256, and
requires the SDK's exact `CREATE_SUSPENDED` contract. It creates the root with
the controlled private environment/cwd and an explicit `STARTUPINFOEX` list
containing only the protected dummy-source handles. The token and job control
handles inherited by the helper are excluded from the root's list.

The parent creates a kill-on-close helper job with an active limit of two
(helper plus nested root). The shim assigns the root's separate, nested
kill-on-close job with active limit one **before returning the real PI to the
SDK**. Neither job enables breakaway. Only the SDK's public create/inject/run
and PID-specific file-configuration APIs are used; no opaque-state fabrication,
attach, debugger or native root resume is used. Read-only IFEO checks reject any
configured Debugger value in either registry view, including filter subkeys,
before the SDK can enter its unset/restore branch. Configuration goes to the
private `DYNAMORIO_CONFIGDIR`.

The parent retains its independent read-only, no-share-delete dummy handles
until both jobs are empty, including failure cleanup. The root receives
inheritable/protected copies. This has the existing trusted-binary limitation:
a process can explicitly clear its own protection flags, but it cannot clear
the parent's independent copies. No original/install handles are opened by
this fixture.

Run from an owned worktree using a **new** local output path:

```powershell
python tools/dr_create_guard_probe.py --dr-root <official-DR-root> --output local/dr-create-shim-probe
```

The successful run is recorded in ignored
`local/dr-create-shim-probe-03/result.json`: helper PID `83424`, root PID
`109828`, both exit codes zero. Its evidence is:

- Helper, pre-run root and executing root had medium integrity `8192`; the
  exact three restricting SIDs, mandatory policy `3`, restricted status and
  disabled UIAccess were checked. The Python factory separately verifies the
  expected default-DACL grants.
- The root received eight protected dummy/ancestor handles, each with flags
  `3`; each protected CloseHandle failed with error `6`.
- Data/attribute/owner/DACL write opens failed with error `5`; DeleteFile and
  removal of the locked empty directory failed with `32`. Directory rename
  failed with `5` (destination creation is also denied). A private file write
  succeeded. The dummy source SHA256 was unchanged.
- Root child creation failed with quota error `1816` and the child's entry
  marker was absent. The root job was assigned before SDK PI admission.
- `client_ready pid=109828` preceded `entry_execution pid=109828` in the DR
  client log. The fixture entry marker appeared only after SDK injection and
  its public run call. There was one tracing mechanism and no debugger.
- Both owned jobs were empty before parent lock release. Injector, config and
  runtime DLL hashes were unchanged after the run.

The first safety run reached the root but stopped on a too-specific assertion
that required sharing error `32` for rename; the actual access denial `5` was
safe. The corrected complete run above passed. Python compilation and the
manifested native/client builds passed. A focused negative run then forced the
helper's initial job assignment to fail with error `5`. The still-suspended
owned helper PID `119268` was terminated and waited before lock release; both
jobs were empty, no entry markers appeared, and dummy/SDK hashes were unchanged
(`local/dr-create-shim-job-failure/result.json`).

This proves a narrow CREATE bridge for these dummy binaries, **not safe retail
coverage integration**. It has no Documents redirection/admission client yet,
no original game run, no retail SDK-startup proof under DBI, and no 69,700-handle
CREATE/inject measurement. The stock SDK remains without a public custom
creator callback; the tested bridge is an explicitly reviewed in-memory import
shim. Further private proof and review are required before changing production
launchers or opening original source handles.

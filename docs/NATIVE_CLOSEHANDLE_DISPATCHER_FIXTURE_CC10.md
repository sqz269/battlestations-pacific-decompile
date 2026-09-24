# Isolated CloseHandle dispatcher continuation

A benign x86 target calls `CloseHandle(GetModuleHandleA(NULL))` from its real
pre-entry TLS callback. The undebugged call returns BOOL 0 and error 6. A
64-bit debugger fixture obtains the same result after continuing one precisely
matched native invalid-handle exception, then completes the existing restored
entry/two-hardware-stop protocol and exits zero. This is mechanism evidence;
the original game's corresponding continuation remains unobserved.

The predicate requires the owned primary PID/TID before entry, first-chance
C0000008 with flags80/no parameters, exact native/WOW64 PCs and caller frames,
the actual module argument, saved native status and syscall return, and the
fixture's genuine call-entered/not-returned phase. It compares five complete
live system bodies with pinned relocated disk bytes and validates the current
three-hop import chain through the exact kernel32 thunk, KernelBase and NtClose.
The guard reads both contexts twice and requires byte equality. It writes no
context, code, stack or API result. Disposition remains outside the guard.

The first candidate correctly rejected an unmodeled kernel32 thunk. The second
version pins that specific six-byte thunk and its current import cell; it does
not follow arbitrary targets. The shared exception-policy header is unchanged.
Unmatched first chances are forwarded and every second chance closes the owned
child. A separate genuinely unhandled RaiseException control exercises rejection.

Eight benign children cover two sets of undebugged baseline, forwarding,
candidate and unhandled-control cases. The successful candidate returns the
same BOOL/error as its baseline, preserves the eight canary words, restores the
32-byte entry window and native/WOW64 DR0-3/6/7, and exits zero. All eight outer
jobs and all six inner debugger jobs have closed-process receipts. No original
or xlive module appears in the six debugger journals; the undebugged baselines
have no module inventory. All original attempts remain **two**.

Primary review verified all 156 indexed artifacts and 634 historical artifact
rows, the verbatim retained outer-job function, and unchanged policy bytes.
The reviewed COFF/PE decoder independently identifies the actual 6,488-byte
linked guard with a unique match outside relocation fields and an exact runtime
function extent. Its direct relocation targets contain no context/code write
or ContinueDebugEvent API; transitive behavior was reviewed in source. Full
architectural/XSTATE preservation is not claimed.

The existing original helper restores the entry page's initial20 protection
even when its restore operation observes current40. The original xlive body
requests40 across the module, but the evidence does not uniquely trace the
writer. A separate protocol review must address preservation of the current
protection before another original attempt. This packet changes no original
helper, copy/profile/options or stale private-log overlay and authorizes no
additional execution. The sampler input and native compiler outcome remain
unknown; startup/gameplay equivalence is not established.

The frozen case matrix, exact hashes and archive receipt are in
`reports/native_closehandle_dispatcher_fixture_cc10.json`.

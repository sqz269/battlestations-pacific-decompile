# Original sampler input capture (CC10)

The single reviewed `original_run03` attempt captured both primary-thread stops
and closed its original child and owned jobs. This is the third original attempt.
It intentionally terminated before executing the CALL at B3C018; it is not a
sampler execution or game-validation result.

| Recorded value | After reached calls, B3BFF9 | Before sampler call, B3C018 |
|---|---|---|
| Primary PID / TID | 88436 / 73368 | 88436 / 73368 |
| Physical P | 0162E86C | 0162E86C |
| ESP | 0162E86C | 0162E864 |
| Borrowed word address P-30h | 0162E83C | 0162E83C |
| Borrowed word bits | 0162E964 | 0162E964 |
| Builder identity | EDI = 0162E964 | ECX = 0162E964 |
| Pass | 16B8E580 | 16B8E580 |
| Root descriptor | 16B80C10 | 16B80C10 |
| Caller return | 00B3C480 | 00B3C480 |

The two 336-byte stack windows independently encode the borrowed word and caller
return. The first pass is at P+14h; the second pass/root are at ESP/ESP+4. The
first root value is a separate helper read of builder+70, outside that stack
window; it agrees with the second raw stacked root. Recorded WOW64 and native
contexts confirm the instruction, stack, builder and execute-breakpoint fields.
The captured B3C47B CALL returns to B3C480 and targets B3B3C0. This establishes a
bounded caller relationship, not a full stack unwind.

The word equals the builder in this invocation. Its origin and validity across
other invocations are not established. The current CE2220 provider at both stops
is a hash-identified kernel32 body; this does not identify an earlier COM Release
target. No source input, padding byte or universal high-byte contract is supplied
by substituting this observed address.

Primary verification checked all 69 frozen files and 1,085 prior artifact rows,
all 24 bound roles, the exact root review receipt and its consumed marker. It
parsed all 765 journal events and matched 215 owned debug-event/continuation
pairs. All 12 captured bodies (six at each stop) match the copied executable's
PE bytes. All 105 module-load records have successful hashes; 100 distinct disk
module paths were independently rehashed.

One original-specific guarded continuation was selected and confirmed. Two
unrelated first-chance exceptions were forwarded; none reached second chance.
The guard's before/after contexts and entry window are identical. Entry arming
completed with protection20 restored; normal restoration preserved the current
protection40. Both entry-write context pairs are identical. The six debug-register
write pairs change only recorded debug fields and permitted EFLAGS reserved bit1.
The original CloseHandle return value/last-error and full architectural
equivalence remain unobserved. Additional threads were not instrumented.

The child exited126 through deliberate diagnostic job termination before the
sampler call. Inner/outer receipts show zero remaining owned processes, wrapper
exit0, no timeout, no cleanup error and no detached child. Native teardown is not
claimed. Seven live-profile manifest rows match in bytes, streams, security,
identity and creation/write times; access times are excluded. The restore plan
is empty and no restoration was applied.

The subsequent full copy inventory hashed all 67,814 files and 67,819 streams
(57,780,055,098 bytes). Only `bsp_debug.log` and `xlive_debug.log` differ from the
accepted pre-run manifest, with their identities, attributes and creation times
preserved. There is no writer attribution or automatic restoration. The prior
one-log overlay is stale and cannot authorize another run. Primary verification
compared the complete recorded manifests and current changed-log content; it did
not repeat the entire disk hash pass.

This packet changes evidence only. B3B280/B5F100 execution, source0 production
admission, full initialization and gameplay remain unproved. Frozen raw evidence
is retained in `local/cc10-platform-evidence/original-sampler-run03.zip`; the
primary report is `reports/native_original_sampler_capture_cc10.json`.

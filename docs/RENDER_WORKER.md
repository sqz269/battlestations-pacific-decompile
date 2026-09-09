# Render worker events and stop protocol

Worker constructor 00b33da0 creates two initially unsignaled, auto-reset Windows
events using 00bd1970 with CL=0. Wake event is at +Ch, idle acknowledgment at
+10h. It creates thread 00b33c20 suspended, stores handle +8h and ID +14h, sets
priority -2, then resumes it. Run +4h and shutdown +5h begin false.

Event vtable 00d6821c maps +4h to SetEvent (00bd1910), +8h to infinite
WaitForSingleObject (00bd17c0), and +Ch to ResetEvent (00bd1960). Its deleting
destructor 00bd19b0 closes the handle and optionally frees the wrapper.

The worker waits for wake, then checks shutdown. While run is true it executes
timed callback/render work. When run becomes false it signals idle acknowledgment,
waits for wake again, and rechecks shutdown. Stop helper 00b33bf0 clears run and
waits indefinitely for idle acknowledgment. This is not a thread join. In
particular, calling stop on a worker that never ran does not inherently produce
an acknowledgment; caller ordering still needs reconstruction.

Destructor 00b33b50 sets shutdown, signals wake, joins the thread with an infinite
wait, closes its handle and destroys both events. It does not itself clear run.
An active worker therefore needs coordinated stop before destruction. This
explains why the queue mode transition waits before disabling renderer locking.

`Win32Event` now ports signal/wait/reset and owns a real event handle through
ordinary C++ lifetime. It exposes API results that native code ignores; native
allocation, vtable and deleting-destructor ABI are not reproduced. Its wait
retains the native infinite timeout. The existing platform probe signals before
waiting, checks automatic reset afterward, and checks explicit reset using
zero-time polling. All checks and both existing CTest targets passed. No new
test target was added. The full render worker and concurrent mode changes were
not executed or ported.

Assembly and original-binary byte parity are recorded in
`reports/worker_event_evidence.json`. Eight new names and an updated stop comment
were saved in Ghidra. Next dependencies are worker-start call ordering, timing,
callback arguments and renderer gate 00b20220.

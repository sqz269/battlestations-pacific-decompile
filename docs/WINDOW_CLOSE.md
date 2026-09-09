# Window close policy

Static reconstruction evidence now connects WM_CLOSE to application exit. It does
not implement or runtime-validate the native confirmation dialog or shutdown.

1. Platform handler 00bed3b0 records a pending close at platform+180h and returns
   zero. The message does not directly set loop-exit byte +181h.
2. Application frame 00737a50 calls update 004e4a40. A raw relative CALL at
   004e4a79 reaches newly recovered 004ca2f0, ProcessWindowCloseRequest.
3. That routine reads and clears platform+180h. No pending request returns early.
   A nonzero byte at singleton 00f8abe8+3E8h also discards the consumed request;
   the meaning of that gate is unresolved.
4. For game-state values 1, 2 or 4 at ECX+5D4h, it sets global exit 00e1ae75.
   These numeric states have not yet been assigned semantic names.
5. Other states construct the key `FE_pc.main_quit_confirm` and pass type 6 and
   callback 004bbc50 to UI helper 00531b00, then invoke 00530c20. String ownership,
   UI singleton access and event dispatch remain unported.
6. Callback 004bbc50 receives a response integer in ECX. It always clears
   platform+180h and sets global exit only when response equals 1. Other responses
   preserve an already-set global exit flag; they do not reset it.
7. Later in application frame, a nonzero global exit sets platform+181h. This
   propagation is sticky, rather than an unconditional copy of the global byte.
   The existing message/frame loop then observes exit and sets finished+43h.

The separate helper 004b4560 reads platform+180h into AL, clears it and returns.
No caller has yet been established for that helper; the close processor uses
the same operation inline. All three previously missing functions were defined,
named, commented and saved in the existing bsp project.

The saved 004e4a40 function body contains only its first two instructions, so
Ghidra's ordinary call references miss the close processor. The relative call
was found in executable bytes and checked in both the saved program and disk
image. `reports/window_close_evidence.json` records hashes and the call check.
The full update routine needs analysis repair before reconstruction; absence of
Ghidra xrefs is not evidence that this path is unused.

Next dependencies are the gate singleton field, UI helper/callback dispatch, full
application update body, and native window creation/destruction. A future window
integration must preserve confirmation rather than treating WM_CLOSE or WM_QUIT
as an unconditional request to exit. No C++ changed in this investigation and no
additional tests were added.

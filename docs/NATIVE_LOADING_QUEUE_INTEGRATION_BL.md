# Native loading queue foundations

Source revision `4c83a760` adds three Win32 translation units: actual FileStore
removal, loading owner entries, and loading job/work storage. The integrated
Release build compiled all three and passed both existing CTests. Focused
worker fixtures exercise the new source; the math tests alone do not validate
these loading routines. Exact inputs and artifact hashes are recorded in
`reports/native_loading_queue_integration_bl.json`.

FileStore removal normalizes a copied header, searches only the resident tree,
and delegates stream ownership to the existing iterator erase. It preserves
the pending tree. Independent review confirmed the owner/head captures before
the returning CRT check and the node reload afterward. The actual-storage
fixture completed a physical read, then checked retained and sole-cache stream
ownership, missing removal and same-key pending preservation.

The actual20h loader owner constructor, getter, current-front callback and event
poll preserve native storage and publication order. The getter captures its
critical section, publishes before registration, and retains publication on
registration failure. Callback504850 reacquires the current loader/front;
BD1920 preserves the low byte of unexpected wait results. The actual-provider
smoke and isolated instrumented scheduling fixture are separate evidence.

The work source implements actual pointer arrays,10h records and24h jobs.
Records own only their string headers; context and resource results remain raw.
It preserves job padding, signed count comparisons, current header reloads,
allocation before publication, and the observed cleanup order. The fixture
exercises all eight APIs using an actual constructed string pool, including
growth, erasure, destruction and initial allocation failure. The private23-byte
504E10 destructor composition is also complete source coverage.

The primary rechecked all49 direct source call rows and the FileBlock audit's
143 direct/tail rows. Imported and indirect calls remain qualified listing
evidence. Worker fixture artifacts were rehashed before these appended document
corrections. Frozen fixture libraries retain their earlier BK hash; the report
separately pins the newly built BL library.

## Ghidra corrections

The restarted bridge's supported metadata endpoints restored returning-free
body membership for4FB310 and5018A0, verifying38 and104 reachable instructions.
The3-byte LEA at50196D is unreachable alignment following an unconditional
jump; it remains outside the function. The504E10 destructor now includes its
complete10 instructions. Allocation-unwind funcletsC68828 andC69200 each include
their full5 instructions through POP ECX/RET. No callee no-return flag changed.

BE7130's incorrect zero-input prototype now models ECX provider and one stack
name argument, with all71 instructions retained. The added unused EDX argument
is an explicit Ghidra fastcall analysis representation. A rejected const-void
parser spelling is retained in the repair receipt. Native ABI identity is not
inferred from this signature.

Seven complete ten-byte FH3 handlers were defined from matching live/PE bytes.
Fourteen source bodies, those seven handlers and two allocation-unwind funclets
now have saved names/evidence and refreshed exports. Earlier comments, names,
labels and prototype preimages are preserved in the receipts. Handler definitions
and source C++ cleanup do not constitute native FH3/SEH replacement thunks.

## Production boundary

This batch does not install a loader in the production runtime. Actual update
509190, drain/destruction5092E0/509FD0/50ACE0, FileBlock ownership, observer
mount/unmount behavior and resource loading require their remaining source
contracts. No placeholder deleting dispatcher or thread shutdown is introduced.

The FileBlock audit establishes real PakRegistry observer work and concrete
identifier, saved-gate-list and name-unmount packets. A separate bounded audit
is examining the worker thread's stop/wake/join boundary. Job stop bytes, loader
worker stop DWORD, zero queued jobs and zero pending physical reads are distinct.

No new game run is claimed for this source-only batch. The preceding BK report
records the native provider pump's bounded USN01 run at96d910d5; it does not
establish asynchronous gameplay loading or full game fidelity.

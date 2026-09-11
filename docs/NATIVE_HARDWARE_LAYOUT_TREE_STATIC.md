# Native hardware-layout tree startup and shutdown

The complete `CD7960..CD79A0` initializer and `CE0C50..CE0C90` shutdown
operate on the actual borrowed 12-byte `108D530` tree header. Each original
body is 64 bytes. The new MSVC Win32 interfaces use the complete raw sentinel
allocator, checked range erase, and existing shared CRT free implementation.

`bind_static_native_hardware_layout_tree_0108d530(actual_tree, invalid_parameters)`
borrows the header and existing invalid-parameter domain. Bind once before
startup and retain both through actual process-exit processing. Setup does
not write raw storage. Repeated initialization, rebinding and invalid backing
storage are outside the native lifetime contract.

`CD7960` receives no arguments and returns the actual CRT registration status
in EAX. It calls complete `B25DC0`, publishes the returned 28h allocation at
header+4, and sets that allocation's sentinel byte+25 to 1. It then reloads
the current head separately for parent, minimum and maximum self-links, in
that order. Count+8 becomes zero before genuine `std::atexit` registration.
Word0 and untouched key/value/padding bytes are preserved. Registration failure
does not roll back initialization. Allocation failure propagates before
publication or registration; the source adds no exception handler.

The CRT initializer pointer at `CE3504` names `CD7960` inside the established
C++ initializer interval `[CE2734,CE36E4)`. That slot and original zero-filled
global storage were verified against live Ghidra and the installed PE in
`reports/native_hardware_layout_tree_global_next.json`. Real host CRT
registration is a service boundary; this API does not replace the original
CRT's callback storage or entire startup dispatcher.

`CE0C50` captures the current head and its minimum before calling complete
`B2F3A0` with the actual first/minimum and last/head iterator pairs. After range
erasure returns, it reloads the current head and frees that allocation.
Only after free returns does it clear header+4 and count+8. Word0 remains
untouched. The original normal return zeros EAX; the new registered callback
uses a void interface. Exceptions from the checked range operation can
propagate; the reconstruction adds no rollback or earlier termination.

The initial Ghidra shutdown function ended at returning free `CE0C77`.
Primary integration restored the continuation through `CE0C8F`, including
head/count clearing, preserved existing comments, and saved the project.
It also created the missing initializer. The prior shutdown inventory label
`CG_static_init_00ce0c50` and comment are retained in the annotation journal.

The strict primary Win32 build and both existing CTests pass. An independent
ignored fixture executes both complete original static bodies with exact
production dependency bridges and real CRT exit registration. Eight child
processes compare native and source empty, populated, replacement-head, and
allocation-failure cases. Real LIFO exit validators bracket the registered
shutdown; the fixture does not manually invoke a substitute shutdown callback.
The comparison matches 5,597 DWORDs, 19 guarded header stores, 17 actual frees
and three successful production registrations per side. Raw sentinel bytes
are compared only where initialized or deliberately set by an observation
boundary.

Detailed independent evidence is in
`reports/native_hardware_layout_tree_static_fixture.json`; integration and
contracts are in `reports/native_hardware_layout_tree_static_audit.json`.
Node values remain borrowed; node erasure does not release hardware-layout
owners. Pool lifetime, COM ownership, full renderer startup, native ABI
compatibility and gameplay validation remain distinct. Descriptive names
are reconstruction hypotheses.

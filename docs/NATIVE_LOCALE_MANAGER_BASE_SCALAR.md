# Raw locale base scalar wrapper

`delete_native_locale_manager_base_00737010` binds the complete 30-byte
CFEA5C slot-zero wrapper to actual aa56 base retirement and the matched current
heap free service. It captures the receiver, calls 7365E0, reads the actual
caller-owned flag DWORD's low byte after normal return, optionally frees the
saved receiver for bit0, and returns its address bits without a post-free read.
No flag is copied or seeded before retirement; other bits are uninterpreted.

Saved assembly covers 00737010..0073702E exclusively: PUSH ESI; capture ECX;
CALL7365E0; TEST BYTE[ESP+8],1; optional captured-pointer BF65AC; return captured
address in EAX; RET4. There is no local EH or cleanup. Existing compiler naming
is retained; the descriptive Source name is a hypothesis.

The new ordinary Source interface uses a genuine volatile DWORD reference and
the same borrowed publication context as construction/base retirement. It is
not a binary entry replacement. Actual heap/profile selection, field/cell/
section/registration lifetime, alias and current-provider proofs are caller
obligations. In particular, selecting the base profile does not dispose of
derived sidecar/string/map payloads. No fallback to full or semantic destruction
is introduced, and no owner/dispatch/startup permission follows.

Compilation/common checks are recorded separately by the integrator. No new
test, probe, body execution, Original ABI/FH3/SEH, runtime or vendor-body credit.

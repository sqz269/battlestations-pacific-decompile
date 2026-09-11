# Native renderer frame-target binding

Complete source for `B24E70..B24FAE` (318 bytes) uses actual renderer and
frame-target owner storage, the established concrete lifetime providers,
actual synchronization globals and the application byte at `F8D398`.
The original interface is ECX renderer, stack group, RET4. The new C++
interface adds explicit context and is not an original caller ABI replacement.

Optional guard entry precedes the identity read at renderer+1908; the read
at `B24EAF` precedes cleanup arming at `B24EB5`. A changed incoming group
causes a second old-pointer load, a second identity test, publication and
incoming atomic increment before outgoing atomic decrement. Final zero
uses the captured outgoing group's current profile and virtual zero, then
full `BD30E0` rereads the profile and selects scalar destruction at +4.
The established concrete two-slot `D5E600` profile reaches full `B1FCF0`,
including its vector/surface/COM/CRT lifetime and native exception behavior.
Foreign profiles are outside this explicit application contract.

Null incoming does no GPU work. Nonnull incoming checks the CURRENT
application enable byte after outgoing destruction. If enabled, the exact
incoming +3C byte becomes DWORD render state C2 through full `B24460`.
Then full `B23D80` unbinds slots1..3, then binds slots0..3 using the
incoming group's current borrowed getters. Full `B21690` binds its current
depth getter. Callbacks may mutate the renderer's retained slot, but all
these child reads continue using the original incoming argument. Each
child getter happens immediately before its corresponding child call.
Earlier publication, reference updates and GPU work are not rolled back.

Both normal branches read current mode before disarming cleanup and call
full `B33B00` with the saved renderer if enabled. Native handler `CBD0A8`
uses FuncInfo `DF5754`, unwind map `DF574C` and guard funclet `CBD0A0`
passing EBP-14 to full `B21110`. A second C++ cleanup exception terminates.
The skipped-entry record is uninitialized, with no invented default fields.

The report pins fresh original live/PE instructions. Vector and owner
integration, strict main build and original-code fixture checks remain
pending. This source does not establish renderer or gameplay validation.

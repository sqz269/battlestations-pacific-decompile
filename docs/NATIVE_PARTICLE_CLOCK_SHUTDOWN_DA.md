# Direct raw particle-clock shutdown (DA)

This source composes six complete native operations: DDA40 release-all (84
bytes), DDAA0 array destruction (23), DE290 secondary destruction (95), B1B680
complete-owner destruction (89), DE340 scalar deletion (30), and DE360
secondary adjustment (8). Each already has a typed or external raw full-function
record. DA adds **zero native function/body-byte credit**. Existing names and
evidence remain descriptive hypotheses.

The prior raw implementation is orchestrator 5's `native_sampler_owner_lifetime`
at source commit abfc4761. CW2 established that its record algorithms use actual
storage, while its release edge requires a canonical owner and host atomic
operation. DA composes the current InterlockedDecrement IAT cell and actual
sink slot0 through CV, without an operation/replay state or owner admission.
Its required context borrows the actual CY resize services, publication cell,
IAT cell, and current CE7D08/CE7D24 table views. These two recovered cache
profiles must identify DDB40 at slot+10. Other profile/target identities remain
unresolved and fail explicitly before sink release; no unknown target is stubbed.
The sink's own current profile must be callable in the supplied owner domain.

DDA40 reads the current count and data, captures the last sink, and only then
reads the current cache profile and slot. Following sink release, it rereads
count and data for record destruction. Its subsequent count decrement is the
native single memory ADD, not a saved count. Each loop test is fresh. The final
resize0 is retained. DDAA0 resizes to zero, then frees the **current** array and
retains the native stale data/capacity fields.

DE290 writes CE7D08 and arms completed array cleanup only around release-all.
It disarms before the second resize0 and current-array free. Native FuncInfo
D8FCB4/mapD8FCAC has state0 -> -1/C66F80; that action loads the retained
secondary owner, adds four, and tail-jumps to DDAA0. Source unwinding performs
that cleanup; a cleanup exception during unwinding terminates. Normal final
resize/free failure does not retry array cleanup.

B1B680 writes CE7D38 and CE7D24, destroys the secondary owner, then performs
CX's unconditional F8D420 clear followed by CE3818. Its completed base cleanup
also runs during source unwinding. Native DF4B04/mapDF4AFC state0 invokes
CBC790 -> B4F10 with the retained complete owner. The raw source does not
reproduce the original private FH3 frame or asynchronous hardware-fault delivery.

DE340's public adapter preserves the original flags stack word and RET4 while
adding context in EDX. Its body destroys the retained owner before reading the
volatile low byte of the original public flags slot, frees on bit0, and returns the retained
address even after free. DE360 subtracts four from actual secondary ECX and
tail-forwards that same slot. This corrects the earlier by-value source wrapper
boundary without changing the native argument-read contract.

All allocation, record/string cleanup and pool services retain their existing
concrete source CRT/Win32 and exception limits. Full original FH3 execution,
unresolved cache/sink profiles, fresh concrete particle destruction, complete
singleton-manager terminal composition and gameplay remain separate validation
requirements. The companion report records native spans, calls, source/provider
hashes, exact build, generated-code review and any focused execution evidence.

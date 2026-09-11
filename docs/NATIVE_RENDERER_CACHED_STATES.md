# Native renderer cached state setters

This packet reconstructs the full actual-storage bodies `B24460..B24509`
(169 bytes), `B24610..B24705` (245 bytes), and `B26170..B262BF` (335 bytes).
The original signatures are ECX renderer plus state/value with RET8, ECX
renderer plus sampler/state/value with RET0C, and ECX renderer with RET.
New C++ interfaces borrow the actual renderer and existing actual synchronization
globals. They are independent of unresolved logical-vertex owner destruction.
Full source passes the strict Win32 build and both existing CTests.
Original-code fixture verification and reconstruction closure remain pending.

Both setters enter the optional guard before cache observation. They read the
current validity byte before arming the native-shaped cleanup state. They then
read the current cached DWORD only when validity is nonzero. A changed/invalid
entry writes validity1, then the requested DWORD, then captures the current
device and its current table. A returning COM call increments the current
attempt counter even for failure HRESULTs. A thrown callback preserves prior
cache/observer writes, skips the counter increment and invokes full guard cleanup.

Render validity/value locations are renderer+40+state and+114+4*state; the
counter is+1BA0 and the device slot is+E4. Sampler validity/value locations are
renderer+11CC+72*sampler+state and+11DC+72*sampler+4*state; the counter is+1BA4
and device slot+114. All arithmetic is wrapped DWORD arithmetic. Samplers
unsigned>=16 become sampler+F1 for COM only; no enum or index validation is added.

Normal cleanup reads current synchronization mode, disarms its own unwind, then
conditionally calls actual B33B00 using the saved renderer. The ignored guard
result's high padding bytes have no semantic use. A skipped entry leaves its
record uninitialized; no default renderer or taken flag repairs it. Native
CBCF98/DF55E4 and CBCFD8/DF563C have one cleanup state and no catches; their
eight-byte funclets CBCF90/CBCFD0 pass the record to full B21110. Second C++
exceptions during cleanup retain the established termination policy.

The default initializer invokes19 ordered render states, followed by seven
ordered sampler defaults on each of20 banks. Calls use the complete setters
and observe callbacks between every individual state, without snapshotting the
device, cache or global guard mode. It has no independent guard or cleanup.

Full renderer reset, drawing, original-caller ABI and gameplay remain unclaimed.

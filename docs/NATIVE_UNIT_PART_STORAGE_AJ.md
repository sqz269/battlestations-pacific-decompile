# Unit-part storage construction and cleanup

Addresses: 004E6480, 004E6570, 007103A0, 007103C0, 00710870, 007108E0,
00710F90, 00710FC0, 00712B40, 00711000, 00710420, 0070FAA0, 00711F70.

The unit-part constructor now has concrete defaults for its two sentinel
allocators and five storage cleanup operations. `NativeUnitPartStorageBindings`
also supplies the recovered collision-node field initializer and the existing
complete collision builder, using the same borrowed global cells and validation
callback. It creates no replacement model, vector, hierarchy or global values.
Selected-set destruction, render-name binding, attachment and final entry
production still require complete providers; this is not game admission.

## Complete storage bodies

`004E6480..004E653B` takes ECX=node and returns without stack arguments. It
copies the current D7A248/D7A244 seed bits through MOVSS, preserving signaling
NaNs and signed zero without x87 conversion. It clears owner+4C, bounds and
count fields, allocates eight child-pointer bytes through BF55BE, then initializes
the intrusive link fields. It leaves the primary table and all other bytes
untouched. Allocation failure occurs before publishing children+FC or the
post-allocation link writes; the caller owns the enclosing allocation.

`004E6570..004E6587` installs CE89E8 and frees nonnull children+FC through
BF6989. It leaves that pointer dangling and does not free the node itself.
Neither child objects nor shape references are destroyed by this base body.

`007103A0..007103B9` and `007103C0..007103D9` have identical bytes after
relocating their BF681B calls. ECX is unused; EAX returns a raw30h sentinel,
with next+0 and previous+4 set to self. The value payload is uninitialized.
Source allocation uses the existing malloc/new-handler/bad_alloc implementation.

`00710870..007108B7` and `007108E0..00710927` are identical after call
relocation. Both take ECX=list and plain RET. They self-link the sentinel and
zero count before freeing captured nodes. Each next pointer is loaded before
freeing its node; the current owner sentinel is reloaded after each free.
Finally they free the current sentinel and null owner+4. Owner+0 is untouched,
and no payload destructor runs. `00710F90` and `00710FC0` are five-byte tail
jumps to these bodies, not additional implementations. A valid live sentinel
is required even for an empty list.

`00712B40..00712B7C` takes ECX=outer row header. For a nonnull begin pointer it
captures begin/end, calls `00711F70`, reloads and frees the current outer begin,
then clears+4/+8/+C. Header+0 is untouched. The complete `00711F70..00711FA5`
range loop is now exposed from the existing group implementation and reused by
both resize and destruction. Native ECX=first, EDX=end, two unused stack words,
RET8; stride10h. Each row uses the existing canonical DWORD-vector clear to
free nonnull inner storage and zero+4/+8/+C without freeing referenced owners.

## Existing entry-array implementation

Full instruction streams match after direct-call relocation, and the callee
mapping closes recursively:

| Unit-part entry | Existing source address | Complete operation |
| --- | --- | --- |
| 0070FAA0, 95 bytes | 00B1C500 | Reserve signed capacity with minimum1 |
| 00710420, 80 bytes | 00B1C770 | Resize and clear newly exposed pointer slots |
| 00711000, 23 bytes | 00B1D1D0 | Resize0, free data, retain data/capacity |

The constructor cleanup default calls the canonical destructor directly. No
second array implementation is introduced. Its existing valid-storage contract
continues to require nonnegative count/capacity, count at most capacity and
representable allocation arithmetic. Native private-stack aliases, corrupt
headers, original CRT exception identity and hardware faults are unproved.

## Evidence and limits

The paired fixture executes thirteen physical original spans against the actual
compiled source library, with canonical source CRT allocation/free boundaries.
Three complete storage-lifecycle scenarios compare normalized raw part images:
empty and populated lists, populated group rows, an allocated empty row vector,
reserve/growth/shrink, retained entry storage, base initialization and cleanup.
Seed variants include signaling NaNs, signed zero and a subnormal bit pattern.
A separate paired null-child branch checks base destruction. The original
callbacks observe 21 node/sentinel frees after detachment. Source free calls
are real CRT calls; their internal event order is not intercepted by the probe.

Two source constructor cases exercise the actual defaults through success and
an exception from the explicit final-entry provider boundary. Cleanup reaches
the selected-set provider only after entries, all lists and groups are cleared,
then destroys the base. The null-unit fixture does not synthesize a string pool.
The concrete collision binding is also called on that existing empty selected
set. These source checks do not execute native FH3 or implement the remaining
selected-set/final-entry providers.

All sixteen direct-call rows pass ownership verification. Two interior
returning-free gaps were repaired and saved. Three additional tails were
byte-verified and decoded, but the current repair API does not extend their
stored Ghidra function bodies: 00710870 ends at007108AB, 007108E0 at0071091B,
and00711000 at00711011. Their complete decoded spans are explicit above and in
the report. No function recreation or callee no-return change was attempted.
Names are descriptive hypotheses, not recovered symbols. Source C++ interfaces
have a new ABI; native exception/fault delivery and gameplay remain unproved.

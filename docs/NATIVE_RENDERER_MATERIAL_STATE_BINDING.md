# Native renderer material state binding

This packet reconstructs complete `00B27A80..00B27AF9` (122 bytes) and
`00B27B90..00B27C11` (130 bytes). They bind the actual render-state owner at
renderer `+34` and sampler-state owner at `+3C`. Names describe recovered
behavior. `B27B00`, general cache clearing and resource release remain separate.

The original entries receive renderer in ECX and the incoming owner on the
stack, return with `RET 4`, and have no semantic result. The new MSVC Win32
fastcall interfaces additionally receive a fixed borrowed context in EDX.
Their one added stack cell holds that context; all original row, identity,
reference and counter instructions are retained. Compiled bodies are 133 and
141 bytes after the documented context and direct-provider bridge changes.
These are new C++ interfaces, not original caller/stack/SEH ABI replacements.

## Actual owner and provider contract

The reached allocation is the actual 14h-byte `NativeMaterialStateOwnerStorage`,
with an established object lifetime: profile at `+00`, four-byte atomic reference
count at `+04`, and raw `{data, signed count, signed capacity}` header at `+08`.
It belongs to the actual shared allocation service used by
`native_material_pass_states`. A companion reference object is not this storage.
The positive original producers install `D61A2C` at `B5F79C` and publish material
pass `+18` at `B5F7B1`, or install `D61A34` at `B5F7C9` and publish pass `+20` at
`B5F7DE`. `B43422..B4343B` reads those cells and calls these two binders.

The context borrows the actual synchronization global storage and immutable
two-DWORD views of `D61A2C={BD30E0,B422F0}` and
`D61A34={BD30E0,B42310}`. Each view needs exactly eight readable bytes when its
identity is reached. No unrelated profile is required, and the binder never
reads or validates the incoming owner's profile merely to apply its rows.
Unsupported terminal identities are outside the source domain. These views
contain original identities, not host callback pointers.

On an old final-zero release, the bridge consumes the captured old profile's
current slot zero, then follows the full `BD30E0` receiver test and **fresh owner
profile/slot four reload**, passing flags `1` to the actual full deleting provider.
Render deletion composes `B422F0/B420C0`; sampler deletion composes
`B42310/B42140`. Their existing shared-heap, valid-extent and exception contracts
remain in force, including negative-capacity reserve handling, current row
header loads, row free, base-profile cleanup and conditional owner free.
The throwing direct providers are used; `NativeMaterialStateReference` and its
`noexcept` terminal are not involved.

The two per-row bridges call full actual `B24460` and `B24610` with the borrowed
synchronization storage. Their actual guard entry/leave/cleanup, current device
and table loads, cache writes, ignored HRESULTs and attempt counters remain
inside those complete providers. The new binders add no outer guard, EH frame,
rollback or null-success policy.

## Native ordering

Both entries first compare the current binding with the original incoming
identity; equality skips all context, reference, row and counter accesses.
A changed path recaptures the current old owner and compares again. It publishes
incoming before the real atomic increment, then decrements the captured old
reference count. An old zero result invokes the terminal described above.
The original incoming identity survives all these calls.

For a nonnull incoming owner, a signed current count greater than zero enables
iteration. Every render iteration reloads the current data pointer, then loads
value before state from the 8-byte row. Every sampler iteration reloads current
data, then loads value, state and slot from the 12-byte row in that order.
The loop increments its original index/byte cursor and rereads current signed
count after each full setter returns. It does not capture count or row base for
the entire operation. The current renderer counter at `+1B94` or `+1B9C` is
incremented with DWORD wrapping only after the changed path returns, including
null replacement. An exception can therefore leave publication, reference or
earlier row changes visible without the final increment.

Neither owned entry has an EH registration. Fresh supporting bytes also pin
the reached provider unwind metadata: cached render `CBCF98 -> DF55E4`, map
`DF55DC`; cached sampler `CBCFD8 -> DF563C`, map `DF5634`; material render
`CBF0B8 -> DF7A08`, map `DF7A00`; material sampler `CBF0D8 -> DF7A34`, map
`DF7A2C`. The material cleanup actions reach full `BD30F0`, which installs
`CEB130`. These chains are checked from each full entry's actual handler PUSH,
through the handler's FuncInfo load and reached unwind map/action. The adjacent
`CBCFB8/DF5610` chain belongs to the separate `B24510` texture-stage setter.

## Verification and boundaries

The strict actual library built through the ignored CMake hook and the normal
`scripts/build.ps1`; both existing CTests and eight freshly verified native
seeds passed. The entire resulting archive, five needed exact archive members
and 29 recursive source/header inputs were frozen before fixture linking.
The library SHA-256 is
`2e7e1ffe5d2bd78f3512311a6ba08a6d9c5bce2990b154778ddfb185d3eda727`.

One ignored focused fixture compared both complete original binder bodies,
plus complete original `BD30E0`, against the frozen library. Original direct
setter and deleting edges use explicit callee-ABI bridges to the complete
actual source providers. The original setter/destructor bodies themselves
were not runtime-replayed. A private read-only executable page contains the
original code, relocated import cells and relocated original profiles. All six
modified code operands and four profile cells are enumerated against fresh
original preimages; its entire 4096-byte postimage is unchanged. An initial
native-address reservation failure occurred before binder execution and was
resolved by that explicit relocation.

The nonempty old owners reached real final-zero deletion: current CRT `free`
released their row buffers and owners in order. A fixture-only import observer
copied live bytes immediately before each free, called the actual target, and
recorded its return. The full source deleting providers executed their own
normal cleanup; the compiler inlined the source destructor bodies into them.

Renderer `+1A10` in this fixture contains a **fixture COM-call view**, not an
actual complete device object. Its two reached entries first forward to the
real HAL device's original `SetRenderState` or `SetSamplerState` target, then
mutate the local incoming row base/count and renderer block counter. Both
original and source paths use the same explicit instrumentation boundary.
Eight real calls returned `S_OK`; the actual device pointer and all 119 driver
table words remained unchanged. Real tracked critical sections and the full
actual optional-guard providers surrounded the calls. No user window or focus
was changed; the fixture used its own hidden window.

Both entries agreed on two live rows despite initial count one, the changed row
base, wrapping final counters, identity skip with an inaccessible source
context, and changed null replacement. There are 74 whole raw comparisons.
All raw originals are retained. Normalization is restricted to explicitly
recorded renderer binding/lock/call-view pointer fields, owner row-data pointer
fields, and the original execution's old-owner profile field at `+00`; no
address-range or arbitrary-word normalization is used.

All 218 mapped sections from the five actual archive objects and fixture
object match their COFF bytes and all 690 relocations, including map entries
with both `f` and `i` flags. All 14 owned COFF sections and 12 relocations are
frozen, including emitted helper sections removed by the linker. The complete
linked runtime code postimage is 13,652 bytes. Both original-to-source owned
instruction transformations, actual import targets and source closure are
checked. The audit records 27 fresh guarded PE-matching spans (1,804 bytes),
including 252 owned bytes.

The fixture covers normal returning provider composition. It does not add
evidence for every allocation failure, C++ or hardware exception, unmasked FP,
concurrent mutation, foreign owner profile, original complete COM-object
execution, arbitrary original callers or gameplay. Existing provider validation
and host exception boundaries are inherited explicitly. Evidence is under
`local/renderer_material_state_binding/`; no permanent tests or shared build,
ledger or Ghidra changes are part of this packet.


## Primary integration

Main registered the unchanged instruction/body implementations and passed the strict Win32 build, both existing CTests and eight fresh original seeds. Every packet used the same frozen main library `047eefa1b510fe1f4d93a994ac700b5609a33efd85aa90b600909185e98fa93f`. Primary verified304 sealed worker files,29 current source/header inputs and27 fresh spans1804bytes. The owned CPP differs from its frozen worker snapshot only in CRLF/LF; decoded text is exactly equal. The unchanged main-library fixture passed74whole raw comparisons and8realHALcalls. All218mappedCOFFsections690relocations71imports and13652runtime code bytes pass, with only listed pointer/profile normalizations. Actual device/table remains unchanged; original child/child-EH and full COM-object execution remain unclaimed. The existing cached-sampler EH record was independently confirmed correct. The primary saved reviewed names/comments while retaining prior values, registered raw source entries, and refreshed all affected exports. Immutable proof: `local/material_state_binding_primary/`. Original-caller/SEH and gameplay limits remain.

# CC12 pending registry getter Source candidate

Baseline: `a6b846ecce1f780962154003f1d30c7f4933870f`.
Packet: `cc12_pending_registry_getter_source`; address `00875280`.
This packet adds an ordinary Source C++ candidate and its evidence only.
It does not register or compile the candidate. Primary CMake integration,
normal MSVC Win32 build, complete emitted-body and actual-provider review,
Core resolution, and admission remain pending.

## Interface and authority

`get_native_pending_registry_00875280` takes, in order, stable borrowed
`void* volatile&` references to the actual registry publication `00F878CC`
and actual manager publication `01090AA0`. It returns `void*` and has no
`noexcept` specification. It adds no global, alternate provider, callback,
default cell, ownership projection, or implicit production activation.

Native `00875280[189]` consumes no explicit input and returns EAX with plain
RET. These two Source references form a new ordinary C++ interface. The
accepted native report records the complete 189 bytes, 51 instructions,
five direct calls and two actual Win32 import calls; the new report retains
that complete byte string and verifies its recorded hash. No new Native
body, handler, table, caller, PE, or Ghidra read was performed in this packet.

Production use requires the already admitted canonical process registry cell
and manager cell, with the matching `GameSingletonHost` deletion bindings kept
alive through manager drain. The process retains the registry cell alongside
its manager authority; retaining the process does not extend a Host's life.
The existing finite `D0DEA0` route supplies the popped owner, actual deleting
flags-byte reference and this same registry cell to the admitted scalar helper.
The corresponding primary report closes those Source composition gates.
This packet supplies no Host call site, bootstrap change, factory activation,
registration outside the getter, or native callable vtable.

## Ordinary operations

The initial volatile registry read is captured. A nonnull capture returns
directly without manager lookup, raw-section access, allocation, or locking.

On the slow path, the actual `get_native_singleton_manager_00415350` receives
the borrowed manager cell. The getter reads the returned manager's raw `+10h`
section without introducing a manager-null guard. It captures that section once
and constructs a trivial local eight-byte guard with `CE37FC` at `+00` and
the captured section at `+04`. Static assertions cover pointer width, both
field offsets, guard size/triviality and the Win32 section's `18h` size.

A nonnull section is passed to real `EnterCriticalSection`, then its actual
unsigned DWORD at `+18h` is incremented modulo 2^32. The outer Source cleanup
scope begins only after this sequence, also covering the null-section path.
The registry is reread inside that scope.

If the recheck is absent, the actual `singleton_lifetime_allocate` receives
`{SingletonAllocationKind::object, 8, 8}`. This call is outside the inner
allocation-cleanup scope. After return, the captured allocation is protected
while the actual `construct_native_pending_registry_00874bc0` runs for a
nonnull allocation; a null allocation yields a null constructed result.
The constructor's returned value is retained independently from the captured
allocation and is the value subsequently published.

The inner scope ends before publication. The actual manager getter is called
a second time after publication; only after that call returns is the current
registry reread and supplied to actual
`register_native_singleton_object_00bd0c30(current_manager, nullptr,
current_registry)`. That current value can differ from the constructed result.
No early cached registration argument or first-manager substitution is used.

Both recheck branches converge on the original captured section. For a nonnull
section the unsigned depth is decremented modulo 2^32 before real
`LeaveCriticalSection`. The outer scope remains active through normal Leave
and the final volatile publication reload. The slow path returns that reload.
There is no normal guard destructor, profile rewrite, section refetch, section
destruction/free, or early cleanup disarm in this getter.

## Cleanup and qualification

The independent accepted getter EH report establishes encoded state 0 after
Enter/increment and state 1 only after allocation returns. State 1 becomes 0
before publication. State 0 remains armed through ordinary Leave/final reload.
The encoded actions are current guard-address cleanup at `00C96420` and
current allocation-spill free at `00C96428`, with state 1 leading to state 0.
The shared Native helper's required frame relation remains unproved.

The Source inner catch frees the captured allocation with actual
`singleton_lifetime_free` and rethrows. The admitted constructor already
clears the borrowed current registry cell and resets the owner's base profile
on Source C++ failure before that outer allocation cleanup is reached.
The Source outer catch invokes admitted
`destroy_native_singleton_guard_00411ee0(&guard)` and rethrows. That service
captures the current raw guard's `+04` section, stamps `CE37FC`, and for a
nonnull section decrements unsigned depth before real Leave; it preserves
guard `+04` and does not destroy or free the section.

Consequently an ordinary allocator exception reaches guard cleanup without an
armed allocation-free scope. An ordinary constructor exception follows its
own base cleanup, captured-allocation free, then guard cleanup, provided those
cleanup steps return. Failure during second manager lookup or registration
has no getter allocation free or publication rollback. It retains whatever
current publication those calls left. The normal Leave is still within the
guard-cleanup scope; the Source code adds no early successful-exit disarm.

These are Source C++ catch/rethrow scopes using the current Source CRT and real
Win32 providers, not a reproduction of original FH3/SEH dispatch. Native
actions read mutable runtime-managed stack spills; Source locals and the raw
local guard do not establish that frame/helper ABI or arbitrary native-spill
alias behavior. Stable borrowed-cell references do not provide portable C++
synchronization or establish concurrency correctness. Hardware faults,
SEH-to-C++ translation, native register effects, exact throw identity, original
CRT heap/new-handler domains, and nested cleanup failure remain qualified.
`singleton_lifetime_free` is the existing `noexcept` Source `std::free` wrapper;
its failure behavior is not evidence about native `BF65AC` failure handling.
No guarantee is made that later cleanup runs if an earlier cleanup fails.

The two supplied references must name the actual distinct authorities and remain
valid throughout each call. Numeric owner/guard/base profiles are evidence
identifiers and cannot be invoked as C++ vtables. The explicit Source interface
is neither an original no-input ABI entry nor a binary/game-validated replacement.

## Evidence and validation

- Accepted complete ordinary evidence:
  `reports/cc12_pending_registry_getter_readiness.json`.
- Independent encoded EH/direct-action evidence:
  `reports/cc12_pending_registry_getter_EH_readiness.json`.
- Admitted constructor and concrete providers:
  `reports/cc12_native_pending_registry_constructor_primary_review.json`.
- Admitted canonical process cell, Host binding and finite scalar dispatch:
  `reports/cc12_pending_registry_process_cell_and_dispatch_primary_review.json`.
- This candidate's exact source, current dependency/evidence hashes, prior
  Source-pin comparisons, complete accepted native bytes, operation mapping,
  scope review, and remaining admission gates:
  `reports/cc12_native_pending_registry_getter_source.json`.

All 49 Source-pin entries selected from the four accepted reports match the
current files (duplicates represent independent prior reviews). Both current
primary document pins also match. Historical build/configuration hashes in
those reports are not represented as this candidate's build evidence.
JSON parsing, accepted-body hash/contiguous-instruction verification, focused
Source scope/order review, four-file ownership, and whitespace checks passed.
No compiler invocation, test, probe, Ghidra mutation/query, ledger change,
additional reconstruction credit, or gameplay execution is claimed here.

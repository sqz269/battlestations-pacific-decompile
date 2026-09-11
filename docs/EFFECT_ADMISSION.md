# Point-effect admission and manager lifetime

`src/effect_admission.cpp` reconstructs the template admission loop and the
eight-byte effect manager's construction, lazy publication, and deletion. The
interfaces use the existing `CameraTransform`, `SystemSingletonCriticalSection`,
and `SingletonLifetimeDomain`; they are not native ABI replacements.

## Admission: 0086A650

Native ABI is ECX template, stack XYZ pointer and reference-transform pointer,
AL result, RET8. The decompiler's 32-bit return is misleading: the final write
only changes AL, leaving prior upper EAX bits. The typed result is `bool`.

The actual reference transform refreshes through `00B6DB70` only when its +5C
bit 2 is clear. Its world translation is +120/+124/+128, corresponding to
`CameraTransform::world[12..14]`. The template contributes actual pointer-array
storage at +08 and count at +0C; the loop captures the array's end once. Neither
the template array nor transform is replaced by a snapshot owner. The typed
projection borrows those actual fields, and the callback receives the original
XYZ and canonical reference-transform objects.

Assembly resolves the floating-point behavior:

- `0086A66F..698` loads/stores the three reference coordinates with x87.
- `0086A69E..6BA` subtracts them from XYZ, spilling each delta to float32.
- `0086A6BE..6DA` evaluates the original x87 stack sequence, including
  `(dy*dy + dx*dx) + dz*dz`, then spills squared distance once to float32.
- Each gated row loads its float +18 at `0086A6E8`, squares it on x87 without
  a float32 spill, and compares that value to the saved distance. `FCOMIP; JA`
  accepts only an ordered strictly greater squared threshold. Equal and
  unordered values reach the virtual callback. Negative thresholds still square;
  there is no absolute-value clamp, square root, or NaN special case.

The separate x87 helpers preserve this operation/spill order and the caller's
floating-point environment. In particular, XYZ equal to threshold
`1.00000011920928955078125f` can pass: the stored squared distance rounds below
the unspilled squared threshold at extended precision. A usual float expression
that rounds both products before comparing is not equivalent.

For each actual row pointer, +10 byte zero admits it and leaves +1C untouched.
Otherwise distance passing or a nonzero low byte from current virtual+1C admits
it. The virtual call uses the row selected before the call, but the normalized
0/1 store goes to the CURRENT pointer in the captured slot (`0086A706/713`).
The loop continues across its captured end even if a callback changes the
template's pointer/count fields. A callback may replace slots and mutate the
reference transform; the saved distance is not recomputed. Exceptions propagate
without writing the pending row result, retaining earlier row writes.

`EffectAdmissionDispatch::project_row` is a required pure, nonthrowing mapping
from each actual native owner to references to its +10/+18/+1C fields.
`virtual_1c` must dispatch that owner's current implementation; no default
success callback exists. The captured array must remain allocated throughout
the call. Concurrent field mutation and recovery from asynchronous native memory
faults are outside this C++ host contract.

## Manager: 00866230, 00866440, 008669D0

`EffectManager` stores only original-image vtable identity and the canonical
section projection. The values D0D3CC and CE3818 are integer evidence identities,
never callable original addresses or reconstructed process vtables. The native
object is 8 bytes; the current Win32 typed owner also happens to be 8 bytes, but
its +04 projection pointer is not a native CRITICAL_SECTION pointer. No native
layout/ABI compatibility is claimed.

`00866230` writes identity D0D3CC, creates a section through `00BD1860`, and
stores it at +04 only after creation returns. No extra manager members are
initialized. The concrete section allocation adapts native 1Ch bytes to the
actual Win32 section, actual +18 count, and borrowing projection. It uses the
canonical raw allocation/new-handler protocol, initializes the OS section,
then zeroes the count. The effect manager's +04 lock is distinct from the
lifetime manager's +10 lock.

Constructor exception handler C94CC8 uses FuncInfo DC6ABC and unwind map DC6AB4:
state 0 goes through C94CC0 to `00865FB0`. That base unwind unconditionally clears
F87650, then restores CE3818 identity. It does not inspect or free an unwritten
+04 value. If native InitializeCriticalSection itself raises an exceptional
failure after allocation, the evidence supplies no local raw-section unwind
free; the implementation does not invent one.

`00866440` returns its initial loaded pointer immediately when nonnull. Otherwise
it obtains the shared lifetime manager, captures its +10 section, enters it,
increments that section's actual +18 count, and rechecks F87650. It allocates 8
native bytes, constructs when nonnull, publishes the result, obtains the shared
lifetime manager again, reloads F87650 for registration, unlocks the originally
captured section, and reloads the result. Null allocation still reaches the
second getter/registration call. The canonical allocator normally returns raw
storage or throws, but this control-flow branch is preserved.

Getter handler C94D13 uses FuncInfo DC6B1C and map DC6B0C. State 1 first frees
raw manager storage through C94D08; state 0 unlocks through C94D00/00411EE0.
A constructor failure runs its base unwind, frees raw manager storage, then
unlocks. Failure after publication (second getter or registration) only unlocks;
it neither clears nor frees the published owner. The next getter can therefore
take the fast path without retrying registration.

`008669D0` writes D0D3CC identity, destroys the actual owned +04 lock using
0041CC80 behavior, clears F87650 unconditionally, writes CE3818 identity, and
frees raw owner storage only for flag bit 1. It returns the original pointer even
when freed. Section destruction drains a positive signed +18 count by leaving
the real lock, deletes the OS section, frees its allocation, then clears +04.
The owning/quiescent thread requirement matches the canonical lifetime manager.
There is no unregister call in this destructor.

`ConcreteEffectManagerLifetimeAccess` uses an existing shared
`SingletonLifetimeDomain`. The published owner and the pointer passed to
`ConcreteSingletonLifetimeManager::register_object` are identical; there is no
surrogate registration token or second lifetime domain. The application's
required `SingletonLifetimeCallbacks::destroy_registered` dispatcher must route
that actual registered owner to `delete_effect_manager_008669d0`, passing the
same F87650 binding and access object. No production dispatcher or game-host
binding is added by this packet. The ignored fixture supplies and exercises an
explicit dispatcher that verifies identity and calls the real reconstructed
deleting destructor, including lock destruction and raw free.

## Evidence and verification

Every live query/export used `bsp.py ghidra`, which verifies the configured
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, language, and image base.
Live count was 63101 at the initial verification. This packet made no Ghidra
mutations and did not run original game entry code. Descriptive names remain
hypotheses; the report records prior names and annotation proposals.

The 008669D0 export has a three-byte missing listing at 008669FE; live bytes and
disk decoding prove `83 C4 04` (`ADD ESP,4`) before the common return. Its full
inclusive end is 00866A06. Missing function starts C94CC8 and C94D13 are ten-byte
EH handlers ending C94CD1 and C94D1C. Existing C94D08 is truncated at its free
call; live/disk bytes confirm its POP ECX at C94D11 and RET at C94D12. These are
reported for the integrator, without mutating shared Ghidra state.

`./scripts/build.ps1` passed both existing CTests (`reconstructed_math` and
`native_math_differential`). Since startup source registration is assigned to
the integrator, that build did not yet compile the new module. The ignored
`local/run_effect_admission_fixture.cmd` separately compiled the new source
with MSVC Win32 `/O2 /W4 /WX /fp:strict`, linked the fixture with
`/MANIFEST:EMBED`, and passed captured-range/distance, slot replacement, callback
exception, threshold rounding, actual lock/registration identity, constructor
EH, and post-publication registration-failure checks. No permanent test was
added. These new functions are fixture-tested, not native-differential-tested,
ABI-compatible, or game-validated. Point-effect construction at 008680B0 and
production owner/dispatch bindings remain separate dependencies.

# Renderer capabilities through the actual lifetime manager

This is an ownership-binding migration of the existing complete B2C8E0 source,
not another reconstruction of its 4094-byte body. The existing raw implementation
already preserves current renderer fields, COM queries, caller scratch preimages,
actual arrays and publication order. Its string-pool context previously required
`SingletonLifetimeDomain`, which prevented composition with the application's
actual 01090AA0 manager publication.

The new `NativeRendererGatherCapabilitiesActualContext` borrows the same scratch,
01090AA8 pool publication and 01090AA4 small-return gate, plus the actual mutable
01090AA0 manager cell. An overload of
`gather_native_renderer_capabilities_00b2c8e0` accepts that context. The old domain
overload remains available. Both call one shared source body, with exactly two
lifetime-selection substitutions: creation of `ActualNativeStringPoolStorage`
and the explicit normal-return pool getter. No new wrapper module, owner map,
manager, pool, capability projection or CMake registration was added.

The existing helper/literal prefix is unchanged. The shared algorithm is identical
to the original source after those two lifetime selections are substituted;
the report pins both comparisons. In particular all 285 ordered stores for the
57 native format rows remain unchanged. This packet adds zero new native bodies
and zero newly reconstructed native bytes.

## Actual providers and publication order

`ActualNativeStringPoolStorage` already has a concrete raw-manager constructor.
Every allocation and armed cleanup release calls the existing raw 00419CC0
getter. Normal return calls that getter directly after disarming, then the
existing BD1510 return body. No semantic manager is reinterpreted as raw storage.

The raw getter reads current 01090AA8 first. On a miss it calls the actual
00415350 manager getter, captures that manager's +10 critical section and tracks
its +18 depth. It rechecks the pool publication under that captured section,
allocates/constructs the actual 8AD4A0h owner, publishes it early, obtains the
manager again, and only then reloads the pool argument for BD0C30 registration.
Both normal release and its existing `__finally` cleanup use the first captured
section. Registration failure retains the published pool/allocation as before.
Raw manager shutdown requires its explicit pool deletion bindings; those bindings
and the actual cells must outlive all operations and any retained ownership.

The renderer remains actual raw storage. Each reached query uses current +1990
and its current real IDirect3D9 COM table: +38 GetDeviceCaps, +14
GetAdapterIdentifier and +28 CheckDeviceFormat. Ignored output-query HRESULTs
do not abort or zero outputs. Reached capability stores remain visible before
later queries and allocations. ATOC failure preserves its old byte. Declaration
and nested format headers use the existing substantive B236B0/B22B30/B2AE20
providers; the arrays are appended to, never replaced with projected vectors.
The existing fixed host memmove/strstr adaptations and their valid-buffer limits
are retained, without claiming original CRT implementation or fault behavior.

## Native frame and parent boundary

The full original span is B2C8E0..B2D8DD inclusive: 4094 bytes, 885 instructions
in a complete linear disk decode. Its ABI is ECX renderer, plain RET. Both source
overloads use the existing new MSVC Win32 interface, ECX renderer and EDX fixed
context. They do not reproduce the native private stack or FS/FH3 caller ABI.

The caller supplies aligned, valid, writable 770h scratch. Native output areas
are scratch+1F4 D3DCAPS9 and +324 D3DADAPTER_IDENTIFIER9, with Description at
+524. The actual string header is +14/+18. The format payload's +25..27 bytes
remain unwritten and are copied into successful records. Valid initialized
preimages are required wherever a query leaves bytes untouched, including a
reachable Description NUL. This overload does not manufacture zeros for them.
Contexts and their fixed bindings must not alias writable operands, and separate
in-flight calls require scratch satisfying the existing per-call contract.

The parent gate is freshly verified at B3289B -> B2C8E0. B32410's B32657..B32675
stores zero into both array headers; subsequent constructor stores zero the
capability limits and flags including ATOC+1B55. Immediately before gather, its
earlier identifier query supplies NVIDIA flag+1D88 and it clears frame count+14.
Those constructor writes do not initialize the gather's own private scratch.
The fixture's deliberately supplied preimages are source-contract evidence,
not recovery of the original constructor's incidental stack bytes. Full parent
constructor/EH composition therefore remains separate from this raw-manager
binding.

The native FH3 handler CBD56B loads FuncInfo DF5DF8. Its sole state0 map at
DF5DF0 points to CBD560, which computes [EBP-768] and jumps to 41DD20. State0
arms only at B2CA0A, after description scan, resize and copy. A failure before
that arm has no local string cleanup; early renderer publications remain.

After arming, a source C++ exception destroys the current header with the
existing full 41DD20 path, then rethrows. That path reads pointer before length
and uses the existing `NativeStringStorage::release` noexcept contract: failure
while lazily obtaining a pool during this cleanup terminates. This limitation
is unchanged and cannot be represented as an ordinary propagated parent failure.

Normal exit captures the nonnull predicate before disarming at B2D8A0, then
captures length before pointer and calls current 00419CC0 followed by BD1510.
A getter exception there can propagate without a second local cleanup. Prior
renderer/array publications and pool registration failure state are not rolled
back. Native asynchronous SEH, stack aliases, original CRT exception identity
and unrestricted reentrancy remain outside the source interface. No x87
instructions occur in the gather itself. No returning-call repair was needed.

## Validation

Fresh capped Ghidra reads verified the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and matched seven complete spans against the
installed PE: gather, raw pool getter, FH3 handler/map, parent call gate and
constructor preimage stores. The report checks 30 direct/parent/helper call sites
and records 11 indirect COM/IAT sites separately. Existing algorithm evidence is
attributed to `native_renderer_gather_capabilities_audit.json`; it is not counted
again as a new body. This worker makes no Ghidra mutations.

The strict Win32 build and both configured CTests pass after eight verified
seeds. One local /MD manifested probe links the rebuilt bsp_core library. It
uses actual raw manager creation, actual pool allocation/registration/return,
actual array providers and canonical pool shutdown. Fixture COM callbacks first
return failures without writing caps/identifier outputs, proving supplied
preimages and earlier publications survive and all 59 format calls still occur.
A deliberately thrown texture-format callback then proves post-arm cleanup
returns the string while twelve appended declaration rows remain published.
These callbacks are test instrumentation, not production providers.

The same probe then calls real Direct3DCreate9 and the real COM gather. On this
machine GetDeviceCaps returned S_OK and 34 formats were published; the unwritten
payload bytes retained their supplied A5 preimage. The same actual manager and
pool survive all phases and are shut down through the existing actual deletion
map. This is source execution and D3D9 query evidence, not execution of original
B2C8E0 bytes, a rendered frame, native FH3 dispatch or gameplay validation.
Immutable source, library, fixture, compiler-input, toolchain, runtime and original
PE hashes are recorded in the report.

## R32 current-main reuse

The validation above describes the historical orch5 packet at `4f12d5636`.
Current-main source adoption and fresh validation are recorded separately in
`reports/native_renderer_capabilities_actual_main_r32.json`. The two source files
are reused unchanged; current main providers and shared deletion bindings remain
in place.

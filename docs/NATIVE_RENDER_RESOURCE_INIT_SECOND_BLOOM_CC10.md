# Render-resource initialization: B11D7E frame and second bloom

Addresses: `00B107F0`, fragment **[00B11D7E,00B11E40)** only.

This stage constructs and publishes the actual frame at service `+1CC`, binds
its color and depth surfaces, then constructs and initializes the second bloom
at `+2C`. It stops before `B11E40 PUSH 26Ch`, the distortion allocation. The
distortion preimage and lifetime problem is outside this packet.

| Routine | Coverage | Interface |
|---|---|---|
| `00B107F0` | Partial: only `[B11D7E,B11E40)`, 194 bytes. Earlier `[B107F0,B11D7E)` belongs to published packets; later `[B11E40,B13029)` remains excluded. | Native containing ECX service, three DWORD stack cells, eventual `B13026 RET 0C`. New C++ retained DOF predecessor, same original context, independent second-bloom state. No native return or argument-cell read. |

The final included instruction is the five-byte `CALL B54F90` at `B11E3B`,
inclusive final byte `B11E3F`. Ghidra's containing body remains `B107F0..B13028`.
There are no missing function definitions or new native names. Read-only BSP
queries verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`.
All 194 live bytes matched the installed PE; SHA-256 is
`079a6739ecbdd8ab83bc15b3c22ff3ed5b31d551388132eaad94f3b3b286adbd`.

## Actual frame and surface order

The new caller-owned state prepares an independent `NativeBloomInitializationBlock`
in the existing bloom context before the first native instruction. The first
bloom block backing `+28` remains retained and is never reused or reset.

`B11D80` allocates `40h` through the existing shared CRT provider; the native
caller executes `ADD ESP,4`. ESP14 changes only after allocation returns. State
52 applies even on the null branch. A nonnull raw allocation is passed to the
actual `B1FBB0` frame constructor at `B11D9D` (ECX/EAX/RET0). The caller does not
clear storage, add another count or construct a surrogate frame.

`B11DA6` captures **current `+44` before publishing `+1CC`**. State disarms at
`B11DA9`, then the actual frame result, including null, is published at `B11DB0`.
No old `+1CC` release is invented. `B11DB6 B4CB20` reads the captured holder's
primary surface. The caller then reloads current `+1CC` and calls full `B1FAB0`
for color slot zero at `B11DC4` (`RET 8`). Actual incoming publication and retain
precede captured old release inside that existing frame provider.

Only after color assignment returns does the caller load current renderer
`F8D394` and its current profile/slot `12C`. The source admits the existing
`D5F0A8` profile with current slot `12C = B20090`; it never calls a numeric host
vtable address. `B11DD7 CALL EAX` resolves to the existing seven-byte `B20090`
getter, which borrows current renderer `+198C` without a retain or COM call.
The native slot bytes at `D5F1D4` and all seven getter bytes matched the PE.
The caller reloads current `+1CC` again before `B11DE0 B1FB00` binds the depth
surface (`RET 4`). A callback's current frame publication remains observable.

## Independent second bloom and x87 sequence

`B11DEA` requests exactly `43Ch` through shared CRT allocation, then updates
ESP14 to that raw allocation. State is 53. A nonnull allocation goes through
the existing `B54E70` actual bloom constructor at `B11E07` (ECX/EAX/RET0).
No allocation enlargement, whole-object initialization or second counter is
introduced.

The source preserves the caller's x87 sequence: `FLD1`, read retained half
aligned height, read retained half aligned width, capture actual current service
`+1C`, then `FSTP` to a DWORD temporary. One inline-assembly region prevents
provider calls while ST0 is live. Its emitted object confirms `FLD1` at `3FA`,
dimension reads at `3FF/407`, current `+1C` read at `40F`, and `FSTP` at `415`,
with no intervening CALL. Under the admitted normal x87 stack this produces
exact bits `3F800000`; original machine-stack aliases and instruction-exact
fault/ABI behavior remain outside the interface.

The retained register meanings now change: **EDI is half aligned height and
EBP is half aligned width**. They no longer mean raw `+74` and service `+A0`.
The captured `+1C` input pass precedes state disarm and publication of actual
bloom result/null into `+2C` at `B11E2F`. `B11E32 B4D170` receives that captured
pass and borrows its `+0C` output holder. The caller reloads current `+2C` for
`B11E3B B54F90`, passing `(holder, half_width, half_height, 71h, 3F800000h)`;
native cleanup is `RET 14h`. The existing initializer and its child blocks
remain responsible for their actual provider contracts and failure behavior.

## Retained state and lifetime boundary

The prior DOF header gains only two phase values and a one-use
`second_bloom_identity`. Entry validation checks the complete predecessor chain,
same original context, published DOF frontier, original argument cells, mask
zero, state `-1` and inherited service+A0 EBP before consuming that frontier.
The new state retains raw/returned frame and bloom pointers, publication flags,
current-load diagnostics, the argument packet and its independent bloom block.
These receipts do not acquire ownership credits; actual current fields/counts
remain authoritative.

On failure all five state records become failed, preserving the current
site/state, actual publications and provider preparation/acquisition records.
Existing child failure semantics stay authoritative. The caller supplies no
rollback, retry, raw free, metadata unbind or automatic reset/destruction.
Partially prepared host blocks also remain retained. All predecessor storage,
the same context, original service/argument cells and child blocks must stay
alive and immovable until their existing explicit quiescence contracts permit
disposition.

The new frame has the existing direct D5E600 lifetime. The second bloom's
D62150 profile is supported by `NativeRenderPassReference`, but an additional
stable service-visible companion must be installed before full-service release
can reach `+2C`. This packet does not wire that companion or the earlier seven
service-pass companions. Nested holders, surfaces and textures keep their
existing direct ownership paths; no blanket registrations are introduced.

The established bloom descriptor at `+430` borrows four DWORDs through `+43F`,
despite the native `43Ch` allocation. The existing initializer does not read
the last word. This packet preserves that exact allocation/provider boundary
and makes no claim about later consumer safety or padding capacity.

At `B11E40`, ESI is the same service, EBX `FFFFFFFF`, EBP half aligned width,
EDI half aligned height and ESP14 the raw second-bloom allocation. ESP20, ESP58,
ESP24, ESP18/1C and ESP1D8 retain their previous dimension values. Mask is zero,
state is `-1`, and original argument cells remain borrowed without reads.
Full initialization, teardown and native FH3/SEH remain unclaimed.

## Verification

`./scripts/build.ps1` passed MSVC Win32 Release `/MD` and all three existing
CTests (`reconstructed_math`, `native_math_differential`, `tool_tests`). No new
tests, native probes or app wiring were added. The call verifier passed nine
direct rows with zero failures; its one indirect row is explicitly skipped by
the mechanical verifier and supported separately by live instruction context,
slot/getter byte equality and the current-profile source guard. Emitted object
inspection confirms the x87 sequence. None of these checks establishes composed
frame/bloom runtime, full native ABI or gameplay parity.

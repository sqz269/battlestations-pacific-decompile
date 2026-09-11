# Native renderer vertex-layout binding

The complete `00B23F20..00B24002` function binds a retained hardware layout
through actual raw renderer storage and existing ownership providers. The
226-byte body contains 67 instructions and ends with `RET 4`. Its original
inputs are ECX renderer and one stack layout pointer. It has no stable semantic
EAX result. The descriptive name is a reconstruction hypothesis.

`bind_native_renderer_vertex_layout_00b23f20` is a new MSVC Win32 C++ interface.
It is not a drop-in native ABI replacement. The context borrows the actual
optional-synchronization globals, `NativeHardwareLayoutOwnerContext`, and the
immutable original-token prefix of layout profile `D62AF4`:

| Offset | Original target | Actual composed operation |
| --- | --- | --- |
| `+00` | `BD30E0` | Read the owner's current profile again, call deleting slot with flag 1 |
| `+04` | `B60770` | Complete hardware-layout destruction and actual pool return |
| `+08` | `B5FF00` | Borrow COM vertex declaration at layout `+40` |

These numeric native addresses are evidence and are never called as host
function pointers. The C++ path reads the current owner profile and borrowed
table slots at the native dispatch points, then invokes the established full
provider. Other incoming or retained-owner profiles are outside this input
domain. No callback, default return, runtime rejection, copied owner, or
semantic `d3d9_states` layout substitutes for the ownership operations.

## Storage and call order

The renderer is borrowed raw storage. Its relevant fields are the retained
layout at `+17B4`, real `IDirect3DDevice9*` at `+1A10`, and wrapping DWORD
counter at `+1BAC`. Optional synchronization uses its existing `+04` lock.

1. Compare the original input against renderer `+17B4`. Equal inputs return
   before guard entry, reference operations, device calls, and counter writes.
2. If current synchronization mode is nonzero, save renderer in the local
   guard and call actual `B33AD0`. Save only the returned low byte afterward.
3. Reload renderer `+17B4` after guard entry and capture that old owner. Arm
   EH state 0 after the capture and comparison.
4. If the captured old owner differs, publish the incoming pointer first,
   increment incoming `+04` with real `InterlockedIncrement` when nonnull,
   then decrement captured old `+04` with real `InterlockedDecrement` when
   nonnull. A zero result reads the old owner's current profile and zero slot.
   `BD30E0` rereads that profile before its flag-1 deleting-slot dispatch.
5. For the original nonnull input, capture the current device pointer, read
   incoming's current profile, capture the device's current method table, and
   select/call the incoming getter at `+08`. After the getter returns, reload
   the current device receiver. Select `SetVertexDeclaration` at `+15C` from
   the device table captured before the getter and call it with that receiver
   and the getter's borrowed declaration. Ignore HRESULT.
6. Increment current renderer `+1BAC` modulo 2^32. Capture current mode and
   disarm EH before normal guard leave. Only nonzero current mode calls leave.

A post-entry identity match skips reference updates but still performs the
nonnull getter/device call and counter increment. A null replacement releases
the old owner and increments the counter but issues no COM unbind operation.
The device call continues to use the original input even if destruction or
observers change the renderer's retained-layout field.

`B60770` composes the existing actual hardware-layout destructor. Its context
borrows current global renderer `F8D394`, established renderer profile
`D5F0A8/+44 -> B2F4C0`, actual tree `108D530`, support `108FEDC`, shared lifetime
domain `1090AA0`, CPU declaration profile `D61D1C`, type sizes `D61CC0`, CPU
declaration pool `108FD38`, and hardware pool `108FE9C`. The comparison runs
those complete compiled providers, including four CPU declaration releases,
tree removal and real initialized pool critical sections. No duplicate string,
tree, allocation or pool implementation was introduced by this packet.

## Native C++ exception map

The full ten-byte handler is `CBCF58`; its 36-byte FuncInfo is `DF558C`.
The complete eight-byte unwind map at `DF5584` contains state 0 to -1 and
action `CBCF50`. That eight-byte action computes guard storage at EBP-14h
and tail-jumps to actual `B21110`. There are no catch handlers or rollback
actions. Entry-time guard failures occur before state 0 is armed. Normal
leave occurs after it is disarmed and is never retried by EH.

An armed local cleanup object expresses this cleanup-only map without adding
a catch/rethrow frame. Its unwind path invokes actual `B21110`. A second C++
exception during cleanup terminates during exception search, through an SEH
filter for `E06D7363`, before further nested unwind can alter the terminal
state. Published layout, reference updates, and completed destruction remain
visible if a later operation throws. The counter is written only after the
nonnull device call returns or the null path reaches that instruction.

The local guard is deliberately uninitialized when entry is skipped. The
interface inherits the actual synchronization provider's current-mode and
storage domain; it adds no repair for mode becoming enabled after skipped
entry. Normal native cleanup loads four bytes from the guard's first word,
although only AL was initialized. The consumed word is ignored by `B33B00`;
the C++ path reads the defined low byte instead of indeterminate padding.

## Verification

The strict worker `bsp_core.lib` build includes the new source through a local
registration overlay. Both existing CTest checks and all eight existing
native seeds passed. No tracked tests or shared build metadata were changed.
The primary integrator owns registration, Ghidra annotations and ledgers.

The private comparison executes the full original binding, zero-reference
terminal and getter, plus complete original handler/action/map/FuncInfo. Only
declared address relocations, explicit ABI adapters to actual compiled
providers, platform intrinsic imports and the host MSVC FH3 entry are patched.
Every byte of all 19 captured native spans is checked against fresh guarded
Ghidra reads and the installed PE, then every execution postimage is verified.

| Focused comparison | Established behavior |
| --- | --- |
| Null and nonnull outer identity | No guard, reference, COM or counter work |
| Null replacement | Old decrement, no COM unbind, wrapping counter |
| Zero-reference replacement | Actual COM Release, tree removal, reverse four CPU owners, actual pool return |
| Disabled entry mode | Defined no-entry/no-leave path |
| Post-entry identity | No retain/release, but getter/device/counter still run |
| Post-entry old-owner change | Release the freshly captured old owner |
| Getter-time device change | Captured pre-getter table with current post-getter receiver |
| Returned failure HRESULT | Counter and normal leave still run |
| Setter throws | Published/refcount state remains; counter skipped; EH guard runs |
| Old COM Release throws | Actual base cleanup runs; COM field and hardware pool return remain incomplete |
| Normal leave throws | Counter already written; no second leave |
| Guard entry throws | No state-0 cleanup or layout/reference changes |
| Current mode disabled by setter | Skip leave using current mode |
| Isolated secondary cleanup exception | Matching terminal state and `uncaught_exceptions == 1` |

The 14 ordinary scenarios match 32,370 DWORDs (129,480 bytes), 139 decoded
events, 31 recorded writes, and 24 getter/COM/caught-exception records. Native
state 0 was observed in two searches, two unwinds and two guard actions.
The two isolated double-fault processes both exit 91 with identical retained
layout, counts, counter, nesting, lock depth, lock recursion and uncaught count.

The fixture uses two real D3D9 HAL devices on the observed NVIDIA GeForce
RTX 5090 driver and real vertex declarations with verified element contents.
COM observers forward actual device/declaration methods. The getter runs its
actual four-byte body; page-access observation records its real read PC and
injects the device-pointer change after that read. Observer writes are marked
separately from production writes. Every getter, COM call-return and production
store PC is checked against native or compiled instructions. Forty distinct
actual module-entry records match the corresponding 32-bit DLL/executable
bytes with only PE-loader relocations. Four complete private native postimages,
14 compiled actual providers, 22 source/header pins, the library, executable,
map, runners and raw results are pinned in the audit.

This is native-body differential evidence against the strict worker library,
not game execution, rendering parity, arbitrary-profile support or native CRT
TLS equivalence. The fixture uses the host FH3 personality and explicit C++
exceptions at observer boundaries. No game installation or saved analysis was
modified. See [the complete audit](../reports/native_renderer_vertex_layout_binding_audit.json).

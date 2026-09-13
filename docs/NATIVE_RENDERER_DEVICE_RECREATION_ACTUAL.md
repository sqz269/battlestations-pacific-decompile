# Actual renderer device recreation

Addresses: 00b29670

The complete B29670..B29B16 normal body is reconstructed in
`src/native_renderer_device_recreation_actual.cpp`. Its 1191 installed bytes
match the saved `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`.
The native entry receives the renderer in ECX, has no stack arguments and
returns without a semantic result. The descriptive name is a reconstruction
hypothesis. The new C++ interface adds a borrowed context and is not a binary
entry replacement.

## Actual dependencies and ordering

The parent composes the existing actual frame-target, binding reset, dynamic
buffer release, resource/cache release, hardware-layout tree, logical-buffer
save/restore, shader save/restore, default-state, gamma and resource-restore
providers. AW supplies the six retained 2D/cube/volume texture callbacks.
All contexts must use the same renderer publication cell, actual owner/string
services, synchronization state and raw native profile words. Current numeric
profile slots select concrete compiled providers; they are not game pointers
called directly or injected host simulation methods.

Entry acquires the optional guard, then the captured renderer+199C lifecycle
critical section and increments its tracked +18 depth. It captures the current
frame-target virtual target before clearing renderer+1D8A. It releases binding,
buffer and resource state, walks the actual checked hardware-layout tree,
then the four owner arrays and retained texture records. Tree validation may
return or throw; the current sentinel is captured before the validation call.
Each owner-array iteration reloads current unsigned count and base. The record
walk retains its old cursor but reloads count before base to compute each end.

The old device receives captured AddRef and refreshed-table Release, followed
by an unconditional release of the newly loaded current renderer+1A10. The
caps D3D pointer is captured before clearing the actual device output word.
GetDeviceCaps and CreateDevice use adapter0/HAL and ignore HRESULT. Caps
DevCaps+1C bit10000 and the low ushort shader version+C4 select flags40 when
version>=101, otherwise20; flag4 is always added. Creation receives the actual
writable presentation parameters+1A28 and device output+1A10, using a fresh
D3D pointer and current HWND+1A44.

Default states precede gamma. A small native instruction wrapper preserves
FLD renderer+196C before the current table+F0 read and FSTP into the outgoing
float slot; the full gamma provider receives that same slot. Resource restore
then precedes the inline dynamic-buffer readiness branch. The first wrapper
and device are read before ready1D8C is published; the second wrapper table is
read before its current device. Current online publication gates the existing
concrete XLive device adapter. Restoring the tree, arrays and retained textures
precedes reloading the current lifecycle lock, decrementing its depth and
leaving it.

## Texture-record producer evidence

Constructor B3256E installs manager D5F088 at renderer+1A74; its array starts at
+1A78. B319B0/B31A55 reaches B30B40, whose current slot+8 call at B30DBD selects
B2C2D0. Successful loader constructor calls B2C60D/B2C724/B2C7F7 establish the
D61948/D61870/D618B0 owners. B23640 retains backing memory into 2D+4C, cube+2C
or volume+30. B30F46 stores the result into temporary record+28; B30F4A calls
B30130 to append the 2C-byte record. B2FCEE in the full B2FC60 copy preserves
the resource pointer unchanged. The admitted profile slots+28/+2C therefore
have a producer path into the parent record walk.

Append occurs before TEST EBX at B30F4F, so failed loading can append null.
Neither the original parent nor this source adds a null guard. The supported
execution domain requires reached live nonnull owners. The full file loader
is provenance evidence here, not a newly reconstructed function claim.

## Exception and validation limits

The native handler CBD2FB..CBD304 loads FuncInfo DF5AD0 and jumps to BF6B43.
The one-state map at DF5AC8 points to CBD2F0..CBD2FA, which invokes B21110 on
the optional guard. State0 is armed before EnterCriticalSection. Exceptional
exit does not leave the lifecycle lock or roll back resource/device changes.
Normal exit reloads current mode, disarms cleanup and reads the full guard
DWORD for B33B00. A skipped guard is uninitialized; later exposing it through
a mode change is outside the valid caller domain. GetDeviceCaps output also
has no initialization beyond what the actual API writes.

Strict MSVC Win32 compilation and the combined build with both existing tests
passed. Independent complete-listing reviews covered the producer chain,
guard scope, current loads, caps flags and gamma argument stack. The exact
final commit build and composed original-parent fixture are still pending.
Leaf fixture results do not establish the complete parent. Native ABI/FS
identity, arbitrary SEH/concurrency, full Reset B2ABD0 and gameplay remain open.

Every normal call site and exact byte/profile/EH span is retained in
`reports/native_renderer_device_recreation_actual.json`; raw captures are in
`local/aw-device-recreation/`.

## AW integration analysis refresh

The integrator saved all ten AW original signatures and reviewed names,
verified their complete stored bodies and refreshed exports. The seven-byte
B3D7B0 body and two ten-byte EH handlers CBD2FB/CBD168 were defined under
owned leases and the Ghidra write lock. Missing-function observations above
describe the earlier worker capture. EH definitions are analysis metadata,
not additional reconstructed normal-body claims. Combined final-commit
validation remains separate from the worker fixture evidence.

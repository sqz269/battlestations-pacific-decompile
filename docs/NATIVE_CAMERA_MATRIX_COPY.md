# Native camera matrix copy

`copy_native_camera_matrix_004134f0` implements the complete 103-byte
004134F0..00413557 raw entry. Original ABI is ECX destination, stack source,
RET4, with EAX retaining destination. The explicit MSVC Win32 fastcall
interface includes an unused EDX argument to keep source on the stack.
The established descriptive Ghidra name is `BSP_Matrix_Copy4x4X87`.

Sixteen sequential `FLD m32` / `FSTP m32` pairs copy 64 bytes using the
ambient x87 control/status/stack. This is a floating copy: masked signaling
NaNs quiet on storage, unmasked exceptions can leave a partial destination,
and forward overlap propagates earlier writes into later reads. Raw pointer
arguments allow unaligned and overlapping byte views. There is no snapshot,
count, validation, global state, helper call or owner operation.

The existing `CameraMatrix` API remains available. This raw entry accepts
actual matrix bytes without casting them to a C++ array object. It does
not resolve the separate native-node hierarchy versus companion-pointer
boundary documented by the camera preparation discovery.

The strict main Win32 build and both existing CTests passed; eight fresh
native seed ranges matched. All 103 bytes of the full COFF section match
the fresh guarded Ghidra/installed-PE body, with zero relocations. The exact
object occurs once in the frozen actual main `bsp_core.lib`. No new test case or original-body fixture is added for
this leaf; existing checks and complete instruction identity are the chosen
verification boundary. Native caller integration and gameplay are separate.


The audit seals the actual main library/object, source/header, build log,
build-command log, original capture and full instruction comparison under
`local/camera_matrix_copy/`. The existing Ghidra name and prior comments
were preserved, reviewed evidence was appended and saved, and the export
and full function record were refreshed. No linked-runtime fixture is
claimed for this leaf.

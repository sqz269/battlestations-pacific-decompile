# Actual renderer float shader constants

`native_shader_constants.cpp` reconstructs both complete 160-byte bodies:

| Native entry | Operation | Current COM slot | Call counter | Byte counter |
| --- | --- | --- | --- | --- |
| `00B21820` | Vertex float constants | `178h` | renderer `+1BD8` | renderer `+1BE0` |
| `00B218C0` | Pixel float constants | `1B4h` | renderer `+1BDC` | renderer `+1BE4` |

The descriptive Ghidra names are retained hypotheses. Both saved-program spans
were freshly checked against the installed executable, including each `RET 0Ch`.
The earlier functions in `d3d9_states.cpp` remain semantic interfaces that expose
HRESULT and use typed state; their real-device readback evidence does not establish
the raw renderer interface implemented here.

## Storage, call order and source boundary

The new entries retain ECX renderer and the original three stack arguments:
start register, data pointer and float4 count. EDX additionally binds the borrowed
actual synchronization globals. The naked adapters pass the address of their
actual callee argument slots without copying those arguments.

Count is read once before the zero test and optional guard entry. Zero skips all
renderer/global accesses. For nonzero count, the current mode determines entry;
the guard stores renderer before calling the existing actual entry provider and
stores its returned low byte afterward. Data is then loaded from its original
argument slot, followed by current device `+1A10` and its current table. The
captured count and data are pushed before the live start-register load. Device
and current entry follow. Cleanup arms after all four outgoing pushes.

The device call's HRESULT is ignored. On normal return, the current call counter
is incremented, the captured count is shifted left four bits, and the result is
added to the current byte counter. Both arithmetic operations wrap as native
DWORD instructions. No overflow rejection or cached counter snapshot is added.
The current mode is read before cleanup disarms; enabled normal leave uses the
saved renderer and the same actual synchronization provider.

Source C++ unwind calls the actual `00B21110` guard cleanup only while armed.
A second C++ exception during cleanup terminates. The source zero-extends the
saved entry byte instead of forwarding native guard padding; the actual leave
provider ignores that argument. No private FH3 frame, nonlocal register,
hardware-fault/SEH, or original caller ABI equivalence is claimed. As in the
existing actual synchronization contract, entry skipped with an uninitialized
record must not be followed by a transition that enables cleanup.

The two original ten-byte FH3 handlers, their 36-byte FuncInfo records, single
eight-byte unwind entries and eight-byte actions were also freshly pinned
(124 bytes). Both actions form native guard address `EBP-14h` and tail-call
`00B21110`. This establishes the cleanup target and one-state map; it does not
equate the source frame layout or execute the original exception machinery.

## Verification

The accepted primary build is frozen in
`local/shader_constants_actual5/build_frozen_v2/`: 13 unchanged input files,
two actual compiler commands with `/W4 /WX /fp:strict`, the library members and
132 compiler dependencies. Both existing CTests passed; all eight native seeds
matched. The initial failed build and its input snapshot are preserved: another
integration changed `cmake/startup.cmake` during compilation and triggered
concurrent CMake regeneration. That capture is not accepted evidence.

The emitted 254-byte source body for each entry was inspected with its 18-byte
adapter, actual guard-provider relocations, source unwind metadata, and complete
native upload/counter instruction mapping. Stack/context and cleanup-state
translations are identified separately from identical instruction bytes.

One local, manifested Win32 probe compares each captured native body with the
actual reconstructed provider linked from the frozen primary library. Only two
absolute mode operands per native body are relocated; native mode remains zero.
A COM ABI capture device verifies argument identity, ignored `E_FAIL`, current
post-call counter reads, DWORD wrapping and the zero-count null-renderer skip.
Both stages also exercise a source-only device C++ exception with the actual
mode-one, null-permitted-lock guard: post-call increments are skipped and nesting
returns to zero. No persistent tests were added.

This probe does not execute native guard/FH3 exception paths or real D3D shader
uploads. It does not wire these entries into a complete raw renderer or establish
gameplay validation. The evidence, artifact hashes and precise fixture limits
are in `reports/native_shader_constants.json`.

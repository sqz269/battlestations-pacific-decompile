# Actual renderer device-format query

`check_native_renderer_device_format_00b21ec0` reconstructs the complete
75-byte body at `00B21EC0..00B21F0A` (end exclusive `00B21F0B`). It borrows
the raw renderer and invokes its current `IDirect3D9::CheckDeviceFormat`
provider. The descriptive Ghidra name `BSP_D3D9Renderer_CheckDeviceFormat`
is retained as a hypothesis, not a recovered symbol.

The native ABI is ECX = renderer, then four stack DWORDs: adapter format,
engine resource flags, resource kind, and check format. `RET 10h` removes
those arguments; AL returns the boolean. The new ordinary MSVC Win32 C++
interface does not preserve the original caller ABI or incidental registers.

## Complete instruction path

| Native interval | Established behavior |
| --- | --- |
| `B21EC0..B21ED5` | Capture flags; save ESI/EDI; retain the original resource kind in EDI. EDX and ECX point into the private flags and kind argument slots. |
| `B21ED6..B21EDA` | Call complete raw `00B20A80` with usage output in ECX, pool output in EDX, and captured flags/kind as stack inputs. |
| `B21EDB..B21EFA` | Capture check format, load current renderer +1990h and its current table, prepare `(factory, 0, 1, adapter_format, translated_usage, original_kind, check_format)`, and load method +28h. |
| `B21EFB..B21F0A` | Invoke the COM stdcall method, test signed HRESULT, return true for every nonnegative result, restore EDI/ESI, and `RET 10h`. |

The pseudocode's repeated `param_4` arguments are misleading: the actual
usage output overwrites a private native argument slot, while EDI retains
the original kind. The translated pool value is not consumed. The source
calls the existing full `translate_native_resource_creation_flags_00b20a80`
provider and uses separate private cells for the two outputs. Its subsequent
volatile DWORD loads preserve the current factory, table, and method reads.
It adds no renderer writes, null checks, format filtering, or normalization.

All reached raw DWORD cells must be readable and four-byte aligned; the
renderer reaches byte +1993h. The factory/table/method must satisfy the
actual COM contract. Private C++ and native stack aliases, precise fault
behavior, asynchronous mutation, and original unwind mechanics are outside
the source interface.

## Evidence and validation

The worker queried the existing `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe` through identity-checking `bsp.py ghidra` calls.
All 75 bytes match the installed executable. Their SHA-256 is
`9b2ccaf50ee22ad1d6492c4d632e345a808c1046e2c27774748e83501c78b881`.
The full installed binary SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
No worker Ghidra mutation is made; annotation and shared ledger integration
belong to the primary agent.

The local artifact directory is
`local/renderer_format_actual5/` in worktree
`battlestations-pacific-decompile-renderer-format-actual5`. The machine-readable
report records build, fixture, source, library, object, and artifact pins.

The focused local probe executes all 75 captured wrapper bytes with exactly
one four-byte call displacement relocated to a fixed ABI bridge for the
same actual source `00B20A80` provider used by the reconstruction. It does
not rerun original translator machine code; that provider's independent
original-code evidence remains in `reports/native_resource_creation_flags_audit.json`.
The probe compares all seven COM arguments, method selection, boolean
result, and unchanged renderer bytes across eight capture rows. These
include nonzero positive success codes, negative HRESULTs, a default-pool
buffer usage adjustment, reserved pool nibble, and replaced current factory
publication/table. It also compares the native wrapper, source wrapper,
and direct local Direct3D9 queries for the `DF16` and `D16` depth-stencil
texture formats.

The strict Release Win32 build passed both existing CTests, and all eight
native seed spans matched. All 1,723 pinned repository build inputs remained
unchanged during the build. The probe passed all eight capture rows and both
actual Direct3D9 rows. On this machine, the three actual-provider paths all
rejected `DF16` with `8876086Ah` and accepted `D16` with HRESULT zero. The
compiled source function is 77 bytes, with its sole direct-call relocation
bound to the complete flags provider in the same built library.

These checks establish the bounded source implementation and COM-query
behavior. They do not establish full renderer startup integration, image
rendering, original binary replacement compatibility, or gameplay parity.

# Camera configuration leaves

This packet reconstructs `00B6FC30`, `00B6FDF0`, `00B6FE10` and `00B6FEC0`
through the existing `CameraProjection` and `CameraFrameState` views. The
interfaces borrow the actual camera fields; they allocate no camera, add no
cache, and do not initialize the supplied fields. The names below describe
observed behavior and are not recovered original symbols.

| Address | Proposed descriptive name | Original ABI and observed result |
| --- | --- | --- |
| `00B6FC30` | `BSP_Camera_GetFovAspectProduct` | ECX camera, ST0 float32-rounded product, RET0 |
| `00B6FDF0` | `BSP_Camera_SetRenderModeAndMask` | ECX camera, stack DWORD mode, EAX same camera, RET4 |
| `00B6FE10` | `BSP_Camera_SetClearFlags` | ECX camera, stack DWORD flags, EAX same flags, RET4 |
| `00B6FEC0` | `BSP_Camera_SetContextDepthScalePointer` | ECX camera, stack pointer, EAX same pointer, RET4 |

## Arithmetic and storage contract

`00B6FC30` loads aspect from `+1C8` with FLD, multiplies by the live fov at
`+1C4` with FMUL, stores the result to a float32 stack slot, reloads that slot
to ST0, and returns. The private reconstructed kernel retains those four x87
operations in that order; its two field-address arguments replace only the
native camera-relative addressing. The public function receives the existing
projection view and passes its actual backing addresses. There is no field
copy, validation, tangent calculation, normalization, or cache refresh. Calling
this a physical horizontal FOV would require evidence beyond this product.

`00B6FDF0` computes `1u << (mode & 31u)`, writes the entire original mode DWORD
at `+198`, then writes the mask at `+19C`. Values above 31 are not rejected or
clamped: 32 produces mask 1; `FFFFFFFF` produces `80000000`; `80000002`
remains the stored mode while producing mask 4. The caller must supply the
same camera's actual `+19C` word alongside the existing frame view. The C++
return is the same `CameraFrameState&`, projecting native EAX same-owner
identity at the typed boundary. It is not a raw native pointer return ABI.

`00B6FE10` stores and returns the complete flags DWORD at `+188`.
`00B6FEC0` stores and returns the supplied pointer at `+43C`, without reading
its pointee or changing a reference count. The pointer remains borrowed and
must be valid when a later camera operation actually uses it. The leaf itself
also accepts null. Neither setter changes camera validity flags, other render
state, or a global owner.

## Evidence and verification

All four native instruction extents were checked through their final RET:
`[00B6FC30,00B6FC45)` is 21 bytes, `[00B6FDF0,00B6FE0C)` is 28 bytes,
`[00B6FE10,00B6FE1D)` and `[00B6FEC0,00B6FECD)` are 13 bytes each.
All 75 bytes match live Ghidra and the installed executable. Each live query
verifies `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
There are no dependency calls, absolute data operands, or required address
relocations. Full hashes and decoded instructions are in
`reports/camera_configuration_audit.json`.

The isolated Win32 native comparison executes those original instructions
from a private page, with no hooks or replacement functions. One sequence
compares five mode values, two clear-flag values, three pointer values, and
five product inputs against the reconstructed functions. Existing views bind
directly to the fields of a raw `0x45C` camera slot with 16-byte canaries on
each side. View construction is checked to leave every raw camera byte
untouched. Each checkpoint compares the entire slot and canaries, returned
bits/identity, x87 status and control word, and MXCSR. An intentionally
unmapped pointer value is stored without being dereferenced.

Product inputs include finite inexact multiplication, negative zero,
signaling NaN, float32 overflow, and subnormal underflow. The environment is
x87 control `027F` and MXCSR `1F80`, with exception flags cleared before each
call. Fifteen checkpoints compare 292 words each: 4,380 matching words.
The local fixture and source pass MSVC Win32 C++20 `/W4 /WX /fp:strict`;
the existing repository build and CTest also pass. Object disassembly was
inspected for the explicit FLD/FMUL/FSTP32/FLD32 sequence, actual field-address
loads, ordered stores, and typed identity return.

This is reconstructed and build/fixture-tested C++, not a drop-in native ABI
replacement or game validation. Other floating-point control modes, unmasked
exceptions, concurrent mutation, and invalid field bindings are outside the
executed fixture. The primary integrator owns CMake registration, Ghidra
annotations, exports, ledgers, and composition into the native camera owner.

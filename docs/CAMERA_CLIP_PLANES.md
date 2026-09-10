# Camera extra clip-plane add/remove closure

The four routines below recover the camera's extra plane at `+2E0`, its
existing plane set at `+2F4`, and bit `0x40` in its existing flags at `+2F0`.
Adding always resets the plane count to six and writes record six with flags
three. Repeated additions replace that record. Clearing resets the count to
six and clears bit `0x40`; it leaves every coefficient and record flag intact.

`CameraClipPlaneState` borrows the actual `CameraPlane&` at `+2E0` and the
same `CameraFrameState&` used by the camera frame path. It accesses the existing
`frame.frustum` and `frame.camera.projection.valid_flags` references. Constructing
this aggregate creates no independent state and performs no reads or writes.
The owner and its stable views must outlive every call. Binding those references
to a complete native camera owner remains the primary integration's work.

## Addresses and original ABI

| Address | Complete bytes | Original inputs and stack cleanup | Proposed name |
| --- | ---: | --- | --- |
| `00B70410` | 82, through `00B70461` | thiscall: ECX camera; stack plane pointer; RET4 | `BSP_Camera_SetExtraClipPlane` |
| `00B70470` | 25, through `00B70488` | ECX camera; no stack inputs; RET | `BSP_Camera_ClearExtraClipPlane` |
| `00B65070` | 13, through `00B6507C` | thiscall: ECX plane set; stack count DWORD; RET4 | `BSP_PlaneSet_SetCount` |
| `00B65990` | 57, through `00B659C8` | thiscall: ECX plane set; stack plane pointer, flags DWORD; RET8 | `BSP_PlaneSet_Append` |

These names are descriptive reconstruction hypotheses. The new C++ functions
have typed interfaces and no binary replacement ABI claim. The saved Ghidra
pseudocode expresses coefficient copies as DWORD assignments. Assembly proves
they are float32 x87 `FLD`/`FSTP` pairs, which is a material distinction for NaNs,
denormals, x87 status, and overlapping inputs.

## Established behavior

`00B70410` reads and stores X, Y, Z, W in forward order into camera `+2E0`.
Only after those four pairs does it set `plane_set.count` to six, then call
`00B65990` with the original source pointer and flags three. It sets bit `0x40`
last. There is no early four-coefficient snapshot and no substitution of the
already stored extra plane for the original source on the second copy.

`00B65990` first reads the old DWORD count at plane-set `+0x140`, increments
it modulo 2^32, and writes the new count. It computes the destination from the
old count with 32-bit `old_count * 20` address arithmetic. It then performs four
forward x87 coefficient pairs and writes the supplied flags DWORD last. The
source may observe the already incremented count or an earlier destination
store when memory overlaps. The fourth coefficient remains in ST0 while the
flags argument is loaded, before its final `FSTP` and the flags store.

`00B65070` stores every count bit without checking, clamping, clearing records,
or changing floating-point state. The typed append contract requires count less
than 16 and sixteen readable source bytes. Its code adds no new error branch.
Native count 16 addresses the count word as the next record and can overwrite
the containing object's tail; larger counts can wrap their destination address.
Those out-of-storage writes are established from assembly and are outside the
typed object's supported storage contract. The camera wrapper always resets to
six before appending, so its index-six path is fully within the set.

`00B70470` sets count six before clearing camera bit `0x40`. Neither camera
routine changes frustum-valid bit `0x4`, projection-valid bits, the first six
records, or unrelated flags. A later frustum refresh is a separate operation;
this packet does not implement or modify `00B658E0`.

## Evidence and verification

The four complete code spans total 177 bytes. Every span was read through the
guarded `tools/bsp.py ghidra bytes` command and matched the installed executable.
The CLI verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` before
each live batch. The binary SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The packet makes no Ghidra mutation; shared annotation, name ledger, and CMake
integration remain with the primary agent.

One local differential fixture executes just those four native spans in their
original relative positions. Every other reserved page is inaccessible, unused
bytes in committed pages are INT3, and the committed pages become execute/read
before calls. There are no original data spans, absolute relocations, imports,
entry-point execution, full-image loading, external hooks, or unresolved stubs.

The host side links the actual built `bsp_core` and compiles the new source with
MSVC x86, C++17, `/O2 /fp:strict /W4 /WX /MD`. Both paths operate on a 1,112-byte
camera field object with verified offsets and actual borrowed transform,
projection, camera, frame, and extra-plane views. Each run starts with an `A5`
preimage; view construction is checked for no raw-state modification.

One ten-phase lifecycle runs at x87 control words `027F`, `067F`, `0A7F`, and
`0E7F`: external exceptional coefficients; clear; forward overlapping add;
raw count `FFFFFFFF`; count eight; append with the count word as source; camera
add with the count word as source; clear; count fifteen; last valid append.
This yields 40 matching complete-state comparisons, or 44,480 bytes per path.
Each phase also matches the low 15 x87 status bits including TOP and condition
bits, preserves its control word, and preserves MXCSR `1F80`.

The initial source includes signaling NaN, quiet NaN, negative zero, and a
subnormal. The signaling payload becomes `7FE12345` through x87 while the
external source stays unchanged. A source at `extra-4` propagates the quieted
first value through the four stores. A direct append from the count word copies
the incremented nine; camera add from that same source stores nine in `+2E0`
but copies seven into record six after the reset/increment sequence. Clear
retains both extra-plane bytes and the hidden record-six bytes and flags.

All eight existing native seeds matched the installed executable, and
`scripts/build.ps1` passed the existing two CTest checks. The new source is
explicitly compiled by the focused fixture; registration in CMake is intentionally
left to primary integration. Local commands, logs, exact spans, and artifact
hashes are recorded in `reports/camera_clip_planes_audit.json`.

This establishes exported, reconstructed, build-tested, and focused native
differential behavior. It does not establish unmasked x87 trap ordering,
concurrent mutation, invalid storage access, every possible alias pattern,
complete native camera lifetime, shadow-cascade rendering, or game validation.

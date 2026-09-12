# Interface sound listener

Addresses: 0068A670. Read-only dependencies: 004134F0, 00B6DB70, 00BF7030;
callers 004BBD00, 004CD610, 00735B50, 008CC850.

| Routine | Coverage | Original ABI | Boundary |
| --- | --- | --- | --- |
| `0068A670` | complete control flow, memory ordering and x87/SSE kernel under the explicit raw-storage/virtual-dispatch contract | ECX interface; stack matrix64 output then velocity12 output; RET8; no semantic return value | final RET8 at 0068A89D, length3, end exclusive 0068A8A0 |

`BSP_Interface_GetSoundListener` is a descriptive hypothesis. The C++ entry
adds a required context; it is not an ABI-compatible replacement. Ghidra's
original void/no-argument prototype and inferred output-variable reuse are
misleading. The listing resolves matrix at incoming ESP+4 and velocity at +8;
the target branch later reuses those stack argument slots as float temporaries.

The packet changes no application hosts and does not claim game validation.
Ghidra remained read only; every live CLI batch used `Client.verify()` for
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

## Storage and ownership

The supplied interface is actual storage; only its applied interface id DWORD
at +4 is read. `00684E33` initializes that field in the existing base constructor.
The semantic `InGameInterfaceManager.base.applied.interface_id` documents the
same meaning but its owning C++ object is not raw interface storage.

`game_00e188a8` publishes actual game storage with pointer words +1ED4 and +19FC.
The target at +1ED4 stays opaque: no particular unit or camera class is inferred.
The camera at +19FC uses the existing actual node/camera hierarchy, byte flags
+5C and world matrix +F0. `refresh_native_camera_world_00b6db70` already consumes
those canonical parent/local/world fields; there is no companion transform copy
or additional camera resolver in this packet.

The context borrows all publications, constants, CRT access and calls. Its
fallback pointer addresses three consecutive live float words. Native initial
bytes are CE3D64=`00 40 1C 46` (10000.0f), D7A218=`00 00 00 00`,
D7A220=`00 00 00 00 00 00 59 40` (100.0), D7A24C=`00 00 80 3F`, and
F87574..F8757F all zero. Constants are not copied into a second state model.

## Ordered behavior

1. Capture current game once. If its +1ED4 is nonnull, reload that field and call
   its current virtual +120 with an **uninitialized** 64-byte local buffer. Copy
   from the returned pointer, which need not be the buffer.
2. Otherwise test and capture the same game's +19FC camera. If its low +5C byte
   lacks bit2, call the existing raw world refresh. Copy that captured camera's
   +F0, even if a service could subsequently change publication. Only when both
   fields are null does the native routine build identity using one D7A24C load.
3. Copy all sixteen matrix words through the existing sequential x87 004134F0
   provider. Its overlap, signaling-NaN and floating-state behavior is retained.
4. Reload current E188D8 after the matrix callback/copy. If nonnull, read the
   supplied interface's current +4 once. Except for ids 29,2B,2C,2D,34, call
   that captured controlled object's current virtual +34 with an uninitialized
   12-byte local and copy the returned three floats sequentially through x87.
   This branch returns without applying the target-only arithmetic.
5. Otherwise reload **current game**, test and reload its +1ED4, and call that
   current target's +34. Do not reuse the initial matrix target or game. If no
   target exists, copy live F87574/78/7C with sequential MOVSS operations.

In the target branch, x/y are first copied through x87, z through MOVSS, and
the destination x/y are spilled in the native order. The x87 kernel computes
`float((x*x + y*y) + z*z)` and compares it with CE3D64. Unordered and <= yield
integer0; greater yields1. **CVTSI2SS converts that integer and overwrites the
squared-length scratch word.** UCOMISS/LAHF/TEST AH,44/JNP determines whether
to skip the remainder. With the native zero comparison constant, only result1
proceeds: BF7030 receives ST0=1.0, its result is spilled to float, and each
component is multiplied by `float(100.0 / float(sqrt(1.0)))`. Thus velocity
`(3,4,200)` becomes `(300,400,20000)`, not a length-normalized vector. The
reassembled kernel retains comparison NaN behavior, x87 precision/spills and
live constants, using the already recovered CRT sqrt and required CRT handler.

There is no native EH frame or cleanup in this function. A dispatched exception
propagates; already completed matrix/velocity writes are not rolled back.

## Calls and remaining bindings

| Site | Callee/slot | Contract |
| --- | --- | --- |
| 0068A69D | captured target virtual+120 | ECX captured object, stack scratch64; returned EAX is copied as matrix source |
| 0068A6BB | 00B6DB70 | ECX captured actual camera, RET; concrete raw world refresh reused |
| 0068A73A | 004134F0 | ECX output, stack source, RET4; concrete sequential x87 matrix copy reused |
| 0068A76F | captured controlled virtual+34 | ECX object, stack scratch12; returned EAX supplies velocity |
| 0068A7AF | current target virtual+34 | same register/stack/returned-pointer contract |
| 0068A830 | 00BF7030 | ST0 operand/result; concrete recovered CRT entry reused |

The virtual host deliberately names **slots**, not invented getter algorithms.
The application's actual dynamic types and current vtable dispatch must supply
these entries, including any callbacks and returned-pointer lifetime. A sampled
MDestroyer vtable CFC3D0 has slot34=812090 (body-axis speed times basis) but
slot120=819840 is setter-shaped; it is not justification for substituting that
class as the +1ED4 target. There is no null-success dispatch or identity fallback
for an unsupported live object. Dynamic override reconstruction and actual game/
interface publication are remaining application bindings, not new private state.

All four known callers pass matrix then velocity pointers and rely on RET8:
735BE2 (conditional interface arm), 4BBD8F (world-view job), 4CD680 (five-iteration
transition service loop), and 8CC977 (load service path). The subsequent sound
update consumes a matrix pointer and by-value vec3; its timing belongs to the
sound runtime's shared clock, not to this provider.

Validation is recorded in `reports/interface_sound_listener.json`: existing
Win32 build/tests, seed verification, live direct-CALL audit and one ignored
local probe covering callback publication/id changes, non-scratch return
pointers, native boolean scaling, actual camera refresh and fallback bit copies.
No new permanent test suite, original-executable differential result or audible
gameplay result is claimed.

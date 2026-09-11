# Camera position mode dispatch

`src/camera_position_modes.cpp` reconstructs the complete bounded dispatch at
`007954A0`, its two angle-producing consumers, the small clamped inverse-sine
helper, and the matrix-angle output wrapper. Camera, target, theta and rho are
descriptive hypotheses, not recovered symbols. The caller at `007972F0` updates
the path parameter and selects/blends adjacent records; that caller is outside
this packet. This is a borrowed C++ interface, not a replacement native object.

| Address | Native ABI | Complete body |
| --- | --- | --- |
| `007954A0` | ECX camera; stack unsigned index; RET4; no useful return | 422 bytes, 139 instructions; final `00795643` RET4, 3 bytes, end `00795645` |
| `00794070` | ECX camera; three by-value stack floats; RET0Ch | 185 bytes, 53 instructions; final `00794126` RET0Ch, 3 bytes, end `00794128` |
| `00794130` | ECX camera; stack actual target owner and float scale; RET8 | 558 bytes, 152 instructions; final `0079435B` RET8, 3 bytes, end `0079435D` |
| `0042CF10` | stack float; RET4; binary32 result in ST0 | 143 bytes, 37 instructions; final `0042CF9C` RET4, 3 bytes, end `0042CF9E` |
| `006E47A0` | ECX actual matrix; EDX output XYZ; EAX same output; RET | 22 bytes, 11 instructions; final `006E47B5` RET, 1 byte |

All five starts already existed in Ghidra. There are no missing starts or flow
gaps in these bodies. Read-only batches verified the configured `bsp` project at
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. Each complete body
matches the installed executable bytes; hashes are in the report. The original
installation and saved analysis were not changed by this worker.

## Record dispatch and identity

The actual begin/end fields at camera `+47C/+480` contain pointers to records.
A null begin or unsigned index outside the count reaches the genuine CRT
`00BF6713` failure boundary. The C++ view requires a valid contiguous pointer
range when begin is nonnull. It does not model malformed native pointer ranges.
The selected record identity is captured once, and its `+48` kind selects:

| Kind | Observed action |
| --- | --- |
| 0 | Capture `+468` float, call `007B04C0` on actual `+464` path owner with actual writable `+394` position, disjoint output XYZ and flags 0. Read position plus output in Y/Z/X order and call `00794070`. |
| 1 | Call `00794130` with actual record `+24` target and verified `D7A260` float `-1`. |
| 2 | Float-copy record `+28/+2C/+30`, then call `00794070`. |
| 3 | Capture actual record `+14` parent, refresh its canonical pose only when its actual C8 byte is zero, transform the record's live `+28` vector by that same pose's `+CC` matrix through canonical `00414CD0`, then call `00794070`. |
| 4 | Sequential x87 copies from record `+34/+38` to camera `+3C4/+3C8`. |
| 5 | Resolve actual record `+14` parent, borrow canonical `0042D7E0` world matrix, call `006E47A0`, then read record `+34` and store its sum with output X, followed by record `+38` and output Y. |
| other | Return without field changes. |

`CameraPositionHost` requires actual implementations for `007B04C0` and
`0042D2E0`; neither has a no-op/default implementation. `006E47A0` forwards the
actual matrix and three adjacent output references to `0042D2E0`, and returns
the same output pointer. No identity or geometry is manufactured to close those
dependencies. Pure resolver methods bind existing fields and the canonical
`PoseRefreshView`; there are no shadow matrices, hierarchy snapshots or retained
owners. Native pointer, matrix and record lifetimes remain caller obligations.

## Angle helpers and rounding

`00794070` stores each target-minus-camera component as binary32, calls the
canonical `camera_vector_length_00419440` with explicit actual CRT access, then
stores `1/length` only for ordered positive lengths; zero, negative and unordered
lengths select positive zero. It separately stores each normalized component.
It calls CRT `_CIatan2` with ST0=Z and ST1=X, float-rounds the result, negates it
into `+3C8`, then passes normalized Y to `0042CF10` and stores `+3C4`. Rho is
visible before the inverse-sine dependency executes.

`00794130` replaces an ordered-negative scale with actual configuration
`+4D0 -> +54`; NaN does not select the fallback. After refreshing the actual
target pose, it computes world-translation-minus-camera deltas. Its inline
distance sequence at `00794196..007941E4` keeps the squared components in x87
until the sum's single float store, unlike `00419440`. Ordered sum greater than
double `1e-10` calls genuine CRT sqrt; below, equal or unordered produces zero.

The target's world-valid byte is tested again at `007941E4`. The separate test
at `007941F4` captures the condition subsequently used at `00794277`: basis
scaling and division lie between those instructions without overwriting CPU
flags. The implementation retains that captured condition. It multiplies world
row `+EC/+F0/+F4` by distance, stores XYZ, multiplies by scale, stores XYZ, then
captures camera `+460`, divides each component and stores XYZ. It conditionally
refreshes using the captured flag, adds live world translation into another
float vector, subtracts live camera position into a float vector, and uses the
same normalization and angle-store sequence as `00794070`.

`0042CF10` uses ordered clamps beyond float `+1/-1`, returning float
`+/-1.57079637050628662109375`. Interior and unordered values retain the observed
staging: save input as double; float-store input squared; float-store one minus
that square; CRT sqrt and float store; divide saved double input by the extended
sum of sqrt and double one; float store; CRT atan and float store; double that
result and float store. BF8490's body reaches `FLD1; FPATAN` at BF84FD and the
`atan` descriptor at E154F0. The code links genuine current host CRT intrinsic
entries; it does not reconstruct CRT dispatch globals, diagnostics or errno.

## Validation and limits

The existing `scripts/build.ps1` Release Win32 build and both existing CTests
passed. The new source independently compiled with `/O2 /W4 /WX /fp:strict`.
Its CMake registration is reserved for the parent integrator because another
packet owns `cmake/startup.cmake`; the existing CMake build therefore does not
yet include this new translation unit.

An ignored, manifest-embedded fixture linked the new object with `bsp_core.lib`
and compared against isolated, disk-matched copies of the five native bodies.
It never loaded the game or called its entry point. Native snippet callees were
relocated to the same canonical pose/length/affine dependencies, genuine host
CRT entries, and explicit fixture implementations of the unresolved boundaries.
At x87 precision controls `007F`, `027F`, and `037F`, 33 inverse-sine, 12 point,
12 target, and 24 dispatch comparisons passed with identical result bits and
pose flags/matrices. This covered all six modes, unknown kinds, zero direction,
negative-scale configuration fallback, path mutation of actual camera position,
and a matrix dependency mutating record angles and the pointer-array begin field
before the caller's stores. A separate invalid-index check verified the required
failure boundary and preserved angle fields. No permanent tests were added.

The shared dependency adapters make this evidence a check of the recovered
consumer arithmetic, control and capture order; they do not independently prove
the shared pose/length/affine kernels or the unresolved sampler/decomposition.
Floating exceptions, original CRT policy, unmasked traps, exhaustive NaN payloads,
malformed pointers, asynchronous mutation and unusual partial aliasing remain
unvalidated. Original object/SEH ABI and full camera/path ownership are absent.
No gameplay, renderer or camera-motion validation is claimed. Ghidra annotation,
save and post-annotation export are parent integration work.

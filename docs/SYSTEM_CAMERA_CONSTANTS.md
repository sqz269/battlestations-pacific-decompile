# System camera constants

`pack_system_camera_constants_00b46a84` reconstructs the bounded interior
`00B46A84..00B46C4F` of `00B46A70`. It patches an explicitly initialized prefix
through a borrowed `float*` and capacity. It neither creates a camera cache nor
clears native-unwritten words. This is a C++ companion interface, not an original
function boundary, memory layout, constructor, or drop-in binary replacement.

## Inputs and write order

The caller supplies the actual `CameraFrameState` associated with its existing
`CameraState`. The service projection borrows the actual matrix at `+1D8` of the
service captured from global `00F8D39C` at `00B46A7B`. It has no owned matrix or
default owner. `get_system_camera_service_matrix_00b0d100` returns that field's
identity: the native helper is exactly `LEA EAX,[ECX+1D8]; RET`.

The sixteen global inputs are borrowed through a `const float*` pointing to the
actual consecutive words at `0108FC30..0108FC6C`. They are read after all camera
calls, without an early snapshot, transpose, inferred default, or global-owner
substitute. The pointer's element type describes storage; copies preserve bits.

| Native calls / stores | Destination | Operation |
| --- | --- | --- |
| `00B46A88`, `00B46A92` | `c0..3` | Captured service `+1D8`, sequential x87 transpose |
| `00B46A99..00B46ACB` | `c6.xyz` | Test world-valid bit `2`, refresh if clear, snapshot world words `12..14` with three `MOVSS` loads before stores |
| `00B46AD1`, `00B46ADE` | `c7..10` | Existing view getter, sequential x87 transpose |
| `00B46AE3..00B46AFD` | `c11..14` | Test world-valid bit `2` again, refresh if clear, transpose actual world matrix |
| `00B46B04`, `00B46B11` | `c15..18` | Existing view-projection getter, sequential x87 transpose |
| `00B46B18`, `00B46B25` | `c23..26` | Existing projection getter, sequential x87 transpose |
| `00B46B2C`, `00B46B39` | `c19..22` | Existing inverse view-projection getter on the same frame companion, sequential x87 transpose |
| `00B46B3E..00B46C47` | `c27..30` | Sixteen live global words, one raw `MOVSS` load/store pair per word |

The six transposes reuse `write_system_matrix_00b404a0`, preserving destination
order and caller x87 state, including signaling-NaN quieting. Eye/global copies
use explicit SSE moves, retaining their raw NaN payloads and signed-zero bits.
Projection writes precede inverse view-projection writes despite register order.
The implementation calls the existing world, view, projection, view-projection,
and inverse view-projection helpers against the supplied objects directly.

This fragment writes only `c0..3`, `c6.xyz`, and `c7..30`. Every other supplied
word remains its preimage, including `c4..5`, `c6.w`, and `c31` onward. Capacity
of 124 words suffices for this fragment; a complete native prefix has 308 words.

## Host boundary and remaining ownership

Capacity is checked for each complete matrix or eye group after its preceding
getter/refresh, and separately for each global word. Failure returns a new host
error and preserves preceding writes and cache changes; the rejected group is
untouched. In particular, a capacity of 92 words rejects `c23..26` before the
inverse view-projection getter, even though `c19..22` would fit. Native code has
no such bounds checks. Output must not overlap camera, service, or global input
storage; all borrowed pointers must remain alive and valid for their stated
capacities. The API introduces no synchronization for concurrent edits.

The native parent receives optional scene in `ECX`, camera in `EDX`, no stack
arguments, and returns with plain `RET`. At this interior, `EDI` retains the
camera, `ECX` initially retains the captured service, `EBP` retains the scene,
and the local prefix starts at post-prologue `ESP+14`. The scene save at
`00B46A84` and camera-register preparation at `00B46C45` are native stack/register
scaffolding, not standalone C++ side effects. The integrator owns the parent
prelude, continuation, caller bindings, and upload composition.

Service construction/lifetime and its native-layout adapter remain unresolved.
`CameraFrameState` remains the existing typed companion rather than a recovered
native camera constructor. The sixteen global words must be bound to live state;
no default values or initialization provenance are claimed. Axis getters, timers,
fog, lighting, shadow, final uploads, and game validation are outside this packet.

## Evidence and validation

All live native queries used `tools/bsp.py ghidra`, which verifies project `bsp`,
program `/battlestationspacific.exe`, x86 language, and image base `00400000`.
`C:/Users/sqz269/bsp.gpr` was present. Assembly was required because decompilation
has overlapping globals and hides the distinction between raw SSE and x87 writes.
The audit records 17 ranges totaling 4,850 bytes, each compared with the installed
PE and hashed. No Ghidra annotations or shared ledgers were changed; the audit
contains annotation preimages and proposals for integrator review.

The new translation unit compiled for MSVC Win32 with `/W4 /WX /fp:strict`, C++17,
and optimization enabled. One local native-fragment probe reused the existing
core library and mapped the installed PE without running its entry point or
resolving imports. It relocated audited absolute operands and replaced only the
fixture's continuation byte at `00B46C50` with `RET`; original files and Ghidra
were untouched.

The probe compared the bounded original fragment with this implementation in
two input states: valid caches containing signaling NaNs/signed zeros, and a
cold parentless camera with finite projection parameters. Both matched all 308
prefix words, six cache matrices, both cache flag fields, and x87 exception bits.
The complete-prefix comparison also verified preservation of every unwritten
word. The same focused probe checked capacity-92 failure preserves prior writes
and leaves the inverse view-projection cache invalid. No tracked test suite or
CMake changes were added. Full integrated build remains the integrator's check;
these results establish bounded fixture agreement, not native ABI or gameplay
validation. See `reports/system_camera_constants_audit.json` for hashes and exact
validation observations.

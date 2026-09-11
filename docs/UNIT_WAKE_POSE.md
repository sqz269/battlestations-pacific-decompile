# Unit wake and prop-wash pose reads

This bounded reconstruction covers `00825A64..00825AF0` and
`00825BDD..00825BF5` within `008255B0`. The wake and prop-wash calculations now
borrow the unit's canonical `PoseRefreshView` and call the recovered
`refresh_pose_00414db0`. `UnitInstanceState` no longer owns copies of the valid
byte or two world-matrix scalars, and `UnitInstanceHost` no longer supplies an
arbitrary pose-refresh implementation.

## Evidence and owner identity

The evidence is the saved `008255B0` assembly and a fresh read-only Ghidra
disassembly of its wake branch on 2026-09-11. The wrapper verified the existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, before the batch.
The final live function count was 63100 against snapshot 63097; the inspected
branch still matched the exported instructions. This packet did not mutate
Ghidra or refresh the shared snapshot.

The original routine receives `this` in ECX and a float on the stack.
`008255CB` copies ECX into ESI; `00825DD2` returns with `RET 4`. Every refresh
call in this branch sets ECX to that same ESI. It is the unit's pose, distinct
from the scene node reached through `unit+4A4h` in other branches.

| Native storage | Required borrowed field |
| --- | --- |
| unit+3Ch | `pose.parent_3c`, the actual mutable parent slot |
| unit+74h | `pose.local_74` |
| unit+C8h | `pose.world_valid_c8`, an actual byte |
| unit+CCh | `pose.world_cc`, 16 floats |
| unit+F0h | `pose.world_cc[9]`, the basis component used by the wake calculation |
| unit+100h | `pose.world_cc[13]`, the world translation Y |
| unit+10Ch | `pose.derived_valid_10c` |

`UnitInstanceState::pose` is a required reference to an already-bound view of
that same unit. The view and all of its storage must outlive the state and
updates. The canonical resolver must resolve actual parent identities; it
must not construct copied matrices, substitute a scene node, or supply a
default hierarchy when the binding is unavailable. The API deliberately has
no default-constructible unit state. No existing consumers or host subclasses
outside the two changed unit-instance files required migration.

## Exact order of the wake calculation

The x87 arithmetic does not change the integer CMP flags. Each CMP therefore
precedes arithmetic whose single-precision FSTP occurs before the associated
JNZ and possible refresh. The helper retains a separate Boolean for each
site's result and does not combine the checks into a loop.

| Validity check | Arithmetic/read before its branch | Conditional `00414DB0` call |
| --- | --- | --- |
| `00825A64` | read descriptor+A0h, multiply by double 0.5, store float half-width at `00825A7D` | `00825A85` |
| `00825A8A` | read world[9] at `00825A99`, multiply by the stored half-width, store float left offset | `00825AA8` |
| `00825AAD` | read world[13] at `00825AB4`, add the stored left offset, store float left point | `00825AC6` |
| `00825ACB` | reread world[9] at `00825AD2`, multiply by half-width, store float right offset | `00825AE1` |

After the fourth possible refresh, `00825AE6` rereads world[13], subtracts the
stored right offset and stores the float right point at `00825AF0`. The C++
helper takes the descriptor width by reference so the width read itself stays
after the first validity check. Each intermediate FSTP is represented by an
explicit float result; the arithmetic uses double intermediates for the
operation on the already-rounded float inputs.

The existing settings/controller/effect boundary follows these reads. When a
prop-wash handle exists, `00825BDD` checks the same C8 byte again,
`00825BE8` conditionally refreshes that same pose, and `00825BF5` reads its
current world[13]. This preserves mutations made by the wake callbacks before
the prop-wash test. Scene-node transforms, controller work, emitter dispatch,
attachments, and other update branches retain their existing boundaries.

## Validation and limits

`./scripts/build.ps1` passed for MSVC Win32 Release with `/W4 /WX /fp:strict`.
Both existing CTests passed: `reconstructed_math` and
`native_math_differential`. These are regression checks, not native
differential coverage of `008255B0`.

One local x86 fixture, `local/unit_wake_pose_probe.cpp`, linked against the
built `bsp_core.lib` with `/link /MANIFEST:EMBED` and passed. Its single
parent/child scenario produced wake points -8 and -12, then invalidated the
parent and child during the wake callback; the prop-wash callback received the
new translation -20 after a real canonical refresh. A final read with C8=80h
preserved both the cached matrix and derived byte 7Fh. The fixture also checks
that the state cannot be default-constructed and rejects scene-node refresh
calls. It does not instrument four individual memory reads; their ordering is
established by assembly and source inspection. The local command and logs are
listed in `reports/unit_wake_pose.json`.

This is a reconstructed, build-tested and fixture-tested fragment in a new
C++ interface. It does not recover the whole unit layout, replace the original
ABI, prove all x87 environment/exception behavior, or establish gameplay or
visual correctness. The existing descriptive function name remains a
hypothesis rather than a recovered symbol.

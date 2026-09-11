# Scene generation-gate parent pose binding

Packet `orch3_scene_parent_pose_k` replaces the parent-offset callback in the
existing typed `0046C550` generation gate with actual canonical pose refresh
and native x87 field additions. Its reconstructed range is the **interior
44-byte fragment 0046C6B7..0046C6E2**, not a newly completed whole function.
The gate's other services remain explicit. No unit-placement constructor or
scene/renderer owner is introduced.

## Input and host migration

`SceneEntityGateInputs.parent_identity` replaces `bool has_parent`. It carries
the actual nullable native argument-3 owner, borrowed for the call. The gate
captures this value once, matching a native pointer argument passed by value,
and receives an explicit `PoseRefreshResolver&`. A later host mutation of the
C++ input projection cannot substitute a different parent. The gate makes its
parent-present decision from this actual pointer; it has no second Boolean
presence state.

The arbitrary `SceneEntityGateHost::parent_world_offset` operation is removed.
The gate now calls `add_scene_parent_world_offset_0046c6b7` directly for a
nonnull parent. Deferred naming remains a required service, but its signature
is now `parent_name(void* captured_parent_identity)`: it receives the same
captured argument instead of relying on a host's implicit or currently selected
parent. Both call sites of the local `build_record` helper preserve this identity.

The tracked caller search found only this function's declaration/implementation
and its input type; no other tracked caller or host implementation required
migration in this checkout. External callers must supply the actual parent
identity and canonical resolver and update their naming override. No unrelated
shared consumer file was changed.

## Recovered fragment and original interface

Native `0046C550` is a thiscall generation predicate with ECX = scene database,
23 stack DWORDs and `RET 5Ch`; argument 3 is a nullable actual parent owner.
At `46C6A2` the parent is loaded into EBP. The surrounding branch tests that
captured pointer and routes null to the separate matrix-composition path.
The owned fragment has no independent call ABI or `RET`:

| Address | Exact operation |
| --- | --- |
| `0046C6B7` | Compare captured parent's byte +C8h with zero |
| `0046C6BE` | Skip refresh when that byte is any nonzero value |
| `0046C6C0` / `0046C6C2` | ECX = same EBP owner; call recovered 00414DB0 |
| `0046C6C7` | FLD actual parent +FCh (`world_cc[12]`) |
| `0046C6CD` / `0046C6D1` | FADD prior X at ESP+1Ch, FSTP float back to X |
| `0046C6D5` | Only then FLD actual parent +104h (`world_cc[14]`) |
| `0046C6DB` / `0046C6DF` | FADD prior Z at ESP+24h, FSTP float back to Z |

The final covered FSTP at C6DF is four bytes, ending C6E2. The following
two-byte JMP at C6E3 goes to C70F; it is the surrounding branch continuation,
not a missing return or flow gap. The fragment is 10 instructions. Read-only
target-verified Ghidra disassembly/bytes in `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, showed complete instructions and flow here.
No Ghidra mutation or function definition was made for this interior address.

The new helper resolves the captured nonnull identity exactly once to an
existing `PoseRefreshView`. It tests that view's actual byte, calls the
canonical refresh when zero, and then uses the **same view's** world storage.
There is no parent-identity reload or resolver call after refresh. The helper
does not cache coordinates before refresh, copy a matrix, create flags or
maintain a parallel parent graph. Recursive ancestor handling is the existing
414DB0 implementation operating on the same borrowed pose fields.

The additions use explicit x87 FLD/FADD/FSTP in native operand order, preserving
the float spill for X before loading Z. They do not use generic model
translations, transpose a matrix, add Y, rotate the local offset, or replace
the parent-present branch with a full local-times-parent product. This branch
really adds parent translation to the local X/Z already held by the gate.
The two output references represent distinct caller-local floats, not pose,
matrix or hierarchy storage. The caller's FPU state is retained.

## Other paths and lifetime boundaries

A null parent does not enter the helper or resolve a pose. It retains the
existing `compose_world_frame` service for native C6E5..C70F. The no-MultiType
early return still builds its deferred record without a pose refresh, even
when a parent exists. Mode/property lookups, parent naming, deferred record
ownership and multiplayer stock registration remain required services. This
packet does not certify the entire generation predicate's native allocation,
exception, raw-array alias or numerical behavior.

The parent owner, resolver, returned view and all backing fields must survive
the gate call, including required host calls. The resolver must perform pure
lookup of that actual owner's existing view, without fallback objects or state
copies. The helper rejects a null identity as a typed precondition failure;
the native surrounding branch already prevents that call. No retention,
deletion or hierarchy ownership changes occur. Native dirty cycles and other
pose invalid-state limits remain those in `POSE_REFRESH.md`.

## Validation

`./scripts/build.ps1` passed strict MSVC Win32 Release `/W4 /WX` and both existing
CTests. The already ignored `local/pose_refresh_fixture.cpp` was extended with
one scene-gate composition group, rather than adding a permanent test suite.
Its recording scene services exercise actual recursive pose storage, local
X/Z plus refreshed parent translation selecting the expected area, null-parent
resolver bypass, captured parent identity during deferred naming after a host
mutates the input projection, and the no-MultiType early bypass. All passed via
`local/run_pose_refresh_fixture.cmd`; output is
`local/scene-pose-binding-fixture.log`.

The fixture retains the prior isolated native pose-control comparison using
shared canonical matrix calls. The newly added scene cases are composed
behavior checks, **not** a differential execution of the full native generation
gate. The parent additions' x87 order is established from assembly and source;
there was no new exhaustive float, FPU-trap or alias sweep. No game process,
graphics/input hardware, window focus or gameplay run was used.

# Scene generation-gate geometry bindings

Packet `orch3_scene_parent_pose_k` replaces the parent-offset callback in the
existing typed `0046C550` generation gate with actual canonical pose refresh
and native x87 field additions. Its reconstructed range is the **interior
44-byte fragment 0046C6B7..0046C6E2**, not a newly completed whole function.
Packet `orch3_scene_null_parent_geometry_k` additionally closes the **42-byte
interior null-parent fragment 0046C6E5..0046C70E** with canonical matrix
multiplication and removes the invented identity-frame fallbacks. Neither
packet completes the whole generation gate. Other services remain explicit. No unit-placement constructor or
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

Both frame fields now point to canonical `CameraMatrix` objects and are required
on every gate call. The gate captures the live `local_frame` pointer and copies
the 64 bytes supplied by `parent_frame` into its own argument snapshot before
any gate service. It rejects a missing binding with `std::invalid_argument`;
native code unconditionally reads the local matrix and always receives the
parent matrix inline, so it has no corresponding missing-frame recovery.
The typed exception diagnoses an invalid binding; it does not reproduce a
native access violation. The parent-frame snapshot represents by-value argument
storage, not a cached pose or another transform hierarchy. Deferred-record frame
copies also use the captured local pointer and parent argument snapshot. The
`compose_world_frame` host method is removed.

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
native local-times-parent matrix branch through the canonical multiply below.
The no-MultiType
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

## Null-parent stack and operand evidence

After the prologue, let S denote ESP: the entry return address is at S+74h,
argument 4 (the local matrix pointer) is at S+84h, and arguments 7..22 occupy
S+90h..S+CCh. The prologue's three SEH pushes, 58h local allocation and four
saved-register pushes account for this 74h displacement. EDI captures argument
4 at C573; MOVSS reads its indices 12 and 14 at C57A/C587 before property lookup.
There is no null check or default matrix on this path.

This by-value interpretation is supported by caller assembly, not only a
decompiler parameter list: within `0046CF40`, D3E2 reserves 40h on the stack,
D3E5 saves that destination in EDI, and D3E9/D3F5 set ECX=16 and REP MOVSD the
matrix before the call at D426. The second call at D655 repeats the 40h reserve
at D611 and sixteen-DWORD copy at D616/D622. These are read-only observations;
the caller is not reconstructed or edited by this packet.

| Address | Exact null-parent operation |
| --- | --- |
| `0046C6E5` / `0046C6EC` | Form S+90h, push the inline right matrix address |
| `0046C6ED` / `0046C6F1` | Form current ESP+2Ch = S+28h, push disjoint destination |
| `0046C6F2` / `0046C6F4` | ECX=EDI captured live left matrix, call 00413920 |
| `0046C6F9` / `0046C6FE` | MOVSS returned product +30h into X at S+1Ch |
| `0046C704` / `0046C709` | Only then MOVSS product +38h into Z at S+24h |

Canonical `00413920` takes ECX=left, stack destination/right, returns destination
in EAX and uses RET 8. Its entry reads right at ESP+48h and destination at
ESP+44h after its own 40h allocation, confirming this call's operand order.
The result is **local * inline parent**, with full matrix multiplication, not
just a translation sum. The new helper uses distinct temporary product storage,
then explicit MOVSS transfers in the native X-before-Z order. The final MOVSS
at C709 is six bytes and ends C70E; the next instruction at C70F is the common
continuation. All 42 bytes/10 instructions are present, with no gap or separate
function return.

The local matrix remains live through multiplication and deferred copying;
only its pointer is captured. Aliasing the caller's two matrix sources is
allowed: the parent bytes are snapshotted first and the multiply destination
is distinct. X/Z helper outputs must be distinct locals outside the inputs.
Arbitrary partial overlap, concurrent source mutation and native stack aliasing
are not part of the typed interface. No native calling-convention replacement,
whole-gate exception parity, or validation of every upstream caller is claimed.

## Validation

`./scripts/build.ps1` passed strict MSVC Win32 Release `/W4 /WX` and both existing
CTests. The already ignored `local/pose_refresh_fixture.cpp` was extended with
one scene-gate composition group, rather than adding a permanent test suite.
Its recording scene services exercise actual recursive pose storage, local
X/Z plus refreshed parent translation selecting the expected area, null-parent
resolver bypass, captured parent identity during deferred naming after a host
mutates the input projection, and the no-MultiType early bypass. The follow-up
replaces the null-parent fixture callback with actual noncommuting matrices:
local translation (1,0,2) multiplied by the supplied basis/translation yields
X=14,Z=37 and selects the expected area; reverse multiplication would not.
It also checks both missing frame bindings fail before resolver or mode-host
calls and that deferred copying retains the local pointer and parent snapshot
after the host changes input bindings and the parent source. All passed via
`local/run_pose_refresh_fixture.cmd`; output is
`local/scene-null-parent-fixture.log`; the latest strict build/2CTest log is
`local/scene-null-parent-build.log`.

The fixture retains the prior isolated native pose-control comparison using
shared canonical matrix calls. The newly added scene cases are composed
behavior checks, **not** a differential execution of the full native generation
gate. The parent additions' x87 order is established from assembly and source;
there was no new exhaustive float, FPU-trap or alias sweep. No game process,
graphics/input hardware, window focus or gameplay run was used.

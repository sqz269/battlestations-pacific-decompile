# Concrete spatial binding for force events

`ForceEventSpatialRuntime` implements the existing `ForceEventSpatialHost` by
calling the recovered entity pose refresh over the same target fields. It
closes the game-operation boundary left by `GAMEPAD_FORCE_EVENTS.md`, using the
borrowed pose view from `POSE_REFRESH.md`. This packet adds runtime composition;
it does not count another reconstruction of `00873560` or `00414DB0`.

The constructor accepts lvalue references to
`ForceEventCurrentTargetLookup` (`std::function<ForceEventTargetPose*()>`),
`ForceEventSubjectTransformLookup` (`std::function<CameraTransform&(void*)>`),
and the existing `PoseRefreshResolver`. It rejects empty lookup functions and
stores references to them. The runtime owns no function object, native owner,
target, transform, matrix, flag, parent link, or hierarchy. Temporary function
objects cannot bind to its constructor.

Each `current_target_00e188a8_1ed4()` invocation calls the borrowed lookup anew.
The application must read the actual current game and its +1ED4 target on each
lookup, returning the existing target view or null for an absent target. The
runtime never saves a current game or target between calls. Each
`subject_transform_110(subject)` passes that exact subject identity to the
borrowed lookup, which reads its current +110h canonical `CameraTransform`.
These are actual identity/field bindings, not replacement geometry callbacks.

For `refresh_target_pose_00414db0(target)`, the runtime:

1. Rejects a null target owner identity.
2. Resolves that identity to its existing `PoseRefreshView`.
3. Requires both `&pose.world_valid_c8 == &target.world_valid_c8` and
   `&pose.world_cc == &target.world_cc`.
4. Calls the actual `refresh_pose_00414db0(pose)` only after both checks pass.

A mismatched flag or world matrix throws `std::invalid_argument` before any
refresh. Equality of values is insufficient: the references must bind the same
storage. The resolver remains responsible for the identity, local matrix,
parent link and other fields being the actual owner's fields. These checks do
not establish validity of arbitrary pointers or permit a fabricated view.

`ForceEventTargetPose::world_valid_c8` changes from `bool&` to `uint8_t&`.
That matches the recovered pose view and the native byte test, including an
arbitrary nonzero value on a clean path. This is an intentional source-level
change for callers that previously supplied Boolean diagnostic storage. No
reinterpretation or copied Boolean mirror is introduced.

## Native evidence and ordering

The original `00873560` ABI remains ECX = allocated event, stack definition and
subject pointers, EAX = this, `RET 8` at `008736FB` (three bytes, inclusive end
`008736FD`). Read-only target-verified disassembly against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` confirms:

| Address | Operation |
| --- | --- |
| `008735BD` / `008735C3` | Read current game E188A8, test its target +1ED4 |
| `008735DF` | Read actual subject +110h transform |
| `008735ED` | Refresh canonical source transform with B6DB70 when flags lack bit 2 |
| `008735FA` / `0087361C` | Reload current game and its current target after source refresh |
| `00873622` / `00873633` | Test captured target byte +C8h, call 414DB0 when zero |
| `00873638` onward | Use that captured target's actual world-position fields |

The existing event producer retains this sequence and its real distance/fade
calculation. The new runtime does not insert an additional current-target
lookup inside the pose refresh. It resolves the captured target's identity,
even if the game's current target subsequently changes. The pose algorithm
itself performs the real parent recursion, canonical matrix product/copy and
validity stores; this adapter contains no substitute pose algorithm.

## Lifetime and validation

Both callable objects and the resolver must stay alive and callable throughout
runtime use. They may capture references to actual owner services, which must
also remain alive. Returned views and their backing fields must remain valid
for the producer's use. Lookups/resolution are pure field/identity access:
they must not manufacture objects, cache a stale singleton, retain owners,
copy matrices or maintain a parallel parent tree. Existing producer guards
for a disappearing target or invalid subject remain unchanged. Empty-callable,
null-identity and field-mismatch exceptions are typed binding failures, not
recovered native exception behavior.

`./scripts/build.ps1` passed the strict MSVC Win32 build and both existing
CTests. The eight native seeds were already verified in the immediately
preceding pose-refresh packet; this composition changes no native reference or
math routine, so seed verification was not repeated.

The existing ignored `local/gamepad_force_events_fixture.cpp` was updated for
byte flags and extended with one composed case. Its actual event producer
uses `ForceEventSpatialRuntime`, canonical source transform refresh, real
recursive target pose refresh, and the actual fading-request registry. A
parent translation plus child local offset produces target (13,24,30), source
(10,20,30), distance 5, radius 10, and initial amplitude 0.4. The fixture checks
that both target views observe the same validity/world writes and that the
producer performs two current-target lookups. A deliberate fixture lookup
mutation exercises reloading; it is a test stimulus, not a production callback
contract. Separate flag and matrix mismatches are rejected before refresh.
The existing ownership and float-edge checks continue to pass.

Run with `cmd /c local\run_gamepad_force_events_fixture.cmd`; output is in
`local/force-event-spatial-fixture.log`. Lookup and device-output services are
explicit fixture bindings; no native game owner, input hardware, window focus,
or gameplay integration was exercised. Native allocation, object layout,
FPU exceptional behavior and lifetime limits remain those documented in the
underlying event and pose modules.

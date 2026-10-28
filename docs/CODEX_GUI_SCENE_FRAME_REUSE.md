# Startup scene frame reuse

`StartupSceneDomain::Trees` and `StartupSceneDomain::Children` previously handed
out their 256 prepared slots once each over the domain lifetime. A successful
call consumed the same capacity as an active or failed call. Repeated raw GUI
work could therefore exhaust the reservation without deep recursion.

The providers now search their handed-out prefix for an outer
`Acquired::complete` slot before extending that prefix. They reset the selected
frame and acquisition before returning it. The 256-slot reservation and its
exhaustion errors remain; capacity bounds simultaneous reservations plus
retained incomplete calls. Slots already handed out but not entered are also
retained. A false `started` flag is never a reuse condition.

## Completion and argument identity

The native contracts reached by these providers are:

| Address | Original ABI | Source completion boundary |
| --- | --- | --- |
| B6F310 | ECX actual node, no arguments | After child calls, attachment erase, decrement and any zero terminal |
| B6D850 | ECX actual node, no arguments | After every recursive unregister returns |
| B6ED80 | ECX actual node, scene/recursion words, RET8 | After scene mutation and child recursion |
| B7C020 | ECX actual light, scene/recursion words, RET8 | After scene append/registration/retain and child recursion |

See `src/native_node_tree_retirement.cpp` and
`src/native_node_scene_attachment.cpp`. Each sets its outer completion flag at
the end, with no subsequent callback or native payload access. Nested calls
therefore retain distinct initialized, address-stable cells until they return.
Resetting completion before the next handoff also prevents a prior successful
call's flag from making a newly active call appear reusable.

B8F4C0 passes the address of its original stack node argument to B7BED0, whose
body is byte-identical to B7B620. That erase consumes the key synchronously.
B6ED80/B7C020's scene/recursion cells correspond to entry stack +4/+8. Reuse
begins only after the whole invocation completes; no original stack argument
identity survives that return. The existing synchronous callback and valid
dependency domains still apply. These source frame layouts do not claim native
private-stack or drop-in ABI compatibility.

## Registry ownership and failure

`NativeSceneRegistryRegistrationAcquired::allocated_node` records an allocation;
it does not own it. B823B0 copies the node key value into the actual heap list
node's +8 field rather than retaining the caller's key-cell address. B83700 links
the allocation into the actual registry before setting successful completion.
Resetting diagnostics after outer completion does not free, unlink or release
that allocation.

An incomplete slot is never reset. This preserves its frame, diagnostic
identities and any allocation prefix for explicit failure disposition. In
particular, B7C020 appends the scene array entry before registration and before
acquiring a scene credit; insertion failure must not be treated as successful
handoff or followed by an invented ordinary-removal rollback. A later child
failure also retains the outer slot even if its nested registration completed.
No native ownership operations or failure rollback were added.

## Scope and validation

The reviewed base is `762fabe3d`. Its A1 mission path borrows only the startup
scene environment and registry at `src/game_native_mission_ship.inc`; it does
not invoke these raw tree/child providers. This change belongs to subsequent
raw GUI work and does not establish repeated GUI-page or mission-reload parity.
The startup switches and immutable A1 test executables are unchanged.

The bounded source review checks both acquisition paths, outer completion,
argument-cell use and registry allocation handoff. `git diff --check` passes.
The strict Win32 build and existing CTests are pending an integrator-scheduled
window after the active A1 validation. No additional test API, probe, game run,
real-allocator runtime or GUI/reload validation is claimed.

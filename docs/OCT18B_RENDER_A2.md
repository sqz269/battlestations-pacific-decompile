# Ordinary installed-model command composition

This packet composes the existing native ordinary command path for the OFF-default
`BSP_NATIVE_MISSION_SHIP=1` diagnostic. It does not change mission simulation or own
Begin/End/Present. The original control sequence remains the renderer frame graph's
single Begin and single native End/Present. Application integration and live visual
validation belong to the primary lane.

## Retained application graph

`game_native_mission_command.inc` constructs command/context/batch lifetime,
collection/upload, target/camera preparation, system gather, material constants,
pass/entry dispatch and queue execution over the renderer's existing providers.
`FrameGraph::bind_ordinary_execution` attaches the same execution/frame identities
to main End, worker End and native queue destruction. There is no second queue.

One pending command has one in-place canonical context companion and two in-place
canonical batch companions. Binding callbacks neither allocate nor retain. Context
lookups first resolve that one command context, then delegate every external native
identity to the canonical model/renderer owner domain. The native context/command
destructors perform all native release/free operations. Retirement callbacks only
record that those native terminals completed. Host continuations retire only after
queue count zero, context null and both batch/context companions retired.

Every submission has a retained system frame and enough fresh constant/pass frames
for its observed entries and light-list upper bound. Allocation or native execution
failure preserves the graph. `requires_process_retention()` is true while any
command is pending, including a native End failure that escapes before completion.
The integration must retain the application on that condition instead of attempting
ordinary destruction or replay. Device-generation changes require a drained graph;
all device/resource references remain the canonical live cells.

The bounded single-model domain admits the existing serial batch-preparation branch.
If synchronization mode is zero and both batches exceed fifty entries, it rejects
before execution and retains the command. It does not manufacture a frame-job
dispatcher. Tracelines, debug-sphere output and point-light predicates have no
invented providers; genuine descriptor/profile branches require their real domains.
Model/node/camera traversal has a new two-domain metadata admission; Traceline
profile D0C928 remains unsupported. Nonnull animator/type work now rejects an absent
type-word provider at B42794 rather than dereferencing a null source-context pointer.

## Original evidence and source globals

Verified `config/target.json`: project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, installed PE
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
The live `bsp.py ghidra proto 00B1D950` batch reported that program. Existing exports
of B1F5E0 and B1D950 match the reconstructed append/traversal/upload and two-batch
execution schedules. B43410 listing B435C0..B43606 confirms indexed virtual13C and
nonindexed virtual140 calls. Existing full reconstructed bodies remain authoritative;
this packet adds application composition and external observations, not replacement
binary entrypoints or new names.

PE `.data` has RVA A08000, virtual size 297EDC, raw size 10000. A PE loader image
view confirms all bytes in these virtual-tail ranges are zero:

| Original range | Source owner | Genuine subsequent writers |
| --- | --- | --- |
| 0108DBE8..0108EBEB | PS header plus 256 float4 bank | B42350 and parameter writers |
| 0108EBF0..0108FBF3 | VS header plus 256 float4 bank | B42350 and parameter writers |
| 0108FBF4 | current effect cache pointer | B55550, B44750 |
| 0108FC30..0108FC6F | sixteen parameter DWORDs | B40860 and B9A030/B9A080/B9A0D0/B9A130 |
| 00F8D4B0 | collection initialization guard | B1DFF0 |
| 00F8D450..00F8D4AF | six float4 collection colors | B1DFF0 |

One intentionally process-retained `MissionRenderGlobals` owns these distinct
relocated cells. It neither resets them between frames nor maps them as callable
original addresses. The canonical VFS `native_owners().batches()` supplies the actual
0108FE8C/0109DBBC pool/lock domain; no new batch singleton is created.

The system prefix frame copies its initialized preimage from the SAME current VS
bank before entering B46A70. This is an explicit source-caller preimage, not a claim
to recover the original uninitialized stack. Native gather overwrites its established
words and preserves remaining bits. The register count must fit the original 4D0h
prefix extent. The default canonical E13078 value is 4D registers.

B46A70 unconditionally uses the service's matrix at +1D8 and its time region +84
(byte +24, float +28), the actual timer interval, F8C274+2C and F8C210+8/+11. The
primary and A1 provide genuine constructed service/foliage owners. Null provider
values are rejected before native submission; neutral records are not substituted.

## Integration contract

Include `bsp/game_native_mission_command.hpp` at the application TU's top level.
Include `game_native_mission_command.inc` after `game_native_renderer_frame.inc`.
Append a `std::unique_ptr<MissionCommandGraph>` after the frame graph owner so reverse
destruction drops the mission attachment first. Construct it only under the switch:

```cpp
auto commands = std::make_unique<MissionCommandGraph>(
    p.log, *p.frames, p.graph, p.devices, p.cameras, p.constructor,
    p.mesh_field_vfs, p.singletons, p.raw, p.vfs.strings,
    p.resources.publication, p.entry_cache_publication, providers);
```

The provider aggregate borrows model geometry/group/strict virtual services, the
SAME raw clock publication context and the SAME current F8C274/F8C210 cells. The
model graph may supply null animator type storage only for static non-animator
branches. A1 can borrow `commands->render_access()` before resource initialization;
constructing this metadata does not submit or execute a native command.

After a drained device transition call `before_device_generation(generation)`.
Between the existing Begin and End call `submit_drawable(scene,camera,targets,model)`.
After successful native End call `complete_frame()`. Do not reset/replay a failed
pending frame. Call the retention predicate from the application's failure policy.

All new observations use `GameHostLog`, with no console or per-leaf printing.
Submission records queue count, both entry counts and group counts. B43410 source
frames record the genuine B24010/B21B40 terminal immediately before its call and
whether that call returned. Frame completion logs those observations. Return is
proof of the reconstructed leaf invocation, not proof that its device gate admitted
a draw or that D3D returned success. Screenshots/game validation remain separate.

## Validation state

Static export/assembly and PE-loader-zero evidence inspected. No new test cases were
added. Primary integration must compile the included graph and run the existing
build/checks, OFF comparison, nonempty queue teardown, native terminal observations
and fixed-frame screenshot checks. This source composition does not establish the
original private-stack/FH3/register ABI or a binary-compatible game replacement.

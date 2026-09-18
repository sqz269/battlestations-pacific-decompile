# Native game grid construction and renderer ownership, R120

Addresses: `0070BD70`, `0070B330`, `0070B220`, `0070B280`; parent composition at `004DDFF0`, `004DE029`, `004DE062` inside `004DDB90`.

## Result

Four complete ordinary bodies, **2,175 original bytes**, now compose the CPU routines from [R119](NATIVE_GAME_GRID_R119.md) with the existing native declaration cache, logical/physical buffer factories, mapping, canonical owner companions and terminal destruction. No renderer result or HRESULT is synthesized.

| Entry | Original contract | Source behavior |
|---|---|---|
| 70BD70, 364 bytes | ECX84h allocation, stacked angle, EAX same owner, RET4 | Initialize exact fields and38h descriptor; initialize the grid; preserve native FSIN/FCOS and binary32 spills for the orientation fields. |
| 70B330, 1,691 bytes | ECX owner, stacked descriptor, RET4 | Copy descriptor, allocate six CPU arrays, initialize geometry/normals, acquire declaration, replace streams, emit both index sides, fill positions/UV/color, unmap and release the declaration temporary. |
| 70B220, 90 bytes | ECX owner, RET | Stamp CFD484, release CPU arrays, release current vertex then index creator references and clear fields after callbacks. |
| 70B280, 30 bytes | ECX owner, stacked flags, EAX original owner, RET4 | Destroy; free allocation iff low flags byte has bit0 set. |

The three parent calls now have a concrete `NativeGameConstructionCalls::call_0070bd70` default. The parent supplies one actual grid context, distinct per-call descriptor preimages, and three retained child operations. A missing context raises a source contract error at the reached call; it does not return an invented owner. The game constructor is still not admitted into ordinary application startup by this packet.

“Grid” remains a descriptive hypothesis, not a recovered class name or an assertion about its game subsystem.

## Preserved behavior

- Constructor constants are captured in original order. The three F87574/F87578/F8757C words are copied separately. The two otherwise-uninitialized descriptor words are explicit caller inputs and pass through the R119 x87 copy, remaining observable during all six allocations.
- Each allocation reloads current owner count and saturates unsigned `count * 12` to FFFFFFFF on overflow. The default uses the source CRT operator-new contract; CPU cleanup uses source CRT free, as the original BF6989 calls do.
- CPU row conversion uses SSE CVTSI2SS before x87 multiplication. Steps, the first-column sine curve, cross products, vertex positions, UV interpolation and orientation preserve their observed spills and operand order.
- The back-face index at70B706 adds **rows**, while the other quad indices add columns. Initial GPU positions divide by rows; UVs divide by columns. There are no corrective clamps or rectangular-grid substitutions.
- Declaration lookup uses actual `gunvc.mvfm`. The later renderer captured before old-vertex release is reused for both factories. Stream fields are published only after each factory returns. Existing references are released before replacement, with field clearing after terminal callbacks.
- Vertex flags1000 select the renderer's **shared dynamic physical buffer at +1974**. Index format65 selects16-bit indices. The native zero-count map calls are retained, including the logical vertex count field becoming zero during the dynamic map.
- The normal vertex attribute is not written by this initializer. CPU normals are calculated, but the GPU normal bytes retain their prior contents. Packed-color and split-color writes preserve native current offsets.
- Failure retains the actual partial CPU/renderer graph and entered map/factory records. Only the completed local format string follows the initializer's observed cleanup state. Failed operations cannot be replayed. Diagnostic acknowledgement releases nothing; the caller must resolve retained resources first. Parent acknowledgement refuses unresolved failed/running grid children.

These are explicit MSVC Win32 source interfaces. Original FH3/SEH, private-stack aliases, hardware-fault ordering and drop-in binary ABI are not claimed.

The initializer's cleanup scope is supported by62additional live/PE bytes: handlerC84748 points to FuncInfoDB2E28, with one unwind entry atDB2E20 and no try blocks. That entry targetsC84740, which loads the local header atEBP-14 and jumps to41DD20. These compiler-support bytes were inspected read-only; the native handler is not ported or executed by the fixture's exception checks.

## Verification

The collector rechecked the existing project/program and all **2,945 live/PE mapped bytes** from R119. Seven original bodies, **2,889 bytes**, were copied into an isolated child. All original branches were relocated; 40 CALLs and five global references were bound explicitly. The original descriptor, normal and CPU-cleanup bodies execute in that lane. Both lanes use the already reconstructed42B260/UCRT normalization and the same concrete renderer/library services.

The child used a real NVIDIA GeForce RTX5090 D3D9 HAL device. It decoded `gunvc.mvfm` through the actual declaration decoder, then exercised the grid's cache-hit load path. It constructed the actual pooled dynamic vertex buffer, private index buffers, native pools and canonical companions. CPU allocation wrappers call the real CRT and select deterministic fresh-buffer preimages; one schedule changes the live vertex count after the first allocation.

**108 paired cases passed, with 2,933,712 observed bytes matching.** They include:

- all12x87 precision/rounding combinations;
- four constructor angles, including a signaling NaN, and explicit signaling-NaN descriptor preimages;
- five initializer dimension pairs, including2x2,3x4,4x3,16x16 and7x9;
- replacement of real preexisting streams, live allocator-count mutation, and scalar flags0..3;
- allocation/free observations, normalized raw owner fields, all CPU arrays, written mapped vertex fields, index-buffer bytes, and renderer registration/count fields.

The mapped vertex comparison excludes only the unwritten normal attribute. It observes this driver's mapped bytes; it is not a rendered-grid image comparison. The split-color branch is assembly-backed but not reached by the natural packed-color declaration in this fixture. The source lane also exercises the game's concrete default grid call. All **481 canonical renderer companions** retire, and the actual declaration/token/string/pool graph closes.

Two source-only failures (third CPU allocation and unsupported reached renderer profile) retain the partial graph, clean the completed local string where applicable, reject replay, and allow explicit diagnostic cleanup. These do not establish original exception-handler equivalence.

The existing parent comparison passed all **33 cases /1,657 observations /121,405,968 bytes**, with explicit grid-context/preimage/child-operation routing. The parent fixture keeps the grid call controlled; the separate seven-body fixture supplies the concrete child evidence. The strict Win32 build and all three existing CTests passed. No permanent tests were added.

A fresh standalone D3D9 probe also passed real buffer uploads, indexed drawing and pixel readback, superseding the earlier device-unavailable observation. The first ordinary application attempt stopped at its singleton-mutex preflight while an independent `cc8` USN02 mission check ran; those processes were left untouched. After their exit was verified, this worktree's unchanged tested executable completed a fresh two-frame run using the hash-checked private XLive runtime and an isolated personal-settings directory: device creation succeeded, one frame was presented and one skipped, the loop exited0, and final device/API COM counts were0. This remains the existing partial frontend path with explicitly unimplemented host rows. It neither reaches this packet's raw game constructor nor establishes gameplay or visual parity.

## Follow-up

The remaining application work includes the actual game allocation/lifetime around73E150..73E1A9 in73D410, the complete game destructor/exception cleanup, real per-frame input and mouse binding, and required subsystem context ownership. Revalidate the live application state before choosing the next admission step. Fixture, build, byte and isolated GPU evidence do not establish a running or gameplay-validated reconstruction.

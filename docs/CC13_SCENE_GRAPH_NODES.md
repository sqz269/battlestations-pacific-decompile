# Segments 88/89: scene-graph nodes, animators, lights and scene resources (cc13-files1)

Segment 88 is `00B6D3C0..00B80ED0` (c3dNode, cCamera, cMesh, cLight family, c3dNodeAnimator,
cOptimized3dNodeAnimator, cAnimTrack); segment 89 is `00B80FD0..00B86E10` (cSceneResource,
resource dump, cNullSceneResource). Names are descriptive hypotheses. "Unreferenced" means
no rel32 CALL/JMP and no absolute dword in the installed executable points at the entry.
Status words: *named* (ledger + Ghidra), *reconstructed* (C++ in `src/scene_graph_cc13.cpp`),
*build-tested*. Nothing here is fixture-tested, ABI-compatible or game-validated.

## Triage

At `origin/main` (index of 2026-10-12) the two bands held 741 indexed functions: 543
covered by earlier lineages, 5 EH funclets and 193 raw `FUN_` entries. Node construction,
destruction, parenting, pools, the model base, cameras, point/directional lights, fog
owners and the resource manager were covered by earlier `NATIVE_NODE_*`, `NATIVE_MODEL_*`,
`DIRECTIONAL_LIGHT_*`, `NATIVE_POINT_LIGHT_*`, `NATIVE_NODE_ANIMATOR_LIFETIME_AW` and cc12
work; none of the raw entries had a lease or a reconstruction record. 115 of the 193 are
unreferenced (pointer-vector, tree and string-copy instantiations, iterator wrappers) and are
named by shape with an address suffix where several identical bodies exist.

A scan of `.rdata` for dwords pointing into the bands at addresses Ghidra had no function
for found 44 hits; 2 were mid-instruction data (`00B70900`, `00B80900`) and `00B81E2E` was
not a start. The other 41 were vtable slots Ghidra had never defined, among them every
`GetRuntimeTypeId`/`IsType`/`GetTypeDescriptor` slot of the node, mesh, model, light and
animator classes and both animator evaluation slots. They were defined with
`tools/ghidra_define_function.py` (record `reports/cc13_files1_defined_functions.json`)
under this packet's lease, so cc13-define1 skips them.

Result: 234 ledger names (193 raw + 41 new bodies), applied to Ghidra. `00BF0A30` (segment
97) was renamed `BSP_CsvTable_SaveStub` once its only caller turned out to be the resource
dump.

## Class type slots

Every class vtable here has the same head: `+00` RefCounted deleting destructor, `+04`
scalar delete, `+08` GetRuntimeTypeId (returns the class's first token), `+0C` IsType (scans
the class's token list), `+10` Clone, `+14` GetTypeDescriptor (returns the descriptor
global written by the class's static initializer).

| Vtable | Class | +08 | +0C | +14 |
| --- | --- | --- | --- | --- |
| 00D62C88 | c3dNode | 00B6D790 [0108FF90] | BSP_Node_IsType | 00B6D7A0 [0108FF98] |
| 00D62CF0 | cCamera | BSP_Camera_GetRuntimeTypeId | BSP_Camera_IsType | 00B6FB70 [0108FFAC] |
| 00D62D60 | cMesh object | 00B72630 [0108FFEC] | 00B73E30 | 00B72640 [0108FFF4] |
| 00D62D78 | model base | 00B74370 [01090044] | BSP_ModelBase_IsType | 00B74380 [01090050] |
| 00D62E60 | c3dNodeAnimator | 00B75D60 [010900E0] | 00B780A0 | 00B75D70 [010900E8] |
| 00D62EB0 | cOptimized3dNodeAnimator | 00B75E70 [010900FC] | BSP_Optimized3dNodeAnimator_HasType | 00B75E80 [01090108] |
| 00D62ED4 | animation channel group | 00B75D00 [010900C8] | 00B78600 | 00B75D10 [010900D4] |
| 00D62F3C | cAmbientLight | 00B7AA00 [01090110] | 00B7C420 | 00B7AA10 [01090118] |
| 00D62F58 | cLight | 00B7AA90 [0109018C] | (named earlier) | 00B7AAA0 [01090198] |
| 00D62FB0 | cDirectionalLight | 00B7AB00 [0109019C] | (named earlier) | 00B7AB10 [010901AC] |
| 00D631A0 | fallback scene resource | 00B86800 [01090210] | 00B868D0 | 00B86810 [01090218] |

Type-name strings come from the initializers: `00B7BFD0` "cLight", `00B7C0C0`
"cDirectionalLight", `00B7C110` "cPointLight", `00B78170` "cCameraAnimator" and
"c3dNodeAnimator".

## Animation tracks (reconstructed)

Track: before-mode `+14h`, after-mode `+18h`, keys `+1Ch`, count `+20h`. Key (28h):
type `+00` (interpolation of the segment that ends at this key), value `+04`, time `+08`,
tension/continuity/bias `+0C/+10/+14`, `+18/+1C` (types 1/2: in and out tangent; type 5:
in handle dt/dv), `+20/+24` (type 5 out handle dt/dv).

| Address | Name | Original ABI |
| --- | --- | --- |
| 00B76CE0 | BSP_AnimTrack_Sample | ECX track, stack float time, ST0, RET 4 |
| 00B75450 | BSP_AnimTrack_WrapTime | stack (time, start, end), ESI int* cycles or 0, XMM0, RET 0Ch |
| 00B75520 | BSP_AnimTrack_HermiteBasis | stack (s, float* h10), ECX h00*, EAX h01*, EDX h11*, RET 8 |
| 00B75820 | BSP_AnimTrack_OutgoingTangent | EAX key, ECX previous (nullable), EDX next, XMM0, RET |
| 00B75A20 | BSP_AnimTrack_IncomingTangent | EAX key, ECX following (nullable), EDX preceding, XMM0, RET |
| 00B75C10 | BSP_AnimTrack_FindKey | ECX keys, EDX count, stack float time, EAX, RET 4 |
| 00B75580 | BSP_AnimTrack_BezierValue | stack (p0..p3, s), ST0, RET 14h |
| 00B75610 | BSP_AnimTrack_BezierSolve | ESI lo*, EDX hi*, stack (p0..p3, target), XMM0, RET 14h |
| 00B75720 | BSP_AnimTrack_EvaluateBezierSegment | EAX key, EDI next, stack time, ST0, RET 4 |

These use whole-program register conventions (XMM0 float returns, pointer arguments in
EAX/EDX/ESI/EDI), so the C++ exposes ordinary functions. The x87 bodies were walked with
`tools/x87_sym.py` per path (SSE moves checked in the listing); floor is CRT `00BF85B0`
(fallback control word 173Fh, round down) and `_ftol2` is `00BF7420`.

Sampling (00B76CE0): 0 keys -> 0; 1 key -> its value. Before the first key (strict
`first.time > t`) or after the last (`t > last.time`) the mode selects: 0 -> 0; 1 -> the
end key's value; 2 cycle -> `WrapTime(t, first, last)`; 3 ping-pong -> wrap with cycle count,
odd counts give `(last.time - first.time) - wrapped`; 4 relative cycle -> wrap and an offset
`(last.value - first.value) * cycles` added to the result; 5 linear -> slope from the end
key's tangent divided by the neighbouring time span; any other value falls through to the
search with t unchanged. In range: `i = FindKey(t)`; `t == key[i].time` or
`t == key[i+1].time` returns that value (+offset); otherwise `s = (t - t_i)/(t_{i+1} - t_i)`
and by `key[i+1].type`: 0/1/2 Hermite with `OutgoingTangent(key i)` and
`IncomingTangent(key i+1)` (the partial sum is stored as a double), 3 linear, 4 step,
5 Bezier, other -> the offset alone. Results are stored to a float before returning.

`WrapTime` returns `start + time - floor(time/len)*len`, which equals a modulo wrap only
when `start == 0`; the cycle count is `-trunc((wrapped - time)/len +/- 0.5)` (+0.5 when
`wrapped > time`). Tangents: type 0 is Kochanek-Bartels without the 1/2 factor, scaled by
the time-span ratio when the far neighbour exists; types 1/2 return the stored tangent
(scaled likewise); type 3 is Catmull-Rom; type 5 is the handle slope, with the division
replaced by `*100000` when |dt| <= 1e-5. `BezierSolve` bisects until the error is <= 1e-4
and has no iteration limit natively (the C++ stops after 200). `EvaluateBezierSegment` reads
the next key's handle from `+18h/+1Ch` whatever its type.

## Animators (reconstructed where they sample)

Ordinary animator (`00D62E60`, layout in `docs/NATIVE_NODE_ANIMATOR_LIFETIME_AW.md`):
`+18h` `BSP_NodeAnimator_EvaluatePose` (00B77360; out matrix, group, time; RET 0Ch) seeds
translation `+08h`, angles `+14h`, scale 1, samples `BSP_AnimationChannelGroup_SamplePose`
(00B77220: channels 0-8 of the group's track array `+10h`, a null track leaves its
component) and composes the matrix through `00B64CE0`/`004134F0`. `+1Ch`
`BSP_NodeAnimator_EvaluateChannel` (00B77430) samples one channel with no channel bound.
Optimized animator (`00D62EB0`): `+18h` decodes a compact frame (`BSP_AnimationTrack_FrameAddress`
then the transposed copy `00B75DE0`); `+1Ch` returns 0.

`BSP_Node_ApplyAnimation` (00B6E150, node vtable `+24h`, RET 0Ch: group, time, recurse):
pose -> node vtable `+38h`; channel 10 -> node `+ACh` = clamp(1 - v, 0, 1); recurse into
children when byte `+134h` and the argument are set. The camera override `00B70DE0` adds
channel 9 -> FOV `+1C4h` = 2*atan(1/v) (`00BF8490` is CRT atan: its x87 fallback uses FPATAN at 00BF84FF) and aims
at the target node `+438h`. So the channel array holds at least 11 tracks: 0-8 TRS,
9 camera focal value, 10 transparency.

## Node queries (reconstructed)

`BSP_Node_FindChildByName` (00B6E750; 13 call sites) checks all direct children first
(equal length, `__stricmp`), then recurses child by child. `BSP_Scene_FindNodeByName`
(00B72270) is the same over the scene's top-level list `+0Ch`.
`BSP_Node_CollectDescendantsMatching` (00B6F9A0; 14 call sites) appends, post-order, every
descendant whose name contains the filter (`strstr`, case-sensitive; all when empty); cc9's
gunnery host already relies on that rule.

## Resource dump (segment 89, reconstructed)

`BSP_ResourceManager_DumpCsv` (00B811C0; caller `BSP_Game_TeardownSessionState`) builds a
string table: header `Name`, `Size`, `RefCounter`, `Size / RefCounter`; one entry per
resource in the manager's map `+14h` (`name +0Ch`, `size +40h`, refcount `+04h`), sorted by
size descending with `qsort` (comparator 00B7C920); each row is name, size, refcount and
`size / refcount` (integer division, no zero test). A `"%8d Name:%s"` line is formatted into
a stack buffer that the following code never reads (00B81421). The table is then handed with
the name `"ResourceDump_" + suffix + ".csv"` to `00BF0A30`, which returns 1 and writes
nothing, so this build never produces the file.

## Other named groups

- Camera: frustum classification of an AABB (00B71590) and point (00B714D0, unreferenced),
  renderer submission (00B700A0), hi-res screenshot tiles (00B708E0), BMP capture
  (00B848C0; its call site 004DBF45 lies outside the Ghidra body of BSP_Game_UpdateDeviceWaitScreen), render-to-targets setup (00B83F30), fields `+190h/+194h`.
- Generated-model material walkers over mesh elements matched by effect name
  (00B84FE0, 00B850F0, 00B85200, 00B85310; provisional names).
- Parser singleton base destructors for the group-params, mesh, skinned-mesh,
  matrix-indexed-mesh, camera and skinned-mesh-animation parsers (00B7D410..00B7D550).
- `BSP_DebugRenderFlags_*` (00B854D0, 00B85510, 00B85540): the 8-byte singleton built by
  `0051F520` (bytes +4/+5/+6 = 0/1/0); its purpose is provisional.

## Reconstruction records

`include/bsp/scene_graph_cc13.hpp`, `src/scene_graph_cc13.cpp` (namespace
`bsp::cc13_scene`): 00B76CE0, 00B75450, 00B75520, 00B75820, 00B75A20, 00B75C10, 00B75580,
00B75610, 00B75720, 00B77220, 00B77360 (pose inputs, not the matrix), 00B77430, 00B6E750,
00B6F9A0, 00B6E150 (visibility rule), 00B70DE0 (FOV rule), 00B811C0 (rows and file name).
Precision: x87 intermediates are modelled in double and rounded where the native code
stores floats; bit-exact agreement with the extended-precision original is not claimed.

## Open questions

- The meaning of node `+50h`, camera `+190h/+194h` and the debug flag bytes.
- Whether `00B72300`'s second name argument is really an exclusion.

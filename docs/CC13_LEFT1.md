# cc13-left1: leftover game bodies and STL tags

Packet `cc13-left1`. Code: `src/cc13_left1.cpp`, `include/bsp/cc13_left1.hpp` (bsp_core, wired to
nothing). Names are hypotheses, not recovered symbols. Status words follow AGENTS.md: these bodies are
reconstructed and build-tested, not ABI-compatible and not game-validated.

Duplicate checks run for every row: `bsp.py lookup` / `docs-for`, `git grep` of the address forms in
`src`, `include`, `tests` and `docs`, and a behaviour grep in the hosts (the trace strings, the
callee names, the property keys).

## Bodies

| Address | Name | Status | Notes |
| --- | --- | --- | --- |
| `005B8810` | BSP_AmbientMessage_PlaySound | reconstructed | `ambient_message_voice_construct_005b8810`. `__thiscall(ECX = object; 0Ch clip by value, entity*)`, RET 10h, EAX = object, SEH handler `00C7215B`. It builds the 30h-byte element that `005B83E0` (`voice_owner_stop_and_clear_005b83e0`, `src/cc13_front7.cpp`) walks at owner+64h: the voice slot at +0h (+0h..+10h zeroed, +14h not written) and the observer at +18h (vtable `00CE75CC`, byte +28h = 1, the entity at +2Ch through `00694A60`). It starts the clip with `00702CC0` into a temporary, assigns it with `005B82C0`, and destroys the temporary with `005B7FC0`. When `007027B0` reports nothing it traces "NOT FOUND". Otherwise it sets the volume `(2000.0 - |entity - camera|) / 2000.0`, or 0 below 0.25 (NaN kept), through `00702130`, and traces "played". The camera is `[[00E188A8]+19FCh]`, refreshed by `00B6DB70` when bit 2 of +5Ch is clear. No x87 beyond that quotient and no RNG. No cc9 host plays ambient messages (no match for the trace strings, the callees or "ambient" in `src`). No rel32 or absolute caller is known. |
| `00605630` | BSP_Math_DirectionAngleDeltas | reconstructed | `direction_angle_deltas_00605630`. `__fastcall(ECX = float* yaw, EDX = float* pitch; a.xyz, b.xyz by value)`, RET 18h; sole caller `006082D0` BSP_PlaneHudScreen_UpdateBombFire. Its arithmetic is instruction for instruction that of `00794750`: only the output stores and the prologue's `MOV EDI,EDX` differ once the two listings are normalised. So the body reuses `camera_direction_delta_00794750` (`src/cc13_camera_controllers.cpp`): yaw = `00438B10`(atan2(b.x, b.z), atan2(a.x, a.z)) and pitch = -`00438B10`(`0042CF10`(b.y), `0042CF10`(a.y)) over the normalised vectors, each 0 when an angle fails `00BFA4EC`. The FDIVRP and FMULP bytes are read (`D9 E8 DE F1`, `D9 C0 DE CA`). Uncertainty: the image stores yaw before the pitch calls, the reuse after them; the precision notes are those of `00794750`. No x87 of its own and no RNG. |
| `006D5220` | BSP_AirField_ReadHangarAndMarkerProperties | reconstructed | `airfield_read_hangar_and_marker_properties_006d5220`, the whole body (listing `006D5220..006D600E` read whole). `__fastcall(ECX = airfield)`, RET, SEH handler `00C81EEF`; `MAirfield` vtable slot +0A4h. Details follow the table. |

### 006D5220 in detail

Host check: no host runs this body.
- cc9's `SceneReaderBinding::instantiate_entity` (`game_hosts_scene_contents.cpp` ~2418) keeps only the authored kind-1 hangar names (`AirOpsDeck::hangars`, `air_operations.hpp` ~582) and resolves them later.
- cc9's `retain_airfield_runway` (~407) models `006D3C10`, not this routine.
- `docs/ARGON_SEG31_32_AIRFIELD_MARKER.md` skipped it.

The body first calls `0095E5B0`. It then branches on the kind of the scene record `[airfield+0C0h]+4h`:

- **Kind 3, the saved Lua data.**
  - A reader over `009238A0`'s LuaObject (`004425C0`) enters `_airField`.
  - The property pairs `{0, "runwayWidth"}` / `{2, &width}` and `{0, "runwayLength"}` / `{2, &length}` (`00BD6830`) carry the original field names. They go to `006BF0D0(airfield+72Ch, width, length)`.
  - Under `hangars`, while `00BD5EB0` finds index i - 1, the inlined resize grows the record vector (+830h data, +834h size, +838h capacity, 12-byte records) to i. The reader's vtable +10h then reads `entityID`, `entryPathID` and `exitPathID` as `{4, &ref}`. The two paths go through `007AC9D0`.
  - Then `gameUnit` (vtable +4h) is passed to `00836510([airfield+8C8h], reader)`, then vtable +8h, the leaves, and the inlined reader destructor (`00CE44FC`, `00441210`).
- **Kind 2, the compiled table.**
  - The count is `[[+0C0h]+8]+138h`. The inlined resize grows the vector to the count.
  - Each hangar has three ushort ids at `+13Ch`, stride 6, resolved through the global record map inlined from `00F89A0C`/`00F89A10`/`00F89A54`/`00F89A60`/`00F89AA8`.
  - The object is stored raw; both paths go through `007AC9D0`.
- **Kind 1, the scene property bag.**
  - `CaptainCameraPos` → +8CCh. Its default is the position + (0, 20.0, 0); the bytes `DC C1` give x + 0 and 0 + z.
  - `AirBaseCameraPos` defaults to (-30, 50, -100) through the world matrix +0CCh; `AirBaseLookAtPos` defaults to the transformed origin.
  - When the squared distance is below 1.0, camera.x gets +1.0 (NaN skips this).
  - Both points go into the local frame (`00414E10`). The matrix +888h is set to identity (`004134F0`), forward row +8A8h = look - camera, up row +898h = (0, 1, 0), then `0085DC80`, then translation row +8B8h = the local camera.
  - `Hangar %d` for i = 1..10 needs a type-6 section and an `Object` that names a kind-1Bh entity, plus part `006D2120(i - 1)`. The record is push_back'd with growth 2n + 2: object `006D3440(entity)`, `EntryPath`/`ExitPath` resolved through `00925A90` and `007AC9D0`. Then `00744A20(object; airfield, part)`.
  - Every part in [+348h, +34Ch) whose type is not 0Bh or 0Ch gets `00744A20` on the scene object named `Section_<type name [00E08138]>_<part+8h>`.
- **Common tail.**
  - `006BA690(+72Ch, 006CFA80())`.
  - For kind 3, `_airBase` is passed to `006BEF30`, and that reader's vtable is reset to `00CE374C` after the tidy.
  - Then `006BA670(+72Ch, 0.0f)`, `006D2980`, and `006BEE40([+7ACh], 0.0f)`.

Uncertainty:
- The kind-3 frame values and the section string headers are uninitialised in the image and zero here. A missing Lua key keeps the previous iteration's reference.
- The constants 20.0 and 1.0 are exact, so float operations match 24-bit x87; 53-bit precision is not modelled.
- No RNG.

## STL tags (no bodies)

Each was read from its listing to its RET and confirmed as an MSVC (VS2008) container instantiation.
None reads a game field or a game constant; the only image constants are SEH handlers. Tagged
`stl_shape` in `config/tags`, evidence `sys6_shape`.

| Address | Name | Shape |
| --- | --- | --- |
| `008DE6A0` | STL_SetPtr_Insert_008de6a0 | `_Tree::insert(const value&)` for a set of pointers: descend from the root (`[+4h]+4h`) while `_Isnil` (+11h) is 0, unsigned key compare (SETC / JNC), `_Dec` `008DA4A0` on the leftmost miss, `_Insert` `008DE010` (`ECX = tree`; result*, add-left, node, value*), and the pair {iterator, bool} written to the result, RET 8. Its callee `008DE010` is the `_Insert` helper (renamed below). |
| `00926670` | STL_list_BuyNodeHitRecord_00926670 | `list::_Buynode(next, prev, value)`: `operator new(68h)`, the two links stored behind the null checks, the value copy-constructed at node+8h by `009253A0` under EH state 0 (handler `00CA6D40`), RET 0Ch. The call is already a hook in `src/entity_mission_seg60_argon.cpp` (`stl_list_buy_node_hit_record_00926670`). |
| `00A17960` | STL_list_copy_construct_00a17960 | `list(const list&)`: `this+4h = 00A168A0()` (the head node), `this+8h = 0`, then `_Insert` range `00A17760`(this, begin, other.begin(), other.end()) under EH state 0 (handler `00CB1E20`), EAX = this, RET 4. |
| `00AA9320` | STL_PopHeap_WidgetByName_00aa9320 | `_Pop_heap(first = ECX, last = EDX, ...)`: value = last[-1], last[-1] = *first, `_Adjust_heap` `00AA8120`(first, 0, last - 1 - first, value, predicate), RET 8. |

## Renamed misnamed throw-site rows

Both rows came from the template inventory (tag `e0030`, `stl_throw_site`). Their names are
corrected in place in `config/tags/008d0000.jsonl` (now `stl_shape`), with names rows recording
the old names, and in Ghidra. The neighbours `008DE200` and `008DE5F0` look like genuine throw
stubs and are unchanged.

| Address | Was | Now | Evidence |
| --- | --- | --- | --- |
| `008DE010` | STL_xlen_throw_008de010 | STL_Tree_Insert_008de010 | VS2008 `_Tree::_Insert(result*, add_left, where, value*)`, `ECX = tree`, RET 10h. The inlined length_error check comes first (`00408720`, `00411700`, `00BF6885`), then `_Buynode` `008DB730`, ++size, the links, and the rebalance (`008DB100` / `008DB150`). Its only caller is `008DE6A0` at `008DE6FE`; rev53U confirmed it. |
| `008DE340` | STL_xlen_throw_008de340 | STL_Tree_EraseIterator_008de340 | VS2008 `_Tree::erase(iterator)`, `ECX = tree; result*, where {container, node}`, RET 0Ch. If where is the head (`_Isnil` +11h), it throws "invalid map/set<T> iterator" (`00411700`, `00BF6885`). Otherwise: ++where (`008DA530`), the unlink with the `_Min` / `_Max` fix-ups (`008DA440` / `008DA460`), the rebalance rotations (`008DB100` / `008DB150`), `operator delete` `00BF65AC` on the node, `if (size > 0) --size`, and the iterator written to the result. Its only caller is `008DE7E0` (STL_Tree_EraseRange). |

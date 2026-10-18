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

## STL tags (no bodies)

Each was read from its listing to its RET and confirmed as an MSVC (VS2008) container instantiation.
None reads a game field or a game constant; the only image constants are SEH handlers. Tagged
`stl_shape` in `config/tags`, evidence `sys6_shape`.

| Address | Name | Shape |
| --- | --- | --- |
| `008DE6A0` | STL_SetPtr_Insert_008de6a0 | `_Tree::insert(const value&)` for a set of pointers: descend from the root (`[+4h]+4h`) while `_Isnil` (+11h) is 0, unsigned key compare (SETC / JNC), `_Dec` `008DA4A0` on the leftmost miss, `_Insert` `008DE010` (`ECX = tree`; result*, add-left, node, value*), and the pair {iterator, bool} written to the result, RET 8. The callee `008DE010` is named `STL_xlen_throw_008de010` in the ledger, but here it is the `_Insert` helper. |
| `00926670` | STL_list_BuyNodeHitRecord_00926670 | `list::_Buynode(next, prev, value)`: `operator new(68h)`, the two links stored behind the null checks, the value copy-constructed at node+8h by `009253A0` under EH state 0 (handler `00CA6D40`), RET 0Ch. The call is already a hook in `src/entity_mission_seg60_argon.cpp` (`stl_list_buy_node_hit_record_00926670`). |
| `00A17960` | STL_list_copy_construct_00a17960 | `list(const list&)`: `this+4h = 00A168A0()` (the head node), `this+8h = 0`, then `_Insert` range `00A17760`(this, begin, other.begin(), other.end()) under EH state 0 (handler `00CB1E20`), EAX = this, RET 4. |
| `00AA9320` | STL_PopHeap_WidgetByName_00aa9320 | `_Pop_heap(first = ECX, last = EDX, ...)`: value = last[-1], last[-1] = *first, `_Adjust_heap` `00AA8120`(first, 0, last - 1 - first, value, predicate), RET 8. |

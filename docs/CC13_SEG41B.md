# Segment 41/33 skipped functions (cc13-seg41b)

The functions the cc13-seg41 and cc13-seg33b packets skipped (docs/CC13_SEG41.md "Not
reconstructed", docs/CC13_SEG33B.md "Skipped"). Code: `src/cc13_seg41b.cpp`, declarations and
ABI notes: `include/bsp/cc13_seg41b.hpp`. New C++ interfaces over the image's Win32 layouts;
reconstructed and build-tested only, not ABI-compatible, not fixture-tested, not wired into the
game, not game-validated. Names are hypotheses, not recovered symbols.

Every call the image makes is either a direct call (0073E780 and 0073E7C0, from
`band_0067_cc13.hpp`) or a named host hook invoked at the same point with the same arguments.
x87 operand direction was taken from the opcode bytes, never from the Ghidra listing text, and the
x87 stack was tracked across each branch; the precision model is the one in
`band_0067_cc13.hpp` (an FSTP to a float slot is a float cast, unspilled intermediates are doubles).

Search first (rule 1): none of the seven addresses had an implementation in `src/` or `include/`
(the only hits were the 00742C70 decision in `land_and_structures.hpp` and the 00748CC0 hook in
`cc13_seg33b.hpp`). The gameplay lane hosts neither the fort load ("_landFort", "gazdi") nor the
land-vehicle InitAll (no reference to 00CFFDE0's slot body, 00864580 or 0074D780 in
`game_hosts_units.cpp`), so both were reconstructed here. No function in this packet draws from the
shared RNG 00BD2F10.

| Address | Function | Original ABI | Behaviour (as reconstructed) |
|---|---|---|---|
| 00742C10 | `land_convoy_post_load_init_00742c10` | __thiscall(ECX convoy), RET | 0077FAD0; +3C0h = +3B4h = 0.0f, +3C4h = -1e10f ([00CE4ADC] = D01502F9h); with a path object +344h: +3C8h = (007ACA30 entity +3Ch != convoy +3Ch); +3A4h = +3Ch. self["Members"] = {} (00927B40, 00B67580 with a NativeStr key), then for i < +354h * +358h (re-read each pass): nil at i+1 (00B664E0) for an empty slot, else [[+3A4h]+3D0h]->vtable[40h]() becomes the member's +4A4h node parent (00B6E680) and the member's Lua self goes to i+1 (00B666C0); each vector access is checked (00BF6713). self["Class"] = {} and its Type = "LandVehicle" (00B67800, 00B66790). With a path entity whose vtable[5Ch](1) is true: self["Path"] = the entity's Lua self (00B675D0). Then 00742400, (+310h)->vtable[0Ch](), vtable[148h](1FFh, 8), and the "Members" object is destroyed (00B67700). |
| 00748CC0 | `land_fort_load_saved_state_00748cc0` | __thiscall(ECX fort), RET | 0095E5B0; with saved data +C0h of type 3: a Lua reader over 009238A0's object (004425C0), enter "_landFort" (00BD8E20); if HasKey "gazdi" (00BD5EB0): enter it, 007453C0("ID") into +738h, ReadField("section", int*) (00BD6830), +73Ch = 0 for -1 else the ADDRESS 0043F8E0 returns for element [section] of the master's +344h pointer vector, leave (00BD7A20). Then reader vtable[4]("gameUnit"), 00836510(director +74Ch, reader), reader vtable[8](), leave, destroy (00441A20). Always: 00809BC0(ECX fort+72Ch)(fort, +30h, +C0h). The read side of 007455F0. |
| 0074D800 | `land_vehicle_init_all_0074d800` | __thiscall(ECX vehicle), RET | 00955420; with saved type 1, +50h set and session [00E188A8]+1FE4h != 2: operator new(558h) (00BF681B), 00864580 into +6DCh, gunnery vtable[4](vehicle+310h) with no null check. When class(+538h)+14Ch is set: per 0Ch anchor in class+140h (count +144h) an identity matrix with the anchor as its translation, 008687C0(&out, parent +4A4h, definition +14Ch with an added reference, matrix, 1, 0, 0), 0074D780(+72Ch, &out) when out is set, then out is released (InterlockedDecrement, vtable[0] at zero). |
| 0073EBA0 | `mesh_decal_clip_triangle_to_box_0073eba0` | (ECX unused)(tri, matrix, float max, float* out), RET 10h, AL | Face normal normalize(cross(p1-p0, p2-p1)) (004F9B30, 00419440, 1/len or 0) to tri+70h; the vertices through 004142E0 with z = 0; rejects when all x (or all y) are < 0 or all are > 1.0f (COMISS; unordered never rejects); up = normalize(cross(t1-t0, t2-t1)), rejects when 0 >= up.z. Per edge (t0, t1, t2 in turn): m = normalize(cross(edge, up)); dist = float(m.(0.5,0.5,0)) - float(m.t_i); dist > max rejects, dist <= 0 counts. Three counts write -face (as -0.0f - face) to out. Then corner planes through t_i with normal normalize(t_i - centroid) (centroid = sum / 3.0 double): 0073E780, 0073E7C0 against (0.5, 0.5, 0) > max rejects. Returns 1. |
| 0073F1B0 | `triangle_sphere_contact_0073f1b0` | __stdcall(p0, p1, p2, centre, float r, float* out), RET 18h, AL | Face normal as above from cross(p1-p0, p2-p1); dist = float(face.centre) - float(p0.face); magnitude = dist when dist > 0 else -0.0f - dist (the FLDZ the length test leaves on the stack is the zero); magnitude > r rejects. Edge planes: m = normalize(cross(edge, face)) through the edge's first vertex, dist > r rejects (one-sided). Then out = face, and only then the corner planes through p_i with normal normalize(p_i - centroid) (0073E780, 0073E7C0 against the centre) > r reject. Returns 1. |
| 0073F6E0 | `mesh_decal_collect_tree_triangles_0073f6e0` | __thiscall(ECX ctx)(node, query), RET 8 | Visits child +1Ch then +20h unless, on some axis, q.min (+68h) > child.max and !(child.min > q.max (+74h)). A leaf (both children null) reads, for i in [+18h, +18h + +1Ah), triangle [[query]+18h][i], its three u16 indices at [query+64h] + tri*6, the three positions (004768D0, stream query+60h), stores them into record [query+8] of query+4 when the count is < 200h, and on 0073EBA0(record, query+0Ch, [query+4Ch], [query+5Ch]) acceptance the three normals (0070FDB0) and count++. ctx is only passed through. |
| 006F3CC0 | `command_building_debug_draw_id_006f3cc0` | __thiscall(ECX building), RET | With 0051F460 +2Ch set (debug overlay) and the camera ([E188A8]+19FCh, 00B6DB70 when +5Ch bit 1 is clear) within 3500 units (squared distance < [00CFAE80]): per garrison member (+794h, count +798h, 00414DB0 when +C8h is clear) a wire sphere at (x, 2.0f, z) of radius 5.0f (00860BE0, colours 00E19A38 / 00E19A4C initialised once under guards 00E19A48 / 00E19A5C) and an arrow to position + 50.0 * 006AC260's flat forward (0085FFF0, colour 00E19A38 by value). Then 0043A660(+FCh, out, 2, 1) == 7 prints "ID: %d" with +174h through 004C7070. |

## Simplifications and uncertainties

- 00742C10, 00748CC0 and 0074D800: exception-unwind paths (EH states, destruction on a throw) are
  not modelled. The 14h-byte LuaObject and LuaReader stack objects are opaque `LuaSlot` buffers.
- 00748CC0: when "section" is absent, the image's int slot still holds a stack address written
  earlier in the frame (00748D67), so 0043F8E0 is called with that value; the C++ starts the slot
  at 0, so for an absent key it asks for element 0. Saved games always write the key with "gazdi".
- 00748CC0 calls `cc13_seg41::save_reader_read_observer_ref_007453c0` (src/cc13_seg41_land.cpp)
  directly; `LandFortLoadHost` derives from its `SaveReaderHost`, whose `read_10` is the reader's own
  vtable[10h]. Until cc13-fix5 (2026-10-16) the call went through a hook because that function
  misread the callee (it wrote 4 through the key tag); it now does
  `reader->vtable[10h](key {a1, a2}, {4, &local}); *a3 = local`.
- 0074D800: the definition reference is taken with InterlockedIncrement before 008687C0 (the callee
  owns it); a null `operator new` result still reaches the gunnery vtable[4] call, as in the image.
- 0073EBA0 / 0073F1B0 / 0073F6E0: dead in this image (0073F6E0 is referenced only by itself, 0073F1B0
  and 0073EBA0's sole caller are unreferenced). The vector helpers 004F9B30, 00419440, 00419510,
  004142E0 and the vertex readers 004768D0 and 0070FDB0 are host hooks (their existing
  reconstructions take CRT-access or partial-projection parameters). x87 intermediates follow the
  53-bit model; the record copies (FLD/FSTP of floats) are plain float assignments.
- 006F3CC0: the function-local colour statics are passed in as `DebugIdColourStatics`.

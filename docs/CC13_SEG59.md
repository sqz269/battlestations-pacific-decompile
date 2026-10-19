# CC13_SEG59: segment 59 remainder (packet cc13-seg59)

Segment 59 is `008EE670..00922240`: the gunner bots, the torpedo, depth-charge and tail-gunner bots,
and the pilot bot. Code is in include/bsp/cc13_seg59.hpp and src/cc13_seg59.cpp (marker
`cc13_seg59`). Nothing is wired into `bsp_game`. Names are hypotheses, not recovered symbols.

## Census (index of 2026-10-19, main e834f4958)

- 322 functions in the range had no reconstruction record. Funclets that have a parent in the
  index are not counted.
  - The classifier tagged 313 of them as library or compiler code: stl_shape 152,
    stl_instantiation 94, stl_manual_cc13 26, scalar deleting destructors 19, xlen throw sites 15,
    adjustor thunks 3, vector deleting destructors 2, array-constructor helper 1, stl_probable 1.
  - The other 9 are six `Unwind@` funclets with no parent in the index and three one-JMP thunks.
- Undefined code: 542 chunks (11 KB) in the range, none of them referenced (`bsp.py undefined
  --direct`).
- The bot gameplay of the segment is hosted in game_hosts_gunnery.cpp and was audited earlier
  (docs/CC13_SEG59C.md, CC13_GPR_40, CC13_VER_23, GUN_BOT_TICKS, GUN_BOT_REMAINDER). There is no
  open game row there and no URGENT item.
- The tags were checked against the bytes. Six tagged rows are game code: they store a class
  vtable or run game callees. These six are reconstructed below.
- The other 316 rows are bound by ledger record only (`library_instantiation_out_of_scope`).
  Each record's evidence carries its tag, size, instruction count and callee list.
  - In the STL rows, the only vtable stores are exception objects on the stack (00D6923C,
    00D69260, 00D6926C, the bad_alloc / length_error throws), and the only float op is a
    map-subscript default (0090E740).
  - The scalar deleting destructors 008EF080, 008F4170, 008F4D70, 008F4DA0, 0090D340 and 0091D760
    wrap destructors that have their own records (008EF030, 008F4160, 008F4D50, 008F4D60, 0090D2F0,
    0091CCD0). The ones with vtable stores inline those destructors' bodies.

## Routines

| Address | C++ | ABI | Evidence |
|---|---|---|---|
| `00904C40` | `world_destruct_00904c40` | `__thiscall(ECX = world)`, RET | See the World destructor section below. Tagged CG_vector_deleting_dtor. Its one caller is `004CB0B0` BSP_World_ScalarDeletingDestructor. |
| `008F4160` | `scene_property_record_map_destruct_008f4160` | `__thiscall(ECX = object)`, tail JMP | `MOV [ECX],00D162C4; JMP 008F3F30`. Tagged CG_adjustor_thunk. Called by the Unwind funclets `00CA4B80` and `00CA4AA0`. |
| `008F4D50` | `enum_symbol_map_destruct_008f4d50` | same | `MOV [ECX],00D162C0; JMP 008F4B60`. Called by Unwind `00CA4AD0`. |
| `008F4D60` | `property_library_map_destruct_008f4d60` | same | `MOV [ECX],00D162C8; JMP 008F4C50`. Called by Unwind `00CA4BC0`. |
| `0091D640` | `scoring_manager_construct_0091d640` | `__thiscall(ECX = manager)`, RET, EAX = manager | See the scoring manager section below. Tagged CG_array_ctor_helper. `004E3E74` allocates 14B8h bytes, constructs, and stores the result at game+21A0h. |
| `0091CCD0` | `scoring_manager_destruct_0091ccd0` | `__thiscall(ECX = manager)`, RET | See the scoring manager section below. Tagged CG_vector_deleting_dtor. Its one caller is the scalar deleting destructor `0091D760`. |

### The World destructor (00904C40)

The body runs `00904C40..00904E49`, and the Ghidra listing now ends at the `RET`. Its steps, in
order:

1. Store vtable `00CE7784`. Set byte +4ACh = 0 and byte `[00E0AF20]` = 0.
2. Call `00926D90(e, 7)` on every mission entity. The walk starts at `[[world+8h]]` and follows
   the +44h link, which is read after each kill.
3. `00874D00(CL = 0)`.
4. Call `00922FD0` on each entity in the `[[world+4h]]` chain (+38h links) that has +5Eh set,
   +6Ch clear, and either no parent (+3Ch) or a parent whose +5Eh is clear.
5. `00874D00(0)`.
6. Three identical rounds:
   1. Call vtable `[DCh](0.0f)` on every entity in the chain whose +5Ch is set.
   2. `00904600(world, 0.0f)`.
   3. `00874D00(0)`.
7. `_free` the +8h header, then the +4h header, zeroing each pointer.
8. For the object at +4A8h: `InterlockedDecrement([obj+4h])` (import `00CE2220`), and call
   vtable `[0]` if the result is zero. Then +4A8h = 0.
9. Clear the std::list at +4B4h: the sentinel's next and prev point back to itself, +4B8h = 0,
   and every node is freed. Then free the sentinel and set +4B4h = 0.
10. `00BF7C6E(+18h, 0Ch, 61h, 004C2D30)`.
11. `004BF8E0(+0Ch)`.

### The scoring manager (0091D640 and 0091CCD0)

Constructor `0091D640`, in order:

1. Store vtable `00D18ACC`.
2. `00BF7CD1(+4h, 284h, 8, 0091CE90, 00593570)`.
3. Build four `_Tree` heads. For each, the head's +4h/+0h/+8h point to the head and the size is 0:
   - +1428h: `00582840`, isnil byte +15h.
   - +1434h and +1440h: `00907CF0`, +1Dh.
   - +1488h: `0063B110`, +11h. Before this one, +147Ch and +1480h are set to 0.

Destructor `0091CCD0`, in order:

1. Store vtable `00D18ACC`.
2. +1488h: erase(begin, end) with `0063FB30`, free the head, zero head and size.
3. If +1480h is non-null: `00419CC0()` returns the pool, then `pool->00BD1510(+1480h, [+147Ch]+1,
   1)`.
4. +1440h and +1434h: `00911B30`, then free and zero as above.
5. +1428h: `0058AE10`, then free and zero.
6. `00BF7C6E(+4h, 284h, 8, 00593570)`.

## Uncertainty

- **EH states.** The SEH frames and their state stores are not modelled. The EH vector
  iterators are host hooks that take the image's arguments; the constructor and destructor
  addresses they pass are fixed and named in the hook comment.
- **Layouts.** The world's entity chain links (+38h, +44h) and flags (+5Ch, +5Eh, +6Ch) follow
  the listing and docs/WORLD_DEFERRED_DESTROY.md. The scoring manager's tree value types are not
  established.
- **No null checks.** `00904C40` dereferences the +8h and +4h headers without a null check
  (00904C73, 00904C9B), and the C++ does the same.
- **+1480h after release.** `0091CCD0` returns the pool block but does not zero +1480h. The C++
  does not zero it either.

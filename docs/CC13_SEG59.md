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
  - Why those six are bound while 008F4160, 008F4D50, 008F4D60 and 0091CCD0 are rebuilt: the
    rebuilt rows are the out-of-line class destructors themselves, the one copy every caller and
    unwind funclet reaches, while each bound wrapper only repeats that body inline in front of
    an optional _free; rebuilding the copy would duplicate the same steps, so its record cites
    the out-of-line row instead.
  - 008F7D00, 008F8500 and 008FB4B0 are single JMPs into game code (00BD3050, 00695870,
    00439000). Their status is `existing_recorded_thunk`, not a library binding, and 008F7D00 /
    008FB4B0 carry `thunk_` names instead of their targets' names (cc13-census9).

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

## Ghidra names replaced

Each annotate run logs the names it replaced:
- `local/ghidra-annotations-20261019T072917Z.json`: 00904C40 CG_vector_deleting_dtor_00904c40,
  008F4160 CG_adjustor_thunk_008f4160, 008F4D50 CG_adjustor_thunk_008f4d50, 008F4D60
  CG_adjustor_thunk_008f4d60, 0091D640 CG_array_ctor_helper_0091d640, 0091CCD0
  CG_vector_deleting_dtor_0091ccd0.
- `local/ghidra-annotations-20261019T075320Z.json`: the 008F7D00 and 008FB4B0 thunk renames.
- `local/ghidra-annotations-20261019T075059Z.json`: the 00749780 thunk rename (census9).

## Torpedo-bot audit notes (008FFF20 against game_hosts_gunnery.cpp)

Fresh read of 008FFF20..0090044A from the bytes. No URGENT mismatch. The notes below are the
places where the image does something the host does not model, or where a gate is inert.

- **008FFF4B, same-side target drop.** `00521EA0(ECX = bot+38h)` resolves the target. The target
  is kept only when it exists, its +5Dh byte is clear, the gun (bot+50h) has a parent (+3Ch)
  whose +5Dh is clear, and `00803510(ECX = gun+54h, EDX = target+54h)` returns non-zero (at
  008FFF71 ECX is still the gun; [gun+3Ch] serves only the parent +5Dh test);
  otherwise `bot->vtable[38h](0)` clears it (008FFF8A). The host has no 00803510 check, so a
  target that test rejects stays targeted in the host. Not tied to a live mission path by the
  audit; no RNG.
- **0090010E, the shallow gate.** After the 00900101 store of [00CE54A0] to bot+5Ch, the image
  calls `008527E0(ECX = bot+64h)` and bails on false, but only when bot+64h is non-zero.
  Census of writers of bot+64h: the class vtable 00D182B0 (20 slots) and its constructor
  008FF260 (the only function holding the vtable immediate) were scanned for stores to
  [reg+64h] (`local/census9_b64.py`). The only writer is 008FF310
  BSP_GunBot_AttachTorpedoClass: bot+58h = bot+50h (the payload), bot+5Ch = 0.0f, then bot+64h =
  payload when `payload->vtable[5Ch](8)` (IsKindOf 8) is true (008FF33F), else 0 (008FF34A).
  The gun-class IsKindOf ids are 24h/22h/20h/1Eh/4/2/1 plus [this+C4h], never 8, so bot+64h
  stays 0 for every gun payload and the gate never runs. The only immediate store of 8 to
  [reg+C4h] in the image is 00852F96 in BSP_SubmarineUnit_Construct, so the gate is for a
  submarine payload; the 85 register stores to +C4h are untraced. Scope: writers of bot+64h
  outside the class's own vtable and constructor were not scanned.
- **00900080, the unit+184h gate.** gun = bot+50h; when gun+1ACh (the seat) is not 8,
  `00927F10(seat)` must return true. Then the walk from the gun up the +3Ch parents stops at the
  first object answering IsKindOf(5), the unit (none: return at 00900069), and the tick bails
  (00900985) when the unit's +184h byte is set. Among units that carry guns, unit+184h is
  set only by an accepted player role-1 take (00780214; other byte-1 stores at +184h, 0048BCD7 /
  0048BE9B on LandingTroop members and 00710B73 on UnitPartInstance, are other classes; packet cc9_player_role_bookkeeping, `kPlayerRoleBookkeepingBound` in
  game_hosts_units.cpp). The audit's reading is that for seats other than 8 the 00927F10 role
  check already covers the player-held case, so the +184h byte adds a case only for a seat-8
  gun on a unit whose role 1 the player has taken. That reading of 00927F10 was not re-proved
  here.
- Also noted: WaterTravelSpeed 0 falls back to the muzzle speed (a labelled host substitution),
  and the result of 008FE140 (009000D8) is discarded. 0090044A..0090097B was not re-read.

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

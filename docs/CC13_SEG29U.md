# cc13-seg29U: segment 29's leftover functions

Segment 29 (00696470-006BEB10) had 71 functions with no reconstruction record and no library
tag (the list in the cc13 lead tree's `local/cc13_undone_seg29.txt`).

| Group | Count | Outcome |
|---|---|---|
| STL template instantiations | 32 | Tagged `stl_shape`, action `reviewed` (name kept). Bind-only. |
| `Catch_All@` EH funclets | 9 | Tagged `stl_shape`, action `reviewed`. They bind with their parents. |
| cc9 landing approach, 006AC000-006ACFFF | 3 | Reconstructed in the second pass (cc9 approval) |
| cc9 air ops, 006BA620 and up | 27 | Reconstructed in the second pass (cc9 approval) |
| Non-gameplay bodies left to reconstruct | 0 | |

The first pass changed only tag rows (`config/tags/00690000.jsonl`, `006a0000.jsonl`,
`evidence.json`) and this document. In the second pass cc9 approved the 30 cc9-lane bodies, and
they are reconstructed in `src/cc13_seg29U.cpp` and `include/bsp/cc13_seg29U.hpp` (see "Second
pass" below). That code is new and standalone: nothing is wired into a game host.

## Ownership

The lanes come from `tools/raw_ownership_cc13.py`:
- `00696470-006BA61F` is non-gameplay (ng). Every row of this list in that span is STL or a funclet.
- `006AC000-006ACFFF` is cc9 (the ship AI landing approach and command-building landing paths).
- `006BA620-006CFFFF` is cc9 (air ops).

The STL row 006ACD80 `STL_List_PushBack_006acd80` sits inside the cc9 span. It was tagged with
the rest of the STL rows, because a tag is not a reconstruction.

## Library rows (bind-only)

The 32 STL rows are the bodies of `std::_Tree`, `std::vector` and `std::list` templates. Each
one's ledger-name evidence (docs/CC13_BAND_0067_0073.md) describes the template shape. The
rows fall into these groups:
- head-node constructors: 0069DCA0, 0069E190, 0069E1C0, 006A20A0, 006A2C40, 006AD5D0, 006AFE50;
- iterator pre-decrement: 0069C570, 0069D0C0;
- tree assign (erase then copy when distinct): 006A1E30, 006A2470, 006A3870, 006A4470,
  006A46D0, 006A4C30;
- tree clear and insert: 006A1260, 006A1520, 006A41A0, 006A49F0, 006A5EF0;
- vector tidy and range helpers: 0069FB50, 006A0B50, 006A13B0, 006A1D30, 006A2200, 006A2240,
  006A2270, 006A23F0, 006A3180, 006A31B0, 006A6BB0;
- list push-back: 006ACD80.

The user ruling of 2026-10-16/17 binds library bodies by default. No game call site in this
segment needs a non-standard ABI from any of these rows, so none were reconstructed.

The nine funclets are 0069F9BE, 006A00F6, 006A1322, 006A22CB, 006A239E, 006A29A1, 006A3E9E,
006A3F27 and 006A4B2E. Each is a `catch(...)` cleanup of a neighbouring STL range or insert
body. Spot checks of 0069F9BE, 006A22CB, 006A3E9E and 006A4B2E found two patterns:
- a call to `_free` 00BF65AC, then a rethrow through `__CxxThrowException@8` 00BF6885 with null
  arguments;
- at 006A22CB, `STL_Vector_Tidy_0069e9b0` and then the same rethrow.

## Second pass: the 30 cc9-lane bodies

These were approved by cc9 on 2026-10-18. They are reconstructed in `src/cc13_seg29U.cpp` /
`include/bsp/cc13_seg29U.hpp` (namespace `bsp::cc13_seg29U`, host interface `Seg29UHost`) and
are not wired anywhere.

Rule 1 found no faithful body on main for any of them. Four have cc9 host models in
`game_hosts_units.cpp`; the comparison is under "Adoption notes" below. Every routine works on
raw image bytes. Each call the binary makes is a named host hook at the same point, with the
same arguments.

Some x87 chains (006BC5E0, 006BC960, 006BCA80) are modelled at the main thread's 24-bit
precision, with each operation rounded to float, per docs/CC13_FPU_PRECISION.md. If they run at
53 bits, they can differ only by a double rounding. FCOS and FSIN in 006BCA80 are modelled as
`std::cos` / `std::sin` in double, then rounded to float.

### Landing points (amphibious troop landing, not carrier recovery)

| Address | Function | ABI | Notes |
|---|---|---|---|
| 006AC370 | `landing_point_update_paths_006ac370` | `__thiscall(ECX = landing point; ship point) RET 4` | Called only from `BSP_LandingShip_Update`, at 0074B228. Walks the list at +1FCh, where node +8h is an 18h record and record +0h is the path entity. It refreshes the entity's world matrix: 00414DB0 on the parent, 00413920 when there is a parent, 004134F0 into +CCh, then +C8h = 1 and +10Ch = 0. Next it builds the 00B63D50 inverse and transforms the ship point (004142E0). The point goes to the path object at entity+1E4h through vtable +04h(0, point). 007AF150 runs unless +1FCh is set, then 0048CA60(ECX = [game+21D0h]; record, 1). The three checked-iterator tests are kept. |
| 006ACB40 | `landing_point_destruct_006acb40` | `__fastcall(ECX)` RET | Four vtable stores. `_free` of each record (and the field zeroed), then two inlined list clears, `_free` of the head and +200h = 0. The callback owner at +1E4h follows: vtable 00CE74FC, then 006952A0 when +1F8h is set, then 00695870. Last comes 009287B0. |
| 006ACDC0 | `landing_point_read_paths_006acdc0` | LandingPoint vtable slot A4h (absolute reference at 00CE9184), `__thiscall` RET | 009295B0 first. For i = 1..3 it builds the key "Paths"+i (004260B0, 0041DD40, 00BF7680, 004261A0, with the inlined pool releases). It looks the key up in `[[this+C0h]+8h]`, wants a type-6 group with "Path", and resolves "Path", "PathEndZ", "PathEnemy" and "PathEnemyEndZ" through 00925A90 with IsKindOf(47h). The result goes into an 18h record (operator new, +10h and +14h zeroed, no null test on the stores) pushed with 006ACD80. A missing optional key leaves its dword as operator new returned it. |

None of the three reads or writes plane +900h or the land-final touched byte (009B1EF2); team-lead
had that note before the code.

### Air ops

| Address | Function | ABI | Notes |
|---|---|---|---|
| 006BA670 | `air_ops_block_tick_006ba670` | `__thiscall(ECX = block; float dt) RET 4` | `[block+3Ch]`->vtable +08h(dt) when set. |
| 006BC030 | `entity_kind_is_1_or_2_006bc030` | `__thiscall(ECX) -> EAX` | The vtable +0Ch kind is 1 or 2. `gameplay_seg52_cc13.cpp` has only a `kind_1_or_2(int)` helper for the inlined copies. |
| 006BC120 | `air_ops_block_owner_missing_006bc120` | `__thiscall(ECX = block) -> EAX` | `[block+7Ch]` null or its +5Dh set. |
| 006BC3E0 | `do_launch_slot_message_construct_006bc3e0` | `__thiscall(ECX) -> EAX` | 0075B430(89h), +18h word / +1Ah / +1Ch cleared, +4h = 1, vtable 00CF85F0. |
| 006BC5E0 | `squadron_target_within_range_006bc5e0` | `__thiscall(ECX = squadron) -> AL` | Moveto (0071BE40 == 00E08F68), then the pose refresh when +C8h is clear. d = +FCh..+104h minus the target position (0071EB60, 006F7DD0). The result is true when float(class+268h × 1.2 [00CEC160 double]) > \|d\| (0042B2F0); unordered gives false. |
| 006BC690 | `air_ops_slot_record_destruct_006bc690` | `__thiscall(ECX)` RET | vtable 00CF6564, 00653400(+38h), 00653390(+14h). |
| 006BC6F0 | `air_ops_assign_slot_class_006bc6f0` | `__thiscall(ECX = slot; class, count) RET 8` | A changed class sets +4h and +10h = class+134h (0 for null). +8h = count always. |
| 006BC730 | `air_ops_slot_refresh_airborne_006bc730` | `__thiscall(ECX = slot)` RET | +8h = 007EE5C0(+28h), or 0 without a squadron. A zero sets +2Ch = 1. |
| 006BC800 | `air_ops_slot_squadron_moving_out_of_range_006bc800` | `__thiscall(ECX = slot) -> AL` | Moveto and not 006BC5E0. |
| 006BC870 | `buffered_callback_owner_destruct_006bc870` | `__thiscall(ECX)` RET | 00BF6989 on +14h when set (not cleared), then a tail jump to 00695870. |
| 006BC890 | `air_ops_block_runway_end_point_006bc890` | `__thiscall(ECX = block; out) RET 4 -> EAX = out` | The traffic object's vtable +44h(out), else the .bss vector 00F87574. |
| 006BC8E0 | `air_ops_route_do_launch_slot_006bc8e0` | `__thiscall(ECX = block; slot) RET 4` | A stack 89h message with +20h = slot, then 0077C2A0(ECX = [block+7Ch]; message, 1, 0). |
| 006BC960 | `air_ops_holder_refresh_touchdown_006bc960` | `__thiscall(ECX = holder)` RET | Sets T. For a mother ship (IsKindOf(9)) T = (0, 0.5, float(h + 10)). Otherwise T = (max0(float((B0h - block vtable +10h) × 0.5)), 0.5, float(h + 35)), with h = float(-B4h × 0.5). |
| 006BCA40 | `air_ops_holder_touchdown_world_006bca40` | `__thiscall(ECX = holder; out) RET 4 -> EAX = out` | T through the frame +8h. |
| 006BCA80 | `air_ops_holder_path_point_006bca80` | `__thiscall(ECX = holder; out[4], float t) RET 8` | Covered by the adoption note below. |
| 006BCCE0 | `air_ops_holder_to_local_006bcce0` | `__thiscall(ECX = holder; out, point) RET 8 -> EAX = out` | The point through the inverse at +48h. |
| 006BCD80 / 006BCE60 | `air_ops_block_hold/release_landing_record_*` | `__thiscall(ECX = block; plane) RET 4` | Under the block lock (+8h, with the +18h count), **every** record whose +0h is the plane gets +0Ch = 1 / 0. The checked-index tests are kept. |
| 006BCF40 | `air_ops_register_party_plane_stock_006bcf40` | `__thiscall(ECX = scene bag)` RET | "Party" +0Ch (no null test). For i = 1..12, "PlaneStock %d" (00467310) leads to a type-6 sub-bag, whose "Type" +0Ch class (no null test) goes to 0095BA60(class, party) and then 0046BE90(ECX = [00E18680]; class). |
| 006BCFD0 | `air_ops_block_read_queue_record_006bcfd0` | `__thiscall(ECX = block; index, out) RET 8 -> AL` | Under the lock, a non-negative in-range index copies 006BA7B0's record into out. The +0Ch and +0Dh bytes cross over. |
| 006BD080 | `air_ops_find_landing_assignment_006bd080` | `__thiscall(ECX = block; plane, out) RET 8 -> AL` | Under the lock, the first record for the plane is copied the same way. |
| 006BD230 | `air_ops_slot_state_from_command_006bd230` | `__thiscall(ECX = block; slot, command) RET 8` | Answer: command 0 gives 3 or 2, command 1 gives 1 or 2, other commands give 0. A slot is ready when ((state 1 or 5) and +8h > 0) or state 4. The stack 82h message gets +20h slot, +24h answer, +28h = the sender's +174h word (sender = owner vtable +114h, then its vtable +18h), +2Ch = 9 and vtable 00CF64C4. It is sent through 0077C2A0(message, 0, 0). |
| 006BD620 | `air_ops_slot_check_squadron_handle_006bd620` | `__thiscall(ECX = record) -> EAX` | Resolves the +34h handle through the two-bank registry (00F89A10 split). A dead entity (+5Dh or +5Eh set) or a null one sets +24h = 5, clears the handle and returns null. |
| 006BD800 | `set_slot_message_construct_006bd800` | `__thiscall(ECX = message; block, slot) RET 8 -> EAX` | 81h: +20h slot, vtable 00CF8624, +24h = 0095B9C0(class), +28h / +2Ch from slot +8h / +10h. |
| 006BD9E0 | `stock_item_message_construct_006bd9e0` | `__thiscall(ECX = message; class, value) RET 8 -> EAX` | 85h: vtable 00CF8638, +20h = 0095B9C0(class), +24h = value. |
| 006BDAC0 | `appear_ready_plane_message_construct_006bdac0` | `__thiscall(ECX = message; plane) RET 4 -> EAX` | 86h: vtable 00CF864C, +20h word = plane+174h. |
| 006BDBF0 | `message_resolve_unit_006bdbf0` | `__thiscall(ECX = message) -> EAX` | The registry entity for the +20h handle (no null or liveness test). |

### Adoption notes (cc9 host models)

1. **006BCA80 (MINOR, per-operation float rounding only; corrected 2026-10-18).** The binary
   leads the path point by the owner's velocity (`[[holder+4]+7Ch]` vtable +34h) × t, storing
   each product as a float before the add. It also rotates T by holder+8Ch × t when that angle
   is not exactly zero. Its callers are 007C7329, 009AFB46 and 009AFD52.
   - My first note called this a live mismatch. I had read only the stand-in's own comment,
     which is stale and which cc9-physics19 is fixing.
   - In `game_hosts_units.cpp`, `land_path_point_006bca80` (~line 32155) returns T only. Its
     009AFD52 caller (`land_geometry_009afaf0`, ~lines 32210-32241) applies the owner-velocity ×
     lead itself, under `kCarrierLandingVelocityLeadBound` (line 7462). That switch has been ON
     since 2026-10-17, with leads=4761 on USN01.
   - holder+8Ch has no writer in 006BEE40, so the rotation arm is dead in practice.
   - The remaining delta is rounding: the host adds `p + v × lead` in double with one final
     rounding, while the binary stores `float(v × t)` and then `float(p + that)`.
   - The function does not use the shared RNG 00BD2F10 and does not touch +900h.
2. **006BCA40: matches.** `landing_t_world_006bca40` (~line 15878) is T through frame +8h.
3. **006BC960: matches** for both arms.
   - Airfield, the comment at ~line 13755: clearance 35.0 (block vtable +10h = 006CA5A0); the
     0 > x clamp keeps NaN.
   - Mother ship, the comment at ~line 13965.
   - The host takes the sums in double (labelled), which can only differ from the 24-bit
     schedule by a double rounding.
4. **006BCD80 / 006BCE60: match.** `land_set_held_006bcd80(bool)` (~line 32776) sets every
   matching record, as the binary does. Its callers are the land/abort enter and exit
   (009B0980 / 009B09A0), which pass true and false. The lock and the checked-index tests are host
   infrastructure.
5. **006BD080: matches** through `landing_request_006c54c0` (~line 16448):
   - The caller 006C54C0 maps out+8h to approach+4Ch, out+0Ch (the record's +0Dh) to
     approach+44h and out+10h to approach+50h. The host's `rec.spacing_8`, `rec.side_0d` and
     `rec.mode_10` are those fields.
   - One difference: the host takes the out+0h word (plane+174h, which 006C550A tests for
     non-zero) as always non-zero.
6. **006BC5E0: labelled substitution, confirmed at the caller.** `player_air_ops_order`
   (`game_hosts_script_orders.cpp` ~lines 2082-2087) sets `holding` from the flight leader's
   order class being moveto. The distance test (class+268h × 1.2 against the target) is not
   read. That differs only for a moveto whose point is not the squadron's own.
7. **006BC730 (MINOR, no observable difference on the host's paths).** It is reached through
   `air_ops_held_slot_order_006ccda0` (`air_operations.cpp` ~line 1025), whose only caller is
   `GameScriptOrdersHost::player_air_ops_order` (`game_hosts_script_orders.cpp` ~line 2090).
   - The `max(unstowed, 0)` clamp has no effect: the caller counts the members itself, from 0
     upward, so the value is never negative.
   - A slot without a squadron is refused by the caller ("the slot holds no squadron this host
     built"). The binary would store +8h = 0 and state 1 there, which 006CD0EC then overwrites
     with state 3.

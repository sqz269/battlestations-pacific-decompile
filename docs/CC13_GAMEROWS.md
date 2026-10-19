# CC13_GAMEROWS: the open game-named and untagged rows of the approved segments

Packet cc13-gamerows (cc13-seg67n, 2026-10-19). Scope: the open rows (no reconstruction record,
not a funclet) of the 16 approved cc13 gameplay segments (40, 47, 48, 52, 53, 67, 59, 51, 58, 30,
41, 42, 50, 61, 54, 46) whose name is `BSP_` / `FUN_` (80 rows) or that carry no tag (58 rows),
outside the struck ranges 007DF000-007DFFFF, 007EE000-007F2FFF, 00836000-00837FFF,
0084E000-0084EFFF and 006C5000-006C5FFF. The census script is `local/gamerows_census.py` (output
`local/gamerows.json`); each row was cross-checked against the hosts by `bsp.py lookup`,
`docs-for`, and git grep by address, name and behaviour.

| Class | Rows | Treatment |
|---|---|---|
| T: pure 5-byte JMP thunks | 49 | `existing_recorded_thunk` (batch 1) |
| H: hosted bodies | 6 | ledger-only, citing the host (batch 1) |
| D: scalar deleting destructor | 1 | bound, citing the out-of-line copy (batch 1) |
| S: small reconstructs | 5 | rebuilt (batch 2) |
| G: MSVC STL instantiations | 52 | bound (batch 3, 26 + 26); 00732560 moved here from S |
| E: engine templates and game virtuals (split out of G) | 25 | rebuilt (batch 4) |
| cc15-blocked | 0 | none sits on a CC15_OBJECT doc |

## Batch 1: T, H and D (56 rows)

**T, 49 thunks.** Each row is `E9 <rel32>` followed by INT3 padding, a pure single JMP with no
store, so it is a thunk record under the destructor rule (docs/CC13_MISTAG.md). Each record names
the target, the target's record and source, and the row's references (unwind funclets, direct
callers or vtable slots). Targets:

| Target | Rows |
|---|---|
| 007B45F0 BSP_LandSharedState_Destroy (src/native_land_state_lifetime.cpp) | 39 |
| 007B65E0 BSP_LandMoveToState_Destroy (same file) | 2: 009A93D0, 009B5620 |
| 009BDE40 BSP_BotStateFollow_Exit (src/cc13_seg67b.cpp) | 2: 009AD1E0, 009B6660 |
| 004B7EF0 (src/native_game_array_lifetime.cpp) | 0099A010 |
| 0099A4C0 (src/native_pilot_bot_task_owner.cpp) | 0099A600 |
| 0099A660 (src/cc13_seg67b.cpp) | 0099A870 |
| 009BAF60 (src/cc13_seg67b.cpp) | 009BB170 |
| 0099B6E0 TRIV_body_0099b6e0 (open, no record) | 009ADD30 |
| 0084B9A0 (a batch 3 library row) | 0084B9F0 |

009B6660 is the level-bomb prepare state's exit slot (vtable 00D201A0). The hosts already route it
to the follow exit (game_hosts_units.cpp:27119, kamikaze_task_cc13.cpp:931), so there is no
mismatch.

Six rows carried their target's name, and they are renamed `thunk_<target name>_<address>`:
0099A010, 0099A600, 009A93D0, 009B5620, 009AD1E0, and 009ADD30 (was `TRIV_body_0099b6e0`). The
other 43 keep their names.

**H, 6 hosted rows.**

| Address | Name | Host | Status |
|---|---|---|---|
| `006C4790` | BSP_AirOps_SquadronNotExcluded | src/game_hosts_units.cpp (labelled) | substitution, adoption note 1 |
| `006D1420` | BSP_AirFieldCollisionNode_ClearShapes | src/collision_shapes.cpp `collision_node_clear_shapes` | exact |
| `0093A570` | BSP_ShipSectionList_Contains | src/game_hosts_units.cpp (labelled) | substitution, adoption note 2 |
| `00822770` | BSP_UnitDamageSmoke_GrowSlots | src/game_hosts_gunnery.cpp `damage_smoke_tick_008227e0` | exact but for precision, note 3 |
| `009ABA30` | BSP_KamikazeApproach_AimPoint | src/kamikaze_task_cc13.cpp `kamikaze_aim_point_009aba30` | ledger backfill |
| `009ADDB0` | BSP_DropKamikazeTask_StepResult | src/kamikaze_task_cc13.cpp `drop_kamikaze_step_result_009addb0` | ledger backfill |

006D1420 is `MOV dword [ECX+F8h], 0; RET`; the host writes the count only, as the binary does.
009ABA30 and 009ADDB0 were checked against the listing: the 1.0 floor at [00D7A24C], and the
00419010 arguments 8.0 [00CE3918], 40.0 [00CE685C], 2.0 [00CE3958], 0 and approach+60h; and
`NEG/SBB/AND 2` on the byte at task+4B9h.

**D, 1 bound row.** `00822700` BSP_UnitVehicleBase_ScalarDeletingDtor is the compiler scalar
deleting destructor around the recorded out-of-line destructor 0081F3A0 (unit_release_steps_0081f3a0,
src/unit_destructor_levels.cpp), then `_free` (00BF6989) when flags & 1. It is bound with its name
kept.

### Adoption notes (cc9 host files are read-only; nothing changed there)

1. **006C4790** (`006C4790-006C47EC`). The binary returns 0 for a null squadron, and otherwise walks
   the std::list headed at [block+B8h], returning 0 when a node's +8h is the squadron and 1
   otherwise. The host (game_hosts_units.cpp, `land_command_still_valid_009b34d0`, the comment at
   line 14711) takes that list as empty and labels it, so it answers `squadron != null`. LIVE in a
   normal mission: yes, on every land-command validity check (009B34D0). It differs only when
   something is on the +B8h list. Shared RNG 00BD2F10: no.
2. **0093A570** (`0093A570-0093A5C6`). The binary scans every 10h-byte record of
   [vector+18h, vector+1Ch) with no early exit and returns 1 when any first dword equals the id.
   The host labels a SUBSTITUTION (game_hosts_units.cpp, comment at line 2513): it asks
   GameGunneryHost::unit_failure_active by the ShipGlobals.Failures names of kinds 5 / 8 / 6.
   That is the same answer while 0093BED0 is the only producer of those records. LIVE: on the
   section-point pick for dive rows (skill 2-5). Shared RNG: no. Torpedo-adjacent (the
   TORPEDO_AIM_LEAD packet), but this is not a new finding: the substitution is already labelled.
3. **00822770**. The host forms `(max + 1) * damage` in double before truncation. The binary does
   `FILD` / `FMUL float` at x87 PC24 and then 00BF7420 `_ftol`. A product that rounds up to an
   integer in 24 bits can truncate one higher in the binary. LIVE: every damage-smoke tick of a
   damaged unit (008227E0). Shared RNG: no. No fix is requested.

## Batch 2: S (5 rows)

Source `src/cc13_gamerows.cpp`, header `include/bsp/cc13_gamerows.hpp`. Each function takes a
`GameRowsHost` for the tree callees (the same ones cc13_left8_seg8 hooks for the traffic record
constructor 0049F9B0), and each was checked against the listing.

| Address | Name | C++ | Shape |
|---|---|---|---|
| `0082E1F0` | BSP_ShipIdleRecord_Construct | `idle_record_construct_0082e1f0` | Idle record (1Ch) default constructor |
| `0082E280` | BSP_ShipIdleRecord_Destruct | `idle_record_destruct_0082e280` | its out-of-line destructor (rebuilt under the rule) |
| `0082E5A0` | BSP_ShipIdleRecord_CopyConstruct | `idle_record_copy_construct_0082e5a0` | its copy constructor |
| `0082DF00` | BSP_TrafficRecord_Assign (was BSP_ShipIdleRecord_Assign) | `traffic_record_assign_0082df00` | 4Ch traffic record operator= |
| `009A7A40` | BSP_VirtualReturnZeroFloat_009a7a40 (was FUN_009a7a40) | `virtual_return_zero_float_009a7a40` | `FLDZ; RET`, slot 00D1F910 |

The Idle record is a map at +0h (head +4h, size +8h), a map at +0Ch (head +10h, size +14h) and
posID - 1 at +18h (ship_class_fields.hpp). The constructor leaves +18h unwritten. The destructor
empties the +0Ch map first, then the +0h map. Each is emptied with its range eraser over
begin..end, its head is freed, and the head and size are cleared.

0082DF00 was named as an Idle record member, but its layout is the 4Ch traffic record of
cc13_left8_seg8.cpp (0049F9B0 / 0049FA70): plain fields +0h..+30h, then maps at +34h and +40h.
The cc13-gp80 record already called it the head of the 50h Traffic record, after which 0082ED80
copies +4Ch. It copies floats +18h..+30h with FLD/FSTP pairs, which is a value copy (a signalling
NaN would come out quiet). Each map is skipped when it is the source's own; otherwise it is
emptied (00492A00 / 00444B10) and refilled (0048D080 / 00499680).

**Moved to G: `00732560`** BSP_GunClass_CopyMuzzleOffsets. The full listing shows checked
iterators: first {+8h, +0Ch}, last {+10h, +14h}, and a back_insert_iterator {+18h, +1Ch} that it
returns through the hidden pointer at +4h. That is a std::copy instantiation over 12-byte points,
so it is bound in batch 3 (G is now 77 rows).

## Batch 3: G, 52 MSVC STL rows bound

The 77 rows first classed G were split before binding, with the lead's approval. 52 are genuine
MSVC 2005 STL instantiations. They are std::list (NarrativeList_*, BSP_List_*,
SupportSite_CopyClassList), the _Tree lower bound, std::string operator+, and std::vector
insert_n / resize / erase / tidy / push_back, plus the seg51 STL_ rows. Each uses
allocator.allocate, _Xlen or the checked-iterator report 00BF6713. They are bound
(`library_instantiation_out_of_scope`) with their names kept.

The other 25 (class E, batch 4) are not library code under the library ruling:
- 6 virtual methods, reached only through vtable slots: 006CA5D0, 007B0E60, 007B1B40, 007B21A0,
  007B2850 and 007B2B90.
- 19 engine template containers that use only operator new and _free, with no _Xlen, allocator or
  checked iterators: the 15 seg53 BSP_Native* rows, 0072A3B0 / 0072A6D0 / 0072B6D0 and 0084FC50.

Records that a cc13 host header declares as a hook (pure virtual) say so; the host supplies the
body.

**Part 1 (26 rows):** 006C6F90, 006C8750, 00731C10, 00732560, 00733430, 00733AF0, 007342B0,
007343D0, 00748150, 007CCD30, 007CD2F0, 007CDDB0, 007CF4B0, 007D1B50, 007D1D30, 007F3D40, 007F6D30,
007F6F80, 007F7070, 0081B2B0, 0081B300, 0081B520, 0081C380, 0081C4D0, 0081C980, 0081CA30. Nine of
them are hook-declared: 006C8750, 00733430, 007342B0 (also in cc15_g24c.hpp, which is
untouched), 00748150, 007CCD30, 007F3D40, 007F6D30, 007F7070 and 0081C980.

**Part 2 (26 rows):** pending.

## Batch 4: E, 25 rows rebuilt

Pending.

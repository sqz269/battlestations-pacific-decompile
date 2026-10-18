# cc13-small2: leftover rows in segments 8 7 43 26 13 19 5 27 9 12 20 1 23 66 3 15

The packet list (`local/cc13_undone_small2.txt` in the cc13 tree) held 129 rows that had
no tag and no reconstruction record. Each row was checked with `bsp.py lookup` and
`docs-for`, and `src/` and `include/` were grepped for the address in four case forms and
for the ledger name. Names below are hypotheses, not recovered symbols.

## Triage

| Class | Rows | Record |
| --- | ---: | --- |
| Library container code, bind-only | 82 | tags (`stl_shape` 74, `tmpl_shape` 7, `cg_array_ctor_helper` 1) |
| Game code hosted by cc9, verified | 7 | `existing_host_verified_ledger_only` |
| Game code reconstructed here | 17 | `reconstructed_build_tested` |
| cc9 keep, not touched | 1 | none (00428800) |
| Reconstructed concurrently by cc13-front8 | 1 | none here (0053D230, `src/cc13_front8.cpp`) |
| Game code deferred | 21 | none |

### Library rows (82)

The 82nd row is 0043FA00, listed as `FUN_0043fa00`. It is `allocator<LuaObject>::construct`:
if dst is non-null it calls `00B66FA0(ECX = dst; src)` and returns with RET 8. It has the
same shape as 0043EEF0, which is already tagged `stl_shape`.

Every row named `STL_*`, `TMPL_*`, `std::operator+`, `operator*`, `` `vector copy
constructor iterator' `` or `~SubAllocator` was read down to its callees. The only game
callees are element copy/assign helpers and the case-insensitive string less (00443D00),
which are the instantiation's element type and comparator. None contains game logic.

- `stl_shape` (evidence `cc13small2_stl`): vector insert-n, assign, tree insert
  hint/unique/equal, buy/construct node, copy-construct, erase-subtree, find, lower-bound,
  equal-range, list splice/merge/sort, vector<bool> copy/erase, wide SBO string destroy and
  tree min/max. Also `std::operator+(string, string)` 004D0860, the checked string iterator
  dereferences 0098E2B0 and 0098F0C0 (the -2 sentinel and the 00BF6713 test, then the SSO
  buffer pointer), and the unsigned byte compare 004B3FC0.
- `tmpl_shape` (evidence `cc13small2_tmpl`): 1Ch and 6Ch record copy-construct, 6Ch / 2Ch
  record and dword vector construct-n / insert-n, and `~SubAllocator` 00778840. That last
  one is `eh_vector_destructor_iterator(this+4, 34h, 3, 00804300)` and is reached only from
  unwind funclets.
- `cg_array_ctor_helper` (evidence `cc13small2_cg`): 00453ED0, the MSVC
  `` `vector copy constructor iterator' `` (dst, src, size, count, copyctor).

### Hosted by cc9 (7, ledger-only)

The bodies were read in the binary and compared with the host. No cc9 file was edited.

| Address | Name | Host | Check |
| --- | --- | --- | --- |
| 007788D0 | BSP_Unit_FormationLeader | `game_hosts_hud.cpp:2122` `group_leader_007788d0`; `game_hosts_commands.cpp:2467` `unit_controller_owner_007788d0` | `g = [unit+284h]; return g ? [g+14h] : 0` |
| 00779820 | BSP_Entity_SharesUnitGroup | `game_hosts_ai.cpp` near 3113 (whole) | same entity -> 1; `[this+284h]` non-zero and equal to `[other+284h]` -> 1; else 0; RET 4 |
| 00779AA0 | BSP_Unit_ClassHasAttackSlot | inlined, `game_hosts_gunnery.cpp` 11889/11918, `game_hosts_ship_ai.cpp` near 2888 | `[[unit+538h]+510h] > 0 \|\| [+514h] > 0` with COMISS/JA, so NaN is false; the host's `!(x > 0)` agrees |
| 00644A20 | BSP_InGameHudRootScreen_OnObjectReleased | `game_hosts_hud.cpp` 3856-3865, behind `kControlledUnitObserverBound` | if obj == `[00E188D8]`: `[this+14h]` = 0 (root+1Ch), `004C0890(ECX = 0)`, byte `+BAh` = 0, word `+BCh` = FFFFh |
| 00647040 | BSP_InGameHudRoot_PushControlledUnitInterface | `game_hosts_hud.cpp:2165`, a labelled model | `0059DA80(ECX = [[00E198C4]+54h])`; `005251C0(ECX = [[00E198C4]+CCh])`; `EAX = [00E188D8]->vtable[140h]()`; `004CC460(ECX = [00E198C4]; 20h, EAX)` |
| 0077C980 | BSP_UnitGroup_ReleaseMember | `game_hosts_lua.cpp` 8185-8210 `run_leave_formation_00899eb0`; `game_hosts_ship_ai.cpp` 11225 | nothing unless `[unit+284h]`; a non-null other must be live and in the same group; `00905300` for `+1ACh <= 7`; message 77h through `0077C2A0(unit; msg, 7, 0)` |
| 0077CA60 | BSP_UnitGroup_SnapshotMembers | `game_hosts_lua.cpp` 7280-7320 `run_disband_formation_0089a020` | member snapshot over `+4F8h` via 0070D060, leader last; per member the same slot bump and 77h |

The two formation rows are hosted with labelled substitutions: the leave runs at the call and
the slot counter is not modelled. Both are cc9's documented choices, so neither is a mismatch.

Adoption note: none of these needs a host change. 00647040 is hosted as a labelled
substitution rather than call for call; anyone wiring the 004CC460 interface push should
take the four calls above as the contract.

### Side fix: seg97T

`docs/CC13_SEG97T.md` gained exact group counts, and 00BF3ACC is now recorded as a
fragment of the existing 00BF3A80 reconstruction (`src/native_physical_file_date.cpp`).
The polarity was checked: 00BF3A98..00BF3AA5 test byte `[[0109CEEC]+78h]` and JZ to
00BF3ACC, while a non-zero byte zeroes the five outputs and returns. The byte is the
date-disabled flag, so the name `EnabledContinuation` stands.

## Reconstructed (17)

The code is in `src/cc13_small2.cpp` and `include/bsp/cc13_small2.hpp`. It is built into
`bsp_core` and nothing calls it. Each body was read from the listing. The x87 operand order
was taken from the stack sequence in the listing (FLD/FMULP/FADDP, with no reversed DC-form
operations), and the products and sums are modelled as float operations because the main
thread runs at 24-bit precision.

| Address | Name | ABI | Notes |
| --- | --- | --- | --- |
| 00778820 | BSP_EntityWeaponDirectorRef_Construct | thiscall(holder; entity) RET 4 | `holder[0] = entity->vtable[114h]()`; entity message 59h (00780649) |
| 00778860 | BSP_Unit_DirectorPathObject0 | thiscall(unit) | director path object 0 (0071BFC0), or 0 |
| 00779EB0 | BSP_Unit_DistanceToRecord | thiscall(entity; Record**) RET 4, ST0 | `(dx*dx + dy*dy) + dz*dz`; 0 at or below 1e-10 (00CE3820) or unordered |
| 0077A650 | BSP_UnitInstance_ExpireAtScheduledTime | thiscall(unit) | clock >= +308h, mode != 2: 0090EBF0, then kill(3) |
| 0077B050 | BSP_Entity_UpdateCommandTargetPosition | thiscall(entity; out) RET 4 | entity slot D4h; resolved target point plus a valid byte |
| 0077C540 | BSP_Replication_FlushQueuedRows (renamed) | cdecl | queues kind-8 children, drains 64 buckets from 63 down, `00F87150 = tick % 12` |
| 0077C880 | BSP_UnitGroup_BroadcastFormation | thiscall(owner; msg) RET 4 | mode 2: stamps +174h at msg+18h and sends to 00770AF0 |
| 0077CD80 | BSP_Unit_LuaThisTableEntry | thiscall(unit; out) RET 4 | `globals.thisTable[tostring(word +18h)]` |
| 0077D3F0 | BSP_Entity_SetGuiName | thiscall(entity; name, arg) RET 8 | assigns +15Ch; mode 1 broadcasts SETGUINAME |
| 0077D7F0 | BSP_Replication_StepLocalPlayerRecord | cdecl | CAMERA message, then the local record goes to vtable[184h] |
| 0077F5E0 | BSP_Replication_DrainChildQueues (renamed) | cdecl | traces and empties the +298h child lists; the local key set is discarded |
| 0077F7B0 | BSP_Unit_ApplyDelayedEntityMessage | thiscall(unit; msg) RET 4 | entity message 51h: schedules +308h, transfers roles, sends clearorders and stop |
| 00645BB0 | BSP_InGameHudRootListener_Current0C | thiscall(listener; unit, event, candidate) RET 0Ch | adopts the candidate, or resets the cursor |
| 00648930 | BSP_InGameHudRoot_EnsureSelectableControlledUnit | thiscall(root) | forces a reselect (006485A0) when 00645060 rejects the controlled unit |
| 0064A270 | BSP_RepairTask_SetRepairPriority | thiscall(task; prio) RET 4 | sets +24h; warning manager "repair" channel |
| 004C3A80 | BSP_Game_UpdatePlayerSlotUnitFlags (named) | thiscall(game) | mode 0Dh: slot +19h no-unit bytes for the local and the rotating `[00E188C4]` slot; 00645160 is fastcall with DL = 0 |
| 005F98F0 | BSP_Unit_WithinFriendlyCaptureRadius (named) | thiscall(unit), AL | kind 6 and not 9; horizontal `dx*dx + dz*dz <= r*r` against a same-team entity's `(int)+7A0h` |

Register details found while reading:

- **00645BB0 and 00648930.** 00645060 is `__fastcall`, and both callers load EDX with the
  team, `[game+18ECh]`. The load at 00645BCE looks dead in the pseudocode, but it is not.
- **0077D3F0.** The PUSH 0 at 0077D476 is the second argument of 0077C7B0. It is pushed
  before the by-value string copy and the arg that the 0077CCC0 constructor (RET 0Ch)
  consumes.
- **0077F7B0.** `age = msg+0Ch - tick` goes through FILD, then FMUL by the float 0.05
  (00D0DE84), then FSTP to a float. The FILD is exact. The C++ rounds the int to float first,
  which only matters when |age| >= 2^24.

0053D230 was also read here, but cc13-front8 landed its reconstruction first (5b97f92bb), so this packet does not keep a second copy. Both readings agree: y is zeroed when vtable[5Ch](6) accepts the unit, then vtable[118h](&x).

Uncertainty: the virtual slots (114h, 148h, 178h, 17Ch, 180h, 184h, 188h) are named
from their call sites only. Mode 1 is read as "server" and mode 2 as "client" because the
cc13 multiplayer docs read them that way.

### Adoption notes

1. **0077A650 / 0077F7B0 / 005F98F0 (selling ships).** This is not urgent and does not use
   the shared RNG.
   - **The chain in the binary.**
     - The SELLING tick 00A11FF0 sends message 51h through 0077C2A0, at 00A123A0..00A123F8.
       It does so for each member with `+308h == 0` that passes 005F98F0.
     - 00780120 dispatches 51h to 0077F7B0 (0078051E..0078052B). That routine sets `+308h`
       and issues clearorders and stop.
     - The next ticks call 0077A650 when `+308h != 0`; the gates are in
       `ship_ai_rudder_hop.cpp:71` and `tick_element_overrides.cpp:216`.
     - Once the clock reaches `+308h`, 0077A650 kills the unit with reason 3.
   - **The host.**
     - `game_hosts_ai.cpp` binds the SELLING tick (`kSellingTickBound = true`, live in
       LOMP07).
     - At 00A123F8 it only records the 51h send, a labelled "record here" (around line 5331).
     - So `+308h` stays 0, and `field_308_crossed_0077a650` (`game_hosts_units.cpp`
       10679-10684) is correctly unreached.
   - **Verdict.** This is a documented substitution, not a mismatch. Its consequence is that
     a ship that reaches its port under SELLING is never sold or removed in the host.
   - **Adoption.** Routing 51h needs these three bodies in this order:
     1. 005F98F0, the gate;
     2. 0077F7B0, which needs GlobalConfig `+100h` / `+104h` and the message's tick at
        `+0Ch`;
     3. 0077A650.2. **00648930 / 00645BB0.**
   - cc9's `game_hosts_hud.cpp` models the HUD root observer, 00644A20, behind
     `kControlledUnitObserverBound`.
   - These two siblings are not hosted.
   - A host that binds the listener should pass the team in EDX.

## Not touched

- **00428800 BSP_Explosion_NotifyBlastSites** is a cc9 keep
  (`cc13_gameplay_terms.txt`, addendum).
- **Earlier skip.** docs/CC13_SWEEPB2.md had skipped the seg 43 rows 0077B280 to 0077FE80
  as cc9 lane. This packet assigned them. The rows recorded here are standalone and
  unwired, and no cc9 file was edited.

## Deferred (21, no record)

- **Too large for this packet:** 0077B280 (sync delta, 294 instructions of x87),
  00673A10 (63 callees), 00674B80 (53 callees), 004A9BD0 (planepartclasses loader; a class
  loader, and cc9 reads its globals), 00428420 (over 250 lines).
- **Still to do:** 004A0D20, 0066EA60, 005E12A0, 00641910, 00641E30, 0064AE20, 005AAC00,
  006697E0, 0066A8A0, 0077FE80, 00780DB0, 0077D270, 0077A080,
  005B8810, 004C7AA0 and 0059D4A0 (x87 with a jump table and a path that leaves locals unset). These are for a later batch.

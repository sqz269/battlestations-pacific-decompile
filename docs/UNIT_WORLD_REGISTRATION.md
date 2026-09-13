# Unit world-list registration

Packet `orch6_unit_world_registration_n` reconstructs the complete registration
body selected by primary-vtable slot `+130h` for each of the 21 instance creators
currently handled by `GameUnitsHost::kUnitMotionDispatches`. Dispatch uses the
existing `VehicleClassDescriptorRow::allocate_instance` identity. It does not
infer membership from `IsKindOf` ancestry, descriptor defaults, or motion type.

All eight ships join list 6. Planes, airfields, shipyards, land vehicles, forts,
and command buildings have other registrations. In particular, calling the
destroyer body `006FE620` for every instance incorrectly populates list 6.
The unchanged host code at this worker's base still needs the integrator's
binding. This packet makes no claim about corrected mission list counts.

## Callable contract

`unit_world_registration_for_creator` and
`unit_world_registration_for_descriptor` return a read-only evidence row, or
`nullptr` for unsupported creators/missing descriptors. The descriptor path
reads its real allocator field. The nonallocating `DummyTargetVehicle` creator
`00749150` is unsupported; it does not silently become a destroyer.

`register_unit_world_lists_for_creator(host, creator, unit)` returns false
without any host calls when the creator is unsupported. Otherwise it executes
the entire selected registration schedule. `UnitWorldRegistrationHost` supplies
the actual borrowed `parent_0030(unit)` owner and
`push_back_00484540(parent, list_offset, unit)` operation. Each append reloads
the parent, including the first append inside
`register_parent_entity_list_00928560`. The unit value is the original receiver
itself. No wrapper at `unit+4h` is read. The host owns the real lists, allocation,
and direct unit identity; the core manufactures no singleton or parent handle.

The native machine ABI for every covered registrar and `00928560` is ECX=unit,
zero stack arguments, plain RET. Every leaf preserves ESI. The new C++ APIs are
not binary-compatible replacements. A valid placed unit and its actual parent
are required. The native caller enforces a nonzero parent before dispatch;
the core does not turn an unavailable parent into an empty registration.
There is no deduplication or rollback if a later append fails. Placement,
locking, removal, and allocator/error behavior remain existing contracts.

## Creator, primary table, and complete native schedules

The descriptor's vtable `+28h` bytes were checked against each allocator.
Each allocator's actual constructor CALL, the constructor's primary-vptr store,
and that installed table's `+130h` bytes were verified through read-only BSP
queries against `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
Each creator and leaf constructor ends with RET4, consuming the one forwarded
four-byte constructor argument. The existing `unit_kind_query` rows supplied candidate identities; constructor
stores and slot bytes, not the candidate ancestry lists, establish this map.
All 21 primary stores use ESI. Full constructor listings show its entry
`MOV ESI,ECX` and epilogue `POP ESI` as the only writes to ESI.

List IDs below abbreviate actual offsets `parent+18h+id*0Ch`. Every range is
inclusive and complete, ending with a one-byte RET. The report carries raw
offsets and all exact call sites.

| Type | Allocator -> constructor | Primary table / store site | +130h slot -> registrar body | Ordered list IDs |
| --- | --- | --- | --- | --- |
| Destroyer | `006fe590 -> 006fe460` | `00cfc3d0` / `006fe46d` | `00cfc500 -> 006fe620..006fe665` | 1, 2, 4, 5, 6, 7 |
| Cruiser | `006fb430 -> 006fb300` | `00cfb738` / `006fb30d` | `00cfb868 -> 006fb4c0..006fb508` | 1, 2, 4, 5, 6, 10 |
| LandingShip | `0074be00 -> 0074bb00` | `00cffa30` / `0074bb2c` | `00cffb60 -> 0074bcd0..0074bd18` | 1, 2, 4, 5, 6, 12 |
| Cargo | `006eb290 -> 006eb160` | `00cfa778` / `006eb170` | `00cfa8a8 -> 006eb410..006eb458` | 1, 2, 4, 5, 6, 11 |
| BattleShip | `006dfef0 -> 006dfc90` | `00cf90b0` / `006dfc9d` | `00cf91e0 -> 006e0010..006e0058` | 1, 2, 4, 5, 6, 13 |
| Submarine | `008531a0 -> 00852f10` | `00d0bf80` / `00852f27` | `00d0c0b0 -> 00853090..008530d5` | 1, 2, 4, 5, 6, 8 |
| TorpedoBoat | `00857e20 -> 00857cd0` | `00d0c648` / `00857cdd` | `00d0c778 -> 00857eb0..00857ef8` | 1, 2, 4, 5, 6, 14 |
| MotherShip | `00758d30 -> 00758550` | `00d01630` / `00758598` | `00d01760 -> 00758f90..00758fd8` | 1, 2, 4, 5, 6, 9 |
| ReconPlane | `008091d0 -> 0074e0f0` | `00d00070` / `0074e0ff` | `00d001a0 -> 0074e670..0074e6bb` | 1, 2, 4, 5, 15, 20 |
| SmallReconPlane | `0084ca50 -> 0084c920` | `00d0ba80` / `0084c92d` | `00d0bbb0 -> 0084cae0..0084cb2b` | 1, 2, 4, 5, 15, 21 |
| LargeReconPlane | `0074e540 -> 0074e2d0` | `00d00308` / `0074e2dd` | `00d00438 -> 0074e700..0074e74b` | 1, 2, 4, 5, 15, 22 |
| Fighter | `007ddae0 -> 007dd9b0` | `00d06920` / `007dd9bf` | `00d06a50 -> 007ddb70..007ddbbb` | 1, 2, 4, 5, 15, 19 |
| DiveBomber | `00956390 -> 00951b60` | `00d19d28` / `00951b6f` | `00d19e58 -> 00956300..0095634b` | 1, 2, 4, 5, 15, 18 |
| TorpedoBomber | `009564e0 -> 00951c40` | `00d1a000` / `00951c4f` | `00d1a130 -> 00956450..0095649b` | 1, 2, 4, 5, 15, 17 |
| Kamikaze | `00956240 -> 00951d20` | `00d1a2d8` / `00951d2f` | `00d1a408 -> 009561b0..009561fb` | 1, 2, 4, 5, 15, 23 |
| LevelBomber | `007d7850 -> 007d7720` | `00d06638` / `007d772f` | `00d06768 -> 007d78e0..007d792b` | 1, 2, 4, 5, 15, 16 |
| AirField | `006d3110 -> 006d1c20` | `00cf8c08` / `006d1c68` | `00cf8d38 -> 006d36d0..006d370c` | 1, 2, 4, 5, 69 |
| Shipyard | `00848380 -> 00848080` | `00d0b770` / `008480b1` | `00d0b8a0 -> 00846cf0..00846d2c` | 1, 2, 4, 5, 70 |
| LandVehicle | `0074df10 -> 0074dcc0` | `00cffde0` / `0074dccf` | `00cfff10 -> 0074de10..0074de4c` | 1, 2, 4, 5, 25 |
| LandFort | `00747000 -> 00745940` | `00cff3f8` / `0074597d` | `00cff528 -> 006f59b0..006f59ec` | 1, 2, 4, 5, 27 |
| CommandBuilding | `006f5c10 -> 006f5610` | `00cfb028` / `006f564e` | `00cfb158 -> 006f5a50..006f5a9b` | 1, 2, 4, 5, 27, 28 |

The shared complete body `00928560..0092856C` executes PUSH ECX;
MOV ECX,[ECX+30h]; ADD ECX,24h; CALL 00484540 at `00928567`; RET.
Thus list 1 precedes every listed leaf contribution. All 49 direct callers of
this shared body were enumerated: each starts PUSH ESI; MOV ESI,ECX; CALL
00928560, passing the same receiver contract. It also has four data references.
Each of the 21 leaf bodies has one observed data reference at its installed
primary table's +130h slot and no direct CALL references.

All leaf bodies use the same exact schedule: preserve ESI=ECX, call the shared
parent registrar, then for each additional list reload `[ESI+30h]`, PUSH ESI,
ADD ECX,the listed offset, CALL 00484540, and finally restore ESI and RET.
The complete instruction listings were checked for every body; no branch or
other call is omitted. This source encodes those established complete bodies
as read-only offset sequences, rather than treating guessed kinds as a table.
The existing `kUnitParentListOffsets` supplies the destroyer sequence and shared
parent offset. The older `register_unit_instance` projection remains a fixed
parent, destroyer-only API; it is not used as universal dispatch here.

## Owner, list producer, and caller correction

The already represented `009258F0` attachment body copies stack argument 2
into unit+30h at `00925902..00925906` (after two saved registers), while
argument 1 goes into the distinct hierarchy-parent field +3Ch at `00925935`.
Its ABI is ECX=unit with three stack arguments and RET0Ch. Registration uses
the +30h parent/list owner, not the +3Ch hierarchy parent.

`004CB030` constructs 97 list heads at owner+18h: at `004CB060..004CB076`
it passes destructor004C2D30, constructor004B7EC0, count61h, stride0Ch, and
owner+18h to vector constructor00BF7CD1. The iterator's RET14h accounts for
all five arguments. The complete constructor004B7EC0..004B7ECC zeroes the
three dwords at +0/+4/+8. Existing list layouts are reused: count/head/tail
at +0/+4/+8 and node prev/next/value at +0/+4/+8.

`00484540..00484596` is the actual append body, ECX=list and one stack value,
RET4 at both `00484583` and `00484594` (three-byte instructions). It calls
operator_new00BF681B with size0Ch at `00484546`, with ADD ESP,4 at `0048454D`;
then stores the supplied unit directly into node+8h at `00484566`. Its empty
and populated branches link the head/tail, previous/next pointers, and count.
The allocator's malloc/new-handler retry and exception service remains external.
No allocator or list implementation is duplicated in tracked source.

The actual placement dispatch exists at **009288F1** in complete body
`00928860..00928914`. ECX is captured into ESI at0092887A. Old parent+30h is
compared with the proposed world-node argument at009288A1..009288AA, setting
BL. After optional old-parent removal and the attachment CALL at009288D6,
the caller requires BL and nonzero current `[ESI+30h]`, loads the installed
primary vtable and its +130h target at009288E7..009288E9, sets ECX=ESI, and
CALLs EAX at009288F1. Placement owns its surrounding world lock. A search only
for `CALL [reg+130h]` misses this load-then-CALL-register schedule.

This corrects the old `game_hosts_units.cpp` missing-dispatch comment and its
universal006FE620 call. `GAMEPLAY_LOOSE_ENDS_1.md` lists the leaf's five extra
destroyer lists separately from its parent call; the total is six. The
`LUA_BINDING_ENTITY_LOOKUP.md` destroyer exemplar is not evidence that every
unit creator has its memberships. Player-owned wrapper registries described
by `local_player_unit_lists.hpp` have a different producer from these direct
unit-valued world lists; their +4h wrapper field must not be applied here.

## Verification and limits

`local/registration_probe.cpp` executes the complete original 21 registrar
bodies, shared00928560, and append00484540, all 23 byte ranges checked equal
between the installed PE and live Ghidra. The functions are relocated with
internal branches unchanged. Registration calls to00928560 target its original
relocated body. Append calls go through a trace bridge that invokes the full
original00484540 body; its operator_new boundary returns a real node from a
bounded fixture arena. This validates registration and uses the actual append
implementation, without claiming to reconstruct its allocator.

All 21 creator/entry pairs passed in two scenarios: a stable parent and a
parent switched after every append. Each scenario registers two distinct
units to exercise initially empty and subsequently populated list heads.
The 42 scenarios compare **488 ordered pushes per side**, direct unit values,
all 194 heads across two parents, counts/tails/prev-next links, and the exact
parent chosen for each push. Three unsupported inputs (null descriptor,
creator0, nonallocating00749150) have no host reads or pushes. Inputs start
with explicitly zeroed heads, two actual fixture units whose +30h points to
parent0, and node-arena bytes A5h. These are controlled fixture inputs, not a
reconstruction of other fields in a full unit or world constructor.

The preserved CSV records creator, scenario, original/rebuilt side, ordinal,
parent index, byte offset, and unit index for every push. Full register/flags,
FP environment, constructor execution, native placement/locks, allocation
failure, exception unwind, and a mission run are outside this fixture.
No tracked tests are added. Win32 build and existing CTest results, direct-call
verification, exact source/native hashes, and the ignored artifact manifest
are recorded in `reports/unit_world_registration.json`. The final Win32 build
and both existing CTest checks passed. The call gate reports 176 checked rows
and zero failures: 175 direct calls are verified, while the placement dispatch
and vector-constructor callback are explicitly indirect among 177 report rows.

All called bodies are defined in the verified Ghidra target; there are no
missing-function ranges or observed flow gaps in this packet. Ghidra was not
mutated. Proposed names are hypotheses recorded through the ledger; existing
correct names are preserved. Runtime integration stays with the primary.

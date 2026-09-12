# The ship AI avoidance request block (`blk+3ECh`..`blk+3F5h`)

Addresses: 009DA6E0 0080E160 009DA1D0 009DABB0 009DABC0 009DABD0 009DABE0 009E4330 009F1420 009E14C0 009E1170 009E1950 009E2020 009E23B0 009EC770 009EF350 009EF910 009F0EA0 009EAFC0 009D7050 009ED6B0 009F3F80

Packet `cc_ai_avoidance_request`. `docs/SHIP_AI_STATE_STEPS.md` left this block as a follow-up
with "`blk+3ECh` has no reader in what was scanned" and "the other two have no reader yet".
Both have readers, there is a fourth byte in the same group, and the block's meaning is settled
by the three **authored** names on the unit's command controller that the three fields pair with.

`blk` is `brain+8h` (`docs/SHIP_AI_STATES.md`). Every writer stores through `brain`, so a
writer's displacement is eight higher than the blk-relative offset: `brain+3F4h` is `blk+3ECh`,
`brain+3F8h` is `blk+3F0h`, `brain+3FCh` is `blk+3F4h`, `brain+3FDh` is `blk+3F5h`. The
constructor `009E4330` is the one routine that addresses the block from `blk`, and its four
stores at `009E468B`..`009E46A3` line the two views up exactly.

## 1. The layout, and the setters that fix it

Four unreferenced one-instruction setters sit together at `009DABB0`, `009DABC0`, `009DABD0`
and `009DABE0`. No call site, vtable slot or data word in the image holds any of those four
addresses (`scan-bytes` over the whole file for each little-endian dword: no matches), so they
are the compiler's unused COMDAT copies of the accessors every writer inlined. Their widths are
the block's widths, and their `this` is `brain`. `009DABF0`
`BSP_ShipAi_SetSpeedCommandedFlag_Unused` is the next member of the same belt and was named by
packet `cc_ai_goal_vector` the same way.

| blk | brain | width | setter | constructor | the enable it is ANDed with |
| --- | --- | --- | --- | --- | --- |
| `+3ECh` | `+3F4h` | byte | `009DABB0` | 1 (`009E4695`) | `director+240h` **`torpedoAvoidance`** |
| `+3F0h` | `+3F8h` | int32 | `009DABC0` | 3 (`009E468B`) | `director+241h` **`shipCollisionAvoidance`** |
| `+3F4h` | `+3FCh` | byte | `009DABD0` | 1 (`009E469C`) | `director+242h` **`landCollisionAvoidance`** |
| `+3F5h` | `+3FDh` | byte | `009DABE0` | 0 (`009E46A3`) | none |

The three enable names are not hypotheses: `docs/WEAPON_DIRECTOR.md` reads them out of the
property dump `008362A0`, which pushes each byte to the sink with a literal name string, and
`0080E160` is the accessor that reaches the controller from the unit. Each of the three request
fields is read at exactly the sites where its partner byte is read, and nowhere else:

```
009DA1D0  if (blk+3ECh == 0) return false; if (0080E160(unit)+240h == 0) return false
009EC770  if ((int)blk+3F0h < 0) return false; if (0080E160(unit)+241h == 0) return false
009F1052  the same pair inlined, 009F1052 and 009F106C
009EF368  the same pair inlined, 009EF368 and 009EF37B
009DA6E6  if (blk+3F4h == 0) skip; if (0080E160(unit)+242h == 0) skip     (x4)
009EFCA6  the same pair, 009EFCA6 and 009EFCBD
```

So the block is one **avoidance request per kind**, published by the state step and gated per
kind by the player's or the mission's controller settings. `blk+3F5h` is the odd one out: it has
no director partner, it is the byte `bsp/ship_ai_states.hpp` already models as
`ShipAiControlBlock::early_out_3f5`, and this packet found its producer (section 3).

`blk+3FCh`, eight bytes past the block, is the unit pointer, not part of it; every reader above
loads it to reach the controller.

## 2. `blk+3F0h` is a Party id, not a tri-state

`entity+54h` is the scene property `Party` (`docs/ENTITY_LIFECYCLE_TAILS.md`), and
`docs/GAME_AVOID_ZONE_RUNTIME.md` records that the value 2 is Neutral. `009EC770`'s tail accepts
when either side is `3` or the two are equal, so `3` is a wildcard that is not a Party value and
`-1` is "off". Three of the four writers only ever store `3` or `-1` — but **`009E23B0` and
`009E2020` store the owning unit's own Party** (`009E2588` and `009E21F0`, both
`MOV EDX,[[brain+0AA8h]+54h]` then into `brain+3F8h`). A ship on an attack run therefore avoids
only ships of its own side and steers through the enemy.

`009EF350` builds a three-entry accept table for sides `0`, `1`, `2` (`009EF357` `XOR ESI,ESI`,
`009EF3B0` `CMP ESI,3`, `JL`) and indexes it with `[ent+54h]` at `009EF423`
(`CMP byte [ESP+EDX+10h],0`). A Party of `3` would read `[ESP+13h]`, one byte past the table and
never written. The `side == 3` arm inside the same inlined predicate (`009EF38F`) is dead there,
because the loop feeds it `ESI`. That is a latent out-of-range read, not a rule, and `009EF350`
is not this packet's to annotate.

## 3. Every writer

The chain order (`docs/SHIP_AI_STATES.md`, `009F50E0`): step 3 is the pre-pass, step 4 is the
state step, and the consumers are steps 9, 10, 12, 13 and 14. Steps 3 and 4 run **only on a
re-plan tick**, so on every other frame the block keeps its last value while the consumers read
it.

| writer | site | `blk+3ECh` | `blk+3F0h` | `blk+3F4h` | `blk+3F5h` |
| --- | --- | --- | --- | --- | --- |
| `009E4330` constructor | `009E468B`..`009E46A3` | 1 | 3 | 1 | 0 |
| `009F1420` pre-pass, tail | `009F1B7B`..`009F1B9A` | 1 | `AvoidAllShipCollision ? 3 : -1` | 1 | 0 |
| `009E14C0` `stop`, stopped | `009E15BB`..`009E15D2` | 1 | -1 | 0 | - |
| `009E14C0` `stop`, making way | `009E15E4`..`009E15FB` | 1 | 3 | 1 | - |
| `009E1170` `cruise`, arm 1 | `009E13B6`..`009E13DC` | 0 | -1 | 0 | **1** |
| `009E1170` `cruise`, arm 2 | `009E11D6`..`009E11EA` | 0 | -1 | 0 | - |
| `009E1170` `cruise`, arm 3 | `009E12EB`..`009E12FF` | `00521E70(unit,0)` | 3 | 1 | - |
| `009E1950` `land`, hold arm | `009E1C28`, `009E1C32` | - | -1 | 0 | - |
| `009E1950` `land`, goal arm | `009E1EA2`, `009E1EC2` | - | 3 | 1 | - |
| `009E2020` kamikaze | `009E21F0`, `009E2235`/`009E22D8` (latch `009E21F6`/`009E2224`) | - | own Party | 0 / 1 | - |
| `009E23B0` attackmove engage | `009E2588`, `009E25CC`/`009E2669` | - | own Party | 0 / 1 | - |

A dash is "not written by that arm", which on a re-plan tick means the pre-pass value one call
earlier. The pre-pass rewrites all four bytes with no condition of its own, so a state step that
writes nothing publishes "torpedo avoidance on, ship avoidance on unless the mission turned
`AvoidAllShipCollision` off, land avoidance on, drive not bypassed".

`009E14C0`'s and `009E23B0`'s latch is `state+8h` / `sub+8h`; the `stop` latch is the making-way
byte `docs/SHIP_AI_STATE_STEPS.md` records, the attackmove one the close/run latch
`docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md` records. `009E23B0`'s own-Party store is already in that
doc at line 158; `009E2020`'s identical store is new here.

### `009E1170`'s three arms and the `blk+3F5h` producer

```
unit = [brain+0AA8h]                                              ; 009E117B
if (!unit || ![unit+740h]) return                                 ; 009E1188, 009E1196
p    = [[unit+740h]+50h] ; slot = [p+1B0h]                        ; 009E119C, 009E119F
if (slot != 8 && !00927F10(p, slot))  -> arm 1                    ; 009E11A5, 009E11B4
if ([unit+184h] != 0)                 -> arm 2                    ; 009E11C2
                                      -> arm 3                    ; 009E1265..
```

`[p+1B0h]` is index 1 of the array `00521E70` indexes; `00521E70` is
`__thiscall(x)(int i) -> (x[1ACh + i*4] == 8) || 00927F10(x, that)`, body read, and
`docs/GUN_BOT_TICKS.md` calls the same idiom the side gate, with `00927F10` answering whether
the Party slot is AI-held. `unit+184h` is the player-controlled byte
(`docs/CRUISE_COMMAND.md`). Arm 3 stores the same predicate's answer for the unit's **own** slot
(`00521E70(unit, 0)`, `009E12E4`) into `blk+3ECh`: a ship asks for torpedo avoidance only while
its own Party slot is AI-held.

**Arm 1 is the only writer of `blk+3F5h` in the image** (`009E13DC`, `MOV byte [EDX+3FDh],AL`
with `AL` = 1), against `009F1B8C` and `009E46A3`, which both write 0. Arm 1 also sets
`brain+0B38h` and clears `[[unit+73Ch]+28h]` to `-1.0f`, the two writes `docs/CRUISE_COMMAND.md`
already reads there. `blk+3F5h` makes `009ED6B0` return `true` at `009EDA06` after only the
heading target and a clamp on `blk+354h`, and makes `009F3F80` jump to `009F4D00` at `009F3FF2`.
It is the whole-controller bypass for a helm a person is holding, and its producer was
`contract: unread` until now.

## 4. `009DA6E0` whole, the land-avoidance consumer

`__thiscall(blk)(float seconds)`, `RET 4`, body `009DA6E0-009DA860`, complete. Chain step 9,
sole call site `009F51D5`. The float argument is never read. Three `20h`-byte searcher records at
`blk+0A24h`, `blk+0A44h` and `blk+0A64h`, seeded by the constructor at `009E4401`..`009E4449`
with `+0h` = 1, `+14h` = -1, `+18h` = 0 and `+1Ch` = 0.

```
for i in 0,1,2:                                        ; 009DA6E6, 009DA730, 009DA77D
    want = blk+3F4h && 0080E160(blk+3FCh)+242h
    if (rec[i]+0h != want):
        rec[i]+0h = want
        if (!want): 004158A0(&rec[i]+18h) ; rec[i]+14h = -1   ; 009DA724, 009DA729
if (blk+3F4h && 0080E160(blk+3FCh)+242h):              ; 009DA7CA, 009DA7DD
    w = (float)((double)blk+3C8h * 3.0)                ; 009DA7EB, 00D7A2B0, float32 store
    if (300.0 > (double)w) w = 300.0f                  ; 00CE3CA8, JBE at 009DA809, 00CE3AE8
    009D7050(&rec[0])(blk+184h, blk+188h, w, w, blk+168h)  ; 009DA854, ECX = blk+0A24h
```

The request byte and the controller byte are re-read for each of the four decisions, so there are
four `0080E160` call sites. Only searcher 0 is ever refreshed here; nothing read in this packet
or in `docs/SHIP_AI_ARM_FINAL_STEP.md` fills searcher 1 or 2, which is the open half of the
follow-up `ship_ai_avoid_zone_searchers`. The five-dword query is built on the stack at
`009DA81B`..`009DA850`; the `PUSH ECX` at `009DA83B` moves every later `[ESP+n]` four bytes, and
the slots resolve to `{x, z, w, w, layerKey}` in that order.

`blk+3C8h` is the per-ship look-ahead ceiling `009E44C4` derives from the class
(`docs/SHIP_AI_NAV_BLOCK_CTOR.md`); `blk+168h` is `[unitClass+570h]`, the avoid-zone layer key
(`009E44B4`); `blk+184h`/`blk+188h` is the hull position.

`009D7050` (`__thiscall(rec)(const float* q)`, body `009D7050-009D724E`, read from pseudocode and
its head listing) keeps the cached box when `rec+14h` still equals `q[4]` **and** both diagonal
corners `(q0 ± q2, q1 ± q3)` are inside `rec+4h` (`00414F50 BSP_Geometry_ContainsPoint`,
receiver `EBX = rec+4h` at `009D707B`); otherwise it grows the box by
`max(q[2] * 1.2, (old width)/2, 500.0)` either side (`00CEC160`, `00D7A280`, `00CE3840`,
`00CE397C`), stores the layer key and refills the selected segment runs through `00419FA0` and
`004224C0 -> 00417A40 BSP_AvoidZoneGroup_SelectSegmentRunsInBox`. It is left to its own packet.

## 5. `0080E160` whole

`MOV EAX,dword ptr [ECX + 0x738]; RET` at `009DA6E0`'s four call sites and four more callers.
`__fastcall(ECX = unit) -> controller`, body `0080E160-0080E166`, complete, two instructions.
It is byte-identical to `0080E150 BSP_UnitInstance_WeaponDirector`, which is the unit's
`vtable[114h]`; every one of `0080E160`'s seven callers calls it directly, so the two are the
virtual and non-virtual copies of the same inline accessor. `unit+738h` is the command
controller (`docs/WEAPON_DIRECTOR.md`, `docs/COMMAND_EXECUTION.md`), already
`kUnitOffWeaponDirector` in `include/bsp/unit_weapons.hpp`.

## 6. What the request does to the neighbour list and the sector scan

**It does not change which ships are in the list.** `009F0D20`, the candidate filter that appends
to `blk+608h`, reads none of the four bytes (a whole-`.text` scan for each displacement puts no
hit in its body); its own side rule at `009F0D9E`..`009F0DAF` compares `[self+54h]` with
`[other+54h]` and the value 2 and is unrelated to the request.

What the request changes is one byte per node. `009F0EA0` (chain step 10) ages the list, and for
each surviving node evaluates the inlined `009EC770` with `side = [[node+14h]+54h]`
(`009F1052`..`009F1092`) and passes the answer to `009EAFC0` as its last argument (`009F10B0`
`PUSH EDX`, the byte at `[ESP+18h]` widened). `009EAFC0` stores `node+69h = (accept == 0)` at
`009EAFDE`; its early return for a destroyed entity sets `node+68h` and `node+69h` together at
`009EAFD0`.

`node+69h` has two readers, and both treat it as "this neighbour is not an obstacle":

| reader | site | effect |
| --- | --- | --- |
| `009DD010` | `009DD01D`, before any geometry | returns immediately, so the sector scan's straight and arc probes never clip against the node (`docs/SHIP_AI_SECTOR_SCAN.md` steps 2 and 3 call it through `neighbour_blocks_sweep_009dd010`) |
| `009EF350` | `009EF47B`, `009EF689`, `009EF6B9` | the node is skipped before `node+74h` is set, in the pass `009F4D10` runs as chain step 14 |

So with `blk+3F0h` at `-1` the ship still builds and ages the same neighbour list, still spends
the same work, and simply treats every other ship as transparent; with `3` every side is opaque;
with a Party value only that side is opaque, plus any entity whose Party is 3.

`blk+3F4h` acts one level up: with it clear, `009DA6E0` clears all three avoid-zone segment lists
and resets their layer keys, so `009EB660`'s avoid-zone clip finds nothing, and `009EF910` skips
the static-zone clearance terms at `009EFCA6` (`docs/SHIP_AI_CLEARANCE_PROFILE.md` host steps 3
to 5). `blk+3ECh` acts on the contact-track loop: `009DA1D0` gates both `009E04E0`'s loop over
`blk+404h` (`009E061B`) and `009DE5B0`'s heading override (`009DE8F8`), the one rule that
discards the arm's rudder answer outright.

## 7. `009DA1D0` whole

`__thiscall(blk) -> bool`, `RET 0`, body `009DA1D0-009DA244`, complete. The only reader of
`blk+3ECh` in the image.

```
if (blk+3FCh && [blk+3FCh]->vtable[5Ch](0Eh)) return false   ; 009DA1D3, 009DA1E9
unit = blk+3FCh
if (!unit+0C8h) 00414DB0(unit)                               ; 009DA1FA, 009DA205
if (-15.0 > (double)unit+100h) return false                  ; 009DA20A, 00CE3D58, JA
if (blk+3ECh == 0) return false                              ; 009DA21D
if (0080E160(unit)+240h == 0) return false                   ; 009DA22C, 009DA231
return true                                                  ; 009DA23A
```

`FLD [EDI+100h]; FLD double [00CE3D58]; FCOMIP ST0,ST1` puts the constant in `ST0`, so the `JA`
at `009DA21B` is `-15.0 > depth` and an unordered compare does not take it. The null-unit path
falls into the pose refresh on the same null pointer; the projection keeps the order and leaves
that case to the host.

## Coverage

| routine | range | coverage |
| --- | --- | --- |
| `0080E160` | `0080E160-0080E166` | complete |
| `009DA1D0` | `009DA1D0-009DA244` | complete |
| `009DA6E0` | `009DA6E0-009DA860` | complete |
| `009DABB0`, `009DABC0`, `009DABD0`, `009DABE0` | each 13 bytes | complete |
| `009EC770` | `009EC770-009EC7BE` | complete, as the four gates plus the existing `009EC79B` tail projection |
| `009F1420` | `009F1B7B-009F1B9A` only | partial: the tail that writes the block. The rest of `009F1420-009F1BBA` is `docs/SHIP_AI_GOAL_VECTOR.md`'s |
| `009E1170` | `009E1176-009E11C8`, `009E12DB-009E12FF`, `009E13B4-009E13DC` | partial: the arm selector and the three blocks of block writes. `009E11CE-009E12DA`, `009E1300-009E13B1` and `009E13E2-009E14B5` are not projected |
| `009E14C0` | `009E15B4-009E1605` only | partial: the two arms' block writes. The step itself is `src/ship_ai_state_steps.cpp` |
| `009E1950`, `009E2020`, `009E23B0` | the block writes only | partial: `009E1C28`/`009E1C32`, `009E1EA2`/`009E1EC2`; `009E21F0`, `009E2235`, `009E22D8`; `009E2588`, `009E25CC`, `009E2669`. The steps are their own packets |
| `009E4330` | `009E468B-009E46A3` only | partial: the four block stores. The constructor is `src/ship_ai_nav_block_ctor.cpp` |
| `009EAFC0` | `009EAFC0-009EAFE6` of the body `009EAFC0-009EB651` | partial: the entity gate and the `node+69h` store only. Everything after `009EAFE6` (the box refresh and the closing-speed maths) is not projected |
| `009F0EA0` | `009F1052-009F10FF` only | partial: the inlined predicate and the argument it hands `009EAFC0`. The ageing loop is `docs/SHIP_AI_SECTOR_SCAN.md`'s |
| `009EF350`, `009EF910`, `009ED6B0`, `009F3F80`, `009D7050`, `009DD010` | read for their gates only | partial: not projected here, cited by address |

Ledger records are claimed only where this packet owns the address: functions at `009DA6E0` and
`009DA1D0`, fragments at `009E11A5`, `009EC770` and `009EAFDE`. The arms inside `009E14C0`,
`009E23B0`, `009E1950`, `009F1420` and `009E4330` are projected in
`src/ship_ai_avoidance_request.cpp` and tabulated above but carry **no** reconstruction record,
because those bodies belong to `cc_ship_ai_state_steps`, `cc_ai_attackmove_substates`,
`cc_ai_follow_land`, `cc_ai_goal_vector` and `cc_ai_nav_block_ctor`; `009E4330` was also held by
`agent/orch6-20260912` while this packet ran. Their owners should fold the block writes in.

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/SHIP_AI_STATE_STEPS.md`: "`blk+3ECh` has no reader in what was scanned" and the follow-up's "the other two have no reader yet" | `blk+3ECh` has exactly one reader, `009DA1D0` at `009DA21D`; `blk+3F0h` has three (`009EC770`, `009EF350`, `009F0EA0`); `blk+3F4h` has two (`009DA6E0`, `009EF910`) | whole-`.text` `scan-bytes` for `ec 03 00 00`, `f0 03 00 00` and `f4 03 00 00`; every hit in `009D0000-009F6060` listed in sections 1 and 3 |
| `docs/SHIP_AI_STATE_STEPS.md`: the block is three fields | four. `blk+3F5h` = `brain+3FDh` is written by the same constructor instruction run, by the pre-pass and by `009E1170`, and has its own two readers | `009E46A3`, `009F1B8C`, `009E13DC`; `009ED9FF`, `009F3FEB`; the setter `009DABE0` |
| `docs/SHIP_AI_STATE_STEPS.md`: "`blk+3F0h = -1` is therefore 'no side filter' and 3 a side" | `3` is the wildcard, not a side. `009EC770` accepts when *either* side is 3, and the only writers that store a real side (`009E23B0`, `009E2020`) store the ship's **own** Party | `009EC79F`..`009EC7B1`; `009E2588` and `009E21F0`, both from `[[brain+0AA8h]+54h]` |
| `docs/SHIP_AI_STATE_STEPS.md`: the stop step "writes `brain+3F8h = [unit+54h]`" (the packet brief's reading) | `009E14C0` writes only the constants `-1` and `3`; the `[unit+54h]` store is `009E2588`, in `009E23B0` | `009E15C4` `MOV dword [EDX+3F8h],0FFFFFFFFh`, `009E15ED` `MOV dword [EDX+3F8h],3` |
| `docs/GAME_EXECUTABLE.md` lines 5057 and 5296: "nothing in this process writes the byte", of `blk+3F5h` | The executable's process does not, but the game does: `009E1170` arm 1, the arm that runs when the group's Party slot 1 is held by a person. The milestone's **0 of 15680** gated steps is a fact about the host's inputs, not about the byte having no producer | `009E13DC` `MOV byte [EDX+3FDh],AL` with `AL` = 1 at `009E13DA`; the whole-`.text` scan for `fd 03 00 00` finds three writers and this is the only one that stores 1 |

## Host methods the executable must implement, in call order

`009DA6E0`, `ShipAiAvoidZoneSearcherHost`:

| # | call site | native | method |
| --- | --- | --- | --- |
| 1 | `009DA6F6` | `0080E160` | `director_land_avoidance_0080e160_242` |
| 2 | `009DA724` | `004158A0` | `avoid_zone_segment_list_clear_004158a0(0)` |
| 3 | `009DA73F` | `0080E160` | `director_land_avoidance_0080e160_242` |
| 4 | `009DA76E` | `004158A0` | `avoid_zone_segment_list_clear_004158a0(1)` |
| 5 | `009DA78C` | `0080E160` | `director_land_avoidance_0080e160_242` |
| 6 | `009DA7BB` | `004158A0` | `avoid_zone_segment_list_clear_004158a0(2)` |
| 7 | `009DA7DD` | `0080E160` | `director_land_avoidance_0080e160_242` |
| 8 | `009DA854` | `009D7050` | `avoid_zone_query_refresh_009d7050(0, query)` |

`009DA1D0`, `ShipAiAvoidanceSteerGateHost`:

| # | call site | native | method |
| --- | --- | --- | --- |
| 1 | `009DA1E9+vtable5C` | indirect | `unit_is_kind_vtable_005c(0Eh)`, `contract: unread` |
| 2 | `009DA205` | `00414DB0` | `refresh_unit_pose_00414db0` |
| 3 | `009DA22C` | `0080E160` | `director_torpedo_avoidance_0080e160_240` |

`unit+0C8h` (`009DA1FA`) and `unit+100h` (`009DA20A`) are plain loads, not calls, and are host
methods only because the projection has no unit.

## no_ghidra_function

| Start | Inclusive end | Evidence for the boundary |
| --- | --- | --- |
| `009DABB0` | `009DABBC` | `MOV AL,byte ptr [ESP+4]` at `009DABB0`, the first instruction after nine `INT3` at `009DABA7..009DABAF`; the previous routine is the five-byte getter `009DABA0` (`FLD dword ptr [ECX+998h]; RET` at `009DABA6`). The only exit is the `RET 4` at `009DABBA`, followed by three `INT3` at `009DABBD..009DABBF` and the next routine at `009DABC0`. Ghidra's `BSP_ShipAiState_GoalReachedThunk` body ends at `009DAB1A`, so none of these four is inside a defined function. |
| `009DABC0` | `009DABCC` | `MOV EAX,dword ptr [ESP+4]` at `009DABC0` after the three `INT3` above; `RET 4` at `009DABCA`; three `INT3` at `009DABCD..009DABCF`. |
| `009DABD0` | `009DABDC` | `MOV AL,byte ptr [ESP+4]` at `009DABD0` after those `INT3`; `RET 4` at `009DABDA`; three `INT3` at `009DABDD..009DABDF`. |
| `009DABE0` | `009DABEC` | `MOV AL,byte ptr [ESP+4]` at `009DABE0` after those `INT3`; `RET 4` at `009DABEA`; three `INT3` at `009DABED..009DABEF`, then the already-defined `009DABF0 BSP_ShipAi_SetSpeedCommandedFlag_Unused` (body `009DABF0-009DABF7`). |

Every other address this packet names has a Ghidra function.

## Follow-up packets

| packet | addresses / files | why |
| --- | --- | --- |
| `ship_ai_contact_track_source` (existing, `docs/SHIP_AI_CLEARANCE_PROFILE.md`) | `blk+400h`, `blk+404h`, `009DC060`, `006952A0`, `00695870` | Narrowed, not closed. The list `009E04E0` walks is gated by `009DA1D0`, which requires `director+240h` `torpedoAvoidance`, so a contact track is very likely a torpedo contact. Whoever creates one will say so, and that settles `blk+3ECh` from the producer side. |
| `ship_ai_avoid_zone_searchers` (existing, `docs/SHIP_AI_ARM_FINAL_STEP.md`) | `009DC2E0`, `009D7050`, `blk+0A44h`, `blk+0A64h` | Confirmed open from this side too: `009DA6E0` enables and clears all three searchers but refreshes only searcher 0, so searchers 1 and 2 still have no producer for their box or layer key. |
| `ship_ai_party_slot_gate` | `00521E70`, `00927F10`, `[00E188A8]+18CCh`, `unit+1ACh`, `[[unit+740h]+50h]+1ACh` | The predicate three of this block's writers depend on, read here only as far as its body. What `unit+740h` is, why slot 1 is the group's and slot 0 the unit's, and what byte `+9h` of a Party record means. |
| `ship_ai_traffic_pass_009ef350` | `009EF350` `009EF350-009EF90C`, `009DCEB0`, `node+74h`, `node+75h`, `node+88h` | The third reader of the party filter and the only one whose table is indexed by a raw Party. It carries the out-of-range read of section 2 and is the pass `009F4D10` runs. |
| `ship_ai_neighbour_node_boxes` (existing, `docs/SHIP_AI_SECTOR_SCAN.md` `ship_ai_neighbour_box_refresh`) | `009EAE20`, `009EAFC0` `009EAFE6-009EB0C0+` | This packet read `009EAFC0`'s entity gate and `node+69h` store only; the two oriented boxes and the closing-speed maths after `009EAFE6` are still unread. |

## Uncertainties

1. The pairing of each request field with its director byte is read off the call sites: the two
   are tested together, in that order, at every one of the eight sites. The *names* come from
   `008362A0`'s literal strings. Nothing in this packet observed a torpedo, a ship collision or a
   land zone at run time, so the pairing is a strong inference, not a run-time observation.
2. `009EF350`'s three-byte table and a Party of `3` (section 2): no entity with `+54h` = 3 was
   observed, and `009F0D20`'s side rule treats 2 as the special value. If Party is only ever
   0, 1 or 2 the read is unreachable. That was not established.
3. `009E1170` arm 1's gate object, `[[unit+740h]+50h]`, is not identified; `00521E70`'s body is
   read but not the class that owns the slot array. `docs/CRUISE_COMMAND.md` reads the same arm
   and leaves the same field open.
4. `009D7050`'s arithmetic is taken from its pseudocode plus its head listing and the four
   constants; the x87 rounding of its box growth is not re-derived here, because the routine
   belongs to `ship_ai_avoid_zone_searchers`.
5. `009DA1D0`'s `vtable[5Ch](0Eh)` is `contract: unread`. `14` is what
   `docs/SHIP_AI_ARM_FINAL_STEP.md` already recorded for it; the predicate family is packet
   `cc_unit_kind_query`'s.
6. `009DA6E0`'s `RET 4` argument is never read in the body. The chain table gives the call site
   `009F51D5` passing `seconds`, so the signature is `(float)`, but nothing in the body confirms
   the type.

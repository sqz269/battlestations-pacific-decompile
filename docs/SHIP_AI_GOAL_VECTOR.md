# Who writes the ship AI's goal vector, and how a goal becomes a path point

Addresses: 009F1420 009E2FB0 009DBCC0 009DB820 009ED6B0 009ED3E0 009F1160 007ADC30 009DABF0 009DAE30 009DB7F0

`brain+0B2Ch..+0B34h` had no recovered writer. It has one: `009F1420`, the brain's pre-pass,
rewrites all three components on every AI sub-tick from whatever command the entity is carrying.
This packet reads that producer end to end, then follows the same value through the state step and
`009DE050` into the path planner and out again as the path point the navigation arm consumes.

Names here are hypotheses, not recovered symbols. Reconstruction:
`include/bsp/ship_ai_goal_vector.hpp`, `src/ship_ai_goal_vector.cpp`. Evidence rows:
`reports/ship_ai_goal_vector.json`.

## The chain, in the order one AI tick runs it

| # | address | what happens |
| --- | --- | --- |
| 1 | `009F516C` | `BSP_ShipAi_ControllerStep` calls `009F1420` with `ECX = ai+58h` and the float at `ai+0B18h`. `009F515B` gates the call on that accumulator having reached `ai+0B14h`, so the goal refresh is a sub-tick, not a frame |
| 2 | `009F1457` | `brain+0B38h = 0`. That byte is not the goal's valid flag, see Corrections |
| 3 | `009F146B` | `0071EB60([brain+0AB8h])` returns the active command descriptor: `slot+58h` when `slot+30h == 1`, `slot+18Ch` when it is 2, otherwise the empty singleton at `00E19B98` |
| 4 | `009F1473` | `009E2FB0(brain+0AF8h)(descriptor)` latches it: the vec3 at `descriptor+8h` when the byte at `descriptor+1h` is set, else the zero vector at `00F87574`, and the object `00521EA0` resolves, with the observer pair moved only when the object changed |
| 5 | `009F1478`, `009F1499` | `brain+0B20h` is the raw object, `brain+0B24h` the same object behind `vtable[5Ch](2)` |
| 6 | `009F14B6` | the countdown `brain+0B58h` against the delta. On expiry it reloads as `(period - dt) + countdown`, so the 2.0s schedule at `brain+0B54h` does not drift |
| 7 | `009F14D2`..`009F1522` | the gate `brain+0B28h`: open on expiry, closed again when the target is on another side and this side's recon (`008053C0` then `009DFBE0`) does not hold it, reopened when `00922DC0` calls the raw target a surface target |
| 8 | `009F1529`..`009F1562` | refresh when the gate is open, and also whenever the stored goal's squared length is below `1.0f`. An unset goal therefore asks again every tick |
| 9 | `009F156B` | `009DBCC0(brain+0AF8h)(&out)`: with no latched object the stored triple is already a world position; with one it is a point in that object's frame and `004142E0` carries it out through the matrix at `object+0CCh` |
| 10 | `009F1572`, `009F157B`, `009F1584` | the three stores into `brain+0B2Ch`, `+0B30h`, `+0B34h` |
| 11 | `009E57B4`, `009E57D4` | the movetopos state step reads the goal back and hands it to `009DE050` at `009E580F` |
| 12 | `009DE050` | latches it on `blk+1DCh` / `blk+1E0h` and forces `blk+1C4h = Navigate` (`docs/SHIP_AI_STATE_STEPS.md`) |
| 13 | `009EE5C2` | `009ED3E0(blk)(seconds)` hands `blk+1DCh` and the pose `blk+184h` to the planner `009E3780` at four sites |
| 14 | `009EE5F4` | `009E3C00([blk+2F4h])(&record)` answers with the path point for the pose |
| 15 | `009EE66C` | `00815F30` publishes the point's lateral band into the unit's own order record |
| 16 | `009EE671` | the output block, already reconstructed in `docs/SHIP_AI_NAVIGATION_ARM.md`, turns the point into a bearing |

So an executable milestone that wants a real goal on an AI ship does not write `brain+0B2Ch`. It
installs a command on the entity's slot so that `0071EB60` stops answering with the empty
singleton. Everything downstream follows.

## The two records

`brain+0AF8h`, the latched target, is produced only by `009F1465 LEA EBX,[EDI+0AF8h]`:

| offset | brain | meaning | evidence |
| --- | --- | --- | --- |
| `+14h` | `+0B0Ch` | the resolved command object, observed while latched | `009E3004`, `009DBCC7` |
| `+18h`..`+20h` | `+0B10h`..`+0B18h` | the command's position, or a point in the object's frame | `009E3018`, `009DBCEA` |

The record `009E3C00` fills at `[ESP+78h]`, read-side only because its producer was not read:

| offset | direction | meaning | evidence |
| --- | --- | --- | --- |
| `+00h`, `+04h` | in | the query position, copied from `blk+184h` / `blk+188h` | `009EE5DF`, `009EE5EB` |
| `+08h`, `+0Ch` | out | the path point x and z | `009EE671`, `009EE694` |
| `+10h`, `+14h` | out | the next leg's x and z | `009EE78B`, `009EE79D` |
| `+18h` | out | the path node; zero means no point and no publish | `009EE5F9`, `009EE600` |
| `+1Ch` | out | the direction code `00815F30` takes | `009EE602` |
| `+20h` | out | the byte the arm reads as "the path continues" | `009EE6D3`, `009EE776` |
| `+21h` | out | the byte that gates the bearing | `009EE801` |

The publish at `009EE66C` is `00815F30(unit+0AECh - 54h * [unit+0B40h])(node, direction, low, high)`
with `low = max(30.0f, [blk+2F4h]+8h)` when the direction code is 1 and `30.0f` otherwise
(`00E0E304`, `009EE60E`..`009EE630`), and `high = node+20h * 1.75` (`00D21A88`, `009EE643`).
`00815F30`'s own contract is `docs/SHIP_AI_ORDER_CONSUMER.md`: the two-slot lateral-offset memory.

## The arm's first half is an alternative, not a prologue

`009EDA28`..`009EE57B` runs only when `blk+3A5h` is set and `blk+3A6h` clear (`009EDA34`,
`009EDA41`) and it leaves through `009EE57B JMP 009EF206`. It never falls through to `009EE580`.
`blk+3A5h` is the "another entity's controller owns me" byte that `009ED753` clears when
`007788B0` at `009ED73F` says otherwise; its writers are `009D5B90`, `009DA0D0`, `009DA3B0`, `009DDBC0` and `009DE5B0`, two of
which (`009DDBC0` at `009F51AE`, `009DA0D0` at `009F51B7`) are controller steps of the same frame.

What the branch computes, read but not projected: it takes the same latched goal `blk+1DCh` /
`blk+1E0h`, differences it against the pose `blk+184h` / `blk+188h` (`009EDAA8`..`009EDABE`), and
projects the difference onto a stored frame at `blk+38Ch` / `blk+390h`, storing the along-track
offset at `009EDAE2` and the cross-track offset at `009EDAF8`. It then folds in a stopping distance from `blk+3C4h` and the class's
`+508h`, a heading error against `blk+394h`, the ship's turn radius from `0082E850`, and the
inverse-trig helper `007789D0`, and writes the result through `EDI = &blk+39Ch` with repeated
`00415510` / `00415550` / `00415620` clamps. That is the two-ship closing test the packet brief
asked about: it is station keeping against a leader, and its output slot `blk+39Ch` is the same
slot the path branch pins to `1.25f` at `009EE5BA`.

## The dead accessors

Three routines in the image read or write these fields and nothing calls them: `009DAE30` and
`009DB7F0` copy `brain+0B2Ch`..`+0B34h` out, and `009DABF0` sets `brain+0B38h`. A whole-file scan
for calls, jumps and 4-byte references found none for any of the three. They are inline accessors
the compiler emitted and the linker kept, and they are useful only as the compiler's own statement
that the triple is one value and the byte is a separate one.

## Coverage

| routine | coverage |
| --- | --- |
| `009F1420` | partial: `009F1420-009F158A` projected. `009F158A-009F1BB8`, the countdowns at `brain+0B44h`/`+0B48h` and `+0B4Ch`/`+0B50h` and the proximity scan they drive, is not projected |
| `009E2FB0` | complete |
| `009DBCC0` | complete |
| `009DB820` | complete |
| `009ED6B0` | partial: `009EE580-009EE670` projected here. `009EE671-009EEAA2` is `src/ship_ai_navigation.cpp`; `009EDA28-009EE57B` and `009EEAAB-009EF228` are unprojected; `009ED6B0-009EDA25` stays as `docs/SHIP_AI_STATES.md` left it |
| `009ED3E0` | partial: read in full from the decompiler and used as a host contract, not projected as C++ |
| `007ADC30` | complete |
| `009F1160` | partial: read for `009F127D`, `009F1290`, `009F12B9`, `009F12C9`, `009F1358`, `009F137A` and `009F138C` only |
| `009DFBE0`, `009E3C00`, `009E3780`, `009EC680`, `00BD2F10` | none: host methods or named callees only |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/GAME_EXECUTABLE.md` milestone 2o: "`brain+0B2Ch` / `+0B34h`, the AI's goal vector, has no recovered writer anywhere" | `009F1572`, `009F157B` and `009F1584` write all three components on every AI sub-tick. Outside them only the constructor writes the field, at `009F1290` and `009F12B9` | a `.text` scan for the displacements `0B2Ch`, `0B30h` and `0B34h` with every hit classified from the listing |
| `docs/SHIP_AI_STATES.md`: "`brain+0B2Ch..+0B38h` is the AI's goal vector and its valid flag" | `+0B2Ch..+0B34h` is the goal. `+0B38h` is a separate per-tick byte: `009F1457` clears it, the cruise step sets it at `009E11F2` and `009E13E4`, and `009F4DAF` is its only reader, where it skips the throttle-ceiling step. Nothing tests it before reading the goal; the goal's gate is `brain+0B28h` with the countdown `brain+0B58h` | the six sites the displacement scan finds in the AI range |
| `docs/SHIP_AI_NAVIGATION_ARM.md` and `docs/GAME_EXECUTABLE.md`: the unread first half starts at `009EDA26` | `009EDA26` is mid-instruction. The `RET 4` at `009EDA25` covers `009EDA25-009EDA27`; the half starts at `009EDA28 CMP byte ptr [ESI+3A5h],0` | the stored listing; `ghidra disasm --start 009eda26` rejects the address |
| implied: `009EDA28..009EE670` runs and then `009EE671` consumes its result | the two are alternatives. The first is gated on `blk+3A5h`/`blk+3A6h` and exits at `009EE57B`. That is also why `EDI` still holds the literal 2 from `009ED7EC` when `009EE580` compares it against the steering mode, rather than the `&blk+39Ch` that `009EDFA1` puts there | `009EDA34`, `009EDA41`, `009EE57B`, `009ED7EC`, `009EDFA1` |
| `docs/GAME_EXECUTABLE.md` milestone 2o: "all 98 goal sets of the run are (0,0)", unexplained | with no command installed `0071EB60` returns the empty singleton at `00E19B98`, whose `+1h` byte is zero, so `009E2FC4` substitutes the zero vector at `00F87574` and `00521EA0` resolves nothing. `009DBCC0` hands back `(0,0,0)`, and the squared-length test at `009F155C` then forces the refresh again every tick instead of letting it settle | `0071EB60`'s body, `009E2FBA`, `009E2FC4`, `009F1532-009F1562`, the 16 zero bytes at `00F87574` |
| the ledger name `BSP_ShipAi_BrainConstruct` on `009F39C0` | provisional and **not applied**: `009F39C0`'s `this` is the AI object and the brain is at `ai+58h`. The brain's own constructor is `009F1160`. `009F39C0` is not leased by this packet | `009F5166 LEA ECX,[ESI+58h]` before the `009F1420` call, whose body uses `brain+0AA8h`/`+0AB8h`/`+0AF8h`; `009F39C0` calls `009F1160` |

## no_ghidra_function

| start | end (inclusive) | evidence |
| --- | --- | --- |
| `009DABF0` | `009DABF7` | `MOV byte ptr [ECX+0B38h],1` (seven bytes) then `RET`. The previous routine `009DABE0` ends with `RET 4` at `009DABEA` (`009DABEA-009DABEC`) and three `INT3` at `009DABED-009DABEF`; eight `INT3` follow at `009DABF8-009DABFF` before `009DAC00`. A whole-file scan finds no call, jump or 4-byte reference to `009DABF0` |
| `009DAE30` | `009DAE50` | `MOV EAX,[ESP+4]` at `009DAE30`, three `FLD`/`FSTP` pairs over `[ECX+0B2Ch]`, `[ECX+0B30h]`, `[ECX+0B34h]`, `RET 4` at `009DAE4E` (`009DAE4E-009DAE50`). The previous routine `009DAE20` ends with `RET` at `009DAE25` and ten `INT3` at `009DAE26-009DAE2F`; `INT3` follows at `009DAE51`. No reference anywhere in the image |
| `009DB7F0` | `009DB813` | `MOV ECX,[ECX+4]` at `009DB7F0`, the same three-component copy, `RET 4` at `009DB811` (`009DB811-009DB813`). The previous routine `009DB7E0` ends with `RET` at `009DB7E5` and ten `INT3` at `009DB7E6-009DB7EF`; `INT3` follows at `009DB814`. No reference anywhere in the image |

## Uncertainties

* `009DFBE0`'s body was not read. "This side's recon holds the target" is what the call-site gate
  supports, not a contract from the body.
* `009E3C00`'s body was not read. The record layout above is the consumer's reading.
* `00BD2F10` was not read. That the countdown seed at `009F138C` is a uniform draw in `[0, period]`
  is a hypothesis from the staggering effect.
* The `vtable[5Ch]` slot `009F1491` calls with the literal 2 was not read.
* `blk+39Ch` is not named. Its consumer was not read this packet.
* `009DB820`'s second caller, `009E2BB4`, has no Ghidra function and was not read; only
  `009F396A` inside `BSP_ShipAi_AttackMoveSubStateStep_14E0` was checked, and that routine belongs
  to another packet.
* No run-time evidence was gathered. `bsp_game.exe` reaches this path only through the AI
  controller, and the executable installs no command, which is the very condition the fifth
  correction explains from the listing.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ship_ai_station_keeping_arm` | `009EDA28`, `009EDB6B`, `009EDD9B`, `009EDE87`, `0082E850`, `007789D0`, `009D5B90`, `009DA0D0`, `009DA3B0`, `009DDBC0`, `009DE5B0` | The arm that runs instead of the path follower, and who fills the leader snapshot `blk+38Ch..+3A6h` |
| `ship_ai_path_source` | `009E3C00`, `009E3780`, `009ED3E0`, `009D9E50`, `00811D80` | Unchanged from `docs/SHIP_AI_NAVIGATION_ARM.md`, plus the planner `009E3780` and the `22h`-byte record above |
| `ship_ai_command_slot_producers` | `0071EB60`, `0071E7F0`, `00721A40`, `00E19B98` | Who sets `slot+30h` to 1 or 2 and fills `slot+58h` / `slot+18Ch`, the Lua navigator bindings included. It is the only lever an executable milestone has on the AI's goal |
| `ship_ai_subtick_rate` | `009F50E0`, `009F3DD0`, `009F5191`, `009F51A8` | `ai+0B14h` and `ai+0B18h`, the accumulator and interval that decide how often the goal refresh runs |

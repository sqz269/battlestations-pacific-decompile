# CC13 deck launch refusals: does the image hold a launch the host refuses? (cc13-decklaunch)

Packet cc13-decklaunch, from cc9 priority 1(ii). In USNOS c21 (SHIP_AI_OPEN_ITEMS section 305,
cc9-ships60, runs `s60_p_c21` / `s60_pj_c21`), every 339 strike on "Hangar, Large, 08 01"
(Airfield1) was refused with "no such slot". This packet asks whether the image would have
queued, retried or waited on those launches.

**Answer: no. The image drops them as well.** Both of the Support Manager's launch paths test the
slot state once, when the player clicks. If no slot qualifies, they return without writing
anything, so no request is held and nothing is re-tried on a later tick. The host's refusal is
the image's behaviour. No switch was added. The early refusals come from the order file: its
air_mass lists fill every deck slot less than one sortie before those hangar strikes. The later
refusals are open (section 3.3).

The path does not touch the struck range 007EE000-007F2FFF. Every function below lies in
00673A10-006CCDA0 or at 0089E3C0, and the squadron director (007EEC50) is reached only after a launch has been
accepted. So no adoption note for cc9 is needed.

## 1. The host refusal and the image function it stands for

`src/game_hosts_script_orders.cpp` `GameScriptOrdersHost::player_air_ops_launch` is reached from
the helm-order `launch <base> <slot> <class> <count> <target>` line (`game_hosts_mission_frame.cpp`,
packet cc9_helm_orders_launch). Each line stands for one player click on the Support Manager
screen. The refusal has two forms:

- **Slot 0** (the form every c21 line uses). The host calls `air_ops_pick_launch_slot_006c7210`,
  which returns `slots.size()` when no slot is in state 1 or 5, and then refuses with
  `no such slot`. This stands for **00673A10** (FUN_00673a10, sole caller 00675C40, the Support
  Manager screen, `[supportmanager]` string) at 00674765-006747E7.
- **Slot N > 0.** The host refuses a slot that is not in state 1 or 5 with `the slot is not in
  state 1 or 5`. The screen's own selected-slot launch, 00675C40 at 0067A54A-0067A56D, is stricter:
  it requires state 5 at slot `screen+324h`. The c21 lines do not use this form.

The second "no such slot" at `game_hosts_script_orders.cpp` (in `player_air_ops_order`) is an
index range check on the held-slot order line. It is not involved here.

## 2. The image's launch path when every slot is busy

### 2.1 00673A10, the automatic slot pick

The deck block is the ship's `+72Ch`, or `+1188h` for an IsKindOf(45h) airfield (006746C0-006746F1).
Its slot count is at `ESI+50h` and its slot array at `ESI+4Ch`, with a stride of 58h.

```
00674765  XOR EBX,EBX                 ; index 0
00674767  CMP [ESP+50h],EBX           ; slot count
0067476B  JLE 00674B5D                ; no slots: function exit
00674773..00674780                    ; 0065CAF0 copies slot EBX into a stack local
00674785  MOV EAX,[ESP+124h]          ; the copy's +2Ch, the state
0067478C..006747C2                    ; state 1 or 5 -> 006747FD (take it); every other state,
                                      ; 4 and 6 included, -> 006747C4
006747C4..006747E5                    ; 006BC690 destroys the copy; next slot
006747E7  MOV ECX,[ESP+184h]          ; walk ran out:
006747EE  MOV FS:[0],ECX              ;   unlink the EH frame,
006747F5..006747FC  POP/RET           ;   return
006747FD  MOV [ESP+1Ch],EBX           ; taken: 0067480E 006BF310 stock test, 00674848 006C4780 fill,
                                      ; then the 82h message
```

Between the end of the walk and the RET there are no stores, apart from the EH state at
`ESP+18Ch` and the epilogue. The routine does not write the screen object, the deck, a global or
a message queue, so no request is remembered. The player's next click repeats the walk from
scratch. In this host the harness line plays that click, and a refused line is not re-issued:
the file's next line at a later frame is the "retry".

### 2.2 00675C40, the selected-slot launch

00675C40 at 0067A54A reads the slot at `screen+324h` from the same block (`+4Ch`, stride 58h,
copied by 0065CA50). At 0067A565 it tests `CMP [ESP+FCh],5 / JNZ 0067A664`. Any state other than
5 skips the whole 0067A57D stock test, the 0067A5A7 006C4780 fill and the 0067A5D1/0067A5E6 82h
send. Here too nothing is stored on the refused arm.

### 2.3 Where the image does hold a launch, and why it does not apply

- **006CC690** `BSP_AirOps_LaunchOrQueueSlot` is reached only from 0089E3C0
  `BSP_LuaBinding_LaunchSquadron`, which is the Lua/AI path (for example luaAirfieldManager).
  006CC690 calls 006C7210 `BSP_AirOps_TakeOrGrowFreeSlot`. When no slot is in state 1 or 5, that function
  **grows** the slot array and launches into the new record. When `block+38h` (a launch in
  progress) is set, 006CC72C takes the queue arm instead of 006C7490. That is the image's
  "queue/wait", and the host models it (`air_ops_launch_squadron_006cc690`, `slots.resize`). It is
  not reachable from the Support Manager: 00673A10 inlines the same walk **without** the growth,
  as the comment at the host's refusal already says.
- After a slot is accepted, the 82h message reaches 006CCDA0 order 3, which hands it to 006CA640's
  launch queue. That queue holds an *accepted* slot until the deck can launch. It never sees a
  click that found no slot.

So the image has no mechanism that keeps a refused Support Manager launch, and there is nothing
for an OFF switch to name.

## 3. The logs

**c21's own logs are gone.** In `cc9-ships44\local` only `s60_p_c21.log.inputs`, `.manifest.json`,
`.done` and `s60_launch_s60_p_c21.txt` (the exit summary) remain for `s60_p_c21` / `s60_pj_c21`.
The per-line refusals and deck states below are therefore taken from the c21 order file plus a
surviving USNOS log that uses the same mechanism. The timing figures are inferred and labelled as
such.

### 3.1 What c21 asked for (`s60_os_c21.txt`, frames at 20 per second)

The c20 -> c21 diff adds only `launch Intrepid|Reprisal 0 339 3 "Hangar, Large, 08 01"` at
frames 4000/4040 + 6000k, each followed two frames later by a fallback line with target
`Airfield1`. The same two decks also run the plan's air_mass lists, each 22 consecutive slot-0
lines:

| frames | Intrepid | Reprisal |
|---|---|---|
| 2040-2082 / 2120-2162 | air_mass 339 x22 | air_mass 339 x22 |
| 4000, 4002 / 4040, 4042 | **hangar, Airfield1** | **hangar, Airfield1** |
| 5040-5082 / 5120-5162 | air_mass x22 | air_mass x22 |
| 8040-8082 / 8120-8162 | air_mass x22 | air_mass x22 |
| 10000 / 10040 (+2) | **hangar** | **hangar** |
| 11040-11082 / 11120-11162 | air_mass x22 | air_mass x22 |
| 14040-14082 / 14120-14162 | air_mass x22 | air_mass x22 |
| 16000 / 16040 (+2) | **hangar** | **hangar** |
| 20040-20082 / 20120-20162 | air_mass 331 x22 | air_mass 331 x22 |
| 22000 / 22040 (+2) | **hangar** | **hangar** |
| 24040-24046 | air_mass 339 x4 | (same pattern, +80 frames) |
| 28000, 34000, 40000, 46000, 52000, 58000, 64000, 70000 (+2) | **hangar** | **hangar** |
| 33040, 51040, 60040 (x4); 42040-42660 (x8, 339/810); 62040, 68040, 74040 (331 x23) | air_mass | same |

Each air_mass list issues 22 slot-0 clicks, so it takes every slot that is in state 1 or 5 at
that moment (each deck has 4 slots). The strikes at 4000-22000 each arrive about 98 s after
that deck's latest list. The later strikes leave longer gaps: 28000 comes 198 s after 24040,
34000 comes 48 s after 33040, and 40000 comes 348 s after 33040.

### 3.2 How long a deck slot stays busy (USNOS census log `cc9-physics1\local\p23h_off_os.log`)

This log uses the same launch entry (`l65_os_c1.txt`, Essex slot-0 lines every 1500 frames) and
shows which slot each accepted click got:

| Essex slot | launched (frame) | still busy at | free again by | busy span |
|---|---|---|---|---|
| 1 | 3000 | 6000, 6040 | 7500 | 150-225 s |
| 2 | 3040 | 6000-7540 | 9000 | 225-298 s |
| 3 | 4500 | 6000-7540 | 9040 | 152-227 s |
| 4 | 4540 | 6000-7540 | 10500 | >150 s |

That log shows eight `no such slot` refusals on Essex (frames 6000, 6040, 7540, 10540, 12040,
13540, 16540, 18040; these are helm lines 82, 84, 111, 157, 189, 236, 367 and 504). They cost no
planes: Essex's 40 class-339 stock is used up by frame 18000 (thirteen 3-plane launches plus one
of 1), and every later line refuses with `no stock of the class`.

On the host, carrier decks also land nothing. `summary landing sequencer deck Essex|Intrepid|
Enterprise ... landed=0`, with `slot_tail_unapplied` 15/13/16, while Airfield1/Airfield3 land 69
and 243. So a carrier slot returns to state 1 only through 006C65B0 / 006C5950 when its squadron
dies or is removed, never through a landing. If the image's squadrons recover aboard after the
strike, its slots may free sooner than the host's. That would shorten the busy span above and
could free a slot for some hangar strikes. It is the host's landing sequencer (cc9's
`cc9_landing_sequencer`, the "tail's slot move to state 4 is not applied" note in
`game_hosts_units.cpp` `landing_request_006c54c0`), not a launch-request drop. It is flagged here
as an open lead, not measured.

### 3.3 Reading c21

The strikes at 4000-22000, and the one at 34000, arrive 48-98 s after an air_mass list has taken
every free slot. Sorties last 150-300 s, so these strikes cannot find a slot in state 1 or 5, and
the image's screen would refuse the same clicks. The order file accounts for these refusals.

The strikes at 28000 and from 40000 on arrive 198-350 s after the deck's latest list. If they too
were refused with `no such slot`, as section 305 says, the slots stayed busy longer than any span
measured in 3.2. Two inputs could account for that, and neither is a launch-request drop:

- the carrier landing gap in 3.2: slots are freed only by squadron death;
- order: the host, like 00673A10, walks the slots before 0067480E tests stock. A deck with no 339
  stock left but its slots still out therefore reports `no such slot`, not `no stock`.

The c21 log would separate these two. It is deleted, so a rerun is needed to tell them apart. The plan's own remedy in section 305, "take
squadrons off the early air_mass lists, rather than adding launches that queue behind busy
slots", is the right one, with one correction: the launches do not queue behind busy slots,
because they are dropped in the image too. A hangar line must come *before* the deck's air_mass
list in the same window, or replace lines in it.

## 4. Self-check

- No queue or switch was added, so there is no ON lifetime to check.
- Struck range: 00673A10, 00675C40, 006C7210, 006CC690, 006C4780, 006C0F00, 006CCDA0, 006CA640 and
  0089E3C0 all lie outside 007EE000-007F2FFF.
- Evidence: listings from `exports/bsp/functions/00673a10` and `00675c40`. Names from
  `bsp.py lookup` (00673A10 FUN, caller 00675C40; 006CC690 BSP_AirOps_LaunchOrQueueSlot, caller
  0089E3C0 BSP_LuaBinding_LaunchSquadron; 006C7210 BSP_AirOps_TakeOrGrowFreeSlot).
- Status: analysis only. Nothing was built and no game was run.

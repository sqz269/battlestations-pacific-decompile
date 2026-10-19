# CC13 takeoff notify: 007C17D0 and the SlowTakeoff +AA0h (packet cc13-takeoff)

cc9-approved packet. Two items, each behind a new OFF switch in
`include/bsp/cc13_takeoff_notify.hpp`, wired in `src/game_hosts_units.cpp`. No game runs were
made. The pairs go to cc9-physics24.

| switch | default | what ON does |
| --- | --- | --- |
| `kPlaneTakeoffStartNotifyCc13Bound` | OFF | runs 007C17D0 at both takeoff/Takeoff enters and delivers its C3h |
| `kPlaneShipyardLaunchAa0Cc13Bound` | OFF | carries unit+AA0h per plane; its four single-player readers use it |
| `kPlaneShipyardLaunchSlowPushCc13Bound` | OFF | sub-switch (shared RNG): the retakeoff push may start in SlowTakeoff, whose enter draws 00BD2F10 |

With every switch OFF the build behaves as before. Each ON arm is inside `if constexpr`. The OFF
arms keep the old `record(...)` calls and the 0.0 constant, and they print no new log lines.

## 1. 007C17D0 BSP_Plane_NotifyTakeoffStart

### The bytes (007C17D0-007C18A9, `__thiscall(ECX = plane)`, RET, SEH frame)

- `007C17EB..007C17F3`: when byte `+904h` is clear, it returns.
- `007C17F9..007C1814`: `cur = +900h`. It returns when `cur == 2` and `[[00E188A8]+1FE4h] == 2`.
- `007C181A..007C1820`: session mode 2 (client) goes to `007C187F`, which calls `007C1430(4)`
  when `cur` is 7, 6, 4 or 5.
- Any other mode, `007C1822..007C1869`:
  - `0075B430(&m, C3h)`, then word `+18h` = 0, `+1Ah` = 0, `+4h` = 1, `+1Ch` = 0, vtable
    `00D03504`, `+20h` = the `cur` byte, `+21h` = 4.
  - `0077C2A0(plane; &m, 7, 0)`. No destructor call follows the route.
- The only caller is the JMP at `009CE28B`, the tail of takeoff/Takeoff's enter `009CE270`. That
  enter first stores `+18h = -1.0` (`00D7A260`) and `+1Ch = 0`.

The standalone reconstruction `plane_notify_takeoff_start_007c17d0` (`src/cc13_seg47b.cpp`)
matches these bytes. It works on the raw plane layout. The units host keeps planes as
`GameUnitSlot`s, so the host carries the single-player path over its slot instead. These are
`GameUnitsHost::Impl::plane_notify_takeoff_start_007c17d0` and `deliver_takeoff_notify_c3h_007cd02c`.

### Single player: the message loops back locally

`[[00E188A8]+1FE4h]` is the session mode: 0 in single player, 1 on a host, 2 on a client.
- In mode 0 the router `0077C2A0` sets `EBX = 1` at `0077C333`. Then `0077C43A TEST BL,1` /
  `0077C44D` posts the message to the loopback queue with `0076E520` (ECX = the session).
- The peer loop (`0077C3CB..`) runs only in mode 1.
- So this is **not** a peer-only send like `0077C7B0`. The C3h comes back to the same plane.
- The pump drains the queue at row 9 of the same fixed step: `00778542` calls `0076C600`, which
  calls `00780670`. Row 9 runs after the entity think, where the takeoff task runs.

`007CCFA0` BSP_Plane_HandleMessage dispatches kind C3h: the byte table `007CD1D8`, then the dword
table `007CD1AC`, then `007CCFEF`. Requested state 4 goes through `007CD22C` to `007CD02C`, with
ECX = the message's `+20h`:

| test | arm | effect |
| --- | --- | --- |
| `+20h == 7` (`007CD02C`) | `007CB5F0` | the touchdown authority arm |
| live `+900h` is 4 or 5 (`0042A7E0` at `007CD042`) | `007C7430` | see below |
| otherwise (`007CD05C`) | `007CA3F0(0)` | the ground state from Locked |

`007C7430` (`007C7430-007C7493`):
- In mode 2 with `+900h` not 4 or 5, it calls `007C71E0`. Single player skips this.
- `+904h` = 0 when set (`007C7460`).
- `+910h` = 0 (`007C746F`).
- `+908h` = 0.0 (`007C7475`).
- `+900h` = 4 when `+BF8h` is clear and `+900h` is 5 (`007C7488`). This is a plain store with no
  `007C11E0`.

### Who reads what 007C7430 changes

- **`+904h`** (the landed byte). The Takeoff step clears it anyway at `009CE2E0`.
  - Readers: `006CE4A0` (site HasLandedOccupant, with `+910h`), the takeoff hold, 006CF5B0,
    the landing passes, 007BB9A0/00604A20, 008906A0/00890510 (Lua) and 009CF8E0's parking choice.
- **`+910h`** (the hangar-queue byte). Readers: `006CE4A0`, `006CF5B0` and the land task.
  The state-7 setter clears it again at lift-off.
- **`+908h`** (the airborne clock).
  - A lift-off from state 4 resets it to 0 anyway (the setter's state-7 arm).
  - Before lift-off it is read by the 4/5 re-derivation `+904h = +908h > 5.0` (007C1680 /
    007C16F0 / 007CA3F0) and by Lua `GetSquadFlyTime` (`008A5D60` -> `007EE150`).
  - The other readers (007CC440's gate, the core law, 0099D300, the Takeoff step's free arms)
    run only in free flight.

### Host wiring (ON)

- **SlowTakeoff -> Takeoff** (`takeoff_rule_009cfc70`, 009CFC70):
  - The rule calls `plane_notify_takeoff_start_007c17d0(unit_)`.
  - The C3h is held on the slot (`takeoff_notify_c3h_pending`, and `takeoff_notify_c3h_current`
    = the `+20h` byte).
  - The host's task tick (`run_takeoff_task_tick_009cfd70`) then delivers it through the host
    method `deliver_takeoff_notify_c3h_007cd02c(unit_)`, after the state's step. This is a host
    call, not an image call.
- **Ground retakeoff push** (`push_ground_retakeoff_0099b107`, 0099B107 -> 009CF8E0 with DL = 0):
  - The Takeoff arm calls `plane_notify_takeoff_start_007c17d0(p)`.
  - The push delivers the C3h at its end.
- **The delivery** (`deliver_takeoff_notify_c3h_007cd02c`):
  - A held `+20h == 7` is counted and logged, not run (see "Labelled").
  - A live state 4 or 5 runs 007C7430's single-player writes.
  - Any other state calls `ground_state_from_locked_007ca3f0(p, deck, false)`. With no holder
    (`+BF4h` = 0) it calls only `plane_site_probe_007c5ac0(p, -1.0f)`, which is where that host
    function stops for a plane without a holder.
- **Counters:** a `summary takeoff start notify:` line, printed only when the switch is ON.
- **Logs:**
  - The first ten posts each log a `takeoff start notify` line with `+900h`, `+908h`, `+910h`
    and `+BF8h`.
  - An `007CA3F0` arm delivery always logs a line.

### Labelled

- **Delivery time.** The image delivers at row 9, after every unit's think.
  - The host delivers after the plane's own takeoff step (SlowTakeoff path) or at the end of the
    push (retakeoff path).
  - In between, only other planes can tell the difference, through this plane's `+904h`/`+910h`
    in `006CE4A0` / `006CF5B0`. At most one think differs.
  - On the SlowTakeoff path the step has already cleared `+904h` (`009CE2E0`) before either
    delivery, as in the image.
- **The 007CB5F0 arm** is not run. `+904h` and state 7 never coexist: every image writer of
  state 7 clears `+904h` (`007C7183`, `007C6481`, `007D6600`, `007C70F2`), and so does every host
  writer. So a posted C3h cannot carry 7. The arm is counted (`arm_7cb5f0`) and logged.
- **The client arm** (mode 2) is not modelled. Single player never takes it.

### ON-only lookups and their containers

- **`GameUnitSlot::plane_landed_904`**, read at the enter. Writers:
  - the 4/5 setters (host `:15607`, `:15618`), the touchdown (`plane_touchdown_007cc440`) and
    `ground_state_from_locked_007ca3f0`, each as `+908h > 5.0`;
  - cleared by the state-7 writers, by the Takeoff step and by 007C7430.
  - Nothing moves the slot. `slots` holds `unique_ptr`s.
- **Fresh launches** reach 007CA3F0 from state 2 after a member was Inside (state 1).
  - `plane_enter_state_one_007cc820` resets `+908h` to 0 there.
  - So 007CA3F0 stores `+904h` = false, and 007C17D0 returns at once.
  - In the image the launch bag of `006C5050` pushes `State` = 1 (`006C51CF..006C51DD`: `flag ? 7
    : 1`), and 007D5D20 then stores `+908h` = 0.0 (`007D65DA..007D662C`). The two agree.
  - **Risk:** the host seeds `+908h` = 3600 at creation (`create_units`). A plane that reached
    007CA3F0 without passing state 1 would carry `+904h` = 1 in the host but not in the image.
    ON would then post a C3h the image does not.
  - The `takeoff start notify` log lines show `+908h` for every post, so an unexpected post on a
    carrier row is visible at once.
- **The held C3h** lives on the slot. It is written and read within one `run_takeoff_task_tick`
  or one push call, so nothing clears it in between.
- **`landing_decks[+BF4h - 1]`** (007CA3F0 arm only). Built at scene load, never erased; `+BF4h`
  is range-checked.
- **`+BF8h`** (`plane_ground_contact_bf8`) is the per-step 006BC530 answer, current at delivery.

## 2. The SlowTakeoff `+AA0h`

### Writers (single player)

- **007D5D20** BSP_Plane_ReadPropertyBag:
  - `+AA0h` = 0.0 (`007D613C XORPS`, `007D614B`).
  - For a kind-1 holder (`[+C0h]+4 == 1`), `008F2260(bag, "ShipYardLaunch")` (`00D0659C`,
    `007D6444..007D644B`). When found, 5.0 (`00CE3850`) if the record's `+0Ch` byte is set, else
    -1.0 (`00D7A260`) (`007D6454..007D646C`). An absent key keeps 0.0.
  - A kind-2 holder (a network create descriptor) uses its `+12Ah` the same way
    (`007D634D..007D636C`). It does not occur in single player.
- **007B83F0** BSP_Plane_SetLaunchDelayAA0, the plane's 7Ah handler (`007CD171`): 5.0 for a set
  byte (`007B83FF`), else -1.0 (`007B8412`). The senders:
  - `009CFC38`, SlowTakeoff's 60 m arm, byte 0;
  - `00813886`, BSP_UnitInstance_RouteShipyardLaunch, byte 0;
  - `007604C0` is the network default constructor.
  - So no single-player message stores 5.0.
- **Where the key comes from.** `ShipYardLaunch` appears in no `.scn` scene and no script of this
  installation. A byte search of `universe\` and `scripts\` found no file. The search is not
  vacuous: `WingCount` hit 155 files and `StartSpeed` 208.
  - The only bag that carries it is the shipyard's `00844FC0` (`008450AA`, value 1).
  - Current-main JM08 builds 12 shipyard units, all `TBoatGen` (Gyoraitei). Every other WINS log
    builds none.
  - So **every plane in every WINS mission has `+AA0h` = 0.0**.

### Readers (single player)

| reader | test | effect |
| --- | --- | --- |
| `009CF8E0` (and the unreferenced copy `009CD3C0`) | DL = 0: `> 0.0` (`009CF975` COMISS, `009CF982` JA) | the retakeoff push starts in SlowTakeoff (`+490h`), else Takeoff (`+470h`) |
| `009CFB60` | `> 0.0` (`009CFB89`, JA) | SlowTakeoff leaves at once, or only after 60 m (then 7Ah -> -1.0) |
| `007BB9A0` | `<= 0.0` (`007BB9B4`) | the local input gate (INTF_PLANE vs INTF_PLANESPAWN) |
| `007BC5B0` | `0.0 >= +AA0h` (`007BC5D5`, JB: NaN fails) | the water-contact gate returns 0 for a plane with a delay |
| `007C2680` | create message | network only |
| `009CD370` (unreferenced) | `> 0.0` | none |

Downstream with a positive `+AA0h`:
- **The install path** (0099A4A0, DL = 1) always goes prepare -> SlowTakeoff. SlowTakeoff then
  holds until the plane is more than 60 m (planar) from where it entered. Its step 009CE160 is
  the slow roll (`run_takeoff_slow_step_009ce160`). The plane rolls out of the hangar before
  Takeoff's full-throttle run starts.
- **The retakeoff push** starts in SlowTakeoff instead of Takeoff.
- **The player's plane** keeps INTF_PLANESPAWN until it has rolled out.

### Host wiring (ON)

- **`create_units`**: a plane created from a record with `shipyard_launch` set gets
  `plane_shipyard_launch_aa0 = 5.0f`.
- **`takeoff_slow_leave_009cfb60`** reads the slot.
  - The 60 m test runs at 24-bit precision control (see below).
  - On leaving, it stores -1.0 (the 7Ah to 007B83F0(0), delivered at once).
- **`push_ground_retakeoff_0099b107`**: SlowTakeoff (with `takeoff_slow_enter_009ce1d0`) when
  `> 0.0`. This also needs the RNG sub-switch, because 009CE1D0 draws `U(-0.3, 0.3)` from 00BD2F10.
- **`plane_local_input_gate_007bb9a0`**: `gate_float_aa0` = the slot value.
- **The water-contact site** (007CB7F0's gate): a positive or NaN `+AA0h` makes `007BC5B0` false.
- **Counters:** a `summary shipyard launch aa0:` line, printed only when the switch is ON.

### x87 and constants

The takeoff task runs in the entity think on the main thread, so I assumed 24-bit precision
control.
- `009CFBC0` / `009CFBCD` store dx and dz to float.
- Then dx*dx, 0*0 and dz*dz are each rounded to float and summed in that order. The result is
  stored to float at `009CFBE7` and compared with the double 3600.0 (`00CE3D70`, FCOMIP/JBE).
- The ON arm computes `float(float(dx*dx) + float(dz*dz))`. At 53 bits the sum would be rounded
  once, a difference of at most 1 ulp.
- The OFF arm keeps the previous one-rounding expression. It is unreachable while `+AA0h` is 0.0.
- Constants: 5.0f `00CE3850`, -1.0f `00D7A260`, 0.0f `00D7A218` (all MOVSS/COMISS floats), and
  3600.0 `00CE3D70` (FLD qword).
- `tools/const_width_sweep.py --all --load-sites` reports A-WRONG 0, B 0, and no row for these
  constants.

### Labelled

- The record keeps only the set bit of `ShipYardLaunch`, so a key that is present but clear stays
  0.0 instead of -1.0. Every reader tests `> 0.0` or `<= 0.0`, and 0.0 and -1.0 agree on both.
- Wing members (`queue_plane_squadron_wing_007f4580`) do not copy `shipyard_launch`.
  - In the image they clone the squadron bag (00922DE0), so they would carry the key.
  - The shipyard builds plane squadrons with `WingCount` 1, so only the fused leader exists.
- 007BC5B0's `+61h`/state-7/6/4 tests are still not modelled at the water site. Only the `+AA0h`
  term is added.
- 007C2680 (the network create message) is not modelled.

### ON-only lookups

- **`GameSceneEntityRecord::shipyard_launch`**, read inside `create_units` from the vector its
  caller passes.
  - Filled by the scene loader (`game_hosts_scene_contents.cpp`, the bag's `ShipYardLaunch`; no
    scene has it) and by `create_shipyard_unit_00844fc0` (`game_hosts_lua.cpp`, always set).
  - It is not cleared before the read.
- **`GameUnitSlot::plane_shipyard_launch_aa0`** is written only at creation and by the 60 m
  leave.

## 3. Reach and expected ON effect per WINS mission

The counts come from current-main OFF logs (cc9-physics1 `p24_off_*` on 2026-10-19, cc9-lua52
`l68r`/`l70r` refs).
- "enters" = SlowTakeoff -> Takeoff lines plus retakeoff pushes into Takeoff, which is
  `summary takeoff start notify: calls=` with the switch ON.
- "+904h set" = retakeoff pushes logged with `landed=1`. Fresh launches carry `+904h` = 0 (see
  1).

| mission (frames) | enters | retakeoffs, landed=1 | shipyard planes | expected ON (notify) | expected ON (+AA0h) |
| --- | --- | --- | --- | --- | --- |
| JM08 (30000) | 84 | 0 | 0 (12 TBoats) | posts=0, identical | identical |
| USN04 (40000) | 15 | 0 | 0 | posts=0, identical | identical |
| USN21 (80000) | 480 | 0 | 0 | posts=0, identical | identical |
| USN01 (30000) | 0 (20 landed touchdowns, no push) | 0 | 0 | calls=0, identical | identical |
| BSM09 (36000) | 53 | 0 | 0 | posts=0, identical | identical |
| BSM01 (24000) | 0 | 0 | 0 | calls=0, identical | identical |
| IJN08 (100000) | 81 | 0 | 0 | posts=0, identical | identical |
| USN15 (90000) | 0 | 0 | 0 | calls=0, identical | identical |
| USNOS (30000) | 109 | 0 | 0 | posts=0, identical | identical |
| LOMP06 (16000) | 0 | 0 | 0 | calls=0, identical | identical |
| JM06 (40000) | 3 | 0 | 0 | posts=0, identical | identical |

Only one row reaches a post: USNOS at 75000 frames (cc9-ships44 `s60_b_c18c`).
- It has 4 airfield retakeoffs in state 4 with `landed=1`: Airfield2_sqn43 II/III at 1676/1686 s,
  Airfield2_sqn67 II at 2201 s, and Airfield1_sqn70 III at 2328 s.
- Expected ON: `posts=4 arm_7c7430=4 five_to_four=0`.
  - Each delivery clears `+910h` and zeroes `+908h` at the push, one think before the Takeoff
    step would have cleared `+904h`.
  - The lift-off from 4 zeroes `+908h` anyway.
  - The visible change is at most a one-tick difference in other airfield planes' taxi hold /
    006CF5B0 answers, plus a `GetSquadFlyTime` reading if a script polls it in that window.
- A non-main BSM01 variant (`l69r_b1d_main_j3`, 40000 frames) shows one such push.

**Best carrier row for the pair:** USN21 at 80000 frames.
- It has 480 carrier Takeoff enters, the most of any row, so it is the strongest check that ON
  posts nothing on fresh launches.
- No carrier row reaches a post. If a carrier row shows `posts > 0`, that is the `+908h`
  creation-seed risk of section 1, not 007C17D0.
- For a row that reaches the delivery, use USNOS at 75000 frames.

## 4. Uncertainties

- `00780670`'s delivery was not re-read beyond the docs (GUNNERY_OPEN_ITEMS 26,
  ENTITY_ORDER_MESSAGE). That covers whether a dead plane (`+5Dh`) still gets its C3h, and
  007CCFA0's first test `vtable[0Ch](62h)` on the message.
- 007CA3F0's arm for states outside 1..5 is the host function's existing approximation, which
  stops after the probe without a holder. No log reaches it.
- Whether any untested mission builds a plane squadron from a shipyard. The shipyard
  `AddShipyardStock` class lists were only read from logs.
- `009CD370` and `009CD3C0` have no reference in the call sweep or in an absolute scan. They were
  recorded, not wired.

# CC13 GPR 52: segment 52 reference models (director, squadron block, shipyard, support site)

Packet `cc13-gpr52` (cc13 lineage, Claude). Segment 52 is `008288D0`-`00851CB0` (gameunit, classid,
stock, torpedoavoidance, object, state). cc9 approved this reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg52_cc13.cpp` and `include/bsp/gameplay_seg52_cc13.hpp`,
  registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

Excluded: ship motion `00825F20` and the propellers `00834E90` (hosted; reference only), ship AI, and
the addresses cc9 had leased when this pass started (`0084BAD0`-`0084BEFC`, `cc9_g39_blastbox`;
`0084E010`, `cc9_lua56_carrier_recovery`). The lease covered the rest of the segment.

Method. Every branch and store was read from the disk bytes with Capstone `disasm-raw`; the x87 was
read from the opcode bytes (`00838463`, `008385E9`: both `D8 C9`, so `ST0 = ST0 * ST1`).
`tools/x87_dc_form_scan.py --range 008288D0-00851CB0` finds six DC-form sites (in `0082FE30`,
`00834820`, `00834CC0`, `00834E90`, `00845670`, `0084BC60`). None of them is in a function modelled
or checked here. The models are **reconstructed and build-tested**. None of them is ABI-compatible,
wired or game-validated. The one float sum (`0084F020`) is modelled in float, as the 24-bit gameplay
precision rounds it.

The segment had 307 functions with no record and no tag. Of these, 51 are EH funclets
(`Catch_All@`/`Unwind@`) and 109 are named as STL or TMPL instantiations. Before this pass, 61 of the
256 non-funclet entries were already named in a `src/` or `include/` comment or host.

## 1. Models

### 1.1 Turning guns: the delayed-fire request `gun+4D4h` (`0084C4E0`, `0084C5B0`)

**This is the finding with a host consequence.** Slot `1DCh` (FireIfReady) of both turning-gun
vtables is `0084C4E0`, not `00727E30`:
- `00CFBF58` (MRTGun, class 24h in docs/GUN_SHOT_CADENCE.md), dword `00CFC134`;
- `00CFC190`, dword `00CFC36C`.

```
0084c4e3: call 0x727e30            ; FireIfReady
0084c4ea: je   0x84c4f7
0084c4ec: mov  byte ptr [esi + 0x4d4], 0   ; a shot clears the delayed-fire request
0084c4f3: mov  al, 1
```

docs/GUN_SHOT_CADENCE.md 10.1 lists three writers of `+4D4h` (`006FDCA8`, `006FDCF4`, `006FDF62`), and
says the only clear is `006FDC90`. `0084C4EC` is a fourth writer and a second clear. The `0ADh` arm
`0072D860` calls `vtable[1DCh]`, so on these two classes every FireIfReady that fires drops the
request. The tick `0084C5B0` sends its extra `0ADh` only while `+4D4h` is still set after `0072D130`
(`0084C5DB`). So a delayed request yields a single ungated shot. After that shot the gun fires only
through `0072D130`'s gates.

The host (`src/game_hosts_gunnery.cpp:7793`, `immediate_4d4`) keeps the request until the latch drops
(`:7728`). It models the `0ADh` arm as `00727E30` (`:7813`).

Open: when the routed `0ADh` of `0072D130` is delivered. If it is delivered at once, its `0084C4E0`
can clear `+4D4h` before `0084C5DB` reads it in the same tick. The model takes `+4D4h` after the
tick as its input.

### 1.2 Shipyard hangars (`00845750`, `00845870`, `00844A10`, `00845AC0`, `00847030`)

The hangar records are `+780h..+788h` (10h each, `+4` the hangar Object, filled by `00849F70` from
`"Hangar %d"`). "Live" is the inline test `+5Ch` set, then `+5Dh`, `+60h` and `+5Eh` clear.

- `00845750` returns the first live object. `00845870` returns that object's `vtable +C4h` position,
  or else the shipyard's world position `+FCh`. Neither tests `record+4` for null.
- **`00844A10` is the artillery aim point**, MShipyard vtable `00D0B770` slot `100h` (dword `00D0B870`),
  which cc9 identified at `src/game_hosts_gunnery.cpp:1093`/`:1116`. It is not a launch pose. It counts
  the live records whose object is not null. With `n >= 2` it draws `k = _ftol(U(0, n - 1))` on
  **stream 1** (`00844AAF MOV ECX,1`, `00844AB4`, `00844AB9`) and takes the k-th live record; with
  `n < 2` it takes the first. The point is that object's world position carried into the shipyard's
  frame through the cached inverse `+110h` (`+10Ch` the valid flag). With no live record the point is
  `00F87574` (zero). docs/CC13_RNG_STREAM_CENSUS.md lists the draw as host NOT-MODELLED.
- **`00845AC0` is the shipyard's copy of the airfield destruction rule** `006D40F0`: slot `1A8h`
  (dword `00D0B918`), the same slot as the airfield's (`00CF8DB0`). While any hangar is live it does
  nothing. Otherwise:
  - a shipyard adopted by a CommandBuilding (`+71Ch != 0`) gets `vtable +194h("InferiorFailure", 0)`;
  - any other gets `vtable +70h(1)`. `00845B55` pushes 1, where the airfield passes 0
    (`src/game_hosts_units.cpp:5797`).
  - The C++ (cc13-fixes2) makes both calls through `ShipyardHangarsLostHost`: the 15-character
    temporary is built from `00CF0B74` (`0041DD40`, memcpy of 16 bytes), passed by address with 0, and its
    buffer released to the sized pool afterwards. The earlier version returned an enum instead.

  **The caller is not found.** The shipyard tick `00846320` does not load `+1A8h`; the airfield tick
  loads it at `006D2526`. A scan for `FF ?? A8 01 00 00` finds no call, and the 68
  `8B ?? A8 01 00 00` loads were not sorted.
- `00847030` dispatches message kinds `A7h..AAh` (change class, change equipment, launch slot, spawn)
  and passes every other kind to `0095ABE0`.

### 1.3 Weapon director (`00835F60`, `00835B40`, `00836040`)

- **`00835F60`** (base slot `7Ch`; the first call of `00836920`) releases the fire target `+238h`. It
  routes message `5Eh` (vtable `00D02E98`, routed `7, 0`) on the owner unless one of these holds:
  - the target is `IsKind(41h)`;
  - the target is live, of another party, and either `IsKind(1Ch)` or of a party other than 2
    (`005457C0`).

  A dead target, a friendly one or a party-2 one is released. The host's director step
  (`src/game_hosts_commands.cpp:2805`) starts after this call.
- **`00835B40`** (derived slot `70h`). The earlier ledger text was wrong, and the name record now
  carries the correction: the image calls `0071EDD0(target, primary)` whatever the object compare
  gives, and returns 0 when that call answers 0. A stage-2 raise (`0071D810` for primary, `0071D9E0`
  otherwise) needs all of these:
  - the same resolved object;
  - session mode != 2;
  - a moveto or attackmove in the slot.
- **`00836040`** (the ship director's slot `14h`, called by `0071E6C0`). A target that resolves to
  the director's own unit (`+34h`) is rewritten to the empty target for `cruise` and `stop` and
  answers 1. `0071E6C0` then refuses the push when a slot is filled. The host override
  (`src/game_hosts_commands.cpp:1120`) always answers 0.

### 1.4 Plane squadron command block, vtable `00D0BD98` (`0084DAE0`, `0084DB90`, `0084DC50`, `0084DD20`)

Slots are 1Ch records from `+54h`. "Kind 1/2" is the command's `vtable +0Ch` in {1, 2} (`006BC030`).

| slot | address | rule |
| --- | --- | --- |
| `+20h` | `0084DAE0` | `0071DEE0(slot)` must answer. Slot 0 holding a resolving moveto is refused when the owner `+224h` and the target answer the same `vtable +13Ch`. |
| `+34h` | `0084DB90` | Needs `0071C0C0`; with nothing queued, yes. It never queues behind land, returntobase or retreat. Behind a resolving moveto only a kind-1/2 command, a moveto or a moveonpath queues. Behind a kind-1/2 command only a kind-1/2 command queues. Anything else queues. |
| `+30h` | `0084DC50` | The base `0071E550` first. A non-kind-1/2 command goes on only as a resolving moveto. The last slot is dropped (`0071D900`) when both kinds are 1/2, or when the last slot holds a resolving moveto. |
| (direct) | `0084DD20` | For a self-target: a moveto answers 1 unchanged, a stop is rewritten to the empty target and answers 1. The ship director rewrites cruise and stop instead (`00836040`). |

### 1.5 Support site (`0084D170`, `0084CE60`, `0084D050`)

- **`0084D170`, the activation.** It reads `maxPlanes` (4), `maxShips` (2), `SPActive` (1) and `Angle`
  (`0E6h`) into `+334h`, `+338h`, `+340h` and `+33Ch`. `JapanList` fills `+31Ch` and `AlliedList`
  fills `+328h`. Each list takes one `{Type, Count, SquadSize=3}` entry per `"Stock %d"` bag with
  `Count != 0`.

  **The two walks share one counter.** EBX is set to 1 once (`0084D25D`), and the AlliedList walk
  starts with `PUSH EBX` at `0084D37E`. With Japan stocks `1..N` authored, the Allied walk reads
  `"Stock N+1"` onward. An Allied list numbered from 1 therefore loses its first N entries. This
  matters for docs/SHIP_AI_OPEN_ITEMS.md 52.2: `ijn_11_operation_to.scn` and the `ijn_02_force_z.scn`
  copies author these lists, and whether their Allied stocks continue the Japan numbering was not
  checked.
- **`0084D050`.** A class with no map entry passes. Only an entry whose enabled byte is clear refuses.
  The ledger's earlier wording "requires a matching enabled entry" overstates it. A present owner
  needs health `> 0`, and a NaN health refuses.

### 1.6 Stationary hit (`0084F020`)

A dead prop is untouched. Otherwise the hit damage against the armour `+180h` (`004706D0` for part
index -1, else `00470510`) is added to `+1A0h` in float, only when it is `> 0` (`COMISS`/`JBE`, so NaN
skips it). `0084EB80` runs either way (`stationary_check_destroyed_0084eb80`, cc13-gp80).

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `turning_gun_fire_if_ready_0084c4e0`, `turning_gun_extra_fire_request_0084c5b0` | the `immediate_4d4` hold `src/game_hosts_gunnery.cpp:7793` and the FireIfReady arm `:7813` (taken as `00727E30`) for the two turning-gun classes | `00727E30`'s answer (the host's gate), `+4D4h` (`immediate_4d4`) | none | OFF: none. ON: an MRTGun stops its ungated per-step `0ADh` after the first shot of a delayed request. Rapid turning guns fire fewer shots. Any per-shot stream-0 draw downstream of a shot (the fire stagger) then shifts, so judge the pair per gun, not by kill totals. |
| `shipyard_aim_point_pick_00844a10` | the labelled omission at `src/game_hosts_gunnery.cpp:1116` (an artillery aim at a shipyard takes the origin raised by Height) | the shipyard's hangar records (`00849F70`), their live bytes and world positions, the shipyard's inverse world matrix | **stream 1**, one `U(0, n-1)` per aim refresh when `n >= 2` | OFF: none. ON: artillery at a shipyard aims at a live hangar, and every later stream-1 consumer shifts by one draw per refresh at a shipyard with two or more live hangars. |
| `shipyard_hangars_lost_00845ac0` | none (the airfield's copy is hosted behind `kAirfieldDestructionRuleBound`, `src/game_hosts_units.cpp:5797`) | the hangar records, `+71Ch` | none | Not wireable until its caller is found. ON: a shipyard with every hangar dead takes InferiorFailure or dies through `vtable[70h](1)`. |
| `shipyard_first_live_hangar_00845750`, `shipyard_launch_position_00845870`, `shipyard_message_arm_00847030` | none; `src/game_hosts_gunnery.cpp:12129` already filters hangars the same way for `0081DE10` | the hangar records | none | none in single player (the launch messages are not sent by a host) |
| `director_release_fire_target_00835f60` | none: it runs ahead of `director_step_00836920` (`src/game_hosts_commands.cpp:2805`) | the fire target (`+238h`), its live bytes, party and IsKind(41h/1Ch); the owner's party | none | OFF: none. ON: a director whose fire target died or turned friendly or neutral sends message 5Eh each step. Its receiver is not traced in this pass. |
| `director_retarget_command_slot_00835b40` | none (the derived slot `70h` is not hosted) | `0071EDD0`'s answer, the resolved objects, the session mode, the slot command | none | none until slot `70h` is bound |
| `director_normalize_self_target_00836040` | `src/game_hosts_commands.cpp:1120` (always 0) | the target, whether it resolves to the director's unit, `00F87574` | none | OFF: none. ON: a cruise or stop aimed at the unit itself is emptied, and a second such push is refused. No single-player host target resolves to `+34h`, so no change is expected. |
| `squadron_*` (`0084DAE0`, `0084DB90`, `0084DC50`, `0084DD20`) | none (the squadron block's queue slots are not hosted; `0084D810`'s construction is referenced at `src/game_hosts_script_orders.cpp:2367`) | the slot commands and kinds, target resolution, `0071C0C0`/`0071BE60`/`0071DEE0`/`0071E550` answers | none | none until the block's slots are bound |
| `support_site_activate_0084d170`, `support_site_set_active_0084ce60`, `support_site_can_supply_class_0084d050` | none (the support manager `00675C40` is cc9's and not hosted) | the site's scene properties; the class map `+314h`; owner health | none | none in the reference rows (no reference scene authors a stock list, SHIP_AI_OPEN_ITEMS 52.2) |
| `stationary_apply_hit_0084f020` | none (no host routes a hit to a stationary prop) | the hit damage (`004706D0`/`00470510`), `+150h`, `+17Ch`, `+1A0h` | none | OFF: none. ON: stationary props take damage and die at `+17Ch` unless protected. |

## 3. Host reconstructions checked against the listing (records added, no host edit)

| address | host | result |
| --- | --- | --- |
| `0082E850` | `class_turn_radius_0082e850`, `src/game_hosts_ship_ai.cpp:1932` | Same terms. The multiplier is the labelled 2.0 stand-in for `[00424C40]+438h`. |
| `00828F20` | `ship_class_ai_derived_motion_00828f20`, `src/ship_ai_throttle_ring.cpp:157` | Same. Each quotient rounds once at its store, so the double intermediates agree with the 24-bit image. |
| `008383D0` | `weapon_hit_accuracy_sample_008383d0`, `src/gameplay_settings_tail.cpp:132` | Same terms, with two differences. (1) A NaN `t` takes the image's interpolation arm (the `FCOMIP`/`JB` at `00838437` jumps on unordered, giving i = 8 and a NaN result); the host takes bucket 0. (2) The host forms `-10t` and `10t` in double, where the image rounds them to 24 bits. At `t = 0.7f` the image takes `i = 6, b = 0` and the host `i = 5, b = 0.99999988`; the results agree to about 1e-7. |
| `008386F0` | `weapon_hit_accuracy_008386f0`, `:173` | Same. |
| `00838530` | `weapon_hit_accuracy_range_fraction_00838530`, `src/ship_ai_settings_block.cpp:234` | Same terms. The host sums `L*(1-w) + S*w` in double, where the image rounds each product to 24 bits (at most 1 ulp). |

## 4. Ledger

Function records (`config/reconstruction`, status `semantic_build_tested`), models in
`src/gameplay_seg52_cc13.cpp`:
- turning guns: `0084C4E0`, `0084C5B0` (partial: the message is the caller's);
- shipyard: `00845750`, `00845870`, `00844A10` (partial: the frame transform is the caller's), `00845AC0`,
  `00847030`;
- director: `00835F60`, `00835B40`, `00836040`;
- squadron block: `0084DAE0`, `0084DB90`, `0084DC50`, `0084DD20`;
- support site: `0084D170` (partial: `0077F0E0` and the head are not modelled), `0084CE60`, `0084D050`;
- stationary: `0084F020`.

Host checks (status `reconstructed_listing_checked`): `0082E850` (partial, the stand-in), `00828F20`,
`008383D0`, `008386F0`, `00838530`.

Name records with evidence appended (names unchanged, nothing applied to Ghidra):
- `00835B40`: the `0071EDD0` correction.
- `00844A10`: the aim-point reading. Rename candidate `BSP_Shipyard_ArtilleryAimPoint`.

## 5. Verification

- `./scripts/build.ps1` passed in this tree: the build and ctest (2 of 2). No test was added.
- `tools/verify_report_calls.py local/gpr52_calls.json` checked 32 call-site rows (one indirect row
  skipped) with 0 failures. The rows cover:
  - the stream-1 draw `00844AB4` and its `_ftol` `00844AB9`;
  - `0084C4E3`, `0084C5D4` and `0084C625`;
  - the director and squadron calls;
  - the support-site calls;
  - the stationary hit calls;
  - the accuracy calls;
  - the tail jump `00828F6F`.

  The row file is not committed.

## 6. Open, and named but not reconstructed

- **The caller of the shipyard's slot `1A8h` (`00845AC0`).**
- **The `0ADh` delivery timing for `0084C5B0`.** Section 1.1 says why it matters.
- **Whether the authored Allied stock lists** (`ijn_11`, `ijn_02` copies) **number from `N+1`.**
- **The receiver of message `5Eh`** (`00835F60`).
- **The `0084C4E0` class labels.** The ledger calls `00CFBF58` MSTGun (24h) and `00CFC190`
  MDepthChargeLauncher. docs/GUN_SHOT_CADENCE.md calls them MRTGun (24h) and MSTGun (27h). The slot
  reading does not depend on the labels.
- **Read, not reconstructed:**
  - `0084D560`, the supply-class filter: STL list surgery, and its decompile is confused;
  - `0084DDA0`, BeginCommand, with the Ghidra split at `0084DDC1`;
  - `00851580`, the stationary class reader. It reads the name into `+10h`, then `HPBlack`, `Armor` and
    `Height` from `+20h`, then `ExplosionEfx` (default `ObjectExplosion`) and `FakeExplosionGroup`.
  - `00851850`, the stationary wreck: presentation plus message `65`;
  - `0084EC10`, the stationary state message (multiplayer);
  - `0084C6E0` and `0084C7E0`, the turning-gun messages (multiplayer);
  - `008390C0` and `00839160`, the kind-value lookups: no caller, no dword, no Ghidra xref.
- **Not read:**
  - the loaders `0082D040`, `0082FE30`, `00846640`, `008433C0` and `008509F0`;
  - the archive pair `0084F080`/`0084F1A0`;
  - the raw `FUN_` entries `0082DC30`, `0082DC80`, `00835700`, `00835710`, `008429E0`, `00842A80`,
    `00843460`, `00845670`, `00847FE0`, `008482F0`, `0084B6B0`, `0084C990`, `0084D980`, `0084DA60`,
    `0084EFE0` and `0084F670`..`0084FC50`.

## 7. Handoff

All work is committed on `agent/cc13-gpr52`, and the lease is released. A follow-up could:
1. find the caller of shipyard slot `1A8h` by sorting the 68 `+1A8h` loads;
2. decide the `0ADh` delivery order with cc9 before binding `0084C4E0`;
3. byte-check the `"Stock %d"` numbering in `ijn_11_operation_to.scn`.

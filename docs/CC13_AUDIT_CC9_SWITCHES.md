# Audit of cc9's switched-ON bindings (cc13-audit4)

Addresses: 0072C6A0 00922E90 008FBE00 008FBF70 008FBFC0 0072F6E0 00729670 0072F76D 00BD2F10
006E4D50 006E3C00 006E42D0 006E08F0 006E0970 006E4F91 006E4FB1 006E5054 006E513C 006E51AC 006E1F00 006E1510 007F3970 007ED260
00926D90 006C4B30 006C06A0 006C8800 006C7680 006BF7F0 006C7540 009B3F50 007B8AD0 009B34D0
00521EA0 00759120 0075927C 00759287 00923B80 00928F50 0095A1CC 0095A380 00954210 00729F10
0072AC20 0072C970 007311B0 0072BF10 00924F90 0046B730 0046B7CA 0046B906 00C336C0 00C358A0

Packet `cc13-audit4`, branch `agent/cc13-audit4`, 2026-10-12. Requested by cc9 through the cc13 lead. **Read only:**
no Ghidra writes, no ledger records, no edits to cc9's files. Base: `origin/main` at `3ff57eb6f`.

**Method.** I located each switch with `git grep` and read its ON path. I then compared it with the disk listing
(`bsp.py disasm-raw`; bytes from the installed executable). The comparison covers control flow, constants read from
the PE, stack cleanup, argument order and RNG draw order. Each scan was checked against a positive control (the
`8b ?? 1c 04 00 00` scan finds the known load at `0072AC2C`).

**Checklist.**
- Rule 2 (boundary before attribution): `ghidra proto --brief` puts each key site in its cited function:
  - `007F3A2A` is in `BSP_Squadron_RemovePlane` (`007F3970`..`007F3A5B`);
  - `009B3F9E` is in `009B3F50`;
  - `0072F76D` is in `BSP_Gun_TargetShotDecision`;
  - `0075927C` is in `00759120`;
  - `0046B906` is in `0046B730`;
  - `0095A3BE` is in `BSP_Unit_ApplyGunAimMessage` (`00959C20`..`0095A5BF`);
  - `006E513C` is in `BSP_MBombPlatform_Drop` (`006E4D50`..`006E56EF`);
  - `006C7711` is in `006C7680`.
- Rule 7 (argument counts): the `RET` immediates cited in item 9 decide its argument count.
- `tools/verify_report_calls.py` takes a `reports/*.json`. This audit has no host table or report, so the tool fails
  to parse the doc, and there is nothing for it to check.

**Status vocabulary.** Every switch below is reconstructed and build-tested on main, and cc9 game-validated each one
with pairs. This audit is a static second reading. It adds no runs.

## Verdicts

| # | switch | verdict | one line |
| --- | --- | --- | --- |
| 1 | `kTailGunnerNoArmourTestBound` | FAITHFUL | `008FBF70` is `0085A9A0` then `vtable[1D4h]`, with no armour term. The host's owner-unit `IsKindOf(0Fh)` matches the `00922E90` ancestor walk for plane-mounted guns |
| 2 | `kAaLineOfFireExpiryDrawBound` | FAITHFUL | One `U(0.8, 1.2)` draw on stream `ECX=1` per cache miss, made after the predicate `00729670`, which draws nothing. Caveat 2a covers the shared stream only |
| 3 | `kBombDropDriftBound` | FAITHFUL | The drift is `(r1 nVz + r2 nVx, 0, -r1 nVx + r2 nVz)`, matching the image by stack trace, and `0.3` / `0.5` match the PE constants. The draws are unconditional and were already taken. The one-step lag is labelled |
| 4 | `kSquadronKillQueueEraseBound` | FAITHFUL | Kill runs when `+3CCh` reaches 0. `ECX` still holds the squadron across `007ED260`. `006C4B30`/`006C06A0` is `std::remove` of every match, in order. Erasing one boundary early is labelled |
| 5 | `kLandTaskRetireQueueEraseBound` | FAITHFUL | The leader test is `+9D8h == 0`. Then the queue swap-erase `006BF7F0`, the in-order `+A8h` erase `006C7540`, and the slot arm (`+0Ch=+8h` at state 2, state 3, timer 0 or 5.0 with `+34h` cleared). The order and constants match |
| 6 | `kLandCommandOwnTargetBound` | FAITHFUL | `009B34D0` compares `vtable[178h]` -> `00521EA0` with `approach+30h`. Every host writer of the `land` class also writes the new field |
| 7 | `kCarrierLiftHeightFromRunwayBound` | FAITHFUL | `0075927C FLD [ESI+818h]` / `00759287 FSTP [ESI+80Ch]` runs on the only path out. `D018BC` is `"runwaycenter"` |
| 8 | `kCaptureStatePartyFromUnitBound` | FAITHFUL (host consolidation) | The image has one field (`00923B80` stores `+54h`). Every host reader of `state.party_54` sits after the resync in the capture step. A timing note is in 8a |
| 9 | `kPlayerTorpedoGroupFireBound` | FAITHFUL (arm); UNCERTAIN (launch-heading precedence, 9b) | The arm matches instruction by instruction, including the order heading = message yaw (settled by `0072C970` `RET` / `007311B0` `RET 0Ch`) and the break on `BL=1` |
| 10 | `kEngineMovieEntryBound` (kill pass) | FAITHFUL for script entities; UNCERTAIN (10a) | Class 3 is not in the `+158h` set, so every script entity is queued and killed with cause 2. Other runtime roots that are not re-created are not covered |
| 11 | hull-contact fix (`rigid_fort`) | FAITHFUL to the geometry, up to float rounding; one residual (11a) | `t + sum (s_i p_i)(R_i/s_i)` = `t + sum p_i R_i`. Boxes, centres, the convex build, contact mapping and the manifold `frame_a` all use the normalized frame consistently |

No item is a DEFECT, so this doc carries no diffs. The UNCERTAIN points each come with the read or run that would
settle them.

## 1. `kTailGunnerNoArmourTestBound` (GUNNERY 164.2)

- **Bot choice.** `0072C6A0` (live decompile): weapon kind 1 with `IsKindOf(22h)`, then
  `00922E90(gun, 0Fh)` picks the bot. A hit gives the TurretClass (TailGunnerBot) and a miss gives the LeadClass
  (AAGunnerBot).
- **The ancestor walk.** `00922E90` (disk) walks `+3Ch` (the parent) and calls `vtable[5Ch](0Fh)` on each ancestor.
  The child list it walks is filled by `00924F90`, which stores `+3Ch`.
- **The accept slots.** `008FBF70` is `0085A9A0` (at `008FBF9C`) and then `[[gun+68h]]+1D4h` (at `008FBFB1`). It
  has no armour term. `008FBFC0` (the AAFlakBot) has the same body.
- **The host.** It tests `unit_is_kind_of(unit_, kUnitGunneryKindPlaneBase = 0x0F)` on the gun's owner unit. A
  plane-mounted gun has its plane among its ancestors, and no ship gun has a plane ancestor, so the two tests agree.
- **The order of the host's checks.** Armour, then window, then line of fire, as in `008FBE00`. The tail gunner
  skips only the armour term. OFF counts the refusals and keeps the old refusal.
- **FPU.** No new floating-point decision.

## 2. `kAaLineOfFireExpiryDrawBound` (GUNNERY 164.1)

- **The miss path.** On a miss (the `+0Ch` record scan finds no `[rec] == target`), `0072F6E0` does the following:
  - calls the predicate through `00729670` (`0072F73A`);
  - pushes `[00CE74F8]` (lo, at `[esp]`) and `[00CE3814]` (hi, at `[esp+4]`);
  - sets `ECX = 1` and calls `00BD2F10` at `0072F76D`;
  - multiplies by `[GlobalConfig+8Ch]` and stores the record.
- **No other draw on this path.** `00729670` reaches `0072CDD0`, whose callees are `00414DB0`, `0042E630`, `0098B130`
  and `0098ADD0`; none of them is an RNG.
- **One draw per (gun, target), after the query.** The record vector belongs to the decision object at `gun+424h`, so
  the key is (gun, target). The host draws after `line_of_fire_blocked_0072cdd0`, once per cache miss, only for kinds
  1, 5 and 6 (the kinds for which `00729560` installs the predicate). Its `draw(..., 0.8f, 1.2f)` keeps lo/hi in the
  same order. OFF draws nothing.
- **2a (caveat, shared stream only; pre-existing).** The host sends kind 6 straight to `line_of_fire_refuses()`.
  - In the image, both kind-6 bots run a gate before `vtable[1D4h]`: the AAFlakBot `008FBFC0` runs `0085A9A0` first
    (`008FBFEC`), and the ArtilleryGunnerBot `006DEF30` runs the arc solve and reach first (GUNNERY 164.2).
  - So under the default shared stream, ON can draw for a kind-6 (gun, target) whose window or arc test would refuse
    it in the image, or draw it earlier than the image would.
  - In the reference launch form (`BSP_GUNNERY_RNG_STREAMS=1`) the draw is keyed and inert, which agrees with 164.4.
  - **To settle:** say where the host applies the kind-6 window and arc gates before `line_of_fire_refuses()`. If
    they come later, or not at all, move the kind-6 call after them.
- **FPU.** None.

## 3. `kBombDropDriftBound` (GUNNERY 173)

**Stack trace of `006E5040..006E51EE`** (offsets relative to the frame before the `vtable[34h]` call; the callee
pops its argument):

- **`w` and `V`.** `w` is `[rack+3F8h]+8` (Wind), stored at `+18h`. `V = (Vx, Vy, Vz)` is stored at
  `+2Ch/+30h/+34h`.
- **`|V|` and `n`.** Each square is stored to float and summed as `(Vy^2 + Vx^2) + Vz^2`. `|V|` goes through
  `00BF7030`, and `n = 1/|V|`, or 0 when `|V| <= 0`. `n` is stored at `+14h`.
- **The products.** `nVx`, `nVy` and `nVz` are stored at `+48h/+4Ch/+50h`. `rack+4ECh = (nVz, 0, -nVx)`.
- **The first draw.** `r1 = U(-w, w)` (lo at `[esp]`, hi at `[esp+4]`; `ECX=1`; `006E513C`). The rack vector is
  scaled by `r1`.
- **The second draw.** `r2 = U(-w*[00D7A280], w*[00CE3DC8])`. The constants are `0.5` and the double
  `0x3FD3333340000000`, which is `0.3f` widened. Then the rack vector gets `+= r2 * (nVx, nVy, nVz)`.
- **Draw order.** The four `006E4D50` draws (`006E4F91`, `006E4FB1`, `006E513C`, `006E51AC`) are unconditional and
  happen in this order. The host draws `a, t, r1, r2` in the same order with the same bounds, so ON does not change
  the stream.
- **The copy and the reader.** `006E1F00` zeroes y. `006E1510` (classDesc `+20h` == 0, the gravity case) does
  `v.y -= 9.81 dt` and then `v += drift * dt`. The drift product is stored to float before each add. The host adds
  `drift * dt` to x and z only for ballistic, gravity-enabled bombs.
- **The OFF path.** It is the previous behaviour: `drift` stays zero, and the step gate requires a non-zero drift.
- **LABELLED (agreed):** the owner `[rack+3F0h]` is taken as the plane, and `unit_linear_velocity` stands in for
  `vtable[34h]`. The host's integration order leaves a one-step position lag, the same lag gravity already has.
- **FPU.** `006E4D50` runs on the main thread at 24 bits (`docs/CC13_FPU_PRECISION.md`), so every x87 op rounds to
  float. The sum order is the same up to commuting two terms. `0.3f*w` is exact as one rounding in both, because
  the x87 extended product is exact and is then stored to float. Expect bit-identical drift from equal inputs.
- **Two small notes.**
  - If the air step runs on a 53-bit worker, the add of two floats is still exact before the float store.
  - `006E1510` also serves MDepthCharge and MTorpedo (entity slot `+19Ch`, GUNNERY 173). The host applies the drift
    to bombs only, which is correct only if no torpedo or depth-charge rack goes through `006E4D50`. **Settled
    here:**
    - An absolute-dword scan for `50 4D 6E 00` finds exactly two references, `00CF9880` and `00CF9AF0`. Each is
      slot `+1D8h` of a vtable starting at `00CF96A8` or `00CF9918`.
    - Those vtables are stored by `006E3C00` (`006E3C45`) and `006E42D0` (`006E42F4`).
    - The class factories are `006E08F0` and `006E0970`. Their descriptor rows at `00CE4588` and `00CE45C4` are
      followed by the names `MBombPlatform` and `MMultipl...`, a second bomb platform deriving from the first
      (`006E42D0` calls `006E3C00`).
    - So only bomb platforms drop through `006E4D50`, and bombs only is right.

## 4. `kSquadronKillQueueEraseBound` (SHIP_AI 208)

- **`007F3970`, from the disk listing.**
  - It returns at once unless `[plane+9D4h]` is set.
  - It compacts `+3D0h[]`, decrements `+3CCh`, stores `0` to `[plane+9D4h]` at `007F3A07`, and calls `007ED260`.
  - At `007F3A16`, when `+3CCh == 0`, it pushes `[plane+70h]` and calls `00926D90` (`007F3A2A`).
- **`ECX` at the Kill call.** `ECX` is not reloaded before `00926D90`. I filtered all of `007ED260` (`007ED260`..`007ED374
  RET`) for writes to `ECX` and for calls. Its only `ECX` operand is `LEA ECX,[ECX]` at `007ED2FD`, a no-op, and it
  makes no calls. So `ECX` is still the squadron (`ESI`), and the Kill is on the squadron.
- **`006C4B30`.** It takes the endpoint's identity through `vtable[4]`, finds the first `[rec] == id` in the 14h-byte
  records at `+14h/+18h`, and calls `006C06A0`. That routine compacts forward and skips every match, which is
  `std::remove`. It then unregisters (`006952A0`) and reduces the count by `(end - new_end) / 14h`.
- **The host.** Its compaction, which keeps the order and drops every match, is the same. The trigger is
  `live_count() == 0` after `remove_member_unit_007f3970`. The erase runs at the leave sweep, one boundary before the
  flush, and this is labelled. OFF only logs.
- **FPU.** None.

## 5. `kLandTaskRetireQueueEraseBound` (SHIP_AI 208.6)

- **`009B3F50`'s tail.** `[task+3FCh]` (the plane) must be non-null and `007B8AD0` must hold. That routine returns
  `[plane+9D8h] == 0`, and `+9D8h` is the member index that `007ED260` writes. Then
  `006C8800([plane+9D4h])` runs.
- **The host's leader test.** It takes the first non-empty member, which agrees with the image because `007F3970`
  compacts `+3D0h[]`. A null squadron gives "leader", and `006C8800` then returns at its `TEST EDI,EDI`, as in the
  host.
- **The walk.** `006C8800` walks `[00E19948]` (next `+BCh`) and calls `006C7680` only when `[blk+4]` and
  `[[blk+4]+7Ch]` are set.
- **`006C7680`, in order:**
  - finds the queue record with `[rec] == squadron` (`[esp+30h]`);
  - erases it with `006BF7F0` (unregister, then the last record copied into its place, then the count decremented);
  - runs `006C7540(squadron)`, the in-order `+A8h` erase;
  - walks the slots `+4Ch` (stride 58h) where `+28h == squadron`: state 2 sets `+0Ch = +8h`, then `+2Ch = 3`,
    `+30h = 0`, and when `+34h` is set, `+30h = [00CE3850]` (`5.0f`) with `+34h = 0`, then `006BF150`.
- **The host.** It follows the same order, with `AirOpsSlotState::kLaunching = 2`, `kLaunched = 3` and
  `kAirOpsSlotCooldownSeconds = 5.0F`. LABELLED as cc9 has them: the `006BF150` publication is not carried, and
  squadron membership stands in for `+9D4h`. OFF only logs.

## 6. `kLandCommandOwnTargetBound` (SHIP_AI 210)

- **`009B34D0` (disk).** The site is kept only when all of these hold:
  - `[approach+0Ch]->vtable[174h]` is non-zero and equals `00E08FA0`;
  - `vtable[178h](&tmp)` -> `00521EA0` equals `[approach+30h]` (`009B351B`);
  - `+2Ch`'s block has an owner with `+5Dh` clear, and `006C4790` passes.
  Otherwise `+2Ch`, `+30h` and `+34h` are cleared.
- **The host.** ON reads `land_command_target_plus_one`. Both host writers of `attack_command_class = 00E08FA0` also
  write that field: the RTB arm at `game_hosts_units.cpp:11591`, and `land_at_site_0099a3dd` at `:11710`.
  `store_unit_attack_command_class` is never given `land`. Every script-order caller passes MoveTo or the chosen
  attack class, and `PilotLand` goes through `0099A3DD`.
- **Conclusion.** The new field cannot go stale against a `land` class, because no other writer of `land` exists. A
  later non-land order changes the class and retires the task under ON, as in the image. OFF reads the old field.
- **FPU.** None.

## 7. `kCarrierLiftHeightFromRunwayBound` (SHIP_AI 210.3)

- **The two reads.**
  - `liftexitpoint`: `00759167 PUSH 2`, `00718000`, the first point to `+808h/+80Ch/+810h`.
  - `runwaycenter`: `PUSH EBX(0)`, the string at `00D018BC` = `"runwaycenter"`, the first point to
    `+814h/+818h/+81Ch`.
- **The store.** The only path out passes `0075927C FLD [ESI+818h]` and `00759287 FSTP [ESI+80Ch]`. The `JB` at
  `0075926D` only skips the string free.
- **The host.** Its `find_named_point_group_00718870(items, "liftexitpoint", 2u)` / `("runwaycenter", 0u)` and
  `e.lift[1] = e.center[1]` match the image. When either point is missing, the image would fault in `00BF6713`, and
  the host skips the store; no installed model takes that path. OFF keeps the model height.

## 8. `kCaptureStatePartyFromUnitBound` (SHIP_AI 205, 207.4)

- **One field in the image.** `00923B80` stores `[this+54h]` and `[this+58h]`, then walks the children. `00928F50`
  (`vtable[2Ch]`) calls `00923B80` first. So the image has one party field, and the host's
  `CaptureBuilding::state.party_54` is a host-only copy.
- **The ON resync.** It copies the unit's `+54h` into the state at the head of the capture step and refreshes the
  gunnery side. That makes the copy follow the field it stands for.
- **The readers.** Every reader of `state.party_54` (`command_building_capture.cpp` lines 11/101/102, and the ship_ai
  log and countdown) runs inside the capture step, after the resync. The level arm before it reads
  `unit_side_0054` directly.
- **8a (timing note, not a defect).** After a SetParty, the gunnery side changes at the next capture step. In the
  image it changes at once, because the guns read `+54h`.
  - This matters only if a gunnery pass runs between the Lua SetParty and the next capture step in the same frame.
  - **Checked:** `refresh_unit_side` has callers only in `game_hosts_ship_ai.cpp` (`:12383`, `:12485`, `:12929`,
    `:12975`), so the Lua SetParty path does not refresh the gunnery side itself. The lag is therefore at most one
    capture step for capture buildings.
  - The same reading means that a SetParty on any other unit never reaches `unit_state[].row.side` in the gunnery
    host. That is outside this switch, and is left for cc9 to judge.
  - A one-line fix would call `gunnery_draws->refresh_unit_side(unit)` from the host's SetParty binding (cc9's file,
    not edited).

## 9. `kPlayerTorpedoGroupFireBound` (GUNNERY 170)

**`0095A1CC..0095A426`, from the disk listing.** The steps run per gun on `unit+48h` (next `+44h`). That list is
appended at the tail by `00924F90` (`+4Ch` is the tail), so the child order is attach order.

1. **The filter.** `IsKindOf(20h)`, then `00954210([msg+1Ch], gun)`. Its group-4 arm is `00729F10` (unit `+720h`,
   gun `+3B8h` and `+5Dh` all clear) and then weapon kind `== 7`. The host never sets `+3B8h` and treats a dead unit
   as a refusal, so its category-7 test is equivalent.
2. **The bearing.** `-00438B10(msgyaw, vtable[50h])` (`0095A217`/`0095A21C`), then
   `0085AB50(tube, bearing, [00E0B588])`, stored at frame `+20h`.
3. **The arm.** `IsKindOf(8)` / `IsKindOf(0Eh)` sends the unit to the press arm `0095A380`.
4. **The surface arm.**
   - If `v != [00D7A278]` (`FUCOMIP; LAHF; TEST AH,44h; JNP` skips when equal), call `0085ABA0(v, 0)`.
   - The network gate follows.
   - If `v` is valid and `[msg+34h]` is set: `|00438B10(v, tube+480h)| < [00CEDF5C]` gives
     `007311B0(msgyaw, 0072C970(), unit)`, `0072AC20`, `vtable[1E8h](1)`. A wider error leaves the trigger as it is.
   - Otherwise: `vtable[1E8h](0)`.
5. **The press arm.** `v` valid, `[msg+35h]` set and `vtable[1D0h](1)` give the order, `0072AC20` and
   `vtable[1F0h]`, then `BL = 1`, and `0095A421` ends the walk.

**The order heading's stack slot.**
- `0072C970` ends in a bare `RET` (`0072CA6A`/`0072CA77`), and `007311B0` ends in `RET 0Ch` (`00731219`). So the
  `PUSH EBP` before `0072C970` is `007311B0`'s third argument.
- `FLD [ESP+20h]` after `SUB ESP,8` therefore reads frame `+14h`, the message yaw, and not `v` (frame `+20h`). This
  matches the host's `player_order_heading = m.yaw_2c`.

**The host matches** each step, including the press arm's `break`. The bearing comes from `atan2` rather than
`-wrap(yaw - heading)`; this is labelled.

**9b (UNCERTAIN: launch-heading precedence).**
- **The image.** The pending order sits at `gun+41Ch`. Its only writers are:
  - `0072AC20`;
  - the network-stream reader `0072F060` (`0072F51A`/`0072F524`);
  - `008FDE69`/`008FDE73` in `BSP_AAFlakBot_RollError`;
  - the constructor and destructor.
  The launch reads it at `0072BFB1`/`0072BFDD` (`BSP_Gun_CreateProjectile`) and at `007300A6`/`007300CF`. Nothing
  clears it at launch, so the image fires with whichever order was installed last.
- **The host.** It applies the player's heading at `game_hosts_gunnery.cpp:7815`. The bot block right after it
  (`:7823`) overwrites `commanded_heading` and takes two `Draw::torpedo_gyro` draws whenever `torpedo_lead_xz` is
  non-zero, that is, whenever the tube's bot snapped a target in that pass.
- **Where they can differ.** On a player press while the tube's bot also has a target, the image uses the player's
  order unless the bot installed a newer one through `0072AC20` before the request was served. The host always
  prefers the bot's.
- **`player_order_heading`** is never reset in the host. The image keeps `+41Ch` the same way, so that part matches.
- **To settle:**
  - In `local\g35_pt{off,on}_j6.log`, check whether `torpedo_gyro_launches` rose by the two player shots. A rise
    means the bot branch overrode them.
  - Read whether the torpedo bot `008FFF20` runs, and calls `0072AC20`, while the seat is player-held.
  - Proposed if confirmed (not applied):

```diff
-        if (kTorpedoGyroHeadingBound && gun.category == bsp::kUnitGunneryTorpedoCategory
-            && torpedo_gun && (torpedo_lead_xz[0] != 0.0f || torpedo_lead_xz[1] != 0.0f)) {
+        const bool player_order_shot = kPlayerTorpedoGroupFireBound && kTorpedoGyroHeadingBound
+            && torpedo_gun && gun.player_order_heading != 10000.0f && <this shot came from the seat request>;
+        if (!player_order_shot && kTorpedoGyroHeadingBound && gun.category == bsp::kUnitGunneryTorpedoCategory
+            && torpedo_gun && (torpedo_lead_xz[0] != 0.0f || torpedo_lead_xz[1] != 0.0f)) {
```

**FPU.**
- The bearing snap and the 5-degree test run on the main thread (24 bits) in the image. The host computes the
  bearing by a different formula, which is labelled.
- A bearing exactly at a window edge can snap differently. JM06's tubes snap to `-0.0`, the edge of the window
  `[-180, -0]`.
- Neither the window choice nor the `< 5 deg` test is likely to flip, except on a knife-edge bearing.

## 10. `kEngineMovieEntryBound`: the kill pass (SQUADRON_LAND_TASK 5fp)

- **The kill pass (disk).** In `0046B795..0046B8CF`, the class is `+C4h`. `00468DB0` is the set find on
  `scene+158h`.
  - When the iterator equals the end node, a root is queued (`0046B7CA` `JNE` over the push at `0046B7DD`).
  - For a root that is in the set, each child (`+48h`, `+44h`, count `+50h`) whose find misses is queued
    (`0046B84F`..`0046B8C7`).
  - Class 3 is not in `{44h, 5Eh, 1Dh, 47h, 3Dh}`. So every script entity is queued, whether it is a root or a child
    of a kept root, and killed with `00926D90(2)`.
- **The host.** `kill_live_script_entities(2)` kills every live script entity, before `luaPrecacheUnits`, as the
  image's order has it. OFF never sets `engine_movie_skip_pending`.
- **10a (UNCERTAIN).** The image kills every queued root, not only scripts, and then re-creates only the `+28h`
  scene records (`0046BA7B..`).
  - The host treats the units as "killed and re-created unchanged", which is LABELLED.
  - That is exact only if every live non-script root on the skip frame is a `+28h` record. An entity that
    `luaEngineMovieInit` creates at run time (a Lua-spawned unit, or any other non-scene root) would die in the
    image and survive in the host.
  - **To settle:** on each pair row, log the live roots that are neither class 3 nor in the scene records at the
    skip.
  - Also scan this installation's `luaEngineMovieInit` bodies for spawn calls. A multiline grep over `scripts\`
    found matches in 7 files, but the 3000-character window can run past the function's end, and
    `jm07.lua:4` opens with dialog tables. So this needs a per-function read.
- **Kill order.** The image kills in list order (attach order, `00924F90`). The host kills in creation order. These
  agree unless an entity was re-parented.
- **FPU.** None.

## 11. The unconditional hull-contact fix (GUNNERY 165/167)

**The geometry is unchanged in exact arithmetic.**
- `world_box` and the narrow phase use the row convention `w = t + f[0..2] x + f[3..5] y + f[6..8] z`.
- `rigid_fort` scales the points per local axis, `p_i -> s_i p_i`, and divides the rows, `R_i -> R_i / s_i`. So
  `w = t + sum (s_i p_i)(R_i / s_i) = t + sum p_i R_i`.
- **The centre.** It is the bounding-box centre of the scaled points, which is `s_i c_i` per axis because `s_i > 0`.
  So the shape's world translation `R̂ (s c) = R c` is also unchanged.
- **The convex hull.** It is built from the scaled points. Hulls commute with an axis scaling, so the hull is the
  same as the original hull, scaled.

**Consistency checks.**
- **The caches.** `hull_shape` rebuilds whenever the raw points differ (memcmp), so a cache built earlier cannot be
  served stale. The only caller with fort units is `hull_fort_narrow_phase`, after `rigid_fort`.
- **The contact points.** `c[0..2]` are mapped through the normalized `fort.frame`, which is the same frame the convex
  was posed with. The manifold's `frame_a` and the `terrain.set_pose` are also the normalized frame.
- **The box.** `fort_boxes_` is computed from the same normalized frame and scaled points.

**Rounding.**
- The image does no such rescale; this is a host correctness fix.
- `(p s)(R/s)` differs from `p R` by a few ulps per term, which is about 1e-6 m at fort scale. So the world geometry
  is unchanged up to float rounding, not bit for bit, for scaled forts.
- Unit-row forts (`|s-1| <= 1e-4`) are passed through untouched, so they are bit-identical.

**11a (residual, as 167 says).**
- The crash data shows that the image maps world directions into the local frame by the transpose. The local
  direction had length `2.003`.
- For the support mapping this gives the same world support point, because the argmax ignores scale. The host's
  scaled points with normalized rows return that same point.
- **Unread:** whether `00C535E0` also maps points or contact positions into a local frame by the transpose. If it
  does, the image's contacts on a scaled fort come out distorted, by `s^2` on local points. In that case the image
  would not "agree" with the host, which would be correct and the image wrong (and it also reads past the seed
  table).
- **To settle:** read `00C535E0`'s local-point output path. This does not affect the verdict that the world geometry
  is kept.

## Summary for cc9

- Items 1-8 and 11 match the image as bound. Item 9's arm and item 10's script-entity kill match it too.
- Open points, none of them blocking:
  - 2a: shared-stream kind-6 draw reach;
  - 8a: the side-refresh lag;
  - 9b: bot versus player launch heading;
  - 10a: non-script runtime roots on the skip;
  - 11a: `00C535E0`'s local points.

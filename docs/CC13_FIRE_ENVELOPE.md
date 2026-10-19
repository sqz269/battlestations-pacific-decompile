# cc13-fire: the fire envelope of 009FC7C0

Packet cc13-fire, granted by cc9 through the cc13 lead. It comes from row B of
`docs/CC13_ADOPTION_TRIAGE.md` Tier 3: `009FC7C0` (`BSP_BotTaskGun_Tick`,
`__thiscall(gun = task+314h, float dt)`, `RET 4`), LIVE and UNIMPLEMENTED.
`docs/DOGFIGHT_GUN.md` has the rest of the controller.

Everything here is behind `bsp::kDogfightFireEnvelopeBound`, which is OFF. With
it OFF the host's tick is byte-identical to before. The status is
reconstructed and build-tested. It has not been run ON.

## 1. The triage row is partly stale

The row lists four missing pieces. Two are already in the host behind
switches that are ON:

* The `007B96F0` target find (into `+74h`) is `df_finder_007e2090` under
  `kPlaneFinderBound`.
* The `0043F080` live gate on that target is `df_slot_live`, applied by the
  finder.

The other two were missing, and so were three smaller pieces of the same
envelope. Sections 2 to 6 cover all five.

## 2. A: 007BA760, the own unit's fire block

`009FCDB6 TEST BL` (the envelope), then `009FCDBA MOV ECX,EBP` (the own unit,
`[task]+2F4h`) and `CALL 007BA760`. The body (`src/cc13_seg47.cpp:1192`) is:

```
CMP byte [ECX+C3Ah],0 / JNZ -> 1
CMP byte [ECX+5Dh],0  / JNZ -> 1
-> 0
```

A true result skips the whole fire arm, including the friendly check (`JNZ
009FCE23`). In the host, `DogfightGunInputs::unit_disabled` was never set.
A plane with `plane_death_c3a` or `state->simulate` set could still raise
`+48h` and start bursts, and each burst start takes a `00BD2F10` draw. Only
the trigger was masked, by `df_slot_live` at the gunfire step.

ON: `bsp::unit_gun_fire_blocked_007ba760(plane_death_c3a, state->simulate)`.

## 3. B: when 007B96D0 is called

`009FCDC7 CALL 007B96D0` runs only after A passes, so only when the envelope
holds and the plane is not blocked. `007B96D0` is
`unit+C50h != 0 && 007DEDB0(unit+C50h)`. `007DEDB0` consumes its own 1 s
clock: it returns the cached `+C8h` while `+80h <= +90h`. Otherwise it takes
`+90h` off `+80h`, clears `+C8h` and walks the friend list.

`007E2010`, slot `+0Ch` of the neighbour object's vtable `00D06B88`, advances
`+7Ch`, `+80h` and `+84h` by dt on every update. That clock therefore runs
whether or not the gun asks.

The host called `friendly_in_line_007b96d0` on every gun tick, before the
envelope was known. So the clock was consumed on ticks where the image never
looks.

ON: the host function is split into two parts. With the switch OFF the
original function calls both in its old order, so it is byte-identical.

* `friendly_in_line_clock_007e2010` holds the 3 s refresh of the `+70h` list
  and `+80h += dt`. It runs on every gun tick, as before.
* `friendly_in_line_eval_007dedb0` holds the evaluation. With the switch ON it
  runs only when `envelope && !A`.

Lifetime: `nb_friend_70` holds slot indices refreshed every 3 s. The
evaluation re-checks `idx < slots.size()`, IsKindOf `0Fh` and `+5Eh`, as
`007DEE0C`/`007DEE16` do. A dead friend whose slot is still listed is skipped
by the `+5Eh` test, as in the image.

## 4. C: the re-aim when a friendly is in line

`009FCDD3-009FCE01`: when `007B96D0` is true and `+49h` is set (the steer arm
ran this tick), the tick does the following:

1. `+4Ah = 0` (`009FCDE5`), and the local steered flag is cleared (`009FCDE9`).
2. `local_20.y += 0.25` (`00D7A348`, a double).
3. `009F9FC0` again (`009FCDFC`). With `+4Ah` now clear it is undamped.

The tail then stores `+4Ah = 0` and skips the `+54h`/`+58h` angle store
(`009FCE2D-009FCE44`).

ON: `df_fine_aim_009f9fc0(gi, reaim)` reapplies the pitch and yaw sticks with
`e_v + 0.25` and no damping. It leaves `terms.prev` unchanged and returns
false, so `ran_4a` is false for the next tick. Without `kFighterGunLeadBound`
the steer is not hosted, so there is nothing to re-aim.

## 5. D and E: the hold timer `+4Ch`

These two pieces are under the sub-switch `bsp::kDogfightFireHoldArmBound`
(OFF). It is read only with the main switch ON.

* **D**, `009FCA06`/`009FCA34` to `009FCAC2`: with an auto target (`+74h`), or
  with the strafe point (`+28h > 0`), `+4Ch = -1.0` (`00D7A260`) before the
  envelope. A hold therefore never forces the envelope on a found target. The
  host kept the hold that the follow state wrote at `009C232A`
  (`hold_4c = 2.0` while `fs_sight_84`). With a finder target, its
  `hold_4c > 0` arm passed the envelope unconditionally and also suppressed
  the steer.
* **E**, `009FCA16-009FCAC0`: with no auto target, `+28h <= 0` and `+4Ch > 0`,
  the tick does the following:
  1. `+28h = 0.1` (`00D7A2F0`).
  2. The lead point is the leader's position plus `+34h` times the leader's
     forward axis `+94h..+9Ch`. The leader is `[[unit+9D4h]+3D0h]`, the
     squadron's first plane, or the unit itself with no squadron.
     - `009FCA47-009FCA53` load that pointer into ECX.
     - ECX is untouched until `009FCA86 CALL 00427EB0`. That call is
       `__thiscall` (`00427EB1 MOV ESI,ECX`), so the position is also the
       leader's.
  3. The hold arm passes the envelope.

  The host had no target in that case and never fired. With the switch ON,
  the host builds the point from the leader slot's `world[12..14]` and
  `world[8..10]`. The leader is resolved as `+3D0h[0]`, as
  `apply_strike_ctl_cruise_cap_009fba9b` does. When that slot is missing it
  falls back to the unit itself.

  Labelled: `+94h..+9Ch` is row 2 of the LOCAL matrix (the block at `+74h`),
  while the host reads the world matrix. The two are equal for a plane with
  no parent.

## 6. The envelope, at the main thread's precision

`009FCB25-009FCCA0`. The main thread's x87 precision control is 24-bit, so
every product and sum rounds to a float mantissa. The model is therefore plain
float arithmetic. `gun_fire_envelope_009fcb25<false>` is the PC24 model and
`<true>` is the 53-bit record.

* `d = 0042B2F0(lead - own)`: `(x*x + y*y) + z*z` is stored as a float. Then
  `sqrt` (`00BF7030`) when the result is above `1e-10` (`00CE3820`), else 0.
  The result is a float.
* The range test is `d > 1` and `max(+30h, float(+34h + 200.0))  > d`.
  `00CE4D70` is the double 200.0. The max is `00415550(a = +30h, b)`. It
  returns `a > b ? a : b` (`FCOMIP` then `JBE`), so it returns `b` when the
  compare is unordered. The host matches that.
* `L = 00414C60` is the 2-D length of the transformed lead. `z` is the
  transformed z.
* `a = +3Ch`, divided by 1.8 (`00D049A8`, the double of the float 1.8) only
  when `+74h != 0`.
* The envelope passes in either case:
  * `+4Ch > 0`; or
  * `L < +38h * d` (JBE fails) and `z > 1` and `z < +34h` (JBE fails), and
    then either `L > a`, or `L > 2 * float(+0Ch * +08h) * d`.

`+3Ch` and `+0Ch` are 0 from the constructor. The last test is therefore
`L > 0`, the same as the host's `area < lateral`. The host's lead is already
in the unit frame, so `d` is taken from it, which agrees for a rigid frame.

With the switch ON, both precisions run every tick and
`fe_pc53_disagree_ticks` counts the ticks where they differ.

Not modelled: on a fire the tick also stores `[gun]+2E0h = 1` (`009FCE18`), the request byte. The
host does not store that byte. Its `+48h` (`fire_48`) stands for it, and the gunfire step
(`kPlaneGunfireBound`) carries it on to `+BC9h`.

## 7. Draws

The new code takes no stream draws. `007BA760`, `007DEDB0`, `0042B2F0`,
`00414C60` and the envelope draw nothing. `007DEDB0`'s start, `U(0, 1) + 1.0`,
is a constructor draw that the host already takes at its midpoint.

The only coupling to the shared `00BD2F10` stream is indirect. The tail takes
one draw per `+4Bh` edge. The host's `release_altitude_draw_00bd2f10` key is
`"#dfgb"`, under `kDogfightGunBurstDrawBound`, and it shares the stand-in
stream unless per-key streams are enabled. The new code changes the edge count
in three ways:

* A removes burst starts for blocked planes. A burst already on still ends,
  with its draw.
* B and C move which ticks see the friendly as busy.
* E adds bursts for follow-state wingmen that have no target.

The aim distortion `009FA7E0` keeps its own gunnery-host draw kinds and is
untouched.

## 8. Reach

These are the current `l69r_*_main_ref` logs (cc9-lua52, 2026-10-19). Each entry
gives the `BotTaskGun::tick` census calls and the number of
`fighter gun burst` lines:

* JM08: 136k to 590k calls, 124 to 201 bursts.
* BSM01: 400k to 937k calls, 14 to 127 bursts.
* ESMP08: 1.08M calls, 182 bursts.
* PRCP01: 401k calls, 83 bursts.
* USN01: 52k to 127k calls, 0 to 42 bursts.
* BSM09: 105k calls, 42 bursts.
* IJN08: 103k to 172k calls, 3 to 6 bursts.
* IJN18, LOMP10 and JM06: ticks, but no bursts.
* USN15, USN12, USN02 and LOMP06: no ticks.

No ON run has been made, so the effect on outcomes is unverified.

## 9. Host seams

All are in `src/game_hosts_units.cpp`:

* `df_gun_tick_009fc7c0` is the ON path.
* `df_fire_envelope_009fc7c0` is new.
* `df_fine_aim_009f9fc0` gains the re-aim.
* `friendly_in_line_007b96d0` is split into `friendly_in_line_clock_007e2010`
  and `friendly_in_line_eval_007dedb0`.

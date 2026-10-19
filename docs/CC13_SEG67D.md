# cc13-seg67d: segment 67, upper half (009C6000-009F6060)

Packet cc13-seg67d, done under cc9's gameplay terms. This is a static pass: there were no game runs, and nothing in it is wired.

Every name below is a hypothesis, not a recovered symbol. New code is in
`src/cc13_seg67d.cpp` and `include/bsp/cc13_seg67d.hpp`, and it is build-tested only. Evidence comes from
disk bytes of `battlestationspacific.exe` and the Ghidra listing.

## Census

The range has 659 function rows. They break down as follows:

- **Records that already existed** (mostly cc13-seg67b and earlier lineages): every `FUN_`/named body outside the
  library tags. Rows tagged `cg_*` (scalar/vector deleting dtors, adjustor thunks), `stl*`, `tmpl*` and
  `triv*` are compiler or library instances. They stay bound by their tags and were skipped.
- **Bare JMP thunks recorded (22).** Each is an E9 rel32 followed by 11 INT3:
  - 18 jump to 007B45F0 BSP_LandSharedState_Destroy: 009C6170, 009C7090, 009C9880, 009C9950, 009C9A70,
    009CA6C0, 009CAC70, 009CB0A0, 009CB4F0, 009CBE80, 009CDCA0, 009CE110, 009CE230, 009CF700, 009D06C0,
    009D0B10, 009D1220 and 009D2380.
  - 009C7260 BSP_BotStateDiveBombDone_Exit (vtable 00D20D28 slot +8h) jumps to 009BDE40
    BSP_BotStateFollow_Exit.
  - 009D66F0 jumps to 007293D0, 009D7040 to 009D5EB0, and 009DBD40 to 006E0860.
- **Reconstructed (2):** 009DBDF0 and 009DFCB0 (below).
- **Re-audited, with stale records replaced (3):** 009E4A60, 009D2720 and 009C8920. One new fragment record was added, for 009D29E0.

## 009DFCB0 BSP_ShipAi_NavigationBlockSeedDefaults (whole body)

- **ABI:** `__thiscall(ECX = blk+1C4h)`, returns EAX = this, plain RET at 009DFEB4.
- **Caller:** the sole caller is 009E4330, at 009E43CC.
- **Earlier coverage:** the existing fragment `ship_ai_nav_block_steering_defaults_009dfcb0` covered only the +200h..+220h window.
- **Two records, two sources:** 009DFCB0 now has two records and two C++ sources: the cc9 window projection in `ship_ai_nav_block_ctor.cpp` and this whole body. Eventually the cc9 projection should derive from the whole body rather than keep its own copy of the nine values.

`cc13_seg67d_nav_block_seed_defaults_009dfcb0` writes every store in image order. The nine window values match the existing fragment.

| Offset | Value |
|---|---|
| +0..+14h, +30h..+44h | 0 (dwords and floats) |
| +20h, +50h | byte 1 |
| +60h | 009DBDF0 sub-object (call at 009DFCF3) |
| +160h..+178h | 0.0f |
| +174h | byte 0 |
| +17Ch, +21Ch | 1000.0f [00CE3804] |
| +180h, +184h | 1.0f [00D7A24C] |
| +188h, +18Ch, +19Ch, +1A4h, +1B0h, +1B8h | 0.0f |
| +190h, +1BCh, +1C0h | -1.0f [00D7A260] |
| +198h, +1ACh, +1B4h | dword 0 |
| +1A0h, +1E0h, +1FCh | byte 1 |
| +1A8h, +1C4h..+1C6h, +1E1h, +1E2h, +1FDh, +1FEh | byte 0 |
| +1D0h..+1DCh, +1ECh..+1F8h | 0.0f |
| +200h | 10.0f [00CE38B8] |
| +204h, +208h, +218h | 400.0f [00CFD710] |
| +20Ch | 0.15f [00CE7818] |
| +210h, +214h | 100.0f [00CE3D08] |
| +220h | 50.0f [00CEB4D4] |

## 009DBDF0 (path-plan pair member at sub-block +60h)

- **ABI:** `__fastcall(ECX = this)`, returns EAX = this.
- **Tag:** it is tagged `cg_array_ctor_helper`, but it is a real member constructor.
- **Construction:** 00BF7CD1 `eh vector constructor iterator`(this, 68h, 2, 009D9CC0, 009DBDB0) builds two path plans. This step is a host call.
- **Stores:**

| Offset | Value |
|---|---|
| +48h, +D4h | this+68h |
| +B0h, +D0h | this |
| +DCh | -1.0f |
| +F0h | 0.0f |
| +D8h..+DAh | byte 0 |
| +E0h..+ECh | dword 0 |
| +F8h, +FCh | 1e9f [00D217E8] |

## Re-audits

**009E4A60, sub-attack range.** The cc13-backfill defect is now fixed in `ship_ai_sub_attack.cpp` under `kSubAttackOwnHalfLengthBound` (ON).

- **The image:** the PUSH 6 at 009E4B20 aliases [ESP+0Ch] onto the distance slot, so the result is r = distance − own length × 0.5 − (ship target ? target length × 0.5 : 0).
- **Range and NaN behaviour:** a negative r becomes 0, and a NaN r is kept.
- **Verdict:** the host matches the image.

**009D2720 / 009D29E0, torpedo done/prepare disarmed arm.** The placeholder is gone. `torpedo_done_disarmed_arm_009d29e0` is wired under `kTorpedoDoneDisarmedArmBound` (ON).

- **Byte check:** 009D29E0-009D2CF1 was checked byte by byte: the ratio, both interpolations, the 300/3.5 gates, the abs by −0.0f, max/min with their NaN arms, and 1+(w−1)c. All are faithful.
- **Labelled substitutions** (no host cell):
  - the aim point is the target position;
  - approach+A4h = 3;
  - [approach+1Ch]+40h;
  - +8Ch is log-only.
- **Note for cc9 (comment only, no behaviour change):** the `torpedo_task_arm.cpp:254` comment says "00CE3D48 1.6 (double)". The bytes hold 1.6f widened, which is 1.600000023841858. I did not edit the file, because it belongs to cc9.

**009C8920, dive-bomb cruise profile.** The live path is the inline in `game_hosts_units.cpp`:

- Blocks 2 (+398h, the U(0,15) draw) and 3 (+39Ch = 9999) match the image.
- +4B0h: the image computes `cur > prod ? cur : prod` (009C8A42-009C8A50), so a NaN cur gives prod. The host (~4736) keeps a NaN cur. This is probably unreachable, but it is not a match.
- +4BCh does **not** match. The image re-seeds +4BCh = 3600.0f [00CFDEB0] at 009C8A74 on every profile call. The host carries only the 009C3FD2 seed; units.cpp:28400-28402 labels this.
- +41Ch is taken as 1.0 (labelled).
- The draw stream is per name.

> **Adoption note (block 1, 009C894F-009C8971).** In the image, `if (!+38Dh) { if (+380h < 0 && !+3A9h) { +394h = tuning+4C0h; +3ADh = 1; } +3A9h = 0; }`. The +3A9h clear sits inside the !+38Dh test.
>
> The dive task's host does not do this:
> - its +394h comes from `strike_ctl_cruising_alt_394`;
> - an expired script altitude is never reset to the tuning cruise altitude;
> - +3A9h is never cleared.
>
> The moveto and level-bomb profiles do implement the gate. This is live only with SquadronSetTravelAlt on a dive squadron. The standalone `dive_bomb_cruise_profile_009c8920` (`dive_bomb_task.cpp`) is still the defective, unused copy.

## Follow-on audit (cc13-seg67e): partial host projections

This audit is verify-only. Every check was made against disk bytes. Fragment records are named
`cc13_seg67e_check_*`.

| Address | Span | Verdict |
|---|---|---|
| 009F1420 | 009F158A-009F1BB8 | Faithful. **No torpedo mismatch.** |
| 009E04E0 | 009E0B52-009E107E | Ordered arms are faithful. Four NaN arms differ (note B). |
| 009E5530 | whole body | The ring and its 60 stream-1 draws are faithful. The three timer seeds do not match (note A). |
| 009EC770 | whole body | Faithful. |
| 009ED3E0 | 009ED3E0-009ED4E4 | Faithful. |
| 009EB660 | 009EBEDB | The ver7 D2 issue is resolved: `kShipAiSectorBlockerBound` is ON. |
| 009ED6B0 | 009ED708 | The ver7 D3 issue is resolved: `kShipAiTurnLeadResetBound` is ON. Five wired fragments cover the whole body. |
| 009EF910 | 009EF91D | The ver7 D5 issue is resolved: `kShipAiClearanceTimerSharedBound` is ON. All six -1.0f writers are shared. |

### 009F1420 tail

**Timers.** Both timers have the same form: c + (p - e) when due, otherwise c - e. JB is taken on NaN.

**Torpedo walk.** The image computes:
- the horizon as float(row+20h + row+14h + float(B44h + 3.0));
- the hull term as float(len × 0.6)²;
- the skip filters: vslot 38h/2Ch, byte +5Dh, and owner = self;
- the three admission tests, in order:
  1. hull > dist² (JBE on NaN);
  2. reach² > dist²;
  3. dot > 0.

`ship_ai_torpedo_admits` matches every arm. The only difference is precision: the host rounds dx² + dz² once in double, while the image rounds each product at 24 bits. This differs by at most 1 ulp, at the boundaries.

**Other spans:**
- 009F1A44..009F1B39 is a cache refresh of world transforms. It holds no gameplay state.
- 009DB8F0 never reads its float argument, so calling it without one is equivalent.
- The 009F1B7B avoidance request matches.

### Adoption notes

**(A) 009E5530.** The constructor seeds nested+121Ch, +1220h and +1224h to -1.0f [00D7A260] (009E5625, 009E5659, 009E5661).

`ShipAiApproachState` defaults all three to 0.0f (`ship_ai_approach_update.hpp:122-124`). On the first approach enter, 009F1BC0 runs with 0 s (009F31BD):

| Field | Image (seed -1.0f) | Host (seed 0.0f) |
|---|---|---|
| +121Ch | 0 > t is true, so the retarget reset runs | false |
| +1220h | own curve refill runs | false, with `kShipAiOwnCurveRefillGateBound` ON |
| +1224h | target curve refill runs | false |

In the host, each of these first fires one pass later. The fix is to default the three fields to -1.0f.

**(B) 009E0C7C step, NaN arms only.**

| Site | Image on NaN | Host on NaN |
|---|---|---|
| 009E0C8A | NaN \|e\| takes the inner arm | wide arm |
| 009E0D07 / 009E0EA4 | NaN e gives gap+lateral | gap-lateral |
| 009E0CA2 / 009E0E54 | NaN lateral gives the normal | -normal |
| 009E0BB9 | NaN gives stand-on | give-way |

None of these is reachable while headings and positions are finite.

## Second follow-on audit (cc13-seg67f): 009ED6B0 op by op

Each of the five fragments was checked op by op against disk bytes. The records are named `cc13_seg67f_check_*`.

| Span | Host | Verdict |
|---|---|---|
| 009ED6B0-009EDA25, direct control | ship_ai_states.cpp + host 10800-10849 | Faithful. The prologue resets run after the arm in the host; the arm reads none of them. |
| 009EDA28-009EE57B, station keeping | ship_ai_station_keeping.cpp | Ordered arms are faithful. NaN-only deviations (note E). |
| 009EE580-009EE670, path pick | ship_ai_goal_vector.cpp + host | **Mismatch: three station bytes are never cleared** (note C). |
| 009EE671-009EEAA2, navigation output | ship_ai_navigation.cpp | **Mismatch: the parked byte +2FDh is never written before the tail** (note D). |
| 009EEAAB-009EF226, arm tail | ship_ai_navigation_arm_tail.cpp | Faithful, but it consumes the +2FDh input that note D shows is missing. |

### Adoption notes

**(C) 009EE586-009EE594.** The image clears blk+38Ah, +388h and +389h on every pass that reaches 009EE580, which is every tick the station-keeping arm does not run. The host never clears `station_close_38a`, `station_aligned_388` or `station_reversing_389`, so the station arm resumes with stale latches.

**(D) 009EE6F1-009EE750.** Before the tail, the image computes:

```
2FDh = ([[blk+2F4h]+1Ch] == 0) ? 1
     : (blk+310h >= blk+30Ch && !blk+2FCh && record+20h)
```

The host has no counterpart: `tail.parked_2fd` is cleared per tick and set only at 009EF045, after the read. This has two consequences:
- The tail's parked stop at 009EEF72-009EEFD1 never fires in the host. In the image it fires when `2FDh && record+20h && 340h < min(0.25 * 3CCh, 0.1 * 1F0h)`.
- 009DA610's reader of `goal.flag_2fd` sees the byte set only from 009EF045.

Separately, the mode-3 skip at 009EE756 is latent: nothing in the host writes mode 3.

**(E) NaN semantics of the by-reference helpers.** The image helpers behave as follows:
- 00415510 is `b > a ? a : b`;
- 00415550 is `a > b ? a : b`;
- 00415620 returns `a` on NaN.

So NaN gives b, b and a. These host copies differ on NaN only:

| Host copy | Host form | Host result on NaN |
|---|---|---|
| ship_ai_station_keeping.cpp:50 max | `b > a ? b : a` | a |
| ship_ai_navigation.cpp:26 min | `b <= a ? b : a` | a |
| ship_ai_navigation.cpp:31 max | `a <= b ? b : a` | a |
| ship_ai_navigation.cpp:39 clamp | returns `high` | high |

`ai_target_weights.cpp:430/435` have the right form. airfield_taxi.cpp:12's max is also NaN-wrong; it is outside this range.

## Left

- **Not re-audited op by op (cc13-seg67e/f):**
  - the rest of 009EB660 and 009EF910;
  - 009E04E0 outside 009E0B52-009E107E;
  - 009E04E0 009E0FBD's delete body.
- **Original list of partial projections (cc13-seg67d):**
  - 009E04E0;
  - 009E5530 (attack-move ring);
  - 009EC770;
  - 009ED3E0;
  - 009F1420 (tail 009F158A-009F1BB8, including the torpedo-response walk);
  - the partial-divergence records 009EB660, 009ED6B0 and 009EF910.

  All of these are hosted in cc9 files. Finishing them means reading those tails against the host, not adding new copies.
- **Dive-bomb done 009C7240/009C7270:** the gaps sit in 009BFEE0/009BEE30, which belong to the lower half.

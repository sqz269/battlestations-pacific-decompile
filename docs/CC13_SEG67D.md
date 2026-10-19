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

**009C8920, dive-bomb cruise profile.** The live path is the inline in `game_hosts_units.cpp`:

- Blocks 2 (+398h, the U(0,15) draw) and 3 (+39Ch = 9999), the +4B0h max and +4BCh match the image.
- +41Ch is taken as 1.0 (labelled).
- The draw stream is per name.

> **Adoption note (block 1, 009C894F-009C8971).** In the image, `if (!+38Dh) { if (+380h < 0 && !+3A9h) { +394h = tuning+4C0h; +3ADh = 1; } +3A9h = 0; }`.
>
> The dive task's host does not do this:
> - its +394h comes from `strike_ctl_cruising_alt_394`;
> - an expired script altitude is never reset to the tuning cruise altitude;
> - +3A9h is never cleared.
>
> The moveto and level-bomb profiles do implement the gate. This is live only with SquadronSetTravelAlt on a dive squadron. The standalone `dive_bomb_cruise_profile_009c8920` (`dive_bomb_task.cpp`) is still the defective, unused copy.

## Left

- **Partial projections in the range that stay partial:**
  - 009E04E0;
  - 009E5530 (attack-move ring);
  - 009EC770;
  - 009ED3E0;
  - 009F1420 (tail 009F158A-009F1BB8, including the torpedo-response walk);
  - the partial-divergence records 009EB660, 009ED6B0 and 009EF910.

  All of these are hosted in cc9 files. Finishing them means reading those tails against the host, not adding new copies.
- **Dive-bomb done 009C7240/009C7270:** the gaps sit in 009BFEE0/009BEE30, which belong to the lower half.

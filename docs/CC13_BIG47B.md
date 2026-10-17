# CC13 BIG47B: segment 47 large bodies, PLANE_SYNC codec and plane save

Packet `cc13-big47b` (cc13 lineage, Claude, 2026-10-17). Pool: three large segment 47 bodies
that `docs/CC13_SEG47.md` and `docs/CC13_SEG47B.md` deferred: `007C7A40` (1040h), `007C8A80`
(650h), `007C9860` (B90h). Code: `src/cc13_big47b.cpp` and `include/bsp/cc13_big47b.hpp`
(marker `cc13_big47b`), registered in `cmake/startup.cmake` (bsp_core). Nothing is wired and
nothing is ABI-compatible; status is **reconstructed and build-tested** only. Descriptive names
are hypotheses, not recovered symbols.

Method: rule 1 first (address, name and behaviour searched in `src`/`include`, `bsp.py lookup`,
`docs-for`); none of the three had a host or C++ body, cc9's plane hosts included (the only
neighbours are seg42's `construct_for_read_plane_sync_0075aab0` and seg47b's sender
`plane_send_sync_message_007c29f0`). The bodies were read from the exported listings, with the
x87 operand order checked against the bytes (`disasm-raw 007C80B0`: `fdivp st(2)`, then the
decompiler agrees on `+5Ch/+64h` and `+60h/+64h`). Every constant was read from the PE. None of
the three has an SEH state other than the save's constructor guard, none draws from the shared
RNG `00BD2F10`, and none touches plane level-bomb or goaway control flow (the save reads the
bomb fields and replaces the pilot bot; reported to team-lead as an FYI).

Precision: the PLANE_SYNC codec runs where session messages are built and parsed, assumed to be
the main thread (24-bit precision control, `docs/CC13_FPU_PRECISION.md`). Under 24-bit PC every
x87 add, subtract and divide in these bodies rounds once to float, which is what the float
operations in the C++ compute. At 53-bit PC the `FADD`/`FSUB` of the doubles 150.0 and 80.0 and
the two `FDIV`s would round twice (53 then 24 bits at `FSTP`); that is recorded as uncertainty,
not modelled. Compares against the double constants are made in double, as the image does.

Reused directly: `bsp::write_native_session_message_header_0075b480`,
`bsp::read_native_session_message_header_0075b4c0`, the `native_bit_cursor_*` readers and writers
(`004290B0`, `00429070`, `00428FF0`, `004295C0`, `00428D70`, `00428D10`, `00428C70`, `004293F0`),
`bsp::clamp_float_by_ref_00415620`, `cc13_seg47::index_block_decrement_all_007b8550`,
`cc13_seg47b::plane_update_pose_derived_state_007c18b0`. `PlaneSaveHost` extends
`cc13_seg47b::Seg47bHost` and adds only the callees no earlier pass names.

## 1. Reconstructed (3)

| Address | C++ | ABI and behaviour |
|---|---|---|
| `007C7A40` | `plane_sync_message_write_007c7a40` | __thiscall(ECX = message; cursor) RET 4, no SEH. Header `0075B480`; `delivery_one` = message+4 == 1. If +8Ch != 7 the peer record (+1Ch, from `007C2810`) +2Ch is set to 5; +A0h = record+2Ch == 0 is sent, then `007B8550`(record). Compact form (+A0h) sends +94h in 2 bits and, at level 0, +9Dh. Unless +9Dh: x/z clamped to +-24000 (22-3*level bits), y+150 clamped to [0,2047] (scale 2048, 20-3*level bits), and the three angles +20h..+28h as delta fields against record +48h..+50h (threshold 0.0005f, scale pi, (7-level)*2 bits). Full form: the three +50h..+58h values (60.0/10 bits when below 60, else 1024/16) and +68h..+70h (0.5/14 or 6.0/18), each behind its size bit. Compact form: the "both ratios +5Ch/+64h, +60h/+64h below 0.01" bit; else +5Ch/+60h as delta fields (0.05 against record +60h/+64h, clamped to +-100 through `00415620`, (8-level)*2 bits); +64h (0.1 against +68h, sent as v+80 clamped to [0,1024], (9-level)*2 bits); +68h..+70h (0.0005 against +78h..+80h, clamped to +-8, 15-2*level bits). Then +9Fh; unless +9Dh, +44h is clamped to [0,1] in the message and +44h/+38h/+40h/+3Ch/+48h are sent as delta fields (record minus message, 0.08, 7/8/8/8/7 bits) followed by +4Eh, +75h, +74h. Full form: +8Ch in 3 bits, +9Eh, and at state 4 +78h scaled by tuning+51Ch (`0042E740`, MaxWireRope), 10 bits. Record +2Ch is decremented while positive. |
| `007C8A80` | `plane_sync_message_read_007c8a80` | __thiscall(ECX = message; stream) RET 4, no SEH; readers get stream+4. The exact mirror of the writer: header `0075B4C0`, +9Dh cleared, +A0h, +94h (2 bits) and +9Dh, position (y minus 150.0), angle presence bits +A6h..+A8h, full form (+76h = 0, size bit per value), compact form (+76h = 1; "both small" sets +A9h/+AAh and zeroes +5Ch/+60h; presence bits +A9h..+AEh, +64h minus 80.0), +9Fh, unit presence bits +A1h/+A3h/+A5h/+A4h/+A2h with +44h/+38h/+40h/+3Ch/+48h, +4Eh/+75h/+74h, then +8Ch (3 bits) and +9Eh with +78h at state 4, or +8Ch = 7 / +9Eh = 0 on the compact form. +78h = 0 unless read; +7Ch = 0; +80h..+88h copied from `00F87574`..`00F8757C`. |
| `007C9860` | `plane_save_to_archive_007c9860` | __thiscall(ECX = plane; archive) RET 4, SEH frame (state 0 only around the PilotBot constructor). `0095BB10`(plane; archive), then under "_plane": originalEquipmentIndex +C54h, forcedDrop +C58h; "_bulletBase" section `006E6E80`(plane+72Ch); with +9D4h set: squadron (kind 4 pointer), formationIndex +9D0h, squadronIndex +9D8h; "control" section (pwr/yaw/roll/pitch/airBrake inputs +9F0h/+9E4h/+9ECh/+9E8h/+9F4h, gunFire +9FAh, turboInput +9F9h); "dynamics" section `007D9B70`(plane+AB0h); "ctrlParams" (+BB8h, +BB0h, +BC0h, +BBCh, +BB4h); "controlFactors" (+BCCh..+BE4h); "controlShake" (+BE8h..+BF0h); `007C5AC0`(plane; -1.0f); bombing +C25h, bombDelay +C28h, playerControllable +C0Ch, numBombsTriggered +C20h, lastGunState +C35h, shootingListIn +C40h; "failures" with four indexed bytes +C36h..+C39h; someFailure +C3Ah, planestate +900h, planeflown +908h, planelanded +904h, planewaterfill +90Ch. Then a new PilotBot (`00BF681B`(BCh), `0099A880`) is stored at +DF4h (the old one is not released) and attached through vtable `00D1F348` slot +4 (`009995A0`) to plane+310h; `007C18B0`(plane; 0.0f); gearsState +C08h; "partAnims" ref (`00BD5010`) for `007DEB70`([plane+DECh]); aiBayDoorState +DF8h, bayDoorTrgState +DF0h; "rotorManager" ref for `007E0D20`(plane+C98h); leave. |

### Simplifications and uncertainty

- `007C7A40`/`007C8A80`: values and scales reach `004295C0`/`004293F0` through `FLD m32`/`FSTP`,
  so a signalling NaN arrives quieted; `x87_copy_bits` reproduces that, and the record stores
  made with `FLD`/`FSTP` use it too. The `+44h` clamp is an SSE `MOVSS` store (bitwise).
- `007C7A40`: the bool for +A0h is pushed as EAX after `SETZ AL` (upper bytes stale) and the
  "both small" bool as the dword at [ESP+10h] whose upper bytes still hold an earlier float temp;
  `004290B0` tests only the low byte (`CMP byte ptr [ESP+4],0`), so passing the flag is exact.
- `007C7A40`: the unit fields store 0 into their record counter before the final 0/-1 store
  (`MOV [rec+cnt],EBP` with EBP = 0); both stores are kept. The rate field +70h is re-sent once
  its counter exceeds 12 (`JG 0Ch`), the units +38h/+3Ch/+40h/+48h once theirs exceed 24
  (`JG 18h`); all other delta fields once theirs is negative.
- `007C8A80`: the image reads the per-value size bits into its argument slot and [ESP+13h]; the
  C++ uses locals. It never writes +9Ch, +4Ch/+4Dh, or +94h on the full form beyond the zero.
- `007C9860`: kind-3 pairs store only the low payload byte; the upper three bytes are stale stack
  and are passed as zero here. A failed `operator new` leaves EAX = 0 and the image faults on the
  vtable load; no guard was added.
- The ledger's `009995A0` record says it is slot +0Ch of `00D1F348`; the PE bytes put it at +4h
  (`00D1F34C` = `009995A0`, +0Ch is `0099ACD0`). The C++ follows the bytes.

## 2. Existing and recorded (0)

None of the three had an implementation.

## 3. Skipped (0)

## 4. Calls checked

- `007C9860` calls `0095BB10` at `007C987F`.
- `007C9860` calls `006E6E80` at `007C9937`.
- `007C9860` calls `007D9B70` at `007C9BBB`.
- `007C9860` calls `007C5AC0` at `007C9FC2`.
- `007C9860` calls `00BF681B` at `007CA294`.
- `007C9860` calls `0099A880` at `007CA2AE`.
- `007C9860` calls `007C18B0` at `007CA2DD`.
- `007C9860` calls `00BD5010` at `007CA32A`.
- `007C9860` calls `00BD5010` at `007CA3B9`.
- `007C9860` calls `007DEB70` at `007CA335`.
- `007C9860` calls `007E0D20` at `007CA3C4`.
- `007C7A40` calls `0075B480` at `007C7A4C`.
- `007C7A40` calls `007B8550` at `007C7A88`.
- `007C8A80` calls `0075B4C0` at `007C8A8C`.
- `007C8A80` calls `0042E740` at `007C9058`.

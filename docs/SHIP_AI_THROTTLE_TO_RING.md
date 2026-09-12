# Where the ship AI's throttle and steering actually go

Addresses: 009F3F80 009F4DA0 0080E170 0080E190 009DA250 009EC7C0 009D6B40 009D8B90 00415620
00415690 006BC0C0 0092D730 00813020 009F4D10 009ED6B0 009F50E0

Packet `cc_ai_throttle_ring`, worker `agent/cc-ai-throttle-ring`, 2026-09-12 UTC. Ghidra was
read-only for this packet: no renames, comments, prototypes, function creation or saves.
Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every live query
verified both. Descriptive names are hypotheses, not recovered symbols.

## Answer to the packet question

**The AI's desired throttle at `blk+1D0h` and desired rudder at `blk+1D4h` reach the ship
motion through the tail of `009F3F80`, which writes the order ring's write slot directly with
`0080E170` and `0080E190`. No order record is issued and `00816A40` is not on this path.**

The routine at `009F3F80` is the last link of the controller's per-frame chain. `009F4DA0` is
chain slot 16 of `docs/SHIP_AI_STATES.md`; its only exit is a tail call at `009F50C6` with
`ECX = brain+8h`, which is `blk`, so `009F3F80`'s `this` is the control block and its unit is
`blk+3FCh`. The last twenty instructions are the whole hop:

```
LAB_009F4B99:
  if (|blk+1D0h| < 0.05          (00D7A270, a double)            ; 009F4BB9, JBE 009F4C04
   && |0092D730([unit+1018h])| < 1.0f  (00D7A24C))               ; 009F4BD4, 009F4BF2
      blk+1D4h = 0.0f                                            ; 009F4BFC
  step = dt * 1.5                (00CE3D78, a double)            ; 009F4C12
  a = blk+1D0h                                                   ; 009F4C04
  if (step <= |ring_slot.a - a|) a = (a <= ring_slot.a) ? ring_slot.a - step
                                                        : ring_slot.a + step
  b = blk+1D4h                                                   ; 009F4C78
  if (step <= |ring_slot.b - b|) b = (b <= ring_slot.b) ? ring_slot.b - step
                                                        : ring_slot.b + step
  0080E190(unit, b)                                              ; 009F4CE8
  0080E170(unit, a)                                              ; 009F4CFB
```

`ring_slot.a` and `ring_slot.b` are loaded at the head of the same routine, at `009F400D` and
`009F4025`, from `[unit + ([unit+97Ch] << 5) + 83Ch]` and `[unit + ([unit+97Ch] << 5) + 838h]`.
`unit+838h` is the order ring and `unit+97Ch` is its **write** cursor
(`bsp/unit_state_message.hpp`, `ring+144h`), so the routine reads the slot it is about to write
and limits how far that slot may move in one frame.

`0080E170` and `0080E190` are five instructions each, `void __thiscall(unit)(float)`, `RET 4`:

```
0080E170: MOV EAX,[ECX+97Ch]; MOVSS XMM0,[ESP+4]; SHL EAX,5
          MOVSS [EAX+ECX+838h],XMM0; RET 4
0080E190: the same with the displacement 83Ch
```

They are not `0080D9B0` / `0080DA00`. Those fill the whole pending span from the read cursor to
the write cursor and then write the live pair at `ring+148h` / `+14Ch` directly. These two touch
one slot and nothing else: no bound, no cursor, no live field, no authoritative flag. The live
pair is then stepped toward the read slot by `00813020` once a frame, and `00825F20` reads it.

### The whole chain, end to end

| step | routine | what it produces |
| --- | --- | --- |
| 1 | the active state's `vtable[0Ch]` | `009DBF90`, `009DFFB0`, `009E0040` on `blk+1D0h`, `+1D4h`, `+1D8h` |
| 2 | `009ED6B0` | `blk+324h` heading target, `blk+32Ch` / `+330h` distances, `blk+35Ch` latch |
| 3 | `009F4D10` | publishes the trio into `unit + 0AECh - 54h * index`, for **other** units |
| 4 | `009F4DA0` | a throttle ceiling on `brain+34Ch` / `+350h`, then the tail call |
| 5 | `009F3F80` | rewrites `blk+1D0h` / `+1D4h`, then the hop above |
| 6 | `00813020` | steps `ring+148h` / `+14Ch` toward the read slot at `slew * dt` |
| 7 | `00825F20` | the ship motion, which reads the live pair |

Step 3 is a side branch. `docs/UNIT_AI_ORDER_SLOT_READER.md` settled that the published slot is
read only by other units; this packet settles what the unit's own motion is driven by instead.

### `009DA250`, the steering law

`float10 __thiscall(blk)(float heading_error)`, `RET 4` at `009DA3A2`, body
`009DA250-009DA3A4`, complete. It is the only writer of `blk+1D4h` inside `009F3F80`: both
stores, `009F44FC` and `009F46A0`, take its result. The error it is given is
`00438B10(blk+324h, heading)`, the same wrapped subtraction `009F43B6` builds.

```
raw = -heading_error / ([[blk+3FCh]+538h]+524h] * 1.2)     ; 009DA258, 009DA268, 009DA276, 009DA280
raw = clamp(raw, -1.0f, +1.0f)                             ; 009DA28C, 009DA2A8
if (blk+35Ch == 2) raw = -raw                              ; 009DA2B0, 009DA2C7
v = 0092D730([unit+1018h])                                 ; 009DA2D7
if (|v| < 0.4) return 0.0f                                 ; 009DA2FA, 009DA302
raw *= clamp(|v| - 0.4, 0.0f, 1.0f)                        ; 009DA31F, 009DA32B
if ((blk+35Ch == 2 && v > 0) || (blk+35Ch == 1 && v < 0)) raw = -raw
return raw
```

So the AI applies full rudder for any heading error beyond `class[524h] * 1.2` radians, holds
no rudder at all below 0.4 units of speed, and reverses the rudder both when it has latched
astern and when the hull is still moving the wrong way for the latch. `class+524h` is the one
value this packet could not name: `00831840`, the ship descriptor reader, writes `+4F8h..+51Bh`
and then `+538h`, so `+524h` sits in a gap no reader read here fills.

### What else writes `blk+1D0h` inside `009F3F80`

Ten stores, all before the hop. They are read for their shape and are not projected.

| site | what it stores |
| --- | --- |
| `009F43D2` | `0.0f` when `blk+35Ch == 0`, the stopped latch |
| `009F4439` | `009EC7C0(error, blk+0A84h, max(1.0f, blk+344h))`, the throttle ceiling |
| `009F4462`, `009F4482` | the same value limited against `blk+344h`, signed by `blk+364h` |
| `009F44AE` | `009D6B40(blk+4h, &blk+1D0h, -1.5f, +1.5f)`, contract unread |
| `009F46E0` | `00415690` clamping into `[-1, 0]`: astern only |
| `009F471B` | `00415690` clamping into `[0, +1]`: ahead only |
| `009F492B`, `009F4951`, `009F4AD2`, `009F4B3B` | the reverse-manoeuvre arms driven by the two `2Ch`-stride tables at `blk+81Ch` and `blk+848h` |

## Coverage

| routine | coverage |
| --- | --- |
| `009F3F80` | partial: `009F4B99-009F4D04` projected operation for operation, and `009F3FF8-009F402E`, the ring read that feeds it, read. `009F3F80-009F3FF7` and `009F4034-009F4B98` are read for the fields they write and are not projected |
| `0080E170`, `0080E190` | complete |
| `009DA250` | complete |
| `00415620`, `006BC0C0` | complete |
| `009F4DA0` | not reconstructed; read for its tail call only |
| `009D6B40`, `009D8B90`, `009EC7C0` | not reconstructed here; `009EC7C0` already carries a name from packet `unit_command_producers` |

## Run-time evidence

`src/ship_motion_probe.cpp --moveto X,Z` now runs the chain through the real hop instead of the
proportional stand-in it carried. `VehicleClass[20]` DeRuyter, 6000 steps at `dt = 0.05`, full
throttle, goal `4000,4000`:

| quantity | value |
| --- | --- |
| start distance | 5656.85 |
| minimum distance | 849.88 |
| first step inside the 2000-unit radius | 4586 |
| time to that radius | 229.31 s |
| final heading error | 0.032551 rad |
| final ring slot | throttle 1.000000, rudder -0.222025 |
| steps taking the `009F4BC6` deadband | 0 of 6000 |

The ship turns onto the bearing and holds it: the heading error stays inside 0.04 rad for the
whole run and the rudder tracks it through `009DA250`. `class+524h` is a stand-in
(`MaxRotAngle`, `class+4F8h`); `--yaw-authority` overrides it.

### Correction, packet `cc_ai_class_field`

The stand-in above is gone. `class+524h` has no Lua key: `00828F20`, the ship-class descriptor's
virtual slot `+14h`, derives it once per class as
`0.5 * MaxRotAngle / MaxRotAngleChangeRatio`. The same run, recomputed with the recovered value:

| quantity | was, `MaxRotAngle` stand-in | is, `00828F20` derived | evidence |
| --- | --- | --- | --- |
| `class+524h` for `VehicleClass[20]` | `0.122173` | `0.099999994` | `00828F5A`, the double `0.5` at `00D7A280`; `MaxRotAngleChangeRatio` `0.610865` at line 13885 of the installed `vehicleclasses.lua` |
| minimum distance | 849.88 | 842.31 | the run |
| first step inside the 2000-unit radius | 4586 | 4580 | the run |
| **time to that radius** | **229.31 s** | **229.01 s** | the run |
| final heading error | 0.032551 rad | 0.027144 rad | the run |
| final ring slot rudder | -0.222025 | -0.226200 | the run |

The trajectory barely moves because this course never saturates the rudder; the threshold for
full rudder does, from 8.4 deg of heading error to 6.9 deg. docs/SHIP_AI_CLASS_FIELD_0524.md has
the derivation, the writer scan and the neighbouring fields.

## Corrections

| what said it | what is true | evidence |
| --- | --- | --- |
| `docs/SHIP_AI_STATES.md` follow-up `ship_ai_throttle_to_ring` lists `009E7B53` and `009EA839` among the sites reading `blk+1D0h` / `+1D4h` | Neither is a `blk` site. `009E7B51` is `FADD [ECX+1D0h]` inside `FUN_009E76D0`, the third term of a four-float sum over `+1C8h..+1D4h` of some other object. `009EA835` is `MOVSS [EAX+1D0h],XMM0` preceded by a clamp against `00D7A24C` and followed by `MOV [EAX+1CCh],ECX`: an inlined copy of `009DBF90`'s body, a **writer**, in a range with no Ghidra function | `009E7B51`; `009EA825`, `009EA835`, `009EA83D` |
| the same follow-up lists `009F4DA0` among them | `009F4DA0` touches neither field. Its stores are `brain+34Ch` and `brain+350h`; it reaches `blk` only through the tail call | the exported pseudocode of `009F4DA0`, and `009F50C0 LEA ECX,[ESI+8]` |
| `docs/SHIP_AI_STATES.md` follow-up `unit_ai_order_slot_reader`: "who reads the promoted slot and turns a heading target and two distances into the order ring's `+148h`/`+14Ch`" | Nothing does. The write slot is fed straight from `blk+1D0h` / `+1D4h`, and `00813020` then steps the live pair toward the read slot | `009F4CE8`, `009F4CFB`, `0080E17F` |
| `src/ship_motion_probe.cpp` step 6, a proportional stand-in over the `+/-pi/4` window with the note that no native hop exists | replaced by the reconstructed hop and rudder law; one stand-in remains, `class+524h` | the run above |
| "Uncertainties" item 1 below, and the follow-up `ship_ai_class_field_0524`: `class+524h` has no name, its Lua key is unknown, and a descriptor reader other than `00831840` must fill `+51Ch..+537h` | There is no Lua key and no missing reader. `00828F20`, virtual slot `+14h` of the ship-class vtable `00D1ACC4`, derives `class+524h = 0.5 * MaxRotAngle / MaxRotAngleChangeRatio` and `class+520h = MaxSpeed / MaxRotAngle` after the Lua load, and `00831840` does write `+51Ch` (`ExplosionEfx`), so the gap is `+520h..+537h` | `00828F5A`, `00828F66`, `00828F54` with `00D7A280`; the ordering from `009650E6` (slot `+8h`) before `0096515D` (slot `+14h`) in `00964790`; docs/SHIP_AI_CLASS_FIELD_0524.md |
| the ledger name `BSP_UnitBot_RequestLoadLevels` for `009F3F80` (packet `unit_command_producers`) | superseded by `BSP_ShipAi_DriveOrderRing`. The six load-latch raises the old name described are real and its evidence is kept, but they are a side effect; the routine exists to drive the order ring | `009F4CE8`, `009F4CFB`, and the old record preserved in git history by `ledger add-name --replace` |

## Uncertainties

1. ~~`class+524h` has no name. `00831840` does not write it, so its Lua key is unknown and the
   probe substitutes `MaxRotAngle`. Every number in the run above scales with it.~~
   **Settled** by packet `cc_ai_class_field`: the field has no Lua key and `00828F20` derives it.
   See the Correction under "Run-time evidence" and docs/SHIP_AI_CLASS_FIELD_0524.md.
2. `009D6B40`'s contract is unread past its first two arms. Its `this` is `blk+4h`
   (`009F44A7 LEA ECX,[ESI+4]`), it tests `this+41h`, and when `low >= high` it stores the
   midpoint. Beyond that it walks a byte table with three `00BF7420` results as indices; the
   decompilation of that loop is not trustworthy and no assembly reading was done on it.
3. The two `2Ch`-stride tables at `blk+81Ch` and `blk+848h`, indexed by
   `((direction * 6) + bucket) * 2Ch`, are the obstacle arms. Their producer was not read.
4. `blk+0A84h`, the value `009EC7C0` is given, is produced by a `BSP_Math_StepTowards` chain at
   `009F41B0..009F42B5` whose `BSP_Math_InterpolateClamped` argument order the decompiler
   scrambles. It was not resolved from the listing.
5. Whether the write cursor can be stale relative to the read cursor on a networked client was
   not re-checked here; `docs/UNIT_STATE_MESSAGE.md` has the cursor rule.

## Follow-up packets

| packet | addresses and files | what is left |
| --- | --- | --- |
| ~~`ship_ai_class_field_0524`~~ | `class+524h`, `00831840`, `0082B170`, `009DA250` | **Done**, packet `cc_ai_class_field`: no reader fills the field, `00828F20` derives it. docs/SHIP_AI_CLASS_FIELD_0524.md, which opens three narrower follow-ups |
| `ship_ai_obstacle_tables` | `blk+81Ch`, `blk+848h`, `009F4034..009F4B98`, `009D6B40`, `009D8B90` | The two `2Ch`-stride tables and the reverse-manoeuvre arms: the six remaining writers of `blk+1D0h`, and the one thing that makes an AI ship back off an obstacle |
| `ship_ai_throttle_ceiling` | `009EC7C0`, `009F4DA0`, `blk+0A84h`, `blk+344h`, `blk+37Ch`, `00419010` | What limits the AI's throttle: `009EC7C0`'s five interpolation stages over the tuning block at `00424C40`, and what `009F4DA0` computes into `brain+34Ch` / `+350h` |

## no_ghidra_function

| start | end (inclusive) | evidence |
| --- | --- | --- |
| none | | Every routine this packet reconstructed has a Ghidra function whose body range the bridge reports: `009F3F80-009F4D06`, `0080E170-0080E18A`, `0080E190-0080E1AA`, `009DA250-009DA3A4`, `00415620-0041565E`, `006BC0C0-006BC111`. The one address in the brief that lies outside any function, `009EA839`, is a correction above and nothing here depends on it; no boundary is claimed for it |

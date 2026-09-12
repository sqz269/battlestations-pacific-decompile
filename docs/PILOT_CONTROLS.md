# Who writes the plane's control axes (packet `cc2_pilot_controls`)

Addresses: `00519520`, `007B8C30`, `007B8C90`, `007BB6E0`, `007BB920`, `007C1430`, `007C1680`,
`007C16F0`, `007C7110`, `007C7430`, `007C6F50`, `007CA3F0`, `007CAF10`, `007CB7F0`, `007CB9E0`,
`007CC7A0`, `007CC820`, `007CCFA0`, `007D0B80`, `007D1360`, `0099B290`, `0099B2A0`, `0099B450`,
`0099B590`, `0099BB40`, `0099BC00`, `0099BEE0`, `0099D300`.

Worker `agent/cc2-pilot-controls`, 2026-09-12. Ghidra was read-only for this packet: no rename,
no comment, no function creation, no save. Every name below is a hypothesis, not a recovered
symbol. `docs/PLANE_FLIGHT.md` is the contract for the flight law and the latch; this document
answers its open question "which routine writes the local player's stick into `unit+9E4h`".

## Headline

**The write-back is `007BB6E0`, and its source is a second block, `unit+9FCh`.** The pilot
control block `unit+9E4h` is not written by the player or the bot directly. There is a *pilot
command block* at `unit+9FCh` - six dwords and a pending byte - and the plane tick quantises it
into the five axes once per step:

```
00519520(view)                                       ; the local pilot's command producer
  -> 007B8C90(unit, const float cmd[6])              ; cmd -> unit+9FCh..+A10h, byte +A14h = 1
007CE040 BSP_PlaneTickElement_FixedStep @007CE865
  -> 007BB920(unit)                                  ; gate byte+61h == 0 && byte+A14h != 0
       -> 007BB6E0(unit, unit+9FCh)                  ; per axis q = ftol(cmd*127 + 128.5)
       -> byte +A14h = 0                             ; 007BB990
  -> 007CC2F0 -> vtable[+1ECh] = 007CAF10            ; clamp, then latch 007B9770
  -> 007DA710                                        ; the rate law reads unit+BB0h
```

`007BB982`-`007BB98B` is the proof of the source: `LEA EAX,[ESI+9FCh]; PUSH EAX; MOV ECX,ESI;
CALL 007BB6E0`, and inside the callee `007BB6E2 MOV EDI,[ESP+0Ch]`, `007BB6E6 FLD [EDI]`,
`007BB72A FSTP [ESI+9E4h]`. `ESI` is the unit in both frames (`007BB921 MOV ESI,ECX`).

## The pilot command block, `unit+9FCh`

`007B8C90` (raw block `007B8C90`-`007B8CD2`, `RET 4`, `__thiscall(unit, const void* cmd)`) is the
whole setter and is short enough to give in full: it copies six dwords from `cmd` into
`unit+9FCh`, `+A00h`, `+A04h`, `+A08h`, `+A0Ch`, `+A10h` (`007B8C96`-`007B8CC3`) and sets the
pending byte `unit+A14h` to 1 at `007B8CC9`.

| command | axis it becomes | quantiser site | `00519520` value |
| --- | --- | --- | --- |
| `+9FCh` | `+9E4h` | `007BB6E6` read, `007BB72A` store | `0.0f`, always |
| `+A00h` | `+9E8h` | `007BB730`, `007BB75D` | the vertical aim error term |
| `+A04h` | `+9ECh` | `007BB763`, `007BB798` | the horizontal aim error term |
| `+A08h` | `+9F0h` throttle | `007BB83A`, `007BB877` | the throttle interpolation |
| `+A0Ch` | `+9F4h` | `007BB87D`, `007BB8EE` | `0.0f` |
| `+A10h` | byte `+9F8h` | `007BB7B9`, `007BB7ED` tests | `0` |
| `+A14h` | - | `007BB929` gate, `007BB990` clear | `1` (set by `007B8C90`) |

Every writer of the block, from a `.text` scan of all four store forms at each displacement:
`007B8C90` (the setter), `007CFEB8`-`007CFED6` in the plane unit constructor `007CFD20` (zero
init), `007D16A3`-`007D16D4` and `007D178D`-`007D17C1` in the raw block `007D1360` (the network
restore, twice), and two overrides inside `007BB920` itself - `007BB954` forces `+A0Ch` to `1.0f`
whenever `unit+900h` is not one of `{4, 5, 6, 7}`, and `007BB97A` forces `+A08h` to `1.0f` when
`unit->vtable[+5Ch](17h)` is true and `unit+C24h` is clear. `004ECB17`-`004ECC17` in `004ECA30`
writes the same displacements but its base register was not traced; `004ECA30` is under
`BSP_SceneRecord_ApplyHeaderProperties`, another orchestrator's area.

## `007BB6E0` as a rule table per axis

`__thiscall(unit, const float* cmd)`, `RET 4`, Ghidra body `007BB6E0`-`007BB91E`. The first three
axes share one rule; the last two are branchy.

| axis | rule | evidence |
| --- | --- | --- |
| `+9E4h`, `+9E8h`, `+9ECh` | `q = ftol(cmd[i] * 127.0 + 128.5)`; `q >= 0FFh` gives `1.0f`, `q <= 1` gives `-1.0f`, else `(q - 128) / 127.0` | `007BB6EA` `00CFD408` = `127.0`, `007BB6F2` `00D05998` = `128.5`, `007BB708` `00D7A250` = `-1.0`, `007BB6FC` calls `00BF7420` |
| `+9F0h` throttle, analogue arm | the same round trip on `cmd+0Ch`, taken only when the boost request byte `cmd+14h` is clear **and** (`vtable[+5Ch](17h)` is false or `unit+C24h` is set) | `007BB7ED` `CMP [EDI+14h],BL`, `007BB82E`-`007BB877` |
| `+9F0h` throttle, boost arm | binary: `1.0f` when the boost byte `unit+9F8h` ends up set, `0.0f` when it does not | `007BB7E3`, `007BB816` `00D7A24C` = `1.0f`, `007BB81E`-`007BB824` |
| byte `+9F8h` | the boost request `cmd+14h` gated on the boost charge `unit+E84h > 0` and the lockout byte `unit+E88h` | `007BB7B9`-`007BB810` |
| `+9F4h` | the same round trip on `cmd+10h`, unconditional | `007BB87D` `FLD [EDI+10h]`, `007BB8EE` |

So the axes are a signed-byte channel: a locally flown plane and a replicated one see exactly the
quantised value, and the block is idempotent because the round trip is a projection.

`coverage: complete` for the five axes and the boost byte. `007BB88E`-`007BB91C`, the tail after
the fifth axis, was not read.

## The local pilot's command producer, `00519520`

Ghidra body `00519520`-`00519823`, `__fastcall(view)`. `view+1Ch` is the unit (`view+1Ch` `+538h`
is the class descriptor, the same field `007C1430` reads) and `view+24h` caches the throttle.
`caller: unestablished` - Ghidra has no call xref and a scan of the whole image for the bytes
`20 95 51 00` finds no vtable entry, so the call site is in code Ghidra has not disassembled. Its
identity rests on the contract instead: it is the only per-frame writer of the command block, and
it is the only routine that combines `004BEC00 BSP_InputManager_GetSingleton` with
`00927F30 BSP_UnitInstance_IsLocalPlayerRole` and `007B8C90`.

It is **not a raw stick**. Both attitude commands are aim-tracking errors:

| command | rule | evidence |
| --- | --- | --- |
| `cmd[0]` roll slot | `0.0f`; the routine never assigns it | `00519569`-`00519575` zero the block, no later store |
| `cmd[1]` -> pitch | `err = SubtractWrappedAngle(inputMgr+2094h * 2.0, unit+C64h)`; outside `[-30deg, +30deg]` the command saturates to `-1.0f` / `1.0f`, inside it is `clamp(err / 2.0 + \|tan(unit+C68h) * 0.0\| / classDesc+1ACh, -1, 1)` | `00CEC724` = `0.5236f`, `00CEC728` = `-0.5236f`, `00CEC730` = `2.0`, `00D7A258` = `0.0`, `classDesc+1ACh` is `PitchSpd` |
| `cmd[2]` -> the horizontal axis | `err = SubtractWrappedAngle(-classDesc+25Ch * inputMgr+2064h, unit+C68h)`; `err < -classDesc+274h` gives `1.0f`, `err > +274h` gives `-1.0f`, else `-err / classDesc+274h` | the three-way compare before `local_10` |
| `cmd[3]` -> throttle | seeded `1.0f` from `00D7A24C`, then `BSP_Math_InterpolateClamped` over `007C47F0(view+24h, speed)` and `007C4810(1.0f, ...)` with `speed = unit->vtable[+38h]()` | `0051979B`, `contract: unread` for `007C47F0` and `007C4810` |
| `cmd[4]`, `cmd[5]` | `0` | the zero init, no later store |

The `|tan(...) * 0.0|` term is dead: `00D7A258` holds `0.0`, so the pitch command reduces to
`clamp(err / 2.0, -1, 1)` with the +/-30 degree saturation.

Two roles gate the tail. `00519807 BSP_UnitInstance_IsLocalPlayerRole(1)` is the pilot seat: only
then does `00519818` push the command block. When role 1 is false and role 0 is true, the routine
instead *tracks*: `005197F9`-`005197FF` copies `unit+9F0h` into `view+24h`, and if the stick moved
more than `0.2` (`00CE3D10`) it calls `0077C470(2, 1)`. `contract: unread` for `0077C470` and for
`004C5070(0ADh)`.

`007B8C30` (raw block `007B8C30`-`007B8C8F`, `RET 4`, `__thiscall(unit, bool)`) is the other
throttle door: when the bool is set and `unit+C0Ch` is clear it queries
`(*(unit+72Ch))->vtable[+38h]()` and writes `unit+9F0h` = `1.0f` at `007B8C68` or `0.0f` at
`007B8C78`, then stores the bool into `unit+C0Ch`. Its one caller is `008A5BD0`, not read.

## `007CAF10`, the clamp: five axes, not three

`docs/PLANE_FLIGHT.md` reports a three-axis clamp at `007CB161`-`007CB1B3`. The block runs to
`007CB1EB` and clamps all five, with **two different lower bounds**:

| axis | bound | store |
| --- | --- | --- |
| `+9E4h`, `+9E8h`, `+9ECh` | `[-1.0f, 1.0f]` (`00D7A260`, `00D7A24C`) | `007CB161`, `007CB183`, `007CB1A5` |
| `+9F0h`, `+9F4h` | `[0.0f, 1.0f]`, the lower bound an immediate zero | `007CB1C7`, `007CB1EB` |

That split is the layout evidence: the first three are bipolar stick axes, the last two unipolar.
The latch `007B9770` is called immediately after, at `007CB1F3`.

Three further blocks were read in the same routine. `007CB1F9`-`007CB2E1` computes an *effective*
control trio: `+BE8h = clamp(|+BB8h| - +BCCh, 0, 1)` at `007CB24F`,
`+BECh = clamp(|+BB4h| - +BD0h, 0, 1)` at `007CB2A9` and `+BF0h = clamp(+BBCh - +BD4h, 0, 1)` at
`007CB2E1` - each latched axis minus a per-axis reduction, so `+BCCh`, `+BD0h` and `+BD4h` are
control-authority losses. The override block (`007CB2E7`-`007CB431`) is gated on
`byte +C39h != 0 || byte +C36h != 0` and then on `(*(unit+72Ch))->vtable[+38h]()`:

* predicate true: `+9F0h` and `+BBCh` are zeroed when `+C39h` is set; `+9F4h` and `+BC0h` are
  always zeroed; the routine returns.
* predicate false: `+9E4h` and `+BB0h` take `0.2f` (`00CE54A0`) when `+C39h` is set, else `-0.8f`
  (`00D05E14`) or `+0.8f` (`00CE74F8`) selected by `+C37h`; `+9E8h`/`+BB4h` and `+9ECh`/`+BB8h`
  are zeroed; `+9F0h`/`+BBCh` take `0.01f` (`00D7A238`) when `+C39h` is set else `0.3f`
  (`00CE69C8`); `+9F4h`/`+BC0h` take `0.2f`. When `unit+900h` is 4 or 5 and
  `unit->vtable[+38h]()` is below `1.3889` m/s (`00CF8AAC`, 5 km/h) it also writes `byte +C01h = 2`.

`coverage: partial` for `007CAF10`: the clamp `007CB143`-`007CB1F3`, the effective-control trio
`007CB1F9`-`007CB2E1` and the override block `007CB2E7`-`007CB431`. `007CAF10`-`007CB143`, the
engine-fire and explosion arms, was read only as pseudocode, and the instruction-level detail of
the override's two `007CB2EA`-`007CB385` predicates was taken from the pseudocode as well.

## The AI side

`0099B450` is a **seeder, not a writer**: `0099B450 MOV EAX,[ECX+2F0h]` fetches the unit and every
`MOVSS` moves *out* of the unit and *into* the bot. `__thiscall(bot)`, `RET 0` (`C3` at
`0099B588`), Ghidra body `0099B450`-`0099B588`. The plan slots are a five-element array of
`{float current; float desired; bool active;}`, stride `0Ch`:

| slot | bot field | seeded from | site | `0099B450` sets | `0099B590` sets |
| --- | --- | --- | --- | --- | --- |
| 0 | `+274h`/`+278h`/`+27Ch` | `unit+9F0h` throttle | `0099B456`-`0099B470` | `active = 0` | `desired = live`, `active = 1` |
| 1 | `+280h`/`+284h`/`+288h` | `unit+9E4h` | `0099B476`-`0099B48E` | `active = 0` | same |
| 2 | `+28Ch`/`+290h`/`+294h` | `unit+9ECh` | `0099B494`-`0099B4AC` | `active = 0` | same |
| 3 | `+298h`/`+29Ch`/`+2A0h` | `unit+9E8h` | `0099B4B2`-`0099B4CA` | `active = 0` | same |
| 4 | `+2A4h`/`+2A8h`/`+2ACh` | `unit+9F4h` | `0099B4D0`-`0099B4D8` | `active = 0` | same |

Both halves of every slot get the live axis, so the seed is "hold what the plane already has".
The tail `0099B4E8`-`0099B588` sets the rest of the command block: `+2B4h` = `classDesc+190h`
(`0099B503 FLD`, `0099B511 FSTP`, the descriptor at `bot+2F4h`), `+2BCh` and `+2C4h` zero,
`+2C8h` = `20.0f` (`00CE3930`), `+2E8h` = `1.0f` (`00D7A24C`), `+2ECh` = `0.24f` (`00E0E2EC`),
the mode words `+2CCh` = 1, `+2D0h` = 2, `+2D4h` = 1, `+2D8h` = 1, and the bytes `+26Ch`,
`+270h`, `+2B0h`, `+2DCh`, `+2E4h`, `+2E5h` zero.

`0099B590`-`0099B629` (**no Ghidra function**, `POP ESI; RET`) is the hold variant: it calls
`0099B450` and then rewrites only the five `desired` halves from the live axes, sets all five
`active` bytes to 1 and zeroes the four mode words. The per-slot mode word is provisionally
`+2D4h` slot 1, `+2D0h` slot 3, `+2CCh` slot 2, `+2D8h` slot 0, from the interleaving at
`0099B5B8`, `0099B5D6`, `0099B5F4` and `0099B622`; instruction scheduling makes that suggestive,
not proof.

`0099B2A0`-`0099B310` (**no Ghidra function**, `RET 4`) is the same seed on a bare slot array:
`__thiscall(slots, unit)`, five triples at `slots+0h`..`slots+38h`, `active` cleared.
`0099B290`-`0099B292` is `FLD [ECX]; RET`, a float getter.

**No code anywhere writes `unit+9E4h`..`+9F4h` from the bot.** A `.text` scan of every store form
at each of the five displacements, plus every `LEA reg,[reg+9E4h]` and every SIB variant, gives
writers only in `004EB9B0`, `007B8C30`, `007BB6E0`, `007CA3F0`, `007CAF10`, `007CFD20`,
`007D0B80`, `007D1360`, `007D5AC0` and the excluded `0081F980`. The bot's output leaves the plan
slots through `0099BEE0` instead: `__thiscall(bot, out, float, float)`, `RET 0Ch`, Ghidra body
`0099BEE0`-`0099BF25`, which calls `0099BC00` with `ECX = bot+274h` (`0099BEF7 LEA ECX,[ESI+274h]`)
and then copies `bot+2DCh`, `bot+2E5h` and `bot+2E4h` into `out+16h`, `out+15h`, `out+14h`.
`0099BC00` (body `0099BC00`-`0099BDA0`) calls `0099BB40` and clamps its result to `[0, 1]`.

`out` has five floats then three bytes at `+14h`..`+16h` - the same shape as the pilot control
block, and the same shape as the `0x84`-byte record `007C2810` allocates with `puVar2[3] = 1.0f`,
the one field the plane constructor also seeds to `1.0f`. **Hypothesis, not established:** `out`
is that record, and the bot's axes reach `unit+9E4h` over the session message path -
`007C29F0` sends `007BDD30(unit, record, 0, 1, unit+674h, unit+9E4h)` through
`00779F90 BSP_Session_SendMessageToHost`, `007C2AF0` serialises the record two-way against a
snapshot at `+38h`..`+4Ch` with dirty bytes `+A1h`..`+A5h`, and `007D166A`-`007D169D` restores
`record+38h`..`+4Ch` into `unit+9E4h`..`+9F8h`. `0099BEE0` has no Ghidra caller and no vtable
entry, so the link was not closed.

`0099D300 BSP_PilotBot_PlanControls` (body `0099D300`-`0099EBAB`) is `analysed only` here: it is
the only reader of a slot's `desired` half (`0099E782`, `0099E99F`, `0099EAC8` read `+284h`;
`0099E77A` tests the `+288h` flag), which is why the slots cannot be the direct path to the unit.

## The `unit+900h` enum

`007C1430` (Ghidra body `007C1430`-`007C156C`, `__thiscall(unit, int state)`) is the setter and
the only guarded writer: it returns 0 unchanged when the state already matches, otherwise stores
the new state at `+900h`, stamps `+C04h` with `-1.0f` and notifies `007C11E0(0)`. Its side effects
name two of the values outright:

| state | writers | what the setter does |
| --- | --- | --- |
| `1` | `007CC857` in `007CC820` | `+301h*4` = `+C04h` = `-1.0f`; then a `vtable[+ACh]` call with `unit+9D4h`'s `+404h` or the bot's `+7Ch` |
| `2` | `007CC7E2` in `007CC7A0` | **zeroes the throttle** `+9F0h` (`007C143C` in the setter, `007CC7CC` in the caller) |
| `3` | the setter's `case 3` only | **forces the throttle to `1.0f`** (`007C153A`) |
| `4` | `007C1697` in `007C1680` (from state 5 only), `007C7488` in `007C7430`, `007CA3F0(0)` | when `classDesc+198h != 0`: `+904h = (unit+908h > 5.0f)` and `+C18h = 3`, then return |
| `5` | `007C171E` in `007C16F0` (from state 4 only), `007CA3F0(1)` | same arm as 4 |
| `6` | `007CBA12` in `007CB9E0`, `007C63F4` in `007C6340`, `007CB9BD` in `007CB7F0` from 4 or 5 | clears `+910h`, `+904h`, `+90Ch` |
| `7` | `007C7183` in `007C7110`, `007C6481` in `007C6340`, `007D6600`, `007C70F2` in `007C6F50` from 4 or 5 | clears `+910h` and `+904h`; `+908h` = `0` when the previous state was 4 or 3, else `3600.0f` (`00CFDEB0`) |

So the enum has **at least eight values, `0`..`7`**, not six. States 4 and 5 are a toggling pair
of ground states - `007C1680` only goes 5 -> 4 and `007C16F0` only goes 4 -> 5, both re-deriving
`+904h` from `+908h` against `5.0f` (`00CE3850`) - and they are the pair the free-flight dispatch
routes to the ground arm. `007BB920` treats `{4, 5, 6, 7}` as one class: those four are the states
in which it does *not* force the fifth command to `1.0f`. Beyond that the labels are not
established; this packet did not name them.

`007CCFA0` (Ghidra body `007CCFA0`-`007CD1AB`, `__thiscall(unit, msg)`) is the plane's message
handler and the hub: its second switch is on `*(byte*)(msg+21h) - 1` and each arm requests the
state of the same number - sub-kind 1 -> `007CC820`, 2 -> `007CC7A0`, 6 -> `007CB9E0`,
7 -> `007C7110`, and sub-kinds 4 and 5 -> `007CA3F0(0)` / `007CA3F0(1)` when the payload byte
`msg+20h` is not 7 and `0042A7E0()` is false, else `007C7430`.

`007CA3F0` (Ghidra body `007CA3F0`-`007CA5E6`, `__thiscall(unit, bool)`) is the arm the plane doc
left `contract: unread`. When `unit+900h` is 1..5 it raises the `"explosion"` effect through
`vtable[+194h]`; otherwise it calls `007C1430(4 + bool)` and resets the controls -
`+9E4h`, `+9E8h`, `+9ECh` and `+9F0h` to zero and `+9F4h` to `1.0f` (the five stores
`007CA4E7`, `007CA4EF`, `007CA4F7`, `007CA4FF`, `007CA507`, with `00D7A24C`). That `+9F4h = 1.0f` next to a zero throttle is the strongest hint about the fifth
axis, and it agrees with `007BB920` holding the fifth command at `1.0f` only while airborne; the
axis was not named.

## Corrections

See `reports/pilot_controls.json` for the `was` / `is` / `evidence` triples. In summary:
`docs/PLANE_FLIGHT.md` recorded the quantiser as an in-place round trip of the axes (it is a
transform from the command block `unit+9FCh` into them, which answers that doc's open question),
recorded the `007CAF10` clamp as three axes (five, with two different lower bounds), and recorded
`unit+900h` as six values (eight, `0`..`7`). The packet brief's summary of `0099B476` as "the AI
writer of the roll axis ... reading pilot-state fields `+280h`/`+284h`" is inverted in both
directions: that instruction pair reads `unit+9E4h` and writes `bot+280h`/`+284h`/`+288h`. The
plane doc's own table has it right, as a consumer.

## `no_ghidra_function`

| address | end | what it is |
| --- | --- | --- |
| `0099B290` | `0099B292` | `FLD [ECX]; RET`, a float getter |
| `0099B2A0` | `0099B310` | `__thiscall(slots, unit)`, `RET 4`, the bare-array slot seeder |
| `0099B590` | `0099B629` | `__thiscall(bot)`, `RET 0`, the hold-all-slots seeder |
| `007CAF10` | `007CB431` | the `+1ECh` clamp; Ghidra has a function here now, created since `docs/PLANE_FLIGHT.md` |
| `007D0B80` | `007D1353` | `RET 4`; SEH prologue `6A FF / 64 A1` at `007D0B80`, so the start the plane doc left open is established |
| `007D1360` | `007D1B4B` | `RET 4`, the network restore; start established by the twelve `int3` at `007D1354` |

## `flow_gaps`

* `007D5AC0`: the stored Ghidra body is truncated - `disasm --start 007D6960` is refused as
  outside it, while the index attributes `007D6983` and `007D69BA` to it. Both are
  `LEA reg,[unit+9F0h]` / `[unit+9E4h]`, so this routine takes a pointer to the control block and
  was not read.
* `007BB6E0`: `disasm --start 007BB782` is refused although `007BB781` is listed, so the stored
  listing has a boundary discontinuity after the `00BF7420` calls. Paging the listing recovers the
  instructions; nothing is missing from the reading above.

## Open questions

* The call site of `00519520`. No xref, no vtable entry; the caller is in undisassembled code.
* Whether `0099BEE0`'s `out` really is the `0x84`-byte record `007C2810` allocates, which is what
  would close the bot's path to `unit+9E4h`. `0099BEE0` has no caller either.
* What the fifth axis `+9F4h` is. It is unipolar, `1.0f` while airborne and after the ground
  reset, `0.0f` in the `007CAF10` override.
* Which physical control each of `+9E4h` and `+9ECh` is. `docs/PLANE_FLIGHT.md` labels them roll
  and yaw from the plan-slot order; the producer says `+9E4h` is never written by the pilot and
  `+9ECh` carries the horizontal aim error, while that doc's rate law scales `YawSpd` by the
  latched `+BB0h` (from `+9E4h`) and leaves `+BB8h` (from `+9ECh`) to `007DA380`. The two
  readings disagree and the packet did not settle it; treat both labels as provisional.
* `007C47F0`, `007C4810`, `0077C470`, `004C5070`, `008A5BD0` and `007C11E0`: `contract: unread`.
* `classDesc+25Ch`, `+274h` and `+190h` have no producer in this packet's reading.

## Correction from docs/PILOT_COMMAND_PATH.md (packet cc2_pilot_command_path)

- **Was:** 0099BEE0's out record is the 0x84-byte block 007C2810 allocates, and the bot's axes reach unit+9E4h over the session message path (marked a hypothesis)
  **Is:** out is a six-dword stack buffer in 0099ACD0 at [ESP+10h], handed straight to 007B8C90 at 0099B0B9. The 0x84 record is the 0C1h session-message payload cached at owner+0Ch; its first six dwords only share the control block's default (0,0,0,1.0f,0,0)
  **Evidence:** 0099B086-0099B09A zeroes [ESP+1Ch]..[ESP+30h] and sets [ESP+28h] to 1.0f; 0099B07F pushes LEA ECX,[ESP+18h]; 0099B0B4 LEA EAX,[ESP+10h] then 0099B0B9 CALL 007B8C90 with ECX = [ESI+50h]. 007BDD30 pre-sets msg+38h..+4Ch to the same defaults at 007BDD88-007BDD9B and 007BDEAF-007BDED6 fills them from [&unit+9E4h]
- **Was:** 00519520 BSP_PlanePilotView_BuildPlayerCommand(view)
  **Is:** __thiscall(view, float dt), RET 4
  **Evidence:** 00519520 SUB ESP,28h; PUSH ESI; MOV ESI,ECX, epilogue RET 4 at 00519821; the caller pushes the float at 00519CED-00519CF4
- **Was:** the control-block writer list names 007D5AC0
  **Is:** 007D5D20, and it does not store to the block: it passes the field pointers to BSP_LuaReader_ReadField, which writes them
  **Evidence:** 007D5AC0's Ghidra body ends at its RET at 007D5D1F; 007D5D20 is a separate SEH function ending at 007D771E; its only references to the block are LEA at 007D6983, 007D69BA, 007D69F1, 007D6A28, 007D6A5F, 007D6A90 and 007D6ACD, each stored into a {2, ptr} argument record
- **Was:** the call sites of 00519520 and 0099BEE0 are open questions (no xref, no vtable entry)
  **Is:** 00519CF7 inside 00519BB0 and 0099B0A0 inside 0099ACD0, both in undisassembled code; each is the only E8 displacement in the image that lands on its target, and neither target appears as a dword anywhere
  **Evidence:** a scan of every executable section of the disk image for E8/E9 rel32 targets and for the four-byte little-endian address
- **Was:** 007D0B80 writes unit+9E8h at 007D1331 and unit+9F0h at 007D134E (the packet brief's reading)
  **Is:** 007D1331 is the only control-block store in 007D0B80; 007D134E is inside the epilogue (MOV FS:[0],ECX at 007D1343, POP EBX at 007D134A, ADD ESP,88h at 007D134B)
  **Evidence:** a grep of the exported listing for every 9Exh/9Fxh/A0xh/A1xh displacement returns the single line 007D1331 MOVSS [ESI+9E8h],XMM0

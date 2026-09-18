# The bank command's exact inputs (packet `cc8_bank_command_inputs`)

Addresses: `0047B880`, `0099BA10`, `0099D0A0`, `0099D300`, `0099B450`, `009AC42B`, `007C18B0`,
`007D1F70`, `007E2A20`, `00415510`, `00415620`, `00415690`, `0074E400`.

This closes the "What is not established" list of `docs/PILOT_PLANNER_PITCH_ROLL.md` (lines
349-373). Every item there is either settled below or restated as a labelled partial.

## Headline

1. **`0047B880` and the `caps_rate_at_one` flag inside `0099D0A0` are one bit and its
   complement.** Both evaluate `vtable[5Ch](10h) || vtable[5Ch](16h)` on the same unit. The
   reconstruction took them as two independent caller-supplied booleans and the host set both to
   `false`, a combination the original cannot produce. This is the one behavioural correction in
   the packet.
2. **`tuning+48h` is not a Lua key at all.** `007E6FB3`-`007E6FD4` computes singleton `+580h` as
   `SoftRollCtrl · (1 - SoftRollMul)`. The continuity the pitch-roll packet raised as "a hypothesis
   about the data" is a property of the loader, so the roll law's soft zone and its outer branch
   join exactly by construction.
3. **`0099BA10` is not a command.** It is `wrap(fmod(angle, 2π))` into `(-π, +π]`. At `0099D6F1`
   and `0099D736` it normalises an angle the *unit* is already holding, before the planner copies
   it into the target slot.
4. **`plan+2E8h` and `plan+2C8h` default to `1.0f` and `20.0f`.** `20.0f > π`, so the bank cap at
   `0099E27B` is inert unless a task lowers it. The reconstruction already had both, and they are
   now proven from the producer.
5. **All six unnamed class fields recovered** from the Lua reader's key strings, `TurnRoll` among
   them: `class+25Ch` really is a maximum bank angle, `1.22173 rad` (70°) in this installation.
6. **`0099D0A0`'s published expression is correct** and now has the surrounding half it was
   missing: a null guard, a small-angle shortcut, an in-place clamp, and the envelope
   `-sign(w) · Q · sin(a) · T`.

## (1) `0047B880`, the roll-limit predicate

`__thiscall(unit ECX)`, `RET` with no immediate, body `0047B880`-`0047B8BB`, 29 instructions. The
argument count is zero from the stack cleanup; the vtable calls' own arguments are popped by the
callees.

```
0047b883  EAX = [ESI]                 ; unit vtable
0047b885  EDX = [EAX+5Ch]
0047b888  PUSH 0x10 / CALL EDX        ; ECX = ESI from 0047b881
0047b88e  JNZ 0047b8ac                ; true -> EAX = 1
0047b895  PUSH 0x16 / MOV ECX,ESI / CALL EDX
0047b89d  JNZ 0047b8ac
0047b89f  XOR EAX,EAX                 ; both false
0047b8a3  TEST AL,AL / SETZ CL / MOV AL,CL     ; both tails invert
```

So it returns `!(vt5C(0x10) || vt5C(0x16))`.

**Provenance of the inputs.** Vtable slot `5Ch` is a class test. `BSP_PlaneUnitInstance_Construct`
stores `00D05F20` at `007CFD78`, and `00D05F20 + 5Ch = 00D05F7C` holds `0074E400`
`BSP_PlaneInstance_IsKindOf`, already in the ledger as `__thiscall bool(int classId)` answering the
ancestor chain `0Fh, 5, 4, 2, 1, 0` plus the dynamic id at `unit+0C4h`. So `0x10` and `0x16` are
class ids, and for a plane the disjunction is true only when `unit+0C4h` is one of them.
`docs/ENTITY_CLASS_IDS.md` puts the plane base at `0Fh` and `PlaneSquadronGen` at `18h`, so `10h`
and `16h` are two concrete plane types between them. **Which two is not established** — no class
name literal was recovered for either id. Every other plane takes the branch.

Its only floating-point use is `0099DFCC`: `cap = 0047B880(unit) ? TurnRollLimitSmall :
TurnRollLimitLarge`, entering as `maxBank = min(maxBank, cap)`, so `Large` is the *weaker* limit
and the two named types are the ones allowed to bank further.

### The identity, and the correction it forces

`0099D0A0` evaluates the same disjunction inline at `0099D1C2`-`0099D1D9`, on the same unit, in the
same tick, to decide whether to cap its rate multiplier at `1.0f`. `include/bsp/plane_ai_control.hpp`
exposed that as `PilotHeadingDiffInputs::caps_rate_at_one` and the roll cap as an unrelated
`PilotBotRollInputs::small_turn_roll_limit`, with the comment "The predicate is NOT identified".
They are the same bit:

```
small_turn_roll_limit == !caps_rate_at_one
```

`src/game_hosts_units.cpp:2516` sets `rin.small_turn_roll_limit = false` and never sets
`caps_rate_at_one`, which leaves it `false` too — the plane then takes `TurnRollLimitLarge` while
the heading-diff scale runs uncapped. `pilot_plan_roll_0099e2ba` now derives the predicate from
`in.scale.caps_rate_at_one` and ignores the field, which is kept only so existing callers compile.
See "Contract for the unit host" below.

## (2) `0099BA10`, the angle normaliser

`__fastcall(float* out ECX, const float* angle EDX)`, `RET` with no immediate, returns `EAX = out`,
body `0099BA10`-`0099BA8B`.

```
0099ba11  FLD [EDX]                        ; the angle
0099ba14  FLD double [00CE3828]            ; 6.28318548203 = 2*pi
0099ba1c  CALL 00BF857A                    ; CRT x87 fmod -> r
0099ba21..0099ba2d                         ; round-trip through [ESP+4], r is a float
0099ba31  FLD double [00CE3D18] = -pi ; FCOMIP ; JC
0099ba3b  r <= -pi  ->  r += 2*pi
0099ba54  FLD double [00CE3D28] = +pi ; FXCH ; FCOMI ; JBE
0099ba62  r >  +pi  ->  r -= 2*pi
0099ba7b  otherwise r unchanged
          *out = r ; return out
```

`*out = angle` wrapped into `(-π, +π]`.

**What it writes at the planner's two call sites.** Nothing of its own — the planner stores the
result:

```
0099d6cc  EDI = plan+2F0h = unit
0099d6d2  XMM0 = unit+9A8h                       ; a commanded BANK angle held on the unit
0099d6da  UCOMISS XMM0,XMM2 / LAHF / TEST AH,44h / JNP 0099d717    ; skip iff equal to 0.0f
0099d6e3  EDX = &[ESP+44h] (value) ; ECX = &[ESP+54h] (out)
0099d6f1  CALL 0099BA10
0099d700  plan+2C4h = *out                       ; the bank target
0099d711  task+2CCh = EBP = 1                    ; arm the roll law against it
0099d717  the same shape for unit+9ACh -> plan+2BCh, task+2D0h = 1 (0099d745 / 0099d756)
```

`XMM2` is `0.0f`: filtering the whole `0099D300` listing for `XMM2` gives writes only at `0099D5AC`
and `0099D5FF` (both `XORPS`) before `0099D6DA`, and the next write is `0099D70E`, after it. So a
**non-zero** `unit+9A8h` / `unit+9ACh` arms the axis; a NaN does not.

These two sit at the end of a five-field direct-override block, `0099D5A4`-`0099D756`, in which
`unit+998h`, `+99Ch`, `+9A0h` are clamped to `[00D7A260, 00D7A24C]` = `[-1, +1]` and stored to
`plan+284h`, `+29Ch`, `+290h` with `active = 1` and the mode word set to `EDI = 0`. The two angle
fields are the only ones that go through `0099BA10` and the only ones that set a mode of `1`.

## (3) `plan+2E8h` and `plan+2C8h`

A `MOVSS` disp32 scan (`F3 0F 11 ?? <off>`) plus the x87 store scan (`D9 ?? <off>`) and the
`MOV r32` scan (`89 ?? <off>`) over `.text`:

| slot | seeded by | value | other writers found |
| --- | --- | --- | --- |
| `plan+2E8h` | `0099B52C` in `BSP_PilotBot_SeedPlanSlots` | `00D7A24C` = `1.0f` | `009AC42B` |
| `plan+2C8h` | `0099B55E` in the same function | `00CE3930` = `20.0f` | `009B183C`, `009B61AE`, `009B9A80`, `009C66CB`, `009D2B1D`, `009A3F66`, `009B96CE` |

`20.0f` is far above `π`, so the guarded clamp at `0099E27B` (`if (plan+2C8h < pi)`) does nothing
out of the plan reset. The bank cap is opt-in, set by a task that wants a tighter turn.

`009AC42B` is the shape of a task arm: `009AC40B` writes the heading target `plan+2C0h`,
`009AC41B` sets `task+2CCh = 2` to demand a bank re-plan, and `009AC42B` writes
`plan+2E8h = 00CE3814 = 1.2f` — a task scaling the allowed bank to 120 % of `TurnRoll`.

`src/pilot_plan_slots.cpp:41-42` already seeds both to the proven values, so nothing in the
reconstruction changes here.

**Partial.** The seven other `plan+2C8h` writers are bot-task state methods with no Ghidra callers;
each was located, none was read. Their values are not established.

## (4) `unit+C84h`, the pitch-axis input

Not a held command. `007C18B0` recomputes it every pose refresh:

```
007c1bbb  XMM0 = unit+AE0h -> [ESP+3Ch]
007c1bd1  XMM0 = unit+AE4h -> [ESP+40h]
007c1be1  XMM0 = unit+AE8h -> [ESP+44h]
007c1cae  FLD [ESP+44h] ; FLD [ESP+3Ch] ; FMUL ST0 ; FLD ST1 ; FMULP ST2 ; FADDP
007c1cc2  FLD double [00CE3820] ; FCOMI ; CALL 00BF7030          ; sqrt of the sum of squares
007c1cf0  FLD [ESP+40h] ; FLD [ESP+0Ch] ; CALL 00BF701A          ; atan2
007c1d05  FSTP [ESI+0C84h]
```

So `unit+C84h = atan2(unit+AE4h, sqrt(unit+AE0h² + unit+AE8h²))` — the **elevation angle of the
vector at `unit+AE0h..AE8h`**, with `+AE4h` the vertical component. `unit+C7Ch` is the same shape
over `unit+AC8h..AD0h`, and `unit+C88h`/`+C80h` are their rates.
`docs/PLANE_ATTITUDE_ANGLES.md` lines 250-279 already establishes `007C18B0` as the sole runtime
writer of the `C7Ch`/`C80h`/`C84h`/`C88h` group, with construction-time initialisation in
`BSP_PlaneUnitInstance_Construct` (`007D0199` for `+C84h`) and `BSP_SceneRecord_Construct`.

**Partial, and this is the one item the packet could not close.** The source vector
`unit+AE0h..AE8h` has **no disp32 writer anywhere in `.text`**: `F3 0F 11 ?? E0 0A 00 00` returns
only `004EBBBD` in `FUN_004EB9B0` (a different class), `D9 ?? E0 0A 00 00` returns nothing, and
`8D ?? E0 0A 00 00` (the `LEA &field` form a helper would take) returns nothing. Across the whole
plane and pilot code the only disp32 reference to `+AE0h` is `007C1BBD`, the read above. An empty
byte scan is not a proof of absence here — it means the producer writes the vector through a base
pointer or as part of a block, which is the known pattern for a field with no literal-address
writer. `unit+AD4h` is loaded as a pointer at `007C6609`, so `AC8h..AE8h` is **not** one nine-float
3×3 matrix; the two vectors are separate.

What is established: `unit+C84h` is a geometric elevation angle recomputed from a live vector each
tick, not a command and not a latched attitude. Whether that vector is the velocity (making
`unit+C84h` a flight-path angle) or a body axis (making it a second attitude pitch) is **open**.

## (5) Class descriptor fields

Recovered from `BSP_PlaneClass_ReadLuaFields` (`007D1F70`, body `007D1F70`-`007D3E51`). Each field
is read by the same three-call shape — `PUSH <key>` / `CALL 00B67800` (lookup by name) /
`CALL 00B66270` (to float) / `FSTP [ESI+<off>]` — so the key string immediately preceding a store
is that field's original identifier.

| `class+` | store | key pointer | **Lua key** | Shooting Star (this installation) |
| --- | --- | --- | --- | --- |
| `+18Ch` | `007D23DA` | `00D06470` | `TravelSpeed` | 141.666672 |
| `+1A8h` | `007D2547` | `00D06428` | `RollSpd` | 1.22173 |
| `+1ACh` | `007D2580` | `00D0641C` | `PitchSpd` | 0.523599 |
| `+1BCh` | `007D270F` | `00D063C0` | `RollAccel` | 2.443461 |
| `+1C8h` | `007D25F2` | `00D06408` | `TurnRollSpd` | 0.10472 |
| `+25Ch` | `007D28B2` | `00D06358` | **`TurnRoll`** | **1.22173 = 70.0°** |

Positive controls: the same extraction gives `+188h = MaxSpd`, `+1B0h = YawSpd`,
`+1B8h = SlideRatio` and `+1D8h = NegativePitchRatio`, and five of the six rows above already
appear with identical names and values in `docs/PLANE_FLIGHT.md` lines 306-325, recovered by a
different packet through a different route. `+25Ch = TurnRoll` is the new one.

Cross-check against the installed
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/Scripts/datatables/autoload/vehicleclasses.lua`,
the `Shooting Star` row (`["ShortName"]` at line 2425, keys sorted alphabetically at 2423-2451):
`["RollSpd"] = 1.22173`, `["SlideRatio"] = 0.6`, `["TurnRoll"] = 1.22173`, `["TurnRollSpd"] =
0.10472`. **This installation is modded** (`docs/GAME_INSTALL_STATE`-class caveat): the file carries
local edits, so these are the values this build reads, not necessarily retail ones.

`TurnRoll = 70°` settles the pitch-roll packet's reading of `+25Ch` "as a maximum bank angle in both
arms" — the name says so.

The two rate fields also make the roll law's `t` legible. With `+1A8h = RollSpd` and
`+1BCh = RollAccel`, `t = RollSpd² / (RollAccel · WaggleLimit)` is `v²/a`: the bank angle the plane
sweeps while stopping its roll. The law `clamp(-A/t, -1, +1)` is therefore "full opposite stick
until the remaining bank error fits inside the roll-out angle".

## (6) `tuning+48h` (singleton `+580h`) has no key — it is derived

`BSP_GameTuning_LoadFromPlaneGlobals` (`007E2A20`):

```
007e6f59  singleton+578h = <SoftRollCtrl>       ; key 00D075E4
007e6f9a  singleton+57Ch = <SoftRollMul>        ; key 00D075D8
007e6fb3  FLD [EBP+57Ch]
007e6fb9  FLD1
007e6fc0  FSUBRP                                ; 1 - SoftRollMul
007e6fce  FMUL [EBP+578h]                       ; * SoftRollCtrl
007e6fd4  singleton+580h = SoftRollCtrl * (1 - SoftRollMul)
007e6ff1  singleton+53Ch = <CruisingAlt>        ; key 00D075CC, the next row
```

There is no `planeglobals.lua` key for `+580h`, which is why the setter table has no row: the
loader computes it. `docs/PILOT_PLANNER_PITCH_ROLL.md` §1c offers
`tuning+48h == SoftRollCtrl · (1 - SoftRollMul)` as "a hypothesis about the data"; it is an
identity of the loader, so the roll command is continuous across `|v| = SoftRollCtrl` for every
possible tuning file.

### The four `HdgDiffCalc` keys

Same loader, same shape, and they are what name `0099D0A0`:

| `tuning+` | singleton | key | used by |
| --- | --- | --- | --- |
| `+4Ch` | `+584h` | `Pilot/General/HdgDiffCalcLimit/1` | `0099D1A8`, the rate curve's `y0` |
| `+50h` | `+588h` | `Pilot/General/HdgDiffCalcLimit/2` | `0099D197`, the rate curve's `y1` |
| `+54h` | `+58Ch` | `Pilot/General/HdgDiffCalcMinPitch` | `0099D140`, the pitch-command floor |
| `+58h` | `+590h` | `Pilot/General/HdgDiffCalcMinRoll` | `0099D174`, the bank-magnitude floor |

`007E71D1` and `007E723F` both push `00D0753C` (`"HdgDiffCalcLimit"`) with `PUSH 0x1` / `PUSH 0x2`
before the lookup, which is the indexed form the existing tuning table already uses for
`TurnRollPitchLimit…/1`,`/2`.

## (7) Corrections to `docs/PILOT_PLANNER_PITCH_ROLL.md`

Appended as a correction section to that document; repeated here with the bodies that settle them.
All three helpers take their operands **by reference**, and none of them is a blend.

**`00415510`** — `__fastcall(const float* a ECX, const float* b EDX)`, `RET` (0 args), x87 return.

```
00415513  [ESP]   = *a
00415518  [ESP+4] = *b
0041551e  FLD [ESP] ; FLD [ESP+4] ; FCOMIP ST0,ST1 ; FSTP ST0     ; compare b with a, stack empty
00415529  JBE 0041553c    -> return *b
          fall through    -> return *a
```

`min(*a, *b)`. The pitch-roll doc calls `0099E5C4` "the `plan+2ECh` blend"; it is a **minimum**,
matching the ledger's `BSP_Math_MinFloatByRef`.

**`00415620`** — `__fastcall(const float* v ECX, const float* lo EDX, const float* hi <stack>)`,
`RET 4`, x87 return, **by value**: it never writes through `ECX`.

```
00415635  FCOMI ST0,ST1 ; JA 00415657     ; lo > v  -> FSTP ST1 leaves lo
00415639  EAX = [ESP+0Ch] = &hi ; FSTP ST0 ; FLD [EAX] ; FSTP/FLD [ESP+0Ch] ; FXCH
0041564b  FCOMI ST0,ST1 ; JBE 00415657    ; v <= hi -> leaves v
0041564f  FSTP ST0                        ; else leaves hi
```

`clamp(*v, *lo, *hi)` returned in `ST0`. The pitch-roll doc read this "purely from the call shape"
and flagged it as unread; the body **confirms the reading exactly**, including that the x87 stack
balances only because it returns a value. It is `BSP_Math_ClampFloatByRef`,
`bsp::clamp_float_by_ref_00415620` in `src/ship_ai_throttle_ring.cpp`.

**`00415690`** — the by-reference sibling `BSP_Math_ClampInPlace`, same argument shape but `ECX` is
in/out: `004156B0` and `004156D6` write the bound back through `ECX` and the x87 stack is left
empty. Used at `0099D181`, where the in-place form is what lets the clamped magnitude be read back
from the frame at `0099D186`.

## (8) `0099D0A0`, verified

`__thiscall(plan ECX, float w)`, `RET 4`, x87 return, body `0099D0A0`-`0099D2F6`. The expression
recorded in commit `a5c3c5ec8` was re-derived from the listing and is **correct**; it is the tail
only. In full, with `θ = unit+C64h` and `a` the clamped magnitude:

```
0099d0a6  plan+2F0h == 0                       -> return 00CE3800 = 0.5f
0099d0c2  a = |w|                                  ; the -0.0f - w idiom, 00D7A218 = 0.0f
0099d0e9  |w| < 00D7A3A0 = 0.1                 -> return -w        ; FCHS at 0099d0fa
0099d140  P = min(plan+2A0h ? plan+29Ch : plan+298h, HdgDiffCalcMinPitch)
0099d181  a = ClampInPlace(a, HdgDiffCalcMinRoll, 00CE380C = 1.5f)
0099d1b4  m = InterpolateClamped(0, HdgDiffCalcLimit/1, TurnRoll, HdgDiffCalcLimit/2, a)
0099d1c2  if (vt5C(10h) || vt5C(16h))  m = min(m, 1.0f)        ; == !0047B880
0099d210  T = a / (RollSpd * m) + 1 / RollAccel
0099d24f  Q = TurnRollSpd*cos(θ) + PitchSpd*P
              + YawSpd*SlideRatio*cos(θ)*cos(a) + 0.8*YawSpd    ; 00CE3D40 = 0.8 (double)
0099d2b7  s = Q * sin(a) * T
0099d2c1  return (w < 0) ? +s : -s                              ; JC on the 0099d244 COMISS of w
```

**Frame note.** The clamp at `0099D181` targets `entry_ESP-1Ch`, which the `|w|` branch wrote as
`[ESP+4]` at `0099D0CB`/`0099D0DF` before `PUSH EBX`/`PUSH EDI` and which the call site addresses as
`[ESP+10h]` after those two pushes *and* the pushed `&hi`. Reading either literal offset without the
12 bytes of pushes makes the clamp look like it operates on an uninitialised slot. `0099D23E
MOVSS XMM0,[ESP+2Ch]` is the anchor: it is the incoming argument, and it fixes the frame for
everything after the `0099D255` `POP EDI` and `0099D25A` `POP EBX` that shift the rest.

`src/plane_ai_control.cpp`'s `pilot_heading_diff_0099d0a0` already implements all of this,
including the `cos(pitch)` correction and the `0.8f`-widened-to-double. Nothing in it changed; the
packet's contribution is the ledger record, this section, and the `HdgDiffCalc*` key evidence that
justifies the name.

## ABI summary

| address | ABI | cleanup | returns |
| --- | --- | --- | --- |
| `0047B880` | `__thiscall(unit ECX)` | `RET` (0) | `AL`, 0 or 1 |
| `0099BA10` | `__fastcall(float* out ECX, const float* angle EDX)` | `RET` (0) | `EAX = out` |
| `0099D0A0` | `__thiscall(plan ECX, float w)` | `RET 4` | `ST0` |
| `00415510` | `__fastcall(const float* ECX, const float* EDX)` | `RET` (0) | `ST0` |
| `00415620` | `__fastcall(const float* ECX, const float* EDX, const float* hi)` | `RET 4` | `ST0` |
| `00415690` | `__fastcall(float* ECX, const float* EDX, const float* hi)` | `RET 4` | via `ECX` |

## Contract for the unit host

`src/game_hosts_units.cpp:2516` (`rin.small_turn_roll_limit = false`) is now dead and should be
deleted, and nothing should set `PilotBotRollInputs::small_turn_roll_limit`. The bit that matters
is `PilotHeadingDiffInputs::caps_rate_at_one`, which the host does **not** currently set and which
should be `unit->IsKindOf(0x10) || unit->IsKindOf(0x16)`. The file is another worker's this turn, so
the change is stated here rather than made. Until the host sets `caps_rate_at_one`, the default
`false` is the correct answer for every plane that is not one of those two types, which is the
common case.

## no_ghidra_function

None. Every routine read in this packet has a Ghidra function: `0047B880` (`0047B880`-`0047B8BB`),
`0099BA10` (`0099BA10`-`0099BA8B`), `0099D0A0` (`0099D0A0`-`0099D2F6`), `00415510`, `00415620`,
`00415690`, `007D1F70` (`007D1F70`-`007D3E51`), `007E2A20`, `007C18B0` (`007C18B0`-`007C1D7A`).
`0074E400` `BSP_PlaneInstance_IsKindOf` has no Ghidra function and was **not** re-read here; its
body `0074E400`-`0074E435` is quoted from the existing ledger record.

## Validation

Both runs use the identical command line, `USN01`, 3000 mission frames at 0.05 s:

```
bsp_game.exe --game-root "<install>" --frames 3200 --press-start-frame 30 --menu-select USN01
  --mission-frames 3000 --mission-frame-seconds 0.05
  --xlive-dll build/win32/Release/xlive_stub.dll --log local/usn01_<before|after>.log
```

### Before (untouched tree at `dd0d274df`)

| aircraft | range first → last | closed | heading error first → last |
| --- | --- | --- | --- |
| Mav1 | 4813.1 → 8765.1 m | -3952.0 m | 2.139 → 0.318 rad |
| Mav2 | 4741.2 → 3373.5 m | 1367.6 m | 2.228 → 0.187 rad |
| Mav3 | 4623.5 → 4060.3 m | 563.2 m | 2.193 → 0.032 rad |
| Mav4 | 5036.8 → 4109.4 m | 927.4 m | 2.207 → 0.000 rad |
| Mav5 | 4922.2 → 5436.2 m | -514.0 m | 2.174 → 0.197 rad |

```
summary mission pilot attack: ordered=5 range_first_mean=4827.3 m range_last_mean=5148.9 m
  closed_mean=-321.6 m worst_closed=-3952.0 m | heading_error_first_mean=2.188 rad
  heading_error_last_mean=0.147 rad final_pitch_mean=0.002 rad
```

### After

The table is **bit-identical to Before**, aircraft for aircraft and digit for digit. `diff` over the
two 1.9 MB logs returns three lines: the log filename in the banner and three heap addresses
(`game resource factory published`, `native renderer constructed`). Nothing in the simulation moved.

| aircraft | range first -> last | closed | heading error first -> last |
| --- | --- | --- | --- |
| Mav1 | 4813.1 -> 8765.1 m | -3952.0 m | 2.139 -> 0.318 rad |
| Mav2 | 4741.2 -> 3373.5 m | 1367.6 m | 2.228 -> 0.187 rad |
| Mav3 | 4623.5 -> 4060.3 m | 563.2 m | 2.193 -> 0.032 rad |
| Mav4 | 5036.8 -> 4109.4 m | 927.4 m | 2.207 -> 0.000 rad |
| Mav5 | 4922.2 -> 5436.2 m | -514.0 m | 2.174 -> 0.197 rad |

```
summary mission pilot attack: ordered=5 range_first_mean=4827.3 m range_last_mean=5148.9 m
  closed_mean=-321.6 m worst_closed=-3952.0 m | heading_error_first_mean=2.188 rad
  heading_error_last_mean=0.147 rad final_pitch_mean=0.002 rad
```

### Why it is unchanged, and why that is not a null change

The corrected predicate **is** live and it **does** move `max_bank`. This installation's
`planeglobals.lua` has `TurnRollLimitSmall = DEG(85)` and `TurnRollLimitLarge = DEG(56)`, the
plane's `TurnRoll` is `1.22173` = 70 degrees, and the host feeds both limits from the loaded globals
(`src/game_hosts_units.cpp:2533-2534`; the run's `plane globals` line reports only `DropDist` and
`TurboAngle` missing, so neither limit is zero). So:

| | cap | `max_bank = min(1.0 x TurnRoll, cap)` |
| --- | --- | --- |
| before | Large, 56 deg | 56 deg |
| after | Small, 85 deg | 70 deg |

It does not reach the output because the **next** limit binds first. The law ends with
`bank_target = clamp(raw, -L, +L)` where `raw = clamp((h - s)/C, -1, +1) * max_bank` and
`L = max(Lc, |bank|)`, with `Lc = InterpolateClamped(TurnRollPitchLimitPitch/1 = DEG(0),
TurnRollPitchLimitRoll/2 = DEG(60), TurnRollPitchLimitPitch/2 = DEG(12),
TurnRollPitchLimitRoll/1 = DEG(15), pitch_error)`. `Lc` therefore falls from 60 degrees at zero
pitch error to 15 degrees at 12 degrees of it, and `L` is at most 60 degrees while the plane is not
already banked past that. USN01 opens with heading errors near 2.19 rad, which saturates the inner
`clamp(..., -1, +1)` at `|u| = 1`; `|u| * max_bank` is then 56 or 70 degrees against an `L` of 60
or less, so both sides clamp to the same `+/-L` on every tick that matters. The two caps only
separate in the narrow band `56 deg < L <= 60 deg`, which needs a near-zero pitch error at the same
moment as a saturated heading error.

That is the honest reading: the input was wrong, it is now right, and this mission does not
discriminate. A mission whose bots fly with small pitch error and large heading error would.
`TurnRollLimitSmall`/`Large` name the **aircraft**, not the limit -- `planeglobals.lua` glosses them
"egy kis gep maximum ekkora rollal fordulhat" and "egy nagy gep ...", so a small plane may bank
further, and class ids `10h` and `16h` are the large types.

### Re-run on the merged tree

After merging `main` (which brings the lead's ordinal-5002 stub, `tools/run_game.ps1`, the torpedo
task arm and the native grid work), `USN01` was run once more through `tools/run_game.ps1` and the
launcher's lock. The per-aircraft table and the `summary mission pilot attack` line reproduce the
Before/After tables above **digit for digit**, so neither the merged stub nor the merged work
disturbs this packet's measurement.

### USN02 gunnery census

Run **on this tree**, with and without the one-line predicate change, same flags, `USN02`, 3000
mission frames. The two logs are **identical over the whole gunnery census** — `diff` across the
block returns nothing:

```
summary mission gunnery units=32 guns=464 passes=32 ticks=96000 bodies=2307 bridge=2307
  sweeps=6921 candidates=3421 rejected=42192 assignment_passes=9006 gun_evaluations=4845
  slot_rejects=0 assigns=4845 clears=28576
summary mission gunnery aim angle_sets=1169733 refusals=185667 steps=71909 arc_blocks=16491
  arc_unsolved=0 trigger_rises=210 fire_messages=118989 fire_if_ready=118989
  can_fire_refusals=118255 shots=734 first_shot=1.40 s
summary mission gunnery projectiles created=734 steps=118430 sweeps=118430 entity_impacts=181
  water=456 expired=62 in_flight=98
summary mission gunnery damage queued_hits=181 dispatched=181 hit_records=181 hull=180 part=0
  fires=0 floods=0 attributions=181 deaths=2 kill_credits=2 total_damage=18525.6 first_hit=42.00 s
```

**These are not the numbers in `docs/GAME_EXECUTABLE.md` lines 8120-8129** (`candidates=3150`,
`gun_evaluations=1373`, `assigns=1373`, `clears=32048`, `shots=215`, `hull=161`,
`total_damage=14729.8`). That baseline predates a long run of intervening commits, including the
torpedo work that shows up here as `TORPEDO 71 guns / 2753 assigns`. The same-tree before/after
above is what attributes the difference: **none of it is this packet's.** Recording the older
figure as "unchanged" without the paired run would have been the wrong call, and a paired run was
the only way to tell.

Contention note: `USN02` needs the single-instance mutex, and a peer agent's `bsp_game` held it
through three attempts (`StartupHost::create_single_instance_mutex` then `error_message_box`, exit
255, a 3.3 KB log). Both runs above waited for the mutex to clear. A run that dies there is a
scheduling failure, not a result.

## A blocker on the way, and who fixed it

The tree at `dd0d274df` could not run at all. `src/native_renderer_end_frame.cpp:108` resolves
`xlive.dll` ordinal 5002 (`XLiveRender`) at construction and throws when it is absent, and the
stand-in `tools/xlive_stub/xlive_stub.def` never exported it, so every mission run died at
`startup failed: loaded XLive library lacks ordinal 5002` before device creation. That import
landed in `e5a4842ec`, after the last validated run (`a5c3c5ec8`).

**The fix on main is the team lead's**, `651b58bd1` (merged as `407e914b4`), and this packet carries
no `tools/xlive_stub` change. The runs reported above were taken against a local copy of the same
patch -- `XLiveRender` at ordinal 5002 returning `S_OK`, the same shape as the other overlay entry
points -- made before that message arrived and dropped afterwards. The two are functionally the
same export, and the lead's own USN02 baseline against the merged stub reproduces this packet's
after-run figure for figure (shots 734, hull 180, deaths 2, total damage 18525.6), so the
measurements stand on the merged stub.

## Running the game alongside other workers

`bsp_game.exe` is single-instance: `StartupHost::create_single_instance_mutex` (`008F8301`) puts a
second instance on a modal "already running" box. Every run is two processes -- the parent spawns a
suspended copy of itself for the native-data handoff, and that child is the real run and the one
that writes the log -- so a collision leaves the child on the box holding the mutex and the parent
waiting forever. The signature is a log that stops near 3 KB just after
`StartupHost::error_message_box`, and exit 255. Three USN02 attempts in this packet died that way.
Take the lock file under `~/.bsp` and wait for any live `bsp_game.exe` before launching;
`tools/run_game.ps1` on main does both.

## Follow-up packets

1. **`unit+AE0h..AE8h`.** Find the producer through the base-pointer or block write that the disp32
   scans cannot see, and settle whether `unit+C84h` is a flight-path angle or a second attitude.
   Same question for `unit+AC8h..AD0h` and `unit+C7Ch`.
2. **Class ids `10h` and `16h`.** Name the two plane types that take `TurnRollLimitLarge` and the
   capped heading-diff rate. No class-name literal has been found for either.
3. **The seven `plan+2C8h` task writers** (`009B183C`, `009B61AE`, `009B9A80`, `009C66CB`,
   `009D2B1D`, `009A3F66`, `009B96CE`) and the second `plan+2E8h` writer near `009ABE56`, which has
   no instruction at the scanned address in the stored listing.
4. **The host contract above**, once `src/game_hosts_units.cpp` is free.

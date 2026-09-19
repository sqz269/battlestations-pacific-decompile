# Handoff: `009C5B01`-`009C5C9B`, the dive-bomb aim error

Addresses: `009C58D0` (the aimdive tick), `009C5B54`-`009C5C9B` (the aim error, unread),
`009C5BD4`/`009C5CEF` (the -30 degree pitch gate, read but unbound).

Written for a cold reader by packet `cc8_dive_geometry` (`agent/cc8-dive-bomb`), which stopped here
on context rather than push a shaky x87 reading. Nothing below needs the session that produced it.
Read `docs/DIVE_BOMB_TASK.md`'s final section first; it has the half that is already done.

---

## 1. Where the dive stands, and what this block has to explain

The roll half of the aimdive is fixed and measured (commit `8407edc1c`): `009C4F80` is bound, the
roll input is `(aim heading - bearing)` and the bomber now holds a steady inverted bank through the
dive instead of tumbling. It still does not release, and the whole of the remaining failure is one
number:

```
local\usn04_geo2.log, the after run, every one of 344 aimdive ticks:
aimdive    1757  pitch -1.0231  bank  3.1259  alt 650.9  range  467.8  roll_in +0.0390  pitch_cmd -1.000
aimdive    1790  pitch -0.3518  bank -3.1215  alt 380.8  range  700.2  roll_in +0.0015  pitch_cmd -1.000
aimdive    1825  pitch +0.7171  bank  3.1222  alt 404.8  range 1031.1  roll_in +0.0016  pitch_cmd -1.000
```

**The pitch command is pinned at -1.0 and the aircraft is inverted, so a full push walks the nose
back up out of the dive** - `-1.02 -> -0.66 -> -0.20 -> +0.72` - and the aircraft flies a stable
inverted arc away from the target into the sea. The `-1.0` is not the image: it is this host's
`db_aim_error_last` stand-in driving `dive_bomb_aimdive_steer_009c5c9f`'s negative arm into its
clamp. Bind this block and the dive either works or fails for a reason you can name.

---

## 2. The frame, established

```
009C58D0  SUB ESP,0x48                          ; 72
009C58D6  PUSH EBX / PUSH EBP / PUSH ESI        ; 12
009C58EE  PUSH EDI                              ; 4   -> 0x58 pushed
```

**Base depth `0x58` for the whole body**, return address at `[ESP+58h]`, the `float dt` at
`[ESP+5Ch]` (`009C58FF FSUB [ESP+5Ch]` confirms it), saved registers at `[ESP+0h..0Ch]`, the `0x48`
local area from `[ESP+10h]`. `ESI` is the state (`009C58D9 MOV ESI,ECX`).

**Both `00438AA0` and `00438B10` restore `ESP` themselves** (`RET 8`): every call site in this body
does `SUB ESP,8`, two `FSTP`s and the call with no `ADD ESP,8` after, and `009C5AAC FLD [ESP+48h]`
then reads a local at the base depth. Assume the base depth everywhere except between a `SUB ESP`
and its call.

### The slot map, each pinned to its producer

| slot | holds | producer |
| --- | --- | --- |
| `[ESP+10h]` | **the aim heading**, `009C4F80`'s result | `009C593A` |
| `[ESP+14h]` | **height of the aircraft above the target point** = `unit+100h - target.y` | `009C59BA` loads `unit+100h`, `009C59C2` widens it, `009C59CD` calls `approach->vtable[0]`, `009C59CF` takes `[EAX+4]`, `009C59D2` `FSUBR`, `009C59D6` stores |
| `[ESP+18h]` | **the wide roll error**, against the latched `approach+D8h/+E0h` | `009C5AF9`, from the `00438B10` at `009C5AF1` |
| `[ESP+1Ch]` | **the planar range to the target**, `sqrt(dx*dx + dz*dz)` with the `1e-10` floor at `00CE3820` | `009C5A0B` / `009C5A16`. **Overwritten at `009C5B63`/`009C5B77` with the folded default roll error**, and again at `009C5BE7` with a cosine - three different meanings in one slot |
| `[ESP+20h]` | folded `\|pose+C68h\|`, the bank the band test reads | `009C5919`-`009C592D` |
| `[ESP+24h]` | **the default roll error**, against the aircraft's own position | `009C5AA8`, from the `00438B10` at `009C5AA3` |
| `[ESP+28h]` | scratch, reused at least four times: the widened altitude (`009C59C2`), `approach+D4h + approach+50h` (`009C5B0A`), then the **sine** at `009C5BC2` |
| `[ESP+34h]`, `[ESP+3Ch]` | `dx`, `dz` from the target to the **aircraft** | `009C5992`, `009C59AB` |
| `[ESP+40h]`, `[ESP+48h]` | `dx`, `dz` from the target to the **latched point** `approach+D8h`/`+E0h` | `009C5959`, `009C596F` |
| `[ESP+5Ch]` | the incoming `dt` - and the body **scribbles over it** at `009C59EA` and `009C5A03` | - |

`EBP` becomes `approach+14h` at `009C5BF4` (`MOV EBP,[EDI+14h]`), the 0x248-stride difficulty row
viewed 0xCh in. `009C5C20` reads `EBP+5Ch` and `009C5C69` reads `EBP+60h`; the host already names
`row+70h`/`+74h` from the same record `kDiveBombAimPrecPullPlus`/`PullMinus`, so these two are the
same record 0x14h earlier and want the same treatment.

---

## 3. What is already read in this range, and is not in dispute

* **The dive abort, `009C5AFD`-`009C5B51`** - already reconstructed as
  `dive_bomb_dive_abort` and bound in the host. It clears `state+19h` and `state+18h` and returns
  when all three hold: `approach+D4h + approach+50h > height`, `pose+C64h > -1.0472` (`00D20338`,
  -60 degrees), and `height * 0.3 + 150.0 > range` (`00CE3DC8`, `00CE3DD8`). Reading it settles that
  the x87 stack at `009C5B54` is `(range, height)` with the range on top.
* **The -30 degree pitch gate, `009C5BD4`.** `MOVSS XMM0,[ECX+C64h]`, `COMISS XMM0,[00CEC728]`
  (`-0.5235988`), and the branch byte at `009C5BDB` is `0f 87` = **JA** to `009C5CEF`, which is
  `MOVSS XMM0,[00D7A260]` = `-1.0` falling straight into the common store at `009C5CFA`. So **above
  30 degrees nose-down the image writes `cmd+29Ch = -1.0` outright and never computes the aim
  error at all.** This host implements neither the gate nor the error. `009C5CEF` is shared: it is
  also the `-1.0` clamp of `009C5C9F`'s negative arm.
* **The two bearings.** `009C594C` and `009C5988` both call `approach->vtable[0]`; the first
  differences the returned point against the latched `approach+D8h`/`+E0h`, the second against the
  aircraft at `unit+FCh`/`+104h`. Each becomes `wrap_0_2pi(pi/2 - atan2(dz, dx))` and then
  `00438B10(aim heading, that)`. **The default roll arm at `009C5D60` uses the aircraft-relative one
  (`[ESP+24h]`); the wide arm at `009C5D33` uses the latched one (`[ESP+18h]`).** The host currently
  passes the aircraft-relative one to both, which is right for the default arm and a labelled gap
  for the wide one.

---

## 4. The block to read, and its shape

`009C5B54`-`009C5C9B`, roughly 90 instructions, almost all x87.

```
009C5B54  [ESP+1Ch] = |default roll error|          ; the range that was there is now dead
009C5B7D  compare |err| with pi/2 (00CE3830)
          |err| >  pi/2 : compare range with height * 0.1 (00D7A3A0)
                          range > height*0.1 -> 00438AA0(aim heading, pi) ... and the result
                                                is POPPED AND DISCARDED at 009C5BBA
009C5BBC  [ESP+28h] = sin([ESP+18h])                ; sine of the WIDE roll error
009C5BD4  the -30 degree gate -> 009C5CEF, cmd+29Ch = -1.0
009C5BE1  [ESP+1Ch] = cos([ESP+18h])                ; cosine of the same
009C5BEE  t = approach+A8h + 100.0 (00D7A220)
009C5C04  a scale: [ESP+30h] * [ESP+70h]            ; DEPTH-SENSITIVE, see below
009C5C49  r1 = InterpolateClamped(t, 0.0, approach+ACh + approach+50h, row+5Ch, <sin term>)
009C5C4E  FSUBR [ESP+28h]                           ; <something> - r1
009C5C92  r2 = InterpolateClamped(t, 1.0, approach+ACh + approach+50h, row+60h, <that>)
009C5C97  FMUL [ESP+28h]
009C5C9B  FSTP [ESP+5Ch]                            ; the aim error
```

Both `00419010` calls are preceded by their own `SUB ESP,0x14`, so **every `[ESP+n]` between
`009C5BFD` and `009C5C49`, and again between `009C5C52` and `009C5C92`, is 0x14 lower than the base
frame** - `009C5C04 FLD [ESP+30h]` is the base's `[ESP+1Ch]` and `009C5C18 FLD [ESP+28h]` is the
base's `[ESP+14h]`. Do not read those displacements at face value; that is the single trap that will
produce a wrong reconstruction here.

### The one thing that must be resolved before transcribing

Under a hand trace the x87 stack depth at `009C5BBC` differs by one register between its three
predecessors: `009C5B9B`'s `JBE` and the `009C5BB1` call path both arrive one deep, and `009C5BB8`'s
`FSTP ST1` / `009C5BBA`'s `FSTP ST0` pair arrives empty. The instruction bytes were checked and the
listing is accurate (`009C5B87 df f1` FCOMIP, `009C5B89 dd d8` FSTP ST0, `009C5B8B 76 2b` JBE to
`009C5BB8`, `009C5BB1 e8 ea 2e a7 ff` CALL `00438AA0`, `009C5BB6 eb 02` JMP, `009C5BB8 dd d9`
FSTP ST1, `009C5BBA dd d8` FSTP ST0), so **the trace is wrong somewhere, not the listing**. Resolve
it with a scripted stack walk over the whole body before writing any C++; the `00438AA0` result at
`009C5BB1` looking discarded is the symptom, and it is suspicious precisely because a
`heading + pi` is the same inverted-flight correction `009C4F80` makes, so getting it wrong would
cost the dive a second time.

---

## 5. How to judge the result

`./tools/run_game.ps1 -Log local\<unique>.log -WaitSeconds 2400 -- --frames 5000
--press-start-frame 30 --menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05`,
one run, and read the `geo trace` lines the census prints (an additive per-tick window over the
turndown and the first 140 ticks of the dive, `GameUnitSlot::db_geo`). The measures, in order:

1. `pitch_cmd` stops being a constant `-1.000`.
2. `pitch_c64` keeps going **down** through the dive instead of walking back up past zero.
3. `range` stops rising monotonically from 468 m.
4. `releases` and `bombs_spawned` above zero, and the aircraft alive after `goaway`.

The before side is `local\usn04_geo2.log` (aim heading bound, aim error still stood in). Do not use
`local\usn04_geo1.log` as a before for this: it predates the roll fix.

---

## 6. After this

`task+41Ch` - `max(MaxSpd/ReferenceSpeed, 1.0)` into `approach+B8h`, `009F9D37`-`009F9D61` - in its
own before/after window, as `docs/HANDOFF_DIVE_BOMB_PROBE.md` section 4 explains. It moves where the
in-range latch closes and so moves every geometry number in these tables, which is why it goes last.

The `007F0280` accumulator arithmetic (`007F06AF`-`007F0916`) is the other thing left, and it is
**not** on the dive-bomb critical path: `docs/BOT_PROBE_007F0280.md` section 0.4 proves the zero the
host passes is exact whenever no unit is within 80/60/120 m of the bomber. It matters for formation
flying, not for the ditch.

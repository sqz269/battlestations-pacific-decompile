# What a bomber does after its attack task finishes (packet `cc8_after_task`)

Addresses: `009D2530`, `009BED80`, `009C7240`, `009C7270`, `009D24E0`, `009D2DA0`, `009BFEE0`,
and read-only over `009BFD70`, `009C1FD0`, `009D2720`, `007F23A0`, `009F9CE0`, `00A26510`.

Everything below is read from the listing. Where a name in this repository is a hypothesis it is
said so; the offsets and the bytes are the evidence.

## 1. "done" and "prepare" are one class, in both tasks

The single fact the rest of this document rests on.

`009D2DA0 BSP_BotTaskTorpedo_ConstructStates` builds the torpedo task's state subobjects. Two of
them are base-constructed through `009C2980 BSP_BotStateFollow_Construct` and then given the **same**
derived vtable `00D21320`:

```
009d2e81  lea  ebp,[esi+220h]
009d2e88  mov  ecx,ebp
009d2e8f  call 009c2980              ; the Follow base constructor
009d2e9c  mov  dword ptr [ebp],0D21320h
009d2ea3  movss dword ptr [ebp+98h],xmm0     ; -1.0f, [00D7A260]
...
009d2f6c  call 009c2980
009d2f79  mov  dword ptr [esi+348h],0D21320h
009d2f83  movss dword ptr [esi+3E0h],xmm0    ; 348h+98h, the same -1.0f
```

A byte scan for the immediate (`python tools/bsp.py scan-bytes '20 13 d2 00'`) returns exactly three
sites: these two and the standalone `009D24E0`. No other class carries this vtable.

The state registry gives the two offsets their authored names. `009D2F8B-009D3002` pushes
`(name, &state)` pairs into `00411E70`:

| `lea` | task offset | string | authored name |
| --- | --- | --- | --- |
| `[esi+14Ch]` | `+544h` | `00D213A0` | `moveto (torpedo)` |
| `[esi+188h]` | `+580h` | `00D2138C` | `follow (torpedo)` |
| `[esi+220h]` | `+618h` | `00D2137C` | **`torpedo/done`** |
| `[esi+2BCh]` | `+6B4h` | `00D2134C` | `torpedo/attackrun` |
| `[esi+348h]` | `+740h` | `00D2133C` | **`torpedo/prepare`** |

`bsp::TorpedoState`'s enum values are the task-relative offsets (`include/bsp/torpedo_task_arm.hpp`),
and `kMoveTo = 0x544` against `[esi+14Ch]` fixes `esi = task+3F8h`. Every other row then agrees:
`0x220 + 0x3F8 = 0x618 = kDone` and `0x348 + 0x3F8 = 0x740 = kPrepare`. Two independent
confirmations — the name table and the `0x128` delta between the two vtable stores.

So `00D21320` serves **both** states, and its slots are:

| slot | address | |
| --- | --- | --- |
| `+0h` | `009D2D00` | |
| `+4h` | `009D2530` | enter |
| `+8h` | `009D2570` | exit |
| `+Ch` | `009D2720` | **tick** |
| `+10h` | `007B3DE0` | |
| `+14h` | `009BE590` | |
| `+18h` | `009A4860` | |

**The ledger names `BSP_BotStateTorpedoPrepare_Tick` and `BSP_BotStateTorpedoPrepare_Exit` are half
names.** `009D2720` is the `torpedo/done` tick as much as the `torpedo/prepare` tick. This is the
same failure mode as `squadron_alt_limit_394` in `docs/TORPEDO_AFTER_THE_DROP.md` section 14.4: a
name chosen from one call site became the contract.

The dive-bomb task is the mirror. `00D20D28` is installed twice in `FUN_009C73A0` (`009C747E`,
`009C74B8`), with `+4h` = `009C7240` and `+Ch` = `009C7270`.

**Reconciliation with the record that renamed these.** `009D2720`'s and `009D2570`'s ledger records
(packet `cc8_torpedo_first_release_authority`, `docs/TORPEDO_FIRST_RELEASE.md`) say `009D2DA0`
"registers `param_1+0xD2` as `torpedo/prepare`" and that "`torpedo/done` is a different state at
`param_1+0x88`". Those are the **same two offsets** read through a decompiler that typed `param_1`
in four-byte elements: `0x88 * 4 = 0x220` and `0xD2 * 4 = 0x348`, and with `esi = task+3F8h` those
are `task+618h` and `task+740h`. That record is right that they are two different state **objects**
and wrong only in the inference drawn from it: `009D2E9C` and `009D2F79` install the *same* vtable
on both, so they share one class and therefore one enter, one exit and one tick. Reading them as
different classes is what put `Prepare` alone into the names.

## 2. The enter, read whole

`009D2530`, body `009D2530-009D256E`, `RET 0`, `__thiscall(state)`:

```
009d2530  movss xmm0,[00D7A260]       ; -1.0f
009d253b  movss [esi+98h],xmm0        ; the drop countdown, disarmed
009d2543  call  009bed80              ; the Follow base enter
009d2548  mov   eax,[esi+4]           ; the approach
009d254b  mov   ecx,[eax+0Ch]         ; the squadron
009d254e  mov   dword ptr [ecx+3E4h],2   ; psFormation := SHAPE 2
009d2558  call  007ed260              ; re-assign the formation indices
009d255d  movss xmm0,[00D7A24C]       ; 1.0f
009d2565  movss [esi+8Ch],xmm0
```

`009C7240`, body `009C7240-009C725B`, `__thiscall(state)`, tail jump:

```
009c7240  movss xmm0,[00D7A260]       ; -1.0f
009c7248  mov   byte ptr [ecx+9Ch],0
009c724f  movss [ecx+98h],xmm0
009c7257  jmp   009bed80
```

**The dive bomber never overrides the shape.** The torpedo's derived enter does, to 2.

## 3. `009BED80`, the Follow base enter, read whole

Body `009BED80-009BEE24`, `RET 0`, `__thiscall(state)`. Ledger name
`BSP_BotStateFollow_Enter_Provisional`.

```
009bed84  call 0042e740               ; the game tuning singleton
009bed8c  add  eax,380h               ; the Pilot/Follow table
009bed92  call 009be150   this=[state+6Ch]    ; cache the block
009beda0  [state+94h] = 0.0f
009beda8  [state+90h] = 0.0f
009bedbb  [state+88h] = *(*(state+6Ch)+8)
009bedc3  [state+84h] = 0             ; byte
009bedc9  [state+8Ch] = 1.0f
009bedd1  ecx = [approach+0Ch]        ; the squadron; JZ 009BEE1C when null
009bedda  [squadron+3E4h] = 1         ; psFormation := SHAPE 1
009bede4  call 007ed260
009bedef  edi = [squadron+3D0h]       ; the flight leader
009bedff  006952A0 / 00694A60 on state+18h, [state+2Ch] = leader
009bee1c  [state+85h] = 0             ; byte
```

So the base enter selects shape 1 and the torpedo's enter overrides it to 2 and re-assigns.
`squadron+3E4h` is `psFormation`, the SHAPE (`docs/PLANE_FORMATION.md` line 52), consumed by
`007F23A0`'s five-entry jump table at `007F2900`:

| shape | body |
| --- | --- |
| 1 | `007F2576` — reconstructed in `src/plane_formation.cpp` |
| **2** | `007F25BD` — **unread** |
| 3 | `007F2739` — unread |
| 4 | `007F26AD` — unread |
| 5 | `007F2838` — unread |

## 4. The tick, and what is a stub rather than a gap

`009D2720`'s first instruction after the prologue is the follow tick:

```
009d272b  push ecx
009d272c  fstp dword ptr [esp]        ; dt
009d272f  mov  ebx,ecx
009d2731  call 009c1fd0               ; the follow tick
009d2736  mov  ebp,[ebx+4]            ; the approach
009d2739  mov  eax,[ebp+0Ch]          ; the squadron
009d273c  movss xmm1,[ebx+98h]
009d274a  mov  edi,[eax+3D0h]         ; the flight leader
009d2753  jbe  009d29e0               ; countdown <= 0 -> the disarmed arm
```

`009C7270` is the same call and nothing else (13 bytes).

In `done` the enter leaves `+98h = -1.0f`, so `009D2753` **always** takes `009D29E0`. That arm,
`009D29E0-009D2CF3`, is 787 bytes and is reduced in `src/torpedo_task_arm.cpp:226-229` to
`outcome = kIdle; return out;`. It is not idle. Its head:

```
009d29e0  test edi,edi / je 009d2cea        ; no leader, nothing to do
009d29e8  cmp byte [edi+0C8h],0 / call 00414db0   ; refresh the leader's pose
009d29f8  approach->vtable[0](&pt)
009d2a06  fld [eax]     / fsub [edi+0FCh]   ; pt.x - leader.x
009d2a18  fld [eax+8]   / fsub [edi+104h]   ; pt.z - leader.z
009d2a35  call 00414c60                     ; Vector2f_LengthWithCutoff
009d2a41  mov eax,[ebp+14h] / fld [eax+8] / fmul [ebp+24h]   ; class row * approach+24h
```

**Coverage note.** `torpedo_done_prepare_tick_009d2720` covers the armed branch
(`009D2759-009D29CB`) and stubs the disarmed branch (`009D29E0-009D2CF3`). That is a
`partial_projection`, and the `kIdle` outcome must not be read as "the image does nothing here".

## 5. The flight leader commands nothing

`009C1FD0`'s head:

```
009c1fe2  mov byte ptr [[state+4]+18h]+26Ch, 2
009c1fea  call 009bfd70
009c1fef  test al,al
009c1ff1  je   009c234e            ; a false return ends the tick
```

`009BFD70`'s leader arm, `009BFEB1-009BFED6`, clears `state+85h`, unregisters the leader observer
(`006952A0`), zeroes `[state+18h]+14h` and returns `AL = 0`. The same false return covers "no
squadron" (`009BFD79`). So a **flight leader** — formation index 0, `[squadron+3D0h] == own unit` —
in `done` writes that one byte and returns. Only wing members reach the station-keeping law.

## 6. `009BFEE0`: its size, its gate, and which part is bindable

Body `009BFEE0-009C1846`. Measured with `local/bfee0_map.py` over the PE on disk: **1795
instructions, 210 blocks, 55 calls (19 distinct)** — the same three numbers
`docs/PLANE_FORMATION.md` section 5 records from Ghidra.

It is `__thiscall(state)` (`009BFEEC MOV ESI,ECX`), and its first test is the whole structure:

```
009bfeee  cmp byte ptr [esi+85h],0
009bfef6  je  009c0026                 ; NOT in good position -> arm B
```

`state+85h` is set by `009BFD70` from two tuning comparisons against the cached `Pilot/Follow`
block at `state+6Ch`:

```
009bfe19  call 0042b2f0                ; |own position - station|
009bfe21  fld dword ptr [eax+18h]      ; block+18h, GoodPositionDist
009bfe24  fcompi st(1)
009bfe28  jbe 009bfea8                 ; dist >= threshold: return true, +85h untouched
...                                    ; inside the radius, the heading dot product
009bfe83  fld dword ptr [edx+14h]      ; block+14h, GoodPositionDir
009bfe88  fcompi st(1)
009bfe8c  jbe 009bfea0
009bfe8f  mov eax,1  ... [esi+85h] = al    ; in good position
009bfea0  xor eax,eax ... [esi+85h] = al   ; in position by distance, not by heading
```

Both arms return `AL = 1`; only `state+85h` differs, and `009BED80` clears it on enter. So:

| `state+85h` | arm | extent | size |
| --- | --- | --- | --- |
| set | **A**, the station HOLD | `009BFEFC-009C0021`, then `jmp 009C16D2` | ~50 instructions |
| clear | **B**, the fly-TO-station law | `009C0026-009C16D1` | ~1500 instructions |

**Arm A, read whole.** It refreshes the member's own pose, builds its frame at `unit+110h` from
`unit+0CCh` (`00B63D50`), transforms the station point `state+30h` into that frame (`004142E0`),
takes a ratio from the cached block (`009BFF9C fld [block]` / `fsub` / `fdiv`), copies the station
point into `state+44h/+48h/+4Ch`, and then adds the leader's heading axis scaled by that ratio:

```
009bffb1  [state+44h] = station.x
009bffb7  [state+48h] = station.y
009bffbd  [state+4Ch] = station.z
009bffc0  edi = [state+2Ch]            ; the leader
009bffd3  fld dword ptr [edi+0ECh]     ; a leader row
009c0004  [state+44h] += lead.x
009c0011  [state+48h] += lead.y
009c001e  [state+4Ch] += lead.z
009c0021  jmp 009c16d2                 ; the common tail
```

That is the `LeaderHeadingSpdTime/Dist` lead the `Pilot/Follow` key names promise.

**The common tail, `009C16D2-009C1846`, read whole.** Both arms end here, and it is an altitude
clamp on the commanded point. It builds three bounds around the leader's altitude `leader+100h` —
`block+4h + leader+100h`, `leader+100h -/+ [00D1F3F8]` (a `qword` double constant) and the tuning
singleton's `+210h` — carries `state+88h` (which the enter seeded from `*(*(state+6Ch)+8)`) through
the same `fcompi`/`jbe` min-max chain, writes the result to `state+34h`, and then clamps
**`state+48h`**, the Y component of the commanded point, into that band:

```
009c17f1  movss dword ptr [esi+34h],xmm0     ; the commanded altitude
009c17f6  fld   dword ptr [esi+48h]          ; the commanded point's Y
009c1811  movss dword ptr [esi+48h],xmm0     ; clamped, three exits: 009C1811 / 009C1827 / 009C183B
009c181d  ret                                ; plain RET, so no stack argument
```

So `009BFEE0` produces two things and nothing else: the commanded point `state+44h/+48h/+4Ch` and
the commanded altitude `state+34h`. Those are the `esi+34h` and `esi+44h..4Ch` stores the write
census found. The plain `RET` also fixes the ABI: `__thiscall(state)` with no stack argument, so
`dt` does not reach `009BFEE0` at all.

**The honest answer to "which minimal self-contained part makes a follower fly TO its station".**
None. Flying to the station *is* arm B, and arm B is the 1500-instruction bulk; there is no small
faithful subset of it. Arm A is small, self-contained and bindable, but it is the *hold*, not the
approach. Binding arm A does not replace
`Impl::place_wing_member_on_station_007f23a0`; it is what the image runs **once the member is
already there**, which is the state this host's placement stand-in manufactures every tick. The
placement therefore remains a **hole**, stated, exactly as `docs/PLANE_FORMATION.md` section 6
labels it.

**`009BFEE0` is not the whole hole.** `009C1FD0` calls a third routine right after it:

```
009c2068  call 009bfee0
009c206d  fld  dword ptr [esp+74h]      ; dt
009c2077  call 009bee30
009c207c  cmp  byte ptr [ebp+85h],0     ; the same good-position flag again
009c2097  movss xmm1,[leader+0C68h]     ; the leader's bank
009c20e3  mov  byte ptr [unit+844h],1   ; and a float at unit+840h
```

`009BEE30` has body `009BEE30-009BFD67`, 3895 bytes — of the order of a thousand instructions, and
also unread. `docs/PLANE_FORMATION.md` section 5 names only `009BFEE0` as the station-keeping hole;
the follow tick is really `009BFD70` (reconstructed) + `009BFEE0` (this document's arm A only) +
`009BEE30` (untouched), about 2900 unread instructions in total. Any plan that says "reconstruct the
follow law" should be sized against that number, not against 1795.

Almost all of `009BFEE0`'s writes are stack locals. Outside `esp` it writes only `esi+34h`,
`esi+44h..68h` and `ebx+10Ch` (the pose-valid byte). It does **not** write the pilot command block;
`009C1FD0` does that itself (`+26Ch`, `+2BCh`, `+2C4h`, `+2CCh`, `+2D0h` off `[approach+18h]`).

## 6a. `+26Ch` is the pilot planner's mode, and `009C1FD0` writes it for the leader too

`009C1FD0`'s head writes `[[state+4]+18h]+26Ch = 2` **before** the `009BFD70` gate, so it runs for
every aircraft in the state, leader included. `[approach+18h]` is the pilot command block (the
object carrying `+2BCh`, `+2C4h`, `+2CCh`, `+2D0h`).

Who consumes the value, by exhaustive byte scan rather than by xref:

* writers, `scan-bytes 'c6 ?? 6c 02 00 00'` plus the register form at `009C1FE2`:
  `009BC8ED` (in `009BC890`), `009C2440` (in `FUN_009C2430`, which writes **1**) and `009C1FE2`
  (which writes **2**). `FUN_009C2430` has the same shape as a state tick — `ebp` the state,
  `[ebp+4]` the approach, `[approach+18h]` the command block — so **different states stamp
  different modes every tick**.
* readers, `scan-bytes '0f b6 ?? 6c 02 00 00'` → none; `scan-bytes '80 ?? 6c 02 00 00'` →
  `0099D309` and `0099EA84`, **both inside `0099D300 BSP_PilotBot_PlanControls`** and nowhere else.

`0099D309` is the second instruction of the planner:

```
0099d305  mov  esi,ecx                       ; the pilot command block
0099d309  cmp  byte ptr [esi+26Ch],2
0099d310  jne  0099d3c5                      ; every other mode leaves this path
0099d329  cmp  byte ptr [ecx+eax+9C2h],0     ; ecx = [esi+2F0h], eax = [00F876B8]*8
0099d337  mov  ecx,[esi+270h]  / cmp / je 0099d3c5
0099d35a  [cmd+284h] = 0.0f
0099d362  [cmd+288h] = 1                     ; byte
0099d369  [cmd+2D4h] = 0
0099d36f  [cmd+29Ch] = 0.0f
```

So the byte is the plan-mode selector and `2` is the mode the follow tick asks for.
`0099D300` is the last call of `009998A0 BSP_PilotBot_Update`'s order, which is how the mode reaches
the controls. ~~Coverage: the mode-2 body past `009D36F` is not read.~~ **CORRECTED in place,
packet `cc8_done_state`:** that body was never unread — `docs/PILOT_BOT_TICK_GATES.md` section (3),
"Entry gate A — the neutral plan", already tabulated every store and all four gate conditions, and
section **6c** below now reads it whole and adds the reachability. It is a neutral plan that
returns, and the byte alone does not command it: see 6c.

The consequence for this host is sharp: when a dive-bomb task reaches `done` and no state tick runs,
**nothing writes `+26Ch` at all** and the block keeps whatever the last attack state stamped.

## 6b. The BEFORE measurement, USN04 on this binary

`local/aftertask_before_usn04.log`, `--frames 5000 --press-start-frame 30 --menu-select USN04
--mission-frames 4800 --mission-frame-seconds 0.05`, built at main `f14732dc4`.

| aircraft | `arm_ticks` | `done` ticks | releases | `rounds_left` | water contact |
| --- | --- | --- | --- | --- | --- |
| `movieval` | 2112 | **303** | 2 | 0 | **yes**, `alt=-0.00 |v|=68.72` |
| `movieval|.-2` | 2370 | none | 1 | 1 | no |
| `movieval|.-3` | 2299 | **492** | 2 | 0 | **yes**, `alt=-0.01 |v|=68.45` |

**Two water contacts, not three**, and the two that ditch are exactly the two whose task reached
`done` with the ordnance spent. `movieval|.-2` still carries a round, never completes, and never
ditches — the same ordnance gating recorded elsewhere, reproduced here.

`plane formation geometry: squadron movieval tick=0 wing=3 ... seat1 index=1` — the squadron is
three aircraft with formation indices 0, 1, 2, and the unit named plain `movieval` is seat 0, the
flight leader. **So one of the two ditching aircraft is the flight LEADER and the other is a wing
member.** They need different halves of the same fix: the member reaches the station-keeping law
through `009BFD70`, while for the leader `009BFD70` returns false at `009BFEB1` and the only thing
the image commands is the `+26Ch = 2` of section 6a.

## 6c. Plan mode 2, read whole — and why the leader's one byte commands nothing

Packet `cc8_done_state`. This section **withdraws section 6a's coverage note** ("the mode-2 body
past `009D36F` is not read"). It was not unread even when 6a was written:
`docs/PILOT_BOT_TICK_GATES.md` section (3), "Entry gate A — the neutral plan", already carried the
whole store table and all four gate conditions. What follows adds the reachability, which no
document had.

**The body, `0099D309`-`0099D3C2`, 39 instructions.** It rejoins nothing. It writes a complete
neutral plan and `RET 4`, skipping the remaining ~1400 instructions of the planner — the per-tick
frame `0099D46E`, the stick overrides, the speed hold `0099D8C1`, the power ceiling `0099DC8F`,
the bank-target arm, the yaw law `0099E81A` and the pitch law `0099E68D`.

| store | slot field | value |
| --- | --- | --- |
| `0099D35A` / `0099D362` / `0099D369` | yaw desired `+284h` / active `+288h` / `+2D4h` | `0.0f` / `1` / `0` |
| `0099D36F` / `0099D377` / `0099D37E` | pitch desired `+29Ch` / active `+2A0h` / `+2D0h` | `0.0f` / `1` / `0` |
| `0099D384` / `0099D38C` / `0099D393` | roll desired `+290h` / active `+294h` / `+2CCh` | `0.0f` / `1` / `0` |
| `0099D399` / `0099D3A1` | power desired `+278h` / active `+27Ch` | `1.0f` (`00D7A24C`) / `1` |
| `0099D3A8` / `0099D3B0` / `0099D3B7` | brake desired `+2A8h` / active `+2ACh` / `+2D8h` | `0.0f` / `1` / `0` |

Wings level, no pitch, no yaw, no air brake, full power, and the four mode/target words cleared.
It commands **no point and no altitude**: nothing in the arm reads the follow state at all — `ESI`
is the command block from `0099D305` to the `RET`, so `state+44h/48h/4Ch` and `state+34h` are not
its inputs. Section 6's reading of `009BFEE0` as the producer of those four fields is unaffected;
they are simply consumed elsewhere.

**The byte is one condition of four** (`0099D309`-`0099D34D`; any failure falls to `0099D3C5`):

| address | condition |
| --- | --- |
| `0099D309` | `cmd+26Ch == 2` |
| `0099D329` | `unit[9C2h + idx*8] != 0`, `idx = word [00F876B8]` |
| `0099D33D` | `cmd+270h != 0`, the plan's target |
| `0099D345` | `target[9C2h + idx*8] != 0` |

`unit[idx*8 + 9C2h]` is the per-step published copy of **`unit+520h`**
(`007CDCD0` in `FUN_007CDC70`; `docs/GAMEPLAY_LOOSE_ENDS_2.md` A4, consumers read the previous
index `[00F876B8]`).

**Who produces conditions 2 and 3: `009BEE30`, which a Done leader never reaches.** An exhaustive
census of `unit+520h` writers (`scan-bytes 'c6 ?? 20 05 00 00'` and `'88 ?? 20 05 00 00'`, both
forms shown to occur) gives `00699B14`, `007B959B`, `007BBCF8`, `00959454`, `0099D431`,
`009BF0B8`, `00698716`, `0095CDD7`, `0099EB73`. The one on the follow path is `009BF0B8`:

```
009bee36  MOV ESI,ECX                       ; the follow state
009bee3f  LEA EDI,[ESI+4]                   ; &approach
009bee42  MOV byte [[approach+18h]+2E5h],1
009bee49  CMP byte [ESI+85h],0 / JZ 009bf9ea   ; GoodPosition; the HOLD arm continues here
009beef2  MOV EBP,[ESI+2Ch]                 ; the flight LEADER (009BEDFF seeds state+2Ch)
009beefc  JZ 009bf0eb                       ; no leader -> skip
009bef09  CMP byte [EBP+EAX*8+9C2h],0       ; THE LEADER's published +520h
009bef11  JZ 009bf0eb                       ; leader's byte clear -> skip
   ... four float tests, 009BF086-009BF0B6 ...
009bf0b8  MOV byte [EBX+520h],1             ; EBX = [approach+4] = this unit   -> condition 2
009bf0c4  MOV [[approach+18h]+270h],EBP     ; the plan's target := the leader  -> condition 3
```

Two consequences.

* **The byte propagates down a formation.** A member arms gate A only if its LEADER already
  carries it; `009BEF09` is gate A's condition 4 tested in the producer.
* **A Done flight leader executes none of it.** `009C1FF1 JZ 009C234E` on `009BFD70`'s false
  return, and `009C234E` is the epilogue (`POP EBP / POP EBX / ADD ESP,68h / RET 4`), not a join.
  So the leader never sets its own `+520h` and never gets a `cmd+270h`, while `0099D431` clears
  `unit+520h` at the top of the normal planner on every think. **Gate A is unsatisfiable for a
  Done flight leader, so the `+26Ch = 2` of section 6a commands nothing for the aircraft that
  writes it.** What keeps a leader flying in the image is the ordinary planner path, which runs
  precisely because gate A did not fire.

**A reachability fact for gate A in general.** `009998A0`'s entry gate (`009998A4`-`009998C6`)
returns at `009999B0` doing nothing when `task+270h == 2` **and** `task+274h != 0` **and** the
target's `9C2h` byte is set — a **superset** of gate A's condition. `tools/callsite_census.py`
gives `0099D300` exactly two call sites, `00999907` and `009999AA`, both inside `009998A0`. So
gate A can only fire when the mode byte was written *during the current tick*, by the state tick
that runs between the entry gate and the planner call — that is, by `009C1FE2` itself.
(`009998CD LEA EDI,[ESI+4]` fixes plan = task+4, so `plan+26Ch` and `task+270h` are one byte.)

**The second reader is not a flight law.** `0099EA84` is `CMP byte [ESI+26Ch],0` — a test for
**non-zero**, not for 2. Its block `0099EA84`-`0099EB89` ends at `0099EB73 MOV byte [EDI+520h],CL`
with `CL = SETZ([[00E188A8]+5FCh]+908h == unit+54h)` and `0099EB81 plan+2ECh = [00E0E2EC]`, so the
mode byte's other consumer only feeds the `+520h` publish loop above. The `[00E188A8]+5FCh`
identity is **provisional**: `00E188A8` is the mission/game singleton, and
`docs/PILOT_BOT_TICK_GATES.md` reads the `9C2h` byte provisionally as "under human control for the
local slot", which a `SETZ` against one named unit fits. It was not recovered here.

## 6d. The finished-task consumer is not the Done state at all

Packet `cc8_done_state`, item 3, read before any run because 6c had already shown the Done state
commands nothing for a leader.

`009998A0`'s slow path, read from the listing (`00999912`-`0099995A`): an accumulator at `+308h`
against the interval `+304h`, then **`0099993C CALL 0099B740`**, then `+2E4h = 0FFh` and
`task->vtable[64h](dt)` — the task arm — then `009FC7C0` and the rest, and `0099D300` last at
`009999AA`. So the abandon test runs **before** the task arm, once per think.

`0099B740`, body `0099B740`-`0099B77A`, read whole:

```
0099b743  ECX = [bot+2FCh]                ; the squadron
0099b74b  JZ  ret
0099b74d  EAX = [bot+2F4h]                ; the unit
0099b755  JZ  ret
0099b757  CMP EAX,[ECX+3D0h] / JNZ ret    ; ONLY the flight LEADER
0099b766  CALL [[bot]+38h]                ; the abandon predicate
0099b76a  JZ  ret
0099b76c  ECX = [bot+2FCh] / PUSH 1
0099b774  CALL 007ED3F0
```

**`007ED3F0`'s body.** `docs/BOT_TASKS.md`'s "Slot `+38h` and the abandon path" says "that naming
is a hypothesis; the body was not read" — but the **ledger already had it**, from packet
`cc8_torpedo_release_orders`, including an exhaustive four-site call census (`0099B774` mode 1,
`0084DB86` mode 1, `009A285E` mode 0, and `008A4C41` on `007ED430` with mode 2, the Lua binding).
`docs/BOT_TASKS.md` is the stale party, not the image. Re-read here:

```
007ed3f0  MOV EAX,[ESP+4] / MOV [ECX+370h],EAX / RET 4     ; squadron+370h := v, unconditional
007ed430  MOV EAX,[ESP+4] / CMP [ECX+370h],EAX / JGE ret
007ed43c  MOV [ECX+370h],EAX / RET 4                       ; squadron+370h := max(., v), a RAISE
```

So the pair is a **set** and a **raise** of one field, `squadron+370h` — the attack mode. An attack
order raises it to 2 (`007ED430(2)`); the leader's abandon test sets it to **1**.

`+38h` is `0099B710` for every pilot-task vtable (`docs/BOT_TASKS.md` line 185,
`docs/PILOT_TASK_HEADING_ARM.md` line 71), and `0099B710` is `MOV AL,1 / RET`. **So for a flight
leader the predicate is always true and `squadron+370h` is driven to 1 on every think.**

**The dive-bomb task reaches it twice per think, not once.** `tools/callsite_census.py 0099b740`
gives ten sites: `009998A0`'s `CALL` and **nine tail `JMP`s, one per class's `+54h` cruise-profile
override** — including `009C8A87` in `009C8920 BSP_BotTaskDiveBomb_UpdateCruiseProfile`. So the
dive-bomb path is not an exception to the abandon test; it runs it from its own cruise profile as
well. `src/dive_bomb_task.cpp`'s `dive_bomb_cruise_profile_009c8920` stops at `009C8A7C` and does
not model that tail, which `src/bot_tasks.cpp` line 204 already flags as "modelled separately".

That is the channel the Done state is not. `squadron+370h` is exactly the field the dive-bomb
machine gates on: `009C83F8` returns `in_range_latch_4c8 || (mode == 2 && latched target)`, and
`009C8483` sends an attacking task back to the **approach** (`moveto`/`follow`) the moment
`engaged` goes false. With the mode at 1, the in-range latch is the only thing sustaining an
attack; when it clears, the squadron leaves the attack states and flies the approach. **That, not
`done`, is what an image bomber does when its attack is spent.**

**Consequence for this host, stated as a defect and not fixed here.**
`src/game_hosts_units.cpp`'s `dive_bomb_transition_inputs` sets
`in.engaged.control_mode_370 = 2` as a **constant**, with the comment "a dive bomber with no
flight lead sits at 2". USN04's dive bombers are not that case: `movieval` is seat 0 of a
three-aircraft squadron, so it *is* the flight lead and `0099B740`'s gate passes for it. Nothing on
the dive-bomb path calls the host's `run_attack_mode_tick_0099b740`, which exists and is bound only
for the torpedo (`unit_.torpedo_attack_mode_370`), and `bsp::pilot_attack_mode_0099b740` is already
reconstructed. With the mode pinned at 2 the `!engaged` edge at `009C8483` can never fire, so a
spent dive bomber never returns to the approach. This is an input of the dive-bomb transition and
belongs to the packet that owns those inputs; it is recorded here, not changed here.

## 6e. The dive-bomb Done state, bound — what is a proof and what is a stand-in

Packet `cc8_done_state`, item 2. `src/game_hosts_units.cpp`'s
`run_dive_bomb_task_arm_009c8790` had no `kDone` dispatch; it now has one, shared with `kPrepare`
because `00D20D28` is installed twice in `009C73A0`.

**Two corrections to section 4 first.** `009C7270` is **16 bytes, body `009C7270`-`009C727F`**, not
13, and it is `__thiscall(state, float dt)` which *forwards* `dt`:

```
009c7270  FLD   float ptr [ESP+4]      ; dt          (4)
009c7274  PUSH  ECX                    ;             (1)
009c7275  FSTP  float ptr [ESP]        ;             (3)
009c7278  CALL  009C1FD0               ;             (5)
009c727d  RET   4                      ;             (3)
```

| what | in this host | kind |
| --- | --- | --- |
| the enter's `009BEDDA` shape 1 | `squadron->formation_shape_3e4 = 1` | proof |
| `009BEDE4`'s `007ED260` | run once per squadron in the placement seam, not per enter | scheduling difference, stated |
| the enter's `state+98h = -1.0f`, `state+9Ch = 0` | not carried; only `009D2720` reads them and the dive-bomb tick never does | justified omission |
| `009BED80`'s `state+6Ch` tuning cache, `+84h`, `+85h`, `+88h`, `+8Ch`, `+90h`, `+94h`, the leader observer at `+2Ch` | not modelled — this host has no follow-state object | **hole**, stated |
| the tick's `009C1FE2` `+26Ch = 2` | written to a field nothing reads | **inert by proof**, see 6c |
| the tick for a flight LEADER | nothing else, which is faithful: `009C1FF1 JZ 009C234E` | proof |
| the tick for a wing MEMBER | the member is **placed** on its station | **hole**: `009BFEE0` + `009BEE30`, ~2900 unread instructions |

**The stand-in's known defect, named before the run.** The placement teleports the member to the
raw station. For this squadron seat 1's station is `local = (-60.0, -25.0, 70.0)` in the leader's frame — so
with the leader level it sits **25 m below him**, and with the leader banked it sits wherever that
frame puts it. The image never commands that unclamped: `009BFEE0`'s common tail
`009C16D2`-`009C1846` (section 6) clamps the commanded Y into a band built around the leader's
altitude `leader+100h` before anything flies to it. The placement has no counterpart to that clamp,
so binding it into `kDone` pins a member under a leader wherever the leader is, including in the
sea. Any change in the member's water contact is therefore an artefact of the stand-in, not
evidence about the image's law; the aircraft to read is the **leader**, whom `009BFD70`'s refusal
keeps the placement away from entirely.

## 6f. The before/after, USN04, same binary apart from the kDone dispatch

`--frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800
--mission-frame-seconds 0.05`, both halves built in this worktree at main `6d9f7064d`.

**The BEFORE is reproducible across commits**, which is what makes section 6b usable as a
reference: this packet's own baseline at `6d9f7064d` matches the `cc8_after_task` baseline at
`f14732dc4` **to the digit** — `movieval` `arm_ticks=2112 done=303 releases=2`, `movieval|.-2`
`2370 aimglide=548 releases=1`, `movieval|.-3` `2299 done=492 releases=2`, the two water contacts
at the same log lines with `|v|=68.72` and `68.45`, and `native renderer final COM release` in
both.

**The result: two water contacts become one, and the one that remains is the LEADER.**

| aircraft | seat | `arm_ticks` | `done` | releases | water contact |
| --- | --- | --- | --- | --- | --- |
| `movieval` | 0, the flight leader | 2112 -> **2112** | 303 -> **303** | 2 -> 2 | yes -> **yes**, `alt=-0.00 \|v\|=68.72`, **mission frame 4284 in both** |
| `movieval\|.-2` | 1 | 2370 -> 2370 | none -> none | 1 -> 1 | no -> no |
| `movieval\|.-3` | 2 | 2299 -> **2370** | 492 -> **563** | 2 -> 2 | **yes -> NO** |

The new census line the binding prints:

```
divebomb movieval     done 009C7240/009C7270: entries=1 ticks=303 placed=0   plan_mode_26c=2 alt 180.6 -> 0.1  hdg=0.8381
divebomb movieval|.-3 done 009C7240/009C7270: entries=1 ticks=563 placed=563 plan_mode_26c=2 alt 205.4 -> 16.9 hdg=-2.6341
```

**The leader is bit-identical, and that is the point.** `placed=0` — `009BFD70`'s refusal keeps the
placement away from seat 0, so the leader ran the bound Done state and *only* the bound Done state:
the enter's shape 1 and the tick's `+26Ch = 2`. Every number it produces is unchanged, down to the
mission frame it hits the water on. Section 6c predicted this statically; the run measures it.
**A faithfully bound Done state does nothing for a flight leader**, because in the image it does
nothing for a flight leader.

**The member's rescue is the stand-in, not the image's law**, and the geometry says so. The
squadron's pairwise distances (`0-2` is the leader to seat 2):

| tick | `0-2` before | `0-2` after |
| --- | --- | --- |
| 3600 | 20.53 | 20.53 |
| 4000 | 1201.35 | **96.52** |
| 4400 | 218.98 | **93.89** |

`0-1` is untouched (466.27, 1418.10 in both) because seat 1 never enters `done`. So from the tick
`|.-3` enters `done` it is held about 94 m from the leader — the station magnitude — and it is
still held there at tick 4400, **more than a hundred frames after the leader is floating in the
sea**. It ends at 16.9 m only because the dead leader's frame happens to rotate the station's
`-25.0` local Y upward. Had that frame been level it would have been placed 25 m under the water.
This is exactly the missing altitude clamp of section 6e, observed. **`movieval|.-3` not ditching
is not evidence about the image**; it is the stand-in pinning an aircraft to a corpse.

Nothing else moved. The three `aim error 009C5C9B` lines are identical in both runs
(`-24.93`/closest `8.06`, `-33.09`/`7.05`, `-17.99`/`9.61`), `releases` and `bombs_spawned` are
unchanged for all three aircraft, and both runs end on `native renderer final COM release`. The
only new log lines are the three first-occurrence host records at 35501-35503, which is exactly why
the leader's water-contact line moved from 40981 to 40984 while its mission frame did not move at
all.

## 6h. `009C8A90`, the dive-bomb break-off predicate, read whole — three defects in its feed

Packet `cc8_done_state`, the goaway-skip investigation. Body `009C8A90`-`009C8B5D`, `__thiscall(task)`,
plain `RET`. Read from the listing, in the image's own order:

```
009c8a96  CALL 0099C230 / TEST AL,AL / JNZ         ; base gate; false -> 009C8A9F return FALSE
009c8aa6  EAX = [ESI+440h] / JZ 009c8b57           ; no latched target -> TRUE
009c8ab4  CMP byte [EAX+5Dh],0 / JNZ 009c8b57      ; target dead   -> TRUE
009c8abe  EAX = [ESI+404h]                         ; the squadron (task+404h = approach+0Ch)
009c8ac4  CMP byte [EAX+369h],0 / JZ 009c8ad6
009c8acd  CMP byte [00E17BF2],0 / JNZ 009c8a9f     ; both set      -> FALSE
009c8ad6  ECX = [ESI+310h] / CALL 009C7910         ; IsAttackState(CURRENT STATE)
009c8ae4  TEST AL,AL / JZ 009c8af1
009c8ae8  CMP byte [ESI+4C9h],0 / JNZ 009c8a9f     ; attack state AND ordnance -> FALSE
009c8af1  ECX = [ESI+3FCh] / CALL 00427EB0         ; EDI = one point
009c8afd  [ESI+3F8h]->vtable[0](&pt)               ; EAX = the other point
009c8b14  three FLD/FSUB pairs into [ESP+8..10h]   ; a 3-D delta, x, y AND z
009c8b30  CALL 0042E740                            ; the GAME TUNING SINGLETON -> EDI
009c8b3b  LEA ECX,[ESP+8] / CALL 0042B2F0          ; |delta|, a 3-D length
009c8b40  FLD [EDI+4C8h] / FMUL [ESI+41Ch]         ; tuning+4C8h * task+41Ch
009c8b4d  FCOMIP / JA 009c8a9f                     ; threshold > distance -> FALSE
009c8b57  MOV AL,1 / RET                           ; otherwise TRUE
```

`src/dive_bomb_task.cpp`'s `dive_bomb_should_break_off_009c8a90` matches the first three arms. The
last two are where it diverges, and `src/game_hosts_units.cpp`'s feed adds a third divergence.

1. **The `IsAttackState` conjunct is missing.** The image returns FALSE on
   `IsAttackState(task+310h) && task+4C9h`; the host's guard is `!has_bomb_ordnance_4c9` alone.
   The image therefore reaches the range test in a case the host does not — not an attack state,
   ordnance still aboard — so this one makes the host **more** reluctant to break off, not less.
   It is still wrong and it is the cheapest of the three to fix, because `009C7910` is already
   reconstructed as `dive_bomb_is_attacking_009c7910` and `task+310h` is `ctx.current`.
2. **`+4C8h` is read off the wrong object in the naming, though the value may be right.** The
   listing loads `[EDI+4C8h]` where `EDI` is `0042E740()`, the **game tuning singleton** — it is
   `Pilot/DiveBomb/SafeDist`, and it is NOT `task+4C8h`, which
   `docs/DIVE_BOMB_TASK.md` line 142 establishes as the approach-relative in-range latch
   (`4C8h - 3F8h = D0h`). Two different objects share the offset. The host's
   `kSafeDistance = 100.0f` is labelled `Pilot/DiveBomb/SafeDist`, so the VALUE is probably right;
   what is unproven here is that 100.0 is what `tuning+4C8h` holds in this installation. **Not
   verified**, and worth one `const_width_sweep` before anyone leans on it.
3. **The distance is 3-D in the image and planar in the host.** `009C8B14`-`009C8B2C` subtracts
   all three components, including `+4h`, and `0042B2F0` takes the length of that three-vector.
   The host feeds `b.distance_to_target = slot.db_planar_bc`. For a dive bomber this is the
   difference that matters: at the moment its last bomb leaves it is still a few hundred metres
   **above** the target, so the image's 3-D distance is large while a planar distance can be small,
   and the two predicates disagree exactly in the window where the goaway edge should be taken.
   Which two points the image measures between — `00427EB0([task+3FCh])` and
   `[task+3F8h]->vtable[0]()` — is **not read here**, so the sign of the disagreement is a
   hypothesis, not a proof.

`b.speed_ratio_41c = 1.0f` is a fourth, already-labelled substitution for `task+41Ch`.

**Why this is the goaway skip.** The transition tests `should_break_off` before the state switch
(`009C8514`), so a TRUE answer takes `aimdive` straight to `done` and the goaway edges at
`009C8664` (ordnance clear) and `009C8677` (pull-out) are never reached. That is precisely
`movieval`'s census: `states[done=303 aimdive=53 flyabove=158 turndown=71 attackrun=1527]`, **no
goaway at all**, and the water 1.5 s later. **Nothing above is bound yet**: per the integrator's
instruction the next step is additive logging of every input of this predicate and of the edge
actually taken at the tick the task leaves `aimdive`, measured before any change.

## 6g. What packet `cc8_done_state` leaves for the next reader

In the order they are worth doing, with the reason each is next.

**Ownership, by the integrator's arbitration of 2026-09-19.** The `control_mode_370` hunk of
`dive_bomb_transition_inputs`, the dive-bomb **transition law**, the **break-off evaluation** and
the **Done dispatch** belong to this stream, not to `cc8-dive-race`, which owns the attackrun,
flyabove, turndown, aimdive and aimglide ticks and their inputs. Items 1 and 2 below are in those
hunks: edit them in your own tree without waiting for the lease, prefer pure insertions, and cite
the arbitration in the commit.

1. **Feed the squadron attack mode to the dive bomb.** Section 6d.

   * **The site.** `src/game_hosts_units.cpp`, `dive_bomb_transition_inputs`, the line
     `in.engaged.control_mode_370 = 2;` (with `in.entry.control_mode_370` copied from it on the
     next line). Drop the constant.
   * **Reuse, do not rewrite.** `bsp::pilot_attack_mode_0099b740` is already reconstructed as a
     pure rule, and `run_attack_mode_tick_0099b740` is already a host wrapper — read the torpedo
     wiring (`unit_.torpedo_attack_mode_370`, `in.unit_is_flight_lead`,
     `in.task_authorises_38h = true`, and the loop that copies the leader's value to every aircraft
     of the flight) and mirror it with a per-squadron dive-bomb field. `task_authorises_38h` is
     **true** for this class: verified at `00D20E50` = `0099B710` = `MOV AL,1 / RET`, section 9.
     Run it once per think **before** the task arm, which is where `009998A0` calls `0099B740`.
   * **Verify from the listing FIRST, both unread here.** (a) Where the in-range latch `task+4C8h`
     (= `approach+D0h`) is **set and CLEARED** — once the mode is 1 it is the only thing sustaining
     `engaged` (`009C83F8`), so its clear condition is what decides when the squadron leaves the
     attack. (b) What `009C8483`'s approach state actually does for a bomber with **no ordnance
     left**: does it fly the squadron's move order, which would be the faithful "fly home", or
     re-attack? Note `in.unit_lacks_follow_target = true` is also hardcoded in the same host
     function, which forces `moveto` over `follow`.
   * **Log before changing.** At the tick the task leaves `aimdive` (and `aimglide`): which
     transition edge was taken, every input of `engaged` (`in_range_latch_4c8`, `control_mode_370`,
     `has_latched_target_440`), and every input of the break-off predicate with **which arm
     returned true**.
   * **Measure**, same-binary USN04 4800 frames: the spent bombers' next state and their altitude
     after it, `plane water contact` for the LEADER `1 -> 0` (the member's is already gone, by the
     stand-in), and `movieval`'s attack numbers unmoved — releases 2, aim error 8.06 m, bombs
     landing within about 25 m.

2. **The three break-off defects of section 6h**, which may close item 1 on their own — the
   integrator's reading is that the two are one problem seen from opposite sides. Smallest first:
   read the two endpoints of the `009C8A90` range test (`00427EB0([task+3FCh])` and
   `[task+3F8h]->vtable[0]()`) to settle whether the host's planar `db_planar_bc` breaks off too
   early or too late; then fix the planar-versus-3-D distance and the missing `IsAttackState`
   conjunct together, since both live in `dive_bomb_should_break_off_009c8a90` and its feed. If the
   goaway edge at `009C8664` comes back, `movieval` stops entering `done` pointed at the sea and
   item 1 may be unnecessary.
2. **The altitude clamp `009C16D2`-`009C1846`.** Section 6e: the placement stand-in has no
   counterpart to it, which is the one place the stand-in can put an aircraft where the image never
   would. The tail is already read whole in section 6; what is missing are `state+88h`'s seed,
   `block+4h`, the `qword` at `00D1F3F8` and the tuning singleton's `+210h`.
3. **`007F23A0` shape 2 at `007F25BD`** (~240 bytes, x87, two `00415550` calls and a reciprocal).
   Until it is read, `007F23A0` answers `produced = false` for shape 2, so the TORPEDO done enter
   cannot be bound the way the dive bomb's now is: `009D254E` overrides the shape to 2 and the
   station would be lost.
4. **`009D29E0`-`009D2CF3`**, the 787-byte disarmed arm the torpedo done tick always takes
   (section 4), still reduced to `kIdle` in `src/torpedo_task_arm.cpp`.
5. `009BFEE0` arm B (~1500 instructions) and `009BEE30` (997, of which only the head and the
   `009BF0B8` block are read here). Nothing smaller is a faithful station-keeping law; section 6
   says why.

Not worth doing: binding `+26Ch = 2` to anything. Section 6c proves the only reader cannot act on
it for the aircraft that writes it.

## 7. Corrections to other documents

**7.1 `approach+0Ch` is a dereference, and the object is the squadron.**
`docs/TORPEDO_AFTER_THE_DROP.md` section 14.4 says "`009F9CE0` sets `approach+0Ch` to `unit+9D4h`,
so the base is **the pilot control block**". The listing dereferences:

```
009f9cf6  mov edx,dword ptr [eax+9D4h]
009f9cfc  mov dword ptr [ecx+0Ch],edx
```

`approach+0Ch = *(unit+9D4h)`, and that object carries `+3CCh`/`+3D0h` (the member count and
array), `+3E4h` (`psFormation`) and `+3E8h` — it is the object `007ED260` takes as `this`.

**The producer settles the identity, and it is the squadron.** Every writer of the `+9D4h` field is
a `PlaneSquadron` method. A scan for the store form (`python tools/bsp.py scan-bytes
'89 ?? d4 09 00 00'`) returns seven sites, and two of them say what is stored:

```
007F4B43  in BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes   (esi = the squadron, ebx = the plane)
007f4b3d  mov eax,[esi+3CCh]              ; the squadron's member count
007f4b43  mov [ebx+9D8h],eax              ; plane+9D8h = its slot in the array
007f4b49  mov [ebx+9D4h],esi              ; plane+9D4h = THE SQUADRON
007f4b4f  mov eax,[esi+3CCh]
007f4b55  mov [esi+eax*4+3D0h],ebx        ; squadron members[count] = the plane

007ED0E6  in BSP_PlaneSquadron_InsertPlaneSorted             (ecx = this = the squadron)
007ed0d7  mov byte ptr [ecx+3ECh],1
007ed0e6  mov [ebp+9D4h],ecx              ; the same pair, the same order
007ed0ec  mov [ebp+9D8h],esi
```

The one routine that appends the plane to `squadron+3D0h` is the routine that writes
`plane+9D4h`, in the same four instructions. So the three adjacent fields are

| field | meaning | evidence |
| --- | --- | --- |
| `plane+9D0h` | the formation index | `009BFDBE` feeds it to `007F23A0`; `007ED260` uses it as `taken[]`'s subscript |
| `plane+9D4h` | **the squadron pointer** | `007F4B49`, `007ED0E6` |
| `plane+9D8h` | the member-array slot | `007F4B43`, and `007ED292` rewrites it per walk |

**Consequence.** `approach+0Ch` is the squadron from both ends: the producer stores the squadron
there, and the consumer `009BFD70` reads `[[approach+0Ch]+3D0h]` and compares it with
`[approach+4]`, the approach's own unit, to refuse to follow itself — a comparison that only makes
sense if `+3D0h` is an array of units. Therefore `+369h`, `+394h` (`Pilot/Torpedo/CruisingAlt`),
`+398h` and `+39Ch` (`BeginAltRange`) are fields of the **squadron's shared cruise profile**, not of
a per-plane control block. Every document that calls that object `ctl` or "the pilot control block"
is wrong about its identity; the values those documents bind are unaffected, because it is the same
pointer either way. `include/bsp/torpedo_goaway_tick.hpp`'s input should be
`squadron_cruising_alt_394`. This also explains
`dive_bomb_cruise_profile_009c8920` step 2, "a following aircraft keeps its own profile": the
follower must not overwrite the shared block.

The
goaway climb at `009D0E7C` reads `+394h` off the same pointer, so section 14.4's **value** is
undisturbed and its binding stands; the **name and the object identity** are wrong.
`dive_bomb_cruise_profile_009c8920` step 2 corroborates it: "a following aircraft keeps its own
profile" is a gate a per-unit block would not need and a squadron-shared cruise block does.

**7.2 `00A26510` is not a planner tick.**
Ghidra has it as `BSP_AiPlanner_SiegeThink`, body `00A26510-00A265E8`, `__thiscall(planner)`,
`RET 0`; it quick-spawns a group tagged `[siege]` (`00D23014`). The host prints
`AiPlanners::planner_tick [00a26510] UNIMPLEMENTED`, and
`docs/HANDOFF_DIVE_BOMB_AIMGLIDE_AND_SPAWN.md` section 3 leans on that line for "Nothing assigns a
follow-up task". **That inference does not follow from this callee** — it is a strategic planner,
not the per-aircraft hand-over. The routine to read for the hand-over is `009998A0
BSP_PilotBot_Update`'s order, whose second call is `0099B740 BSP_BotTask_AbandonIfStale`. Neither
has been read by this packet.

**7.3 The torpedo side already ticks `done`.**
`docs/HANDOFF_TORPEDO_DONE_STATE.md` says the host "has no branch for `kDone`".
`src/torpedo_task_arm.cpp:291-294` has had one since `8b204e966` (2026-09-17): it calls
`follow_base_tick_009c1fd0` and then `torpedo_done_prepare_tick_009d2720`, in the image's order.
What is missing is not the branch but the body of `009D29E0` (section 4).

## 8. Coverage

| Native | Coverage | Kind |
| --- | --- | --- |
| `009D2530` enter | complete, `009D2530-009D256E` | proof |
| `009C7240` enter | complete, `009C7240-009C725B` | proof |
| `009C7270` tick | complete, `009C7270-009C727F` (**16 bytes, corrected in 6e**; section 4's 13 and this table's `-009C727C` were both short) | proof |
| `009BED80` base enter | complete, `009BED80-009BEE24` | proof |
| `00D21320` / `00D20D28` slot maps | complete | proof |
| `009D2DA0` | partial: the two Follow-derived constructs and the whole registry table | proof for those |
| `009BFD70` `state+85h` decision | complete, `009BFE19-009BFEB0` | proof |
| `009BFEE0` arm A | complete, `009BFEFC-009C0021` | proof |
| `009BFEE0` arm B | **not read**, `009C0026-009C16D1` | **hole**, stated |
| `009BEE30` | partial (6c): head `009BEE30-009BEF11` and the formate-lock block `009BF031-009BF0D9` read; the rest of `009BEE30-009BFD67`, ~900 of 997 instructions, **not read** | **hole**, stated |
| `0099D300` gate A | complete, `0099D309-0099D3C2` (6c) | proof |
| `0099D300` normal path | partial: `0099D3C5-0099D46E` and `0099EA84-0099EBA9` only | **hole**, stated |
| `0099B740` | complete, `0099B740-0099B77A` (6d) | proof |
| `007ED3F0` / `007ED430` | complete, `007ED3F0-007ED3FA` / `007ED430-007ED442` (6d) | proof |
| `009BFEE0` common tail | complete, `009C16D2-009C1846` | proof |
| `009D2720` disarmed arm | **not read past its head**, `009D29E0-009D2CF3` | **hole**, stated |
| `007F23A0` shape 2 | **not read**, `007F25BD` | **hole**, stated |

## 8a. Handoff for a cold reader

Nothing in this packet is bound. Everything below is scoped from the readings above.

**The squadron shape, USN04.** Thirteen squadrons print formation lines and every plane squadron in
the log is `wing=3` with `seat1 index=1` — indices 0, 1, 2, one leader and two members. `D3A Val
#1.1` is identical in shape to `movieval`. **Caveat, stated:** the printer names the squadron, not
the seat, so "plain `movieval` is seat 0" is inferred from the naming convention
(`movieval`, `movieval|.-2`, `movieval|.-3`) and not read off a seat-labelled line. A successor that
needs it proved should print the seat with the unit name rather than trust this.

**Cut A — the dive-bomb Done state.** `run_dive_bomb_task_arm_009c8790`
(`src/game_hosts_units.cpp:5186`) has no per-state tick dispatch for `kDone`; add one mirroring
`src/torpedo_task_arm.cpp:291-294`. Bind the enter `009C7240` (27 bytes, read whole here: two field
writes then a tail jump into `009BED80`, which assigns the formation indices and leaves
`psFormation` on **shape 1**, already proved in `src/plane_formation.cpp`) and the tick `009C7270`
(13 bytes: `009C1FD0(state, dt)` and nothing else). Measure on USN04 at 4800 mission frames against
section 6b: **two** water contacts today, not three, so the target is 2 -> 0.

*The leader caveat.* One of the two ditching aircraft is the flight leader, and for a leader
`009BFD70` returns false at `009BFEB1`, so `009C1FD0` ends at `009C1FF1` having written only
`[[state+4]+18h]+26Ch = 2`. Cut A can only remove the leader's ditch if that byte is what keeps a
spent aircraft flying. Section 6a proves the byte is the pilot planner's mode selector and that
`0099D300 BSP_PilotBot_PlanControls` is its only reader, but **the mode-2 body past `0099D36F` is
not read**. Finish that first; it is cheap and it decides the cut.

*If the leader ditches anyway*, the next reads in order are `009D2720`'s disarmed arm
`009D29E0-009D2CF3` (787 bytes — the one branch a Done leader does run, and section 4 shows it
reading the leader and a class-row distance), then the task consumer `009998A0 BSP_PilotBot_Update`
and `0099B740 BSP_BotTask_AbandonIfStale`.

*A trap to check before reading any after-run as proof.* `kPlaneFormationPlacementEnabled` is `true`
(`src/game_hosts_units.cpp:1722`) and `follow_base_tick_009c1fd0` calls
`place_wing_member_on_station_007f23a0(slot_, false)` — `once = false`, so a member reached through
that seam is pinned onto its station every tick. That would mask whatever a newly-bound tick
commands for the members (not for the leader, who never reaches it).

**Cut B — `007F23A0` shape 2**, body at `007F25BD` (at most to `007F26AC`, ~240 bytes, x87 with two
`00415550 MaxFloatByRef` calls and a reciprocal). This is what the **torpedo** Done enter selects
(`009D254E` writes `psFormation = 2`), so until it is read a spent torpedo flight has no station and
binding the torpedo Done enter faithfully would make things worse, not better.

**Cut C — `009D29E0-009D2CF3`**, the 787-byte disarmed arm, today `kIdle` in
`src/torpedo_task_arm.cpp:226-229`. Labelled `partial_projection` in section 8.

**The follow law's real size.** `009BFD70` (reconstructed) + `009BFEE0` arm B (~1500 instructions)
+ `009BEE30` (3895 bytes) is roughly **2900 unread instructions**. Any packet proposing to
"reconstruct the follow law" should be sized against that, not against the 1795 of `009BFEE0`.

**Artefacts left in `J:\PROG\battlestations-pacific-decompile-cc8-after-task`** (read-only for the
successor, who starts in a fresh tree): `local/bfee0_listing.txt` (`009BFEE0` decoded whole),
`local/c1fd0_listing.txt` (`009C1FD0` decoded whole), `local/bfee0_map.py` (the mapper — takes
`<start_hex> <end_hex> <out>`, works on any body, detects stores by mnemonic because Capstone
reports `fstp [mem]` as a read), and `local/aftertask_before_usn04.log`, the BEFORE run at main
`f14732dc4`.

## 9. Uncertainty

* `block+18h` as `GoodPositionDist` and `block+14h` as `GoodPositionDir` are read from their use
  (a distance compare and a heading dot-product compare against the cached `Pilot/Follow` block),
  not from the key loader. The authored key names come from `docs/PLANE_FORMATION.md`; the mapping
  of key to offset inside the block is **not** proven here.
* `state+44h..4Ch` is called the commanded point because arm A writes the station plus a lead into
  it and the common tail clamps its Y into an altitude band around the leader. What turns it into a
  pilot command is `009C1FD0` after the call, which this packet has read only at its head; that last
  link is a hypothesis.
* `[edi+0ECh]` and `[leader+0CCh+20h/+24h/+28h]` are read as leader frame rows on the strength of
  `007F23A0`'s use of the same block. The row-to-axis mapping is not re-derived here.
* Shape 2's geometry is unread, so nothing is claimed about what formation a spent torpedo flight
  actually takes — only that it is a different shape from the one the dive bombers take.
* `unit[9C2h + idx*8]`, and therefore `unit+520h`, is read here only as a byte that
  `FUN_007CDC70` publishes per step and that gate A, `009BEE30` and `0099C270` all require.
  `docs/PILOT_BOT_TICK_GATES.md`'s provisional reading — "under human control for the local slot" —
  fits `0099EB73`'s `SETZ` against one named unit, but **the byte's meaning is not recovered**, and
  nothing in section 6c depends on it: the leader's case is settled by `009BEE30` being
  unreachable, not by what the byte means.
* `0099B740`'s abandon predicate is the task vtable's `+38h`, and for the **dive-bomb class this is
  verified directly here**, not taken from `docs/BOT_TASKS.md` line 185: `scan-bytes '20 89 9c 00'`
  finds `009C8920` (the dive bomb's `+54h` cruise profile) at exactly one site, `00D20E6C`, so the
  vtable base is `00D20E18`, and the dword at `00D20E18 + 38h = 00D20E50` is `10 b7 99 00` =
  **`0099B710`**, which is `MOV AL,1 / RET`. The claim that *every* pilot-task vtable shares it is
  still taken from that doc row and `docs/PILOT_TASK_HEADING_ARM.md` line 71; nothing in 6d depends
  on the other nine classes.

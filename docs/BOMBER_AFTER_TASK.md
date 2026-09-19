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
the controls. **Coverage: the mode-2 body past `009D36F` is not read**, so this document does not
claim what the mode commands, only that the value is consumed there and nowhere else.

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
| `009C7270` tick | complete, `009C7270-009C727C` | proof |
| `009BED80` base enter | complete, `009BED80-009BEE24` | proof |
| `00D21320` / `00D20D28` slot maps | complete | proof |
| `009D2DA0` | partial: the two Follow-derived constructs and the whole registry table | proof for those |
| `009BFD70` `state+85h` decision | complete, `009BFE19-009BFEB0` | proof |
| `009BFEE0` arm A | complete, `009BFEFC-009C0021` | proof |
| `009BFEE0` arm B | **not read**, `009C0026-009C16D1` | **hole**, stated |
| `009BEE30` | **not read**, `009BEE30-009BFD67`, 3895 bytes | **hole**, stated |
| `009BFEE0` common tail | complete, `009C16D2-009C1846` | proof |
| `009D2720` disarmed arm | **not read past its head**, `009D29E0-009D2CF3` | **hole**, stated |
| `007F23A0` shape 2 | **not read**, `007F25BD` | **hole**, stated |

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

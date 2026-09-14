# Entity virtual slot `0FCh`: the sub-entity list

Addresses: `00432480`, `004323D0`, `004322D0`, `0063BCD0`, `0042EE00`, `0042EDD0`, `00BF6713`,
`006D4DD0`, `006D1C20`, `006D1C68`, `006D5220`, `006D54BF`, `006D54F1`, `007F44E0`, `007F2C60`,
`007F2CAD`, `007F4580`, `007F4B4F`, `007F4B55`, `007F4B60`, `00928630`, `00928662`, `006E7B00`,
`006E7B67`, `00745940`, `0074597D`, `0077EED0`, `0077EF5B`, `004F11C0`, `004F1272`, `00A31730`,
`00A3176B`, `00521EA0`, `0071EBF0`, `00864FE0` (read only, leased elsewhere), `008651B4`,
`008651BD`, `008651F5`, `008651F8`, `00865516`, `0086551B`, `00865521`, `00865528`, `00865531`,
`0086553D`, `0086554A`, `00865570`, `0086557A`, `00865584`, `008655F9`, `00865679`, `0086567E`,
`00865680`, `00865689`, `008656D1`, `008656DB`, `008656E5`, `00865763`.
Vtables: `00CF8C08`, `00CF8D04`, `00D087C0`, `00D088BC`, `00D192E0`, `00D192C0`, `00CF9DF0`,
`00CF9D78`, `00CF9D90`, `00CFF3F8`, `00CFF3AC`, `00D03E80`, `00D03E54`, `00D0877C`, `00CEA090`,
`00CEA05C`, `00D231A0`, `00D23168`.

Leased and named by this packet: `00432480`, `006D4DD0`, `007F44E0`. Everything else here was
read, not annotated. `00864FE0` belongs to the gunnery-pass packet; it is a contract here.

## Headline

The slot is not empty and it is not a list of parts. **For 92 of the 94 entity classes in this
binary - every ship, every gun platform, every projectile - slot `0FCh` appends `this`.** The
sub-entity list of a ship is the ship itself. Only two classes override it: `MAirfield` appends
its intact hangar buildings, and the plane squadron appends its live planes.

This contradicts `docs/GAME_EXECUTABLE.md` in three places; see **Corrections** below.

## 1. The call site, and what it hands the callee

`00864FE0 BSP_UnitGunneryAi_Tick` reaches the slot twice. Both arms are identical in shape:

```
00865508: ADD ESI,0xbc              ; ESI = this+0BCh, the scratch pointer vector
00865516: CALL 0x0063bcd0           ; clear it
0086551b: MOV ECX,[ESP + 0x38]      ; the target entity
0086551f: MOV EDX,[ECX]             ; its primary vtable
00865521: MOV EAX,[EDX + 0xfc]      ; slot 0FCh
00865527: PUSH ESI                  ; the vector
00865528: CALL EAX                  ; __thiscall(target, vector&)
```

and at `0086566B`-`00865689` the same five instructions for the second target. The callee cleans
its own argument (`RET 4` in all three implementations), so the call is `__thiscall` with one
stack argument: **the caller's vector, not an out-parameter the callee allocates**.

`0063BCD0` is the clear: it erases `[container+4h] .. [container+8h]` through `0063AD10`. That
settles the vector's shape without guessing - `+4h` is the first element, `+8h` the last, and
`004323D0 BSP_PointerVector_PushBack` adds `+0Ch` as the capacity end and `SAR ECX,2` as the
element size. Elements are 4 bytes: pointers.

After the call both arms walk the vector through `0042EE00` (begin) and `0042EDD0` (end), which
return `{container*, element*}` iterator pairs into stack slots; `00BF6713` is the checked-iterator
trap the walk calls when the container pointers disagree or the cursor passes `[container+8h]`.
Each element is dereferenced (`MOV ESI,[EDI]`, `ADD EDI,4`), tested by `00864D90`, pose-refreshed
through `00414DB0` when the byte at `entity+0C8h` is zero, and its world position read from
`entity+0FCh`/`+100h`/`+104h`.

> `+0FCh` means two unrelated things twenty instructions apart: the **vtable slot** at `00865521`,
> and a **world-position float** on the enumerated entity at `008655AC`. They are not related.

### The two arms differ only in which target they enumerate

| Arm | Call site | Target | How the target is produced |
| --- | --- | --- | --- |
| first | `00865528` | `[ESP+38h]`, the command target | `0071EBF0` then `00521EA0`, reached through `[EDI]->vtable[140h]->vtable[114h]` at `00865456`-`008654A3`; skipped when it equals the fire target or is null (`008654AC`, `008654B6`) |
| second | `00865689` | `[ESP+18h]`, the fire target | `[this+5Ch]->vtable[4h]` at `00865442`-`0086544A` |

`00521EA0` is an entity-handle resolve: a handle `{byte valid; ushort index; void* cached}` looked
up in the 16-byte-record tables at `DAT_00F89A54` / `DAT_00F89AA8`, object pointer at `+0Ch`. So
the target is an ordinary entity out of the global registry, which is why the slot is virtual.

Both arms append into the **same** candidate table at `[ESP + EBX*8 + 1D0h]` with `EBX` carried
across (`00865605`, `0086576F`), so the second arm continues the first arm's numbering.

## 2. Finding the implementations

A vtable's identity is a hypothesis unless something ties it down, so the census was built from
constructors, not from the vtable data:

1. Scan `.text` for `mov dword ptr [reg], imm32` with `mod=00` and the immediate inside `.rdata`.
   That is the store of a vtable **at object+0**, which is the pointer `MOV EDX,[ECX]` loads.
   Secondary vtables (`[esi+10h]`, `[esi+24h]`, ...) use `mod=01`/`mod=10` and are excluded.
2. Keep the immediates that begin a run of more than 63 consecutive `.text` pointers - a vtable
   must have a slot 63 to have a slot `0FCh`. 306 in the image.
3. Read `vtable[63]` for each.
4. Tie each vtable to a class by the function that stores it, and separate the entity hierarchy
   by whether that function transitively calls `00925CE0` (`BSP_SceneNode_Construct`) or
   `00928630 BSP_GameEntity_Construct`.

Step 4 is the class attribution and it is only as good as the constructor names already in the
ledger; what it does **not** depend on is any RTTI, because this image has none reachable - the
dword before a vtable is not a complete-object locator, and an RTTI-anchored scan finds zero
vtables with 64 or more slots.

`00928630` is the anchor that makes the offset-0 rule concrete:

```
00928662: MOV dword ptr [ESI],0xd192e0        ; primary, the one [ECX] loads
00928668: MOV dword ptr [ESI + 0x10],0xd192c8
0092866f: MOV dword ptr [ESI + 0x24],0xd192c0
00928676: MOV dword ptr [ESI + 0x170],0xd192bc
```

`00D192E0 + 0FCh` holds `00432480`. `00D192C0` is a two-entry secondary; reading "slot 63" off it
lands 20h inside the primary and produces a fictitious `0043F0B0` (`RET 4`). Seven such phantoms
appeared in the first pass and every one was disproved by reading its constructor:

| Phantom | Real primary | Constructor | Evidence |
| --- | --- | --- | --- |
| `00CF9D78`, `00CF9D90` | `00CF9DF0` | `006E7B00` | `006E7B67 MOV [ESI],0xcf9df0`; the other two go to `[EDI]`=`ESI+170h` and `[EBX]`=`ESI+244h` |
| `00CFF3AC` | `00CFF3F8` | `00745940` | `0074597D MOV [ESI],0xcff3f8`; `007459B9 MOV [EDI],0xcff3ac`, `EDI = ESI+72Ch` |
| `00D03E54` | `00D03E80` | `0077EED0` | `0077EF5B MOV [ESI],0xd03e80`; `0077EF48 MOV [EDI],0xd03e54`, `EDI = ESI+1E4h` |
| `00CEA05C` | `00CEA090` | `004F11C0` | `004F1272 MOV [ESI],0xcea090`; `004F1290 MOV [EBX],0xcea05c`, `EBX = ESI+1E4h` |
| `00D23168` | `00D231A0` | `00A31730` | `00A3176B MOV [ESI],0xd231a0`; `00A3177F MOV [EDI],0xd23168` |
| `00D0877C` | `00D087C0` | `007F2C60` | `007F2CAD MOV [ESI],0xd087c0`; `007F2CD5 MOV [EDI],0xd0877c`, `EDI = ESI+310h` |

The sixth row is the one that mattered: it turned `00D0877C` into a phantom **and** confirmed
`00D087C0` as a real primary with a real override.

### The census

| Slot `0FCh` | Classes | What it appends |
| --- | --- | --- |
| `00432480` | 92 | `this` |
| `006D4DD0` | 1, primary `00CF8C08` (`006D1C68 MOV [ESI],0xcf8c08` in `BSP_AirField_Construct`) | intact hangar buildings |
| `007F44E0` | 1, primary `00D087C0` | live squadron planes |

Two independent completeness checks:

* The dword `00432480` occurs **92 times in `.rdata` and every occurrence is at exactly `+0FCh`
  from a primary vtable start**. It occurs at no other vtable offset, and Ghidra's xrefs for it are
  92 `[DATA]` references and no code reference.
* The only xref to `006D4DD0` is `00CF8D04` = `00CF8C08 + 0FCh`; the only xref to `007F44E0` is
  `00D088BC` = `00D087C0 + 0FCh`. Both overrides are reachable only through the slot.

A vtable stored through a register (`mov eax, imm32; mov [ecx], eax`) would escape step 1. Scanning
for that form finds 27 sites in the image and none of their immediates begins a run longer than 3
`.text` pointers, so none is a vtable. The 92+2 census is therefore the whole set.

Classes on the base implementation include, by their named constructors: `BSP_UnitInstance`,
`BSP_PlaneUnitInstance`, `BSP_SubmarineUnit`, `BSP_UnitVehicleBase`, `BSP_UnitTickableEntity`,
`BSP_UnitGameObject`, `BSP_UnitOwnerEntity`, `BSP_GameEntity`, `BSP_SceneNode`, `BSP_LandVehicle`,
`BSP_LandConvoy`, `BSP_LandFort`, `BSP_TurningGunBase`, `BSP_SingleTurningGun`,
`BSP_RapidTurningGun`, `BSP_RapidFixedSlaveGun`, `BSP_MultipleBombPlatform`,
`BSP_ProjectileTickableEntity`, `BSP_DepthChargeProjectile`, `BSP_FlakProjectile`,
`BSP_RocketProjectile`, `BSP_ScriptEntity`.

## 3. The three implementations

### `00432480` - the base, `out.push_back(this)`

Ghidra has no function here; the body is `00432480`-`00432494` inclusive.

```
00432480: push ecx              ; a 4-byte local holding `this`
00432481: lea eax, [esp]        ; &local
00432484: mov [esp], ecx        ; local = this
00432487: mov ecx, [esp + 8]    ; ecx = arg0, the vector  (+0 local, +4 ret, +8 arg)
0043248b: push eax
0043248c: call 0x4323d0         ; BSP_PointerVector_PushBack(vector, &this)
00432491: pop ecx
00432492: ret 4
```

One element, no null test, no filter. `004323D0` copies `*arg` - four bytes - to `[vector+8h]`,
growing through `004322D0` when `[vector+8h] == [vector+0Ch]`.

ABI: `__thiscall(entity* this, PointerVector* out)`, `RET 4`, no return value (`EAX` is left as
`004323D0` returned it and no caller reads it).

### `006D4DD0` - `MAirfield`, the intact hangars

Body `006D4DD0`-`006D4E36`, `RET 4` at `006D4E34`.

```
006d4dd4: MOV ESI,[EDI + 0x830]         ; base
006d4dda: MOV EAX,[EDI + 0x834]         ; count
006d4de0: LEA EAX,[EAX + EAX*2]         ; count*3
006d4de3: MOV ECX,ESI
006d4de5: LEA EDX,[ECX + EAX*4]         ; base + count*0Ch     -> stride 0Ch
006d4de8: CMP ESI,EDX / JZ end
006d4df1: MOV EAX,[ESI]                 ; record[0], the hangar building
006d4df3: TEST EAX,EAX / JZ skip
006d4df7: MOVSS XMM0,[EAX + 0x370]
006d4dff: COMISS XMM0,[0x00d7a218]      ; = 0.0f
006d4e06: JBE skip                      ; append only when strictly above zero
006d4e08: MOV [ESP + 0x10],EAX
006d4e0c: LEA EAX,[ESP + 0x10] / PUSH EAX
006d4e11: MOV ECX,EBX                   ; the caller's vector
006d4e13: CALL 0x004323d0
006d4e27: ADD ESI,0xc                   ; and both base and count are re-read each pass
```

**Producer.** `006D5220 BSP_AirField_ReadHangarAndMarkerProperties` builds the vector: `006D541E`
stores the base at `+830h`, `006D545F` the count at `+834h`, and the fill loop writes
`MOV [EDI],ECX` at `006D54BF` from the `entityID` property (string `00CF8E70`) and
`MOV [EDI+4],EAX` at `006D54F1` from `entryPathID` (`00CF8E64`) through `007AC9D0`; `exitPathID`
(`00CF8E58`) follows into `+8h`. That is the same 12-byte record `docs/AIRFIELD_TAXI.md` already
documents as `AirfieldHangarRecord`, and its `object+370h` gate is the same one `006D2640
BSP_AirField_PickHangarExitPath` applies - that doc calls it the hangar-failure rule. So the
airfield's sub-entities are its **standing** hangars; a destroyed hangar drops out.

`006D4DD0` re-reads `[EDI+830h]` and `[EDI+834h]` on every iteration, so it tolerates the vector
being reallocated underneath it - but it computes the end from the freshly read base each time,
which means a reallocation mid-walk restarts the bound, not the cursor. Nothing in the gunnery
path reallocates it, so this is a note, not a behaviour.

### `007F44E0` - the plane squadron, the live planes

Body `007F44E0`-`007F4570`, `RET 4` at `007F456E`.

```
007f44e6: CMP [ECX + 0x3cc],EBP(0) / JLE end     ; the live plane count
007f44f9: LEA EBX,[ECX + 0x3d0]                  ; the inline array
007f4500: MOV EAX,[ESI + 0x4]                    ; vector first
007f4505: MOV EDI,[EBX]                          ; the plane pointer
      ... the inlined push_back fast path: size = ([+8h]-[+4h])>>2,
          capacity = ([+0Ch]-[+4h])>>2, and when size < capacity
007f452c: MOV [EAX],EDI / ADD EAX,4 / MOV [ESI+8],EAX
      ... otherwise 007F4550 CALL 0x004322d0, the growing insert
007f455c: ADD EBX,0x4
007f455f: CMP EBP,[EDX + 0x3cc]                  ; the count is re-read every pass
```

No null test and no filter: every occupied slot is appended.

**Producer.** `007F4580 BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes`:

```
007f4b4f: MOV EAX,[ESI + 0x3cc]
007f4b55: MOV [ESI + EAX*4 + 0x3d0],EBX      ; array[count] = plane
007f4b60: ADD [ESI + 0x3cc],0x1              ; count += 1
```

with the plane's back-links written immediately before (`plane+9D8h` the spawn index,
`plane+9D4h` the squadron). `docs/PLANE_SQUADRON.md` already records the same pair - `+3CCh` the
live plane count, `+3D0h..+3E0h` **five** inline pointers, decremented and compacted by
`007F39ED`/`007F3970` - so the array is dense and the walk needs no null test.

## 4. Category 7 and step 8.5 - confirmed from the listing

`docs/GAME_EXECUTABLE.md` says "Step 8.5 never sweeps the recon list for category 7 - the native
code excludes it". That is right, and this is the instruction:

```
008651b4: CMP dword ptr [ESP + 0x14],0x7     ; the category index
008651b9: JNZ 0x008651c0
008651bb: MOV EAX,ECX
008651bd: MOV BL,byte ptr [EAX + 0x7c]       ; category 7 takes its enable from this+7Ch
...
008651db: TEST BL,BL
008651e1: JZ 0x00865442                      ; -> straight to the fire/command target block
008651e7: CMP byte ptr [EBX + 0x59],0x0
008651eb: JNZ 0x00865442
008651f1: MOV ESI,dword ptr [ESP + 0x14]
008651f5: CMP ESI,0x7
008651f8: JZ 0x00865442                      ; category 7 skips the sweep unconditionally
008651fe: MOV ECX,dword ptr [EDI + 0x54]     ; the sweep proper: 008053C0 then [recon+DE8h]
```

`008651F8` is unconditional for category 7 regardless of `this+7Ch`, because both the `this+7Ch`
branch and the explicit compare land on the same `00865442`. So a torpedo mount is fed only by
steps 8.6/8.7 - the two slot-`0FCh` arms above.

## 5. Corrections

| Document | Was | Is | Evidence |
| --- | --- | --- | --- |
| `docs/GAME_EXECUTABLE.md` (the gunnery-stand-in section) | "a ship answers `vtable[0FCh]` with no sub-entity" | a ship answers with **itself**; its class sits on `00432480`, which appends `this` | the eight-instruction body at `00432480`; `00CFC3D0 + 0FCh` for `BSP_UnitInstance_Construct`, `00D05F20 + 0FCh` for `BSP_PlaneUnitInstance_Construct`, `00D0BF80 + 0FCh` for `BSP_SubmarineUnit_Construct`, all `00432480` |
| `docs/GAME_EXECUTABLE.md` step table, row 8.7 | "nothing, because `vtable[0FCh]` answers with no sub-entity" | one element, the target itself, for every unit class in the mission | as above |
| `docs/GAME_EXECUTABLE.md` category-7 note | "step 8.7 appends sub-entities and never the target itself. A ship with an empty sub-entity list therefore contributes nothing at all." | step 8.7 appends **the target itself** for 92 of 94 classes; the empty list is an artefact of the reconstruction's host, not of the native code | `00432480`; `src/game_hosts_gunnery.cpp`'s `sub_entity` returns `nullptr` |

The category-7 *conclusion* in that doc - zero torpedo assignments in the reconstruction - is
still a correct description of what `bsp_game.exe` does today. What is wrong is the reason given.
The native code would hand a torpedo mount the target, exactly once, per arm.

I did not edit `docs/GAME_EXECUTABLE.md`; it belongs to another packet.

## 6. Proven vs. assumed

Proven from the listing or from the image bytes:

* The slot is `0FCh`, `__thiscall`, one stack argument, `RET 4`, and the argument is the caller's
  vector at `this+0BCh`. Both call sites read.
* The vector layout `+4h` first, `+8h` last, `+0Ch` capacity end, 4-byte elements.
* `00432480` appends `this`, unconditionally, once.
* `006D4DD0` appends `record[0]` of each `0Ch`-stride record at `airfield+830h`/`+834h` whose
  object is non-null and whose `+370h` is strictly above `0.0f`.
* `007F44E0` appends all `squadron+3CCh` pointers from `squadron+3D0h`, unfiltered.
* The census: 92 classes on `00432480`, one on each override, no others in the image.
* Category 7 skips the step 8.5 sweep at `008651F8`.
* The record layouts, from their producers `006D5220` and `007F4580`.

Assumed, or read from another document rather than from the listing here:

* The class names attached to the vtables are the ledger's constructor names. `00CF8C08` is
  `MAirfield`'s because `BSP_AirField_Construct` stores it at offset 0; `00D087C0` is the plane
  squadron's on the same evidence. The other 92 attributions are only as good as those names.
* `00864D90` is described as a visibility test on the strength of the packet brief and
  `include/bsp/unit_gunnery_pass.hpp`; **its body was not read here**. Likewise `00414DB0`.
* What `entity+370h` *is* - `docs/AIRFIELD_TAXI.md` reads it as a hangar condition. Only the
  comparison against `0.0f` is proven here.
* Whether the game ever constructs an `MAirfield` or a squadron as a *gunnery target*. The two
  overrides exist; that they are reached through `00865528`/`00865689` at run time is not shown.

Coverage: `00432480` complete; `006D4DD0` complete; `007F44E0` complete except the growing-insert
callee `004322D0`, which was not read (`007F4550` is the only call to it here). `00864FE0` is
partial by design - only `008651B4`-`008651FE`, `008654A8`-`00865609` and `00865609`-`00865773`
were read.

Not run-time verified. `bsp_game.exe` cannot exercise this path today because its host returns an
empty list, which is the gap this packet exists to close; the native executable was not run.

## 7. Follow-up packets

1. **Wire the host.** `src/game_hosts_gunnery.cpp`'s `sub_entities_slot0fc` should return 1 and
   `sub_entity(0)` should return the target for every unit class the reconstruction builds, which
   is the `00432480` arm. That belongs to the gunnery-host packet, not here; this packet must not
   edit that file. The expected effect is that category 7's 71 torpedo guns stop getting zero
   assignments.
2. **Read `00864D90`** and name it from its body. Every slot-`0FCh` element passes through it, so
   the sub-entity list alone does not decide what a torpedo mount is handed.
3. **Read `004322D0`**, the vector's growing insert, and reconcile it with `004323D0`.
4. **`entity+370h`.** Find its writer and settle what the hangar condition is.
5. **The 92-class census as a ledger artefact.** `local/primary_vtables.txt` is the offset-0
   vtable table this packet built; it is reusable for any other slot and is worth promoting out of
   `local/`.

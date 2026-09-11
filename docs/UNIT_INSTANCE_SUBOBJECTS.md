# The sub-objects the vehicle base constructor builds inside a unit instance

Addresses: 0081ED40 00809270 00809650 0093BCC0 00815600 00810020 0074E7B0 00812D40 0081EB90
0081EC00 00818100 00822460 0080FAE0 0043F620 00440A30 00818EA0 00745940 00424C40 00BD1860
0041CC80 00D09004 00D09480 00D09624 00D00514

Packet `cc2_unit_subobjects`, worktree `agent/cc2-unit-subobjects`, Ghidra read-only. Every
descriptive name below is a hypothesis, not a recovered symbol.

`docs/UNIT_INSTANCE_LAYOUT.md` transcribed every scalar field write of `0081ED40`
(`BSP_UnitVehicleBase_Construct`, body `0081ED40..0081F354`) but recorded the sub-object
constructors only as call sites, and its coverage row says so. This document reads those
constructors' bodies and fixes the interior layout of each sub-object.

## Summary

| Offset in the unit | Size | Built by | What the evidence establishes |
| --- | --- | --- | --- |
| `+72Ch` | `0Ch` | `00809270` at `0081ED7B` | Base with one owned virtual-release slot at `+8h`; its vptr is replaced later by the most-derived class |
| `+838h` | — | `00812D40` at `0081EE95` | The order ring. Contract: `docs/UNIT_STATE_MESSAGE.md` |
| `+A00h` | `10h` | `00BF7CD1` at `0081EF24` | Four 4-byte elements zeroed by element constructor `0043F620` |
| `+A20h` | `54h` | `0093BCC0` at `0081EF47` | Unit back-pointer, a 20-element dword array, two floats copied from the settings singleton |
| `+A98h`, `+AECh` | `54h` each | inline loop `0081EF70..0081EFB1` | Two identical records, each with a 2-element inner array built by `0043F620`'s sibling `0080FAE0` and a unit back-pointer at record `+50h` |
| `+B44h` | `10h` | `00BF7CD1` at `0081EFCE` | Four 4-byte elements, same element constructor |
| `+B54h` | `14h` | `00BF7CD1` at `0081EFED` | Five 4-byte elements, same element constructor |
| `+BD0h` | `3DCh` | `00815600` at `0081F03D` | Owned critical section plus a 40-slot ring of five floats with a write cursor; re-seeded from the unit's world position |
| `+10A0h` | `34h` | inlined at `0081F102..0081F154` | Two owned containers and two float constants; the standalone copy of the same constructor is `0081EB90` |
| `+10D4h` | `40h`+ | `0074E7B0` at `0081F15F` | The leak manager. Use is a contract: `docs/UNIT_CONTROLLER_UPDATE.md` |

`EBX` is zero across the whole of `0081ED40`: the only write to it in the body is
`0081ED60 XOR EBX,EBX`, established by filtering the full listing for `EBX`. Every
`MOV [ESI+off],EBX` below is therefore a zero store, and `LEA EBP,[EBX+1]` at `0081EF69`
makes `EBP` 1, so the loop that follows runs twice.

Float constants used below, read from the image:

| Address | Value |
| --- | --- |
| `00D7A24C` | `1.0f` |
| `00D7A260` | `-1.0f` |
| `00CE3804` | `1000.0f` |
| `00CE386C` | `200.0f` |
| `00CEB4B0` | `60.0f` |

## `+72Ch`: a base with one owned virtual-release slot

`00809270`, `__thiscall(self)`, plain `RET`, body `00809270..00809280` (11 bytes, complete):

```
00809270  MOV EAX,ECX
00809272  XOR ECX,ECX
00809274  MOV dword ptr [EAX],0xd09004
0080927a  MOV dword ptr [EAX + 0x4],ECX
0080927d  MOV dword ptr [EAX + 0x8],ECX
00809280  RET
```

| Offset in the sub-object | Unit offset | Type | Initial value |
| --- | --- | --- | --- |
| `+0h` | `+72Ch` | vptr | `00D09004` |
| `+4h` | `+730h` | dword | 0 |
| `+8h` | `+734h` | owned pointer | 0 |

The receiver is `unit+72Ch`: `0081ED6F LEA EDI,[ESI+72Ch]` then `0081ED75 MOV ECX,EDI`.
`docs/UNIT_INSTANCE_LAYOUT.md` records that `006FE4A9` later stores `00CFC384` over the same
dword, so `00D09004` is this base's own vtable and `00CFC384` is the unit's override of it.

`00D09004` is a one-slot vtable: the dword at `00D09004` is `00809B80`
(`CG_scalar_deleting_dtor_00809b80`) and the next bytes, at `00D09008`, are the string
`"RepairZoneArea"`, so the table cannot be longer.

The meaning of `+8h` comes from the sibling routine `00809650`, which writes the same vtable and
then releases the slot:

```
00809653  MOV dword ptr [ESI],0xd09004
00809659  MOV ECX,dword ptr [ESI + 0x8]
0080965c  TEST ECX,ECX
0080965e  JZ 0x0080966f
00809660  MOV EAX,dword ptr [ECX]     ; the pointee's vptr
00809662  MOV EDX,dword ptr [EAX]     ; slot 0
00809664  PUSH 0x1                    ; the scalar deleting flag
00809666  CALL EDX
00809668  MOV dword ptr [ESI + 0x8],0x0
```

So `+8h` holds an optional pointer to a polymorphic object that this base owns and destroys
through slot 0 of its vtable with the deleting flag set. `+4h` has no established reader.

`00745940`, a second unit-base constructor in the segment whose keyword set contains
`landvehicle` and `landfort`, builds the same sub-object at the same unit offset
(`0074596C LEA EDI,[ESI+72Ch]`, `00745972 MOV ECX,EDI`, `00745978 CALL 00809270`), so `+72Ch`
belongs to a base shared by at least two unit families, not to the vehicle base alone.

## `+838h`: the order ring (contract)

`00812D40` `BSP_UnitOrderRing_Construct`, receiver `unit+838h` from
`0081EDEF LEA ECX,[ESI+838h]`. Already reconstructed; see `docs/UNIT_STATE_MESSAGE.md` and
`src/unit_state_message.cpp`. Not re-derived here.

## `+A00h`, `+B44h`, `+B54h`: three zero-filled element arrays

All three go through the CRT helper `00BF7CD1` `` `eh_vector_constructor_iterator' `` with the
same element constructor `0043F620` and destructor `00440A30`. The arguments are pushed in the
CRT order `(base, element size, count, constructor, destructor)`:

| Site | Base | Element size | Count | Span |
| --- | --- | --- | --- | --- |
| `0081EF24` | `ESI+A00h` | 4 | 4 | `+A00h..+A0Fh` |
| `0081EFCE` | `ESI+B44h` | 4 | 4 | `+B44h..+B53h` |
| `0081EFED` | `ESI+B54h` | 4 | 5 | `+B54h..+B67h` |

`0043F620` has no Ghidra function; its nine bytes, read from the image at `0043F620`, are
`8B C1 C7 00 00 00 00 00 C3` = `MOV EAX,ECX / MOV dword ptr [EAX],0 / RET`. Each element is one
zeroed dword. The array element type is therefore a single-dword object with a non-trivial
destructor (`00440A30`), which is why the compiler emitted an iterator rather than a `memset`.

Scalars written around them by `0081ED40` itself: `+A10h` is set to `0.0f` at `0081F1F2`,
`+A14h`, `+A18h`, `+A1Ch` are zeroed at `0081EF29..0081EF3B`, and `+B40h` is zeroed at
`0081EFC8`.

## `+A20h`: the block that holds a unit back-pointer and a 20-element dword array

`0093BCC0`, `__thiscall(self, unit)`, `RET 4`, returns `EAX = self`, body
`0093BCC0..0093BD7D`, SEH scope table `00CA7996`. The call site is
`0081EF3B PUSH ESI` / `0081EF3C LEA ECX,[ESI+A20h]` / `0081EF47 CALL 0093BCC0`, so the single
argument is the unit itself.

| Offset | Unit offset | Type | Initial value | Site |
| --- | --- | --- | --- | --- |
| `+0h` | `+A20h` | unit back-pointer | the argument | `0093BD13` |
| `+4h` | `+A24h` | container header word | not written | — |
| `+8h` | `+A28h` | array first | 0, then `00822460` | `0093BCE4` |
| `+0Ch` | `+A2Ch` | array last | 0, then `00822460` | `0093BCE7` |
| `+10h` | `+A30h` | array end | 0, then `00822460` | `0093BCEA` |
| `+18h` | `+A38h` | dword | 0 | `0093BCF1` |
| `+1Ch` | `+A3Ch` | dword | 0 | `0093BCF4` |
| `+20h` | `+A40h` | dword | 0 | `0093BCF7` |
| `+24h` | `+A44h` | dword | 0 | `0093BD15` |
| `+28h` | `+A48h` | float | `1.0f` (`00D7A24C`) | `0093BD06` |
| `+2Ch` | `+A4Ch` | float | settings singleton `+3B0h` | `0093BD22..0093BD2D` |
| `+30h` | `+A50h` | float | settings singleton `+3ACh` | `0093BD30..0093BD3F` |
| `+34h` | `+A54h` | float | `0.0f` | `0093BD18` |
| `+38h` | `+A58h` | float | `0.0f` | `0093BD1D` |
| `+3Ch` | `+A5Ch` | float | `0.0f` | `0093BD50` |
| `+40h` | `+A60h` | float | `0.0f` | `0093BD55` |
| `+44h` | `+A64h` | byte | 0 | `0093BD69` |
| `+45h` | `+A65h` | byte | 1 | `0093BD48` |
| `+46h` | `+A66h` | byte | 1 | `0093BD4C` |

The singleton is `00424C40`, already established elsewhere as the settings singleton
(`docs/GAME_DYNAMICS_LIST.md`, `docs/UNIT_CONTROLLER_UPDATE.md`); it is called twice, once per
field, and each result is used immediately (`FLD [EAX+3B0h]` then `FLD [EAX+3ACh]`). The two
fields are tuning constants captured at construction, not per-unit state.

The array is sized by `00822460`, called with `ECX = self+4h` and the arguments
`(first = ESP scratch holding 0, count = 14h)`:

```
0093BD3E  PUSH ECX                      ; reserve the scratch dword
0093BD42  MOV EAX,ESP
0093BD5A  PUSH 0x14                     ; 20 elements
0093BD5C  MOV ECX,EDI                   ; EDI = self+4h, set at 0093BCDD
0093BD5E  MOV dword ptr [EAX],EBX       ; the fill value, 0
0093BD60  CALL 0x00822460
```

`00822460` reads its receiver's `+4h` and `+8h` as the first and last pointers and computes the
count with `SAR EAX,2`, so the element size is 4. Relative to the sub-object that is `+8h` and
`+0Ch`, which is why the constructor zeroes `+8h`, `+0Ch` and `+10h` and leaves `+4h` alone:
`+4h` is the container object's own header word and `+8h/+0Ch/+10h` are its first/last/end
pointers. The container is an external STL-style contract and is not ported here.

The upper bound `54h` on the sub-object comes from the next initialised unit field, the byte at
`+A74h` (`0081EF4C`), which is `+54h` past `+A20h`.

Purpose: not established. The evidence fixes the shape (a unit back-pointer, a 20-entry dword
table, two captured tuning floats, four floats and three flag bytes) but no reader of these
fields was read in this packet. `contract: unread` for the consumers.

## `+A98h` and `+AECh`: two identical records with an inner 2-element array

`0081EF63..0081EFB1` is an unrolled two-iteration loop. `EDI` starts at `ESI+A98h`, `EBP` is
`EBX+1 = 1`, the body advances `EDI` by `54h` and the loop ends when `EBP` goes negative, so it
runs for `EDI = unit+A98h` and `EDI = unit+AECh`.

Per record:

| Offset in the record | Type | Initial value | Site |
| --- | --- | --- | --- |
| `+0h` | float | `0.0f` | `0081EF8D` |
| `+4h` | float | `-1.0f` (`00D7A260`) | `0081EF91` |
| `+8h..+3Fh` | 2 elements of `1Ch` | `00403560` `` `vector_constructor_iterator' `` with constructor `0080FAE0` | `0081EF7D` |
| `+40h` | float | `0.0f` | `0081EFA3` |
| `+44h` | float | `0.0f` | `0081EF9E` |
| `+48h` | float | `0.0f` | `0081EF99` |
| `+4Ch` | byte | 0 | `0081EFA8` |
| `+50h` | dword | 0, then the unit pointer | `0081EF96`, then `0081F1E6` / `0081F1EC` |

The iterator arguments at `0081EF70..0081EF7D` are `(EDI+8, 1Ch, 2, 0080FAE0)` - a plain
`vector_constructor_iterator`, with no destructor argument, so the element type is trivially
destructible.

`0081F1E6 MOV dword ptr [ESI+AE8h],ESI` and `0081F1EC MOV dword ptr [ESI+B3Ch],ESI` fill in the
`+50h` field of both records after the loop: `A98h+50h = AE8h` and `AECh+50h = B3Ch`. Each
record therefore keeps a non-owning pointer back to the unit.

`0080FAE0` has no Ghidra function. Its 20 bytes at `0080FAE0..0080FB03`, decoded from the image:

```
0080fae0  0F 57 C0                 XORPS XMM0,XMM0
0080fae3  F3 0F 10 0D 04 38 CE 00  MOVSS XMM1,dword ptr [0x00CE3804]   ; 1000.0f
0080faeb  8B C1                    MOV EAX,ECX
0080faed  33 C9                    XOR ECX,ECX
0080faef  F3 0F 11 40 04           MOVSS dword ptr [EAX + 0x4],XMM0    ; 0.0f
0080faf4  F3 0F 11 48 08           MOVSS dword ptr [EAX + 0x8],XMM1    ; 1000.0f
0080faf9  F3 0F 11 00              MOVSS dword ptr [EAX],XMM0          ; 0.0f
0080fafd  88 48 0C                 MOV byte ptr [EAX + 0xC],CL         ; 0
0080fb00  89 48 18                 MOV dword ptr [EAX + 0x18],ECX      ; 0
0080fb03  C3                       RET
```

so each `1Ch` element is `{ float 0, float 0, float 1000.0f, byte 0, dword 0 at +18h }` with
`+10h..+17h` and `+1Ch` padding left untouched.

Purpose: not established; two records with a unit back-pointer each, holding a two-entry table
whose third float defaults to `1000.0f`.

## `+BD0h`: an owned critical section and a 40-slot ring of five floats

`00815600`, `__thiscall(self)`, plain `RET`, returns `EAX = self`, body `00815600..00815672`.
The call site is `0081F02A LEA ECX,[ESI+BD0h]` / `0081F03D CALL 00815600`.

```
00815606  MOV dword ptr [ESI],0xd09480
0081560c  MOV ECX,0x27
00815611  LEA EAX,[ESI + 0x18]
00815614  MOVSS dword ptr [EAX + -0x4],XMM0    ; XMM0 = 0, zeroed at 00815600
00815619  MOVSS dword ptr [EAX],XMM0
0081561d  ADD EAX,0x18
00815620  SUB ECX,0x1
00815623  JNS 0x00815614
00815625  CALL 0x00bd1860                      ; BSP_CriticalSection_Create
00815637  MOV dword ptr [ESI + 0x4],EAX
```

`ECX` runs `27h` down to `-1`, so the loop body executes 40 times and the last pair written is
`ESI+3BCh` / `ESI+3C0h`.

| Offset | Unit offset | Type | Initial value |
| --- | --- | --- | --- |
| `+0h` | `+BD0h` | vptr | `00D09480` |
| `+4h` | `+BD4h` | owned critical section | `00BD1860` result |
| `+8h` | `+BD8h` | 40 slots of `18h` | see below |
| `+3C8h` | `+F98h` | dword write cursor | `27h`, set by `00810020` |
| `+3CCh` | `+F9Ch` | byte | 0, set by `00810020` |
| `+3D0h` | `+FA0h` | float | copy of `00F87574` |
| `+3D4h` | `+FA4h` | float | copy of `00F87578` |
| `+3D8h` | `+FA8h` | float | copy of `00F8757C` |

The slot base and stride come from the fill routine, not from the zero loop:
`008100D9 LEA EAX,[EDX+EDX*2]` then `008100DC LEA EAX,[ECX+EAX*8]` is `self + index*18h`, and
the fill writes `[EAX+8]`, `[EAX+0Ch]`, `[EAX+10h]`, `[EAX+14h]` and `[EAX+18h]`. Slot `i` is
therefore at `self + 8h + i*18h` with five floats and four bytes of tail padding; slot 39 ends at
`+3C4h`, immediately below the cursor at `+3C8h`. The constructor's zero loop writes the fourth
and fifth float of every slot (`slot+0Ch` and `slot+10h`) and leaves the first three to the fill.

40 slots of `18h` starting at `+8h` reach `+3C8h` exactly, which is the independent check that
the ring is 40 slots and that `+3C8h` is not inside it.

`00815600` then calls the fill:

```
0081562A  FLDZ
0081562C  PUSH ECX
0081562D  FSTP float ptr [ESP]        ; angle = 0.0f
00815630  PUSH 0xf87574               ; &vec3
00815635  MOV ECX,ESI
0081563A  CALL 0x00810020
```

`00810020` is `__thiscall(self, const float* position, float angle)`, `RET 8`, body
`00810020..00810159`. It sets `self+3CCh = 0` and `self+3C8h = 27h`, takes `FCOS` and `FSIN` of
the angle after wrapping it into range against `00CE3830` and `00CE3828`, and then writes all 40
slots in one pass (`008100D7..0081014B`, `CMP ESI,0x28`), decrementing the slot index with a wrap
back to `27h`. So the ring is filled with a single pose and the cursor is left at the last slot.

After the call the constructor copies the same global vec3 into `+3D0h..+3D8h`.

The vtable `00D09480` has one slot, `0081ACF0` (`CG_scalar_deleting_dtor_0081acf0`): the next
bytes at `00D09484` are the string `"TorpedoStock"`. The plain destructor is `00818100`, three
instructions - restore the vptr, `ADD ECX,4`, tail-jump to `0041CC80`
`BSP_CriticalSection_DestroyOwned` - which confirms independently that `+4h` is a critical
section the object owns.

The one re-seed site read in this packet is inside `00818EA0` (body `00818EA0..0081939E`,
confirmed with `ghidra proto 00819381`):

```
00819369  MOV EAX,dword ptr [EDX + 0x50]   ; a virtual slot of another object
0081936C  MOV ECX,ESI
0081936E  CALL EAX                         ; result on the x87 stack
00819370  PUSH ECX                         ; reserve the argument slot
00819371  LEA ECX,[ESI + 0xfc]             ; the unit's position vec3
00819377  FSTP float ptr [ESP]             ; the angle argument
0081937A  PUSH ECX
0081937B  LEA ECX,[ESI + 0xbd0]
00819381  CALL 0x00810020
```

`00818EA0` is the routine that also calls `BSP_PointEffect_CreateFromMatrix` (`00868420`) and the
matrix helpers `004134F0`, `00413920`, `00414DB0`.

Purpose: provisional. The object is a 40-entry history of a pose - a vec3 plus a cosine and sine
pair, re-seeded from the unit's world position and a computed angle - protected by a critical
section, which means a thread other than the one that writes it reads it. Naming it a pose
history ring is a hypothesis from that shape; no consumer was read in this packet.

## `+10A0h`: two owned containers, constructed inline

`0081ED40` builds this one inline at `0081F102..0081F154`. `0081EB90` (body
`0081EB90..0081EBF0`, no callers in the image) is the standalone copy of the same constructor:
it writes the identical field set at the identical relative offsets, including the two it skips.

| Offset | Unit offset | Type | Initial value | Inline site | Standalone site |
| --- | --- | --- | --- | --- | --- |
| `+0h` | `+10A0h` | vptr | `00D09624` | `0081F102` | `0081EBB2` |
| `+4h` | `+10A4h` | float | `1.0f` (`00D7A24C`) | `0081F10C` | `0081EBB8` |
| `+8h` | `+10A8h` | container header | not written | — | — |
| `+0Ch` | `+10ACh` | first | 0 | `0081F114` | `0081EBBD` |
| `+10h` | `+10B0h` | last | 0 | `0081F11A` | `0081EBC0` |
| `+14h` | `+10B4h` | end | 0 | `0081F120` | `0081EBC3` |
| `+18h` | `+10B8h` | container header | not written | — | — |
| `+1Ch` | `+10BCh` | first | 0 | `0081F12E` | `0081EBCE` |
| `+20h` | `+10C0h` | last | 0 | `0081F134` | `0081EBD1` |
| `+24h` | `+10C4h` | end | 0 | `0081F13A` | `0081EBD4` |
| `+28h` | `+10C8h` | dword | 0 | `0081F140` | `0081EBD7` |
| `+2Ch` | `+10CCh` | dword | 0 | `0081F146` | `0081EBDA` |
| `+30h` | `+10D0h` | float | `60.0f` (`00CEB4B0`) | `0081F14C` | `0081EBE1` |

The two skipped words are the reason for reading the destructor. `0081EC00` (body
`0081EC00..0081ED14`) writes the same vtable and then walks two containers:

```
0081EC20  MOV dword ptr [EBX],0xd09624
0081EC32  LEA ESI,[EBX + 0x8]
0081EC35  MOV ECX,dword ptr [ESI + 0x4]     ; = EBX+0Ch, the first pointer
0081EC3C  MOV EAX,dword ptr [ESI + 0x8]     ; = EBX+10h, the last pointer
0081EC41  SAR EAX,0x3                       ; element size 8
0081EC8E  CALL 0x00867b10                   ; per element
0081EC9E  MOV ECX,dword ptr [EBX + 0x1c]
0081ECA3  LEA EDI,[EBX + 0x18]
0081ECB6  CALL 0x004cc760                   ; per element
0081ECBF  CALL 0x00bf65ac                   ; _free
```

So `+8h` and `+18h` are the header words of two containers of the same shape as the one at
`+A20h+4h`, with first/last/end at `+4h/+8h/+0Ch` of each container, and the constructor
correctly leaves the header words to the container's own base. The first container holds
8-byte elements released through `00867B10`; the second holds elements released through
`004CC760` and then freed.

The vtable at `00D09624` has one slot, `00822660`
(`CG_scalar_deleting_dtor_00822660`, which calls `0081EC00`). The dwords from `00D09628` onward
belong to a different, adjacent table: `0080DF90` reads `this+3B4h` and `00811AB0` reads
`this+30h` and `this+364h`, offsets that only fit a whole unit, and `+364h` is a documented unit
field in `docs/UNIT_INSTANCE_LAYOUT.md`. A `0.0` dword sits at `00D09620`, immediately below the
table, so there is no RTTI locator and the class name cannot be recovered that way.

Purpose: provisional. `00867B10`, which releases the first container's elements, is also called
by `00818EA0`, the routine that creates point effects; that makes an owned list of spawned
effects the leading hypothesis, but no producer of either list was read in this packet.

## `+10D4h`: the leak manager (use is a contract)

`0074E7B0`, `__thiscall(self)`, plain `RET`, body `0074E7B0..0074E7ED`. Receiver from
`0081F154 LEA ECX,[ESI+10D4h]`.

| Offset | Unit offset | Type | Initial value |
| --- | --- | --- | --- |
| `+0h` | `+10D4h` | vptr | `00D00514` |
| `+18h` | `+10ECh` | dword | 0 |
| `+1Ch` | `+10F0h` | dword | 0 |
| `+20h` | `+10F4h` | dword | 0 |
| `+24h` | `+10F8h` | byte | 0 |
| `+28h` | `+10FCh` | float | `0.0f` |
| `+2Ch` | `+1100h` | float | `0.0f` |
| `+38h` | `+110Ch` | float | `200.0f` (`00CE386C`) |
| `+3Ch` | `+1110h` | float | `60.0f` (`00CEB4B0`) |

`00D00514` is a six-slot vtable - `0074EC00`, `0074EDA0`, `0074EB20`, `0074EB80`, `0074EB00`,
`004499C0` - and the bytes at `00D0052C` are the strings `"sumLeaks"` and `"sumForces"`, which
is where the segment keyword set comes from.

`docs/UNIT_CONTROLLER_UPDATE.md` establishes the use: this is the flooding model, labelled
`leakManager` by the `_ship` diagnostic dump at `00818340`, walked by `0074F2E0` and accumulated
by `0074F930`, and `+28h` (`unit+10FCh`) is its total water. That is a contract here and was not
re-derived. `+38h = 200.0f` and `+3Ch = 60.0f` are new: the constructor's two tuning constants.

Open question: `0074F2E0` reads the list length at `+14h` of this object, and `0074E7B0` does not
write `+4h..+14h`. Either an allocator zeroes the unit's storage before the constructor chain
runs or another routine initialises the list head. This was not settled in this packet.

## Host table: one row per native call site

`ECX` is the receiver. All five constructors are `__thiscall`; only `0093BCC0` takes a stack
argument, and its `RET 4` confirms the count.

| Site | Callee | Host method | this | args | ret | Gate |
| --- | --- | --- | --- | --- | --- | --- |
| `0081ED7B` | `00809270` | `construct_owned_ref_slot` | `unit+72Ch` | none | none | none |
| `0081EDEF`/`0081EE95` | `00812D40` | `construct_order_ring` | `unit+838h` | none | none | none |
| `0081EF24` | `00BF7CD1` | `construct_dword_array` | - | `(unit+A00h, 4, 4, 0043F620, 00440A30)` cdecl | none | none |
| `0081EF47` | `0093BCC0` | `construct_sub_object_a20` | `unit+A20h` | `unit` | `EAX = this` | none |
| `0081EF7D` | `00403560` | `construct_record_inner_array` | - | `(record+8, 1Ch, 2, 0080FAE0)` cdecl | none | loop, 2 records |
| `0081EFCE` | `00BF7CD1` | `construct_dword_array` | - | `(unit+B44h, 4, 4, 0043F620, 00440A30)` cdecl | none | none |
| `0081EFED` | `00BF7CD1` | `construct_dword_array` | - | `(unit+B54h, 4, 5, 0043F620, 00440A30)` cdecl | none | none |
| `0081F03D` | `00815600` | `construct_pose_history_ring` | `unit+BD0h` | none | `EAX = this` | none |
| `0081563A` | `00810020` | `fill_pose_history_ring` | `ring` | `(00F87574, 0.0f)`, `RET 8` | none | inside `00815600` |
| `00815625` | `00BD1860` | `create_critical_section` | - | none | `EAX = handle` | none |
| `0093BD22` | `00424C40` | `settings_singleton` | - | none | `EAX = singleton` | none |
| `0093BD30` | `00424C40` | `settings_singleton` | - | none | `EAX = singleton` | none |
| `0093BD60` | `00822460` | `array_resize` | `sub+4h` | `(&zero, 14h)` | none | none |
| `0081F15F` | `0074E7B0` | `construct_leak_manager` | `unit+10D4h` | none | none | none |
| `00819381` | `00810020` | `fill_pose_history_ring` | `unit+BD0h` | `(unit+FCh, angle)` | none | in `00818EA0` |

## Coverage

| Routine | Coverage |
| --- | --- |
| `00809270` | complete |
| `0093BCC0` | complete |
| `00815600` | complete |
| `0074E7B0` | complete |
| `0081EB90` | complete |
| `0081ED40` sub-object region `0081ED6F..0081F1F8` | complete |
| `00812D40` | contract, `docs/UNIT_STATE_MESSAGE.md` |
| `00810020` | partial: the field writes and the loop bounds are read; the x87 interpolation between `008100E4` and `00810139` is transcribed only as "writes five floats per slot" |
| `0081EC00` | partial: the two container walks and their element releases are read; the tail after `0081ECF1` is unread |
| `00822460` | partial: the element size and the resize path to `0081E600` are read; the growth branch is an STL contract |
| `00867B10`, `004CC760`, `0080DF90`, `00811AB0` | `contract: unread` |
| readers of `+A20h`, `+A98h`, `+B44h`, `+B54h`, `+10A0h` | `contract: unread` |

## Follow-ups

| Packet | Addresses | Subject |
| --- | --- | --- |
| `unit_pose_history_consumers` | 00810020 00818EA0 00867B10 | Who reads the `+BD0h` ring under its critical section, and whether the five floats per slot are a position and a heading pair |
| `unit_subobject_a20_consumers` | 0093BCC0 00822460 | The readers of the 20-entry dword table at `unit+A28h` and of the two settings floats captured at `+A4Ch`/`+A50h` |
| `unit_effect_lists_10a0` | 0081EC00 00867B10 004CC760 | The producers of the two containers at `unit+10A8h` and `unit+10B8h` |

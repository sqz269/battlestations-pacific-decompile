# unit+61h and the manual pair at unit+0FC4h / unit+0FDCh: the complete writer scan

Addresses: 0081ED40 0081F05D 0081F06D 007B8840 007C6760 007C6ADD 007C6B02 008266C1 008266CE
0082674C 00834E90 00834EBE 008350BD 009F50FC

Packet `cc_ai_order_hop`, worker `agent/cc-ai-order-hop`, 2026-09-12 UTC. Ghidra was read-only
for this packet. Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every
live query verified both. Descriptive names are hypotheses, not recovered symbols.

## Answer to the packet question

**Neither `unit+61h` nor the pair it selects has a writer anywhere in `.text` outside the unit
constructor.** `docs/UNIT_AUTOPILOT_PAIR.md` reached that result for the pair; this packet
reproduces it over a different set of encodings, extends it to the selector byte itself, and
settles what the regime would command if the byte were ever set: full ahead and full rudder,
because `0081F05D` and `0081F06D` store `1.0f`.

The consequence for the packet that asked whether the executable's `--order` could go "through
the same byte" is that it cannot, or rather that doing so would drive the ship at a constant the
constructor picked. The three readers all take the pair verbatim; nothing scales, blends or
refreshes it.

## The three readers, complete

| site | containing function | what the byte selects |
| --- | --- | --- |
| `009F50FC` | `009F50E0`, the ship AI controller step | `CMP byte [EAX+61h],0; JNE 009F524F`: the whole AI update returns without a host call |
| `008266C1` | `00825F20`, the ship motion tick | `CMP byte [EDI-2AFh],0; JZ 00826754` with `EDI = unit+310h`, so the test is on `unit+61h`. When set, `008266CE..0082674C` fills every ring slot from the read cursor to the write cursor and then writes `ring+148h` and `ring+14Ch` directly, sourcing `[EDI+0CB4h]` and `[EDI+0CCCh]`, which are `unit+0FC4h` and `unit+0FDCh` |
| `00834EBE`, `008350BD` | `00834E90`, the propeller update | `MOV AL,[EBP+61h]` then `unit+61h ? unit+0FC4h : unit+980h` for the throttle (`008350C4`/`008350CE`) and `unit+61h ? unit+0FDCh : unit+984h` for the rudder (`008350E0`/`008350EA`) |

## The pair's only writer

```
0081F04B: MOVSS XMM2,[00D7A24C]        ; 1.0f
0081F055: MOVSS [ESI+0FD0h],XMM2
0081F05D: MOVSS [ESI+0FC4h],XMM2       ; the throttle
0081F065: MOVSS [ESI+0FE8h],XMM2
0081F06D: MOVSS [ESI+0FDCh],XMM2       ; the rudder
```

inside `BSP_UnitVehicleBase_Construct` (`0081ED40-0081F354`). `00D7A24C` is `1.0f`, so both
fields start at the extreme of their `[-1,+1]` range and stay there.

Scanning `.text` for the four-byte little-endian displacement `0FC4h` finds exactly two
instructions in the whole image: `0081F05D` above and `008350C8`, the propeller read. The same
scan for `0FDCh` finds eleven, of which `0081F06D` and `008350E4` are the pair's and the other
nine are unrelated objects (`004BAE54`, `004EAF05`, `00834EC9`, `00920021`, `0092010D`,
`0092025D`, `0092038F`, `009204FF`, `009806F2`). The shifted base `unit+310h` accounts for the
motion tick's two reads at displacements `0CB4h` and `0CCCh`, and the `0CB4h` scan finds one site,
`00826720`. That closes the set: three readers, one writer, no producer.

## The selector byte: every byte-store encoding at displacement 61h

The unit's own base is displacement `61h`; `docs/UNIT_INSTANCE_UPDATE.md` lists the secondary
bases the class installs at `+010h`, `+024h`, `+170h`, `+1E4h`, `+310h`, `+38Ch` and `+72Ch`, so
the same field is `51h` and `3Dh` from the first two and a negative displacement from the rest.
Every `mov`, `mov imm`, `cmp`, `grp1 imm` and `setcc` byte form was scanned at each, with a
wildcard register field, over `mod=01` disp8 and `mod=10` disp32, plus the `SIB` forms.

| result | sites |
| --- | --- |
| `mov byte [r+61h], imm8` | `00454244`, `00456E30`, `007C6B02` |
| `mov byte [r+61h], r8` | `004540B4`, `007B884B`, `009C9134`, `009C9252`, `00B006E4`, `00B01B12` |
| the same at the five negative displacements | none |
| `SIB` forms at `61h` | four, all `[ESP+61h]` (`SIB` base `ESP`, no index): `005AA86B`, `00775844`, `00A4878C`, `00B2CF62` - stack locals, not object fields |
| `setcc byte [r+61h]` | none |

None of the nine object stores can be tied to a unit. The two that look closest are both on the
weapon side and both write an object the weapon controller owns:

* `007C6B02 MOV byte [ECX+61h],1` sits inside `FUN_007C6760` with `ECX = [EBP+0DECh]`, and
  `007C6ACC` calls `BSP_WeaponController_HasOrdnanceOfKind` with `ECX = EBP`, so `EBP` is a weapon
  controller and `[EBP+0DECh]` is something it owns. The same block writes `+64h` (a float) and
  `+68h` (a byte), neither of which any unit routine this project has read ever touches.
* `007B8840..007B885A` is the out-of-line `if (obj+61h != v) { obj+61h = v; obj+68h = 1; }`
  setter for that same shape, and it has no reference of any kind in the image.

So the scan's answer is a negative result about the unit, not an identification: the byte is read
in three places on the unit and written in none.

## Routines and coverage

| routine | state | coverage |
| --- | --- | --- |
| `0081F05D`, `0081F06D` | read from the listing; the constructor is not reconstructed | none beyond the two stores |
| `008266C1..0082674C` | read from the listing; `docs/UNIT_STATE_MESSAGE.md` reconstructs the two ring setters it inlines | complete as a reading, not reconstructed here |
| `008350BD..008350F2` | read from the listing | complete as a reading |
| `unit_ordered_pair_source_008266c1` | unchanged from `docs/UNIT_AUTOPILOT_PAIR.md` | complete |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/UNIT_AUTOPILOT_PAIR.md`: "No producer of `unit+0FC4h` / `unit+0FDCh` exists outside the constructor" (the value it stores was not stated) | Still true, and the constructor's value is `1.0f` from `00D7A24C`, the same constant the motion tick uses as a clamp ceiling. A unit that ever had `unit+61h` set would be ordered full ahead with full rudder | `0081F04B`, `0081F05D`, `0081F06D` |
| `docs/UNIT_AUTOPILOT_PAIR.md` left the writer of `unit+61h` open ("the player's direct control is the candidate") | No writer of `unit+61h` exists in `.text` under any byte-store encoding at the field's displacement from the unit's own base or from any of the seven secondary bases. The player-control candidate is specifically ruled out for `007C6760`, the one direct-control-looking site: its object comes from a weapon controller | the encoding table above; `007C6ACC` |

## no_ghidra_function

| start | inclusive end | evidence for each boundary |
| --- | --- | --- |
| `007B8840` | `007B885A` | start: `int3` padding `007B883D..007B883F`, then `MOV AL,[ESP+4]` opening a `__thiscall` with one byte argument. End: `RET 4` at `007B885A`, then `int3` `007B885D..007B885F`, then `FUN_007B8860`. No reference anywhere in the image |

`007C6760` has a Ghidra function (`FUN_007C6760`) but its body is truncated: the call graph
records one callee while the listing at `007C6A88` and `007C6ACE` shows calls to `007DE9A0` and
`007B91C0`. That is a flow-repair candidate, not a missing function.

## Uncertainties

* The object `007C6760` writes at `+61h`/`+64h`/`+68h` is identified only as "owned by a weapon
  controller". Naming it would be an invention.
* The scan covers byte stores at a constant displacement. A write through a pointer the compiler
  materialised into a register, or a bulk `rep stos` over the object, would not appear. Both were
  also missed for the pair in the earlier packet, and both remain the only way the negative result
  could be wrong.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `unit_flag_61_origin` | `007C6760`, `007B8840`, `009C8F40`, `00454244`, `00B006E4` | Identify the class each `+61h` store belongs to, so the negative result for the unit becomes a positive identification for whatever class does own the field |
| `unit_construct_flow_repair` | `007C6760` | `python tools/ghidra_flow_repair.py 007C6760`: the stored body misses the two calls the listing shows |

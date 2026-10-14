# Segment 7: soldier types, scene entities and helper instantiations (cc13-seg7)

Code: `src/classes_seg7_cc13.cpp`, `include/bsp/classes_seg7_cc13.hpp`, namespace
`bsp::cc13_seg7`. Every body listed here is reconstructed and build-tested. None is
ABI-compatible or game-validated. Names are hypotheses.

## Model

Same as segment 5 (`docs/CC13_SEG5.md`). Objects are byte pointers, and every field is
read or written at the image's offset (`native_at<T>`). Every call the binary makes,
virtual or direct, goes through a named `Seg7Host` method at the same point. Globals
(`[00E188A8]`, `[00F87544]`, byte `[00E081C0]`) are read and written through the host.
Calls between bodies reconstructed here are direct C++ calls. x87 arithmetic is written as
double arithmetic stored to float, because the image runs at 53-bit precision. Where an x87
listing uses `FMUL ST(1)`, the bytes were checked for the DC form.

The range checks that call 00BF6713 are kept when an index comes from the caller
(00481430, 00484A10, 00487940). They are omitted only where they repeat a loop condition.

## Record copies and matrices

| Address | Behaviour |
|---|---|
| 0047D250 / 0047F930 | 54h-byte copy: dwords +0, +4; floats +8..+48h (FLD/FSTP); byte +4Ch; dword +50h. RET 4, EAX = dest. |
| 0047D2E0 / 0047F9C0 | 4Ch-byte copy: dwords +0, +4; floats +8..+44h; byte +48h. |
| 0047F0C0 | 40h-byte copy: dwords +0, +4, +10h, +2Ch; floats elsewhere. |
| 0047E540 | `__fastcall(ECX matrix, EDX translation)`: identity (1.0f, 00D7A24C), row 3 = translation. |
| 00481510 | Box corners from {min, max}: +60h..+6Ch = 0, centre c = float(float(min + max) * 0.5), 24 floats. |
| 0047F200 | node+74h = 00413920(rotation-Y(-0.0f - heading) from 00B646E0 (copied with 004134F0), translation of +A4h..+ACh), copied with 004134F0. |

## Scene property bag

| Address | Behaviour |
|---|---|
| 0047E480 | Iterator next: chain +0Ch, else the next non-empty bucket below 40h; returns node != 0. |
| 00480690 | Iterator begin: first non-empty bucket. When every bucket is empty, the loop test reads bucket[40h] (bag+108h), as the binary does. |
| 00484D20 | Current key (node+4), or the .bss "" at 00E186ED. |
| 00487040 | new(0Ch) iterator over owner+4, positioned by 00480690. |
| 0048E9F0 | Contains: NativeString key (0041E870), 0043B8B0(bag+4, &key, &argument slot) != 0, key storage released. |
| 0048F5D0 | Float or default: fallback when 0048E9F0 fails; else 008F2260's +0Ch as float (type 1) or int converted. |

## Scene entities, points and proxies

| Address | Behaviour |
|---|---|
| 004802F0 | 00867B10, +9 = 1. |
| 00480810 / 00480860 | Three floats into 007AD030(index); 00480860 clears +18h first. |
| 004808A0 / 004808B0 | +18h = 0, then tail jump to 007B2850 (RET 8) / 007B1B40 (RET 4). |
| 004808F0 | new(20h) zeroed (eight 0.0f), or null. |
| 00480960 | AX = word [[+14h]+174h]. |
| 00480BA0 | +174h vtable+8(entity); +178h vtable+4(1) and clear; tail jump to 00923040. |
| 00480BF0 | Effect +180h: 00867B10, +9 = 1. |
| 00480C20 | Unless [00E188A8]+1FE4h == 2: +DCh vtable+1ACh(float). |
| 00481090 | View check with fade: alpha +1Ch fades by [00E188A8]+21ECh * 0.5 towards 1 inside float(300.0 / [[00E188A8]+19FCh]+1C4h) of the camera ([00E188A8]+1ED4h), towards 0 outside. At zero, a set +3Ch gives false; otherwise alpha = 0.0001f. Then the class AABB of +20h goes through 004B48E0. |
| 004812F0 | Class AABB of +14Ch, or the stored box +19Ch when +180h is set, through 004B48E0. |
| 00481430 | Layer child reset (range-checked): child vtable+114h(0), 00926D90(2), slot cleared. |
| 004814C0 | 00481430 over every child of the four layer vectors at +18Ch. |
| 00484A10 | Quad cell reset: row i of +4Ch (10h elements), child j; same kill and clear, range-checked. |
| 00484AE0 | Quad-tree prune. When cell size +7Ch exceeds the box width, every cell in the 0047BA40 range goes through 00484A10. Otherwise it recurses into the four children (+60h..+6Ch, boxes +0/+18h/+30h/+48h), then frees the node (00BF65AC) and clears the slot. |
| 004879B0 | Box from extent ± centre +70h..+78h. In view: 00498C40(tree, slot, extent); else 00484AE0. |
| 0048C900 | 004879B0(proxy, proxy+80h, proxy+58h); RET 4 with an unused argument. |
| 004847A0 | 00B6DA70(node, 0.0f, 0); 00922F80(entity, 0) when +5Ch is set. |
| 004847D0 | Instance: new(1ACh), 007135C0(memory, entity, factory vtable+8(0, 1.0f)) into +178h. Its node is then parented through the IsKindOf(44h) owner's +3D0h vtable+40h, or the IsKindOf(4) holder walk, or 00B6D890(root [00E188A8]+19ECh). |
| 00484940 | Node vtable+38h(entity+74h). Then 00B6DA70(node, 1.0f, 1) when +174h is null, else 00B6DA70(node, [+174h]+1Ch, 1) unless +184h is set. |
| 00484BC0 | Box +68h in view: create +24h on first use (+20h vtable+8, vtable+98h(0, [00E188A8]+19CCh, identity), vtable+114h(proxy), vtable+A0h, 004134F0(object+74h, proxy+28h)), then vtable+D8h. Out of view: kill +24h and clear it. RET 4. |
| 0048C920 | Proxy destructor body: vtable 00CE6660, kill +24h, clear, 00489DD0. |
| 00487230 | 00928570, then 004837D0 on the first node of [+30h]+36Ch holding the path. |
| 00487940 | When +94h is set: clear it; kill and clear each member i < +24h of the +74h vector (range-checked). |
| 004897B0 | 004134F0(node+74h, matrix), +C8h = 0, +10Ch = 0, 0042ED50 per child. |
| 0048CC60 | Flip byte 00E081C0. When it turns off, call 0048C090 on each value of the map at +28h, advancing with 00427FE0. Returns the new byte. |
| 0048D6C0 | Name into +154h: 0041DD40(strlen, flag 0), memcpy when data is set. |
| 00480930 | IsKindOf for ids 47h, 1, 0 and +C4h. |
| 004845D0 | RefPtr assign (InterlockedIncrement 00CE221C on the new object, release of the old one through 00CE2220 and vtable slot 0). |
| 00487210 | 00928560, then 00484540([+30h]+36Ch, entity). |
| 00487270 | Each value of the list at +0Ch: vtable+4(delta). The next node is read first. |
| 00489760 | Position +A4h..+ACh, pose flags cleared, 0042ED50 per child. |
| 0048DBD0 | 00923840; class name copied into +154h; tail jump to vtable+120h. |
| 00487380 | Class-37 base destructor body: three vtables, stop the +180h effect, delete +178h, release +180h, 00925780. |
| 00489F30 | Class-37 base constructor: 00925CE0, three vtables, +180h = 0, +C4h = 37h, +17Ch = 00BD2F10(0, -clip * 0.25, clip * 0.25), +184h = 0. |

## Pointer arrays, observers and strings

| Address | Behaviour |
|---|---|
| 00482510 / 00482570 | Reserve (byte-identical): min 1; new through 00BF55BE, copy (skipping a null block), free through 00BF6989. |
| 00485390 / 004883B0 (00482510) and 004853D0 / 004883F0 (00482570) | Push-back with doubling. |
| 00485990 / 004859E0 | Resize: reserve, zero-fill, step the count down, store. |
| 0048CE30 / 0048CE50 | Resize(0), free the block. |
| 00483910 (void) / 00486B10 / 004893F0 (return slot) | Observer slot set: 006952A0(old, slot), store, 00694A60(new, slot). |
| 004884F0 / 00488520 | Release when subject vtable+4() equals +14h: clear, then 006952A0(id, slot). |
| 00486F80 / 0048D740 / 0048D830 | memmove_s (00BF67A7) of a dword range; returns dest end. |
| 0048CFC0 | NativeString storage back to the pool (00419CC0, 00BD1510). |
| 0048E0A0 | +0 = 0, free +8, zero +8..+10h. |
| 0048E3B0 | NativeString copy plus a dword at +8; RET 8. |
| 0048E630 | Release a RefPtr box and free it. |
| 0048F200 / 0048F550 | Set, clear or sync bit (+8 & 1Fh) in the word from 0048E330 (0048F550 asks 0048F230(source)). |

## Soldier globals and soldier types

| Address | Behaviour |
|---|---|
| 00484700 | vtable 00CE6458, +1Ch = 0, 0084CB70 (the SoldierGlobals.lua load). |
| 00485520 | Double-checked singleton [00F87544] under the 00415350 manager's section (+10h, counter +18h), new(20h) + 00484700, registered with 00BD0C30. |
| 00487770 | FakeEffects row: MaxTime (10.0f), MinTime (2.0f), Radius (10.0f), Weight (10.0f) into +4..+10h. A non-zero ExplosionEfx id gets a handle (00870CD0) assigned into the RefPtr +0. |
| 0048DC40 | Soldier type: vtable 00CE66DC, class +4, Weight (00B66270), FireChance 0, Damage 5, MinDistance 5, MaxDistance 10, MaxEntity 14h, MininRow 1, MaxinRow 2, Defence (byte +39h), OutingDelay 1.5 (+28h). Each FakeEffects table value becomes a 14h-byte row pushed onto +8. RET 8. |

## Uncertainties and simplifications

- Vtable stores are written as image addresses.
- Float copies through FLD/FSTP would quieten a signalling NaN in the image; the C++ copies the bits.
- 00480960 returns the word; the image leaves the pointer's upper half in EAX.
- The .bss byte 00E186ED is taken as "" (zero at load, no writer was searched for).

## Belongs to cc9

The soldier and landing-troop behaviour in this range is per-frame unit behaviour. It is
already written in `src/soldier_traffic_cc13.cpp`. cc13-backfill (152a7e698) audited it and
added ledger records: verified 0048A5A0, 0048BF80, 0048C090, 0048C190; minor deviation
0048A6D0, 0048AF00, 0048B120; labelled host substitution 0048C230 (lane writes returned, +180h
latch left to the host); defect (plan or flag returned instead of the calls) 0048A010,
0048A1E0, 0048A650, 0048AEC0. 00487470 is only referenced as a flag there and has no
implementation or record.

## Not reconstructed

00487A70 and 0048C6A0 (large; 0048C6A0 has unrecovered register inputs), and 0048EAB0,
0048EE90 and 0048EFD0 (traffic path writers).

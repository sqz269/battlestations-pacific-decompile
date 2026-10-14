# Segment 5: engine-movie action runs and track helpers (cc13-seg5)

Code: `src/actions_seg5_cc13.cpp`, `include/bsp/actions_seg5_cc13.hpp`, namespace
`bsp::cc13_seg5`. Status of every body: reconstructed, build-tested; not ABI-compatible,
not game-validated. Names are hypotheses.

## Model

The bodies are written against the image's object layout. An object is a byte pointer
(`Object`) and each field is read or written at the image's offset through `native_at<T>`,
so the C++ performs the same loads and stores as the binary. Every call the binary makes
(virtual or direct) goes through a named `Seg5Host` method at the same point with the same
arguments, and every global is read through the host each time the binary loads it. The
layout code assumes Win32 (static_assert on pointer size).

Calls between functions reconstructed here are direct C++ calls (for example the run
bodies call `movie_find_track_by_entity_0045c690`).

Layouts used (all from the listings cited per function):

- Engine-movie action: +04h ActivateTime, +10h type, +14h resolved ReferPath entity,
  +20h/+24h/+28h Num1..Num3, +2Ch NativeString Enum (size +2Ch, data +30h), +34h OffsetX,
  +3Ch OffsetY, +38h OffsetZ.
- Movie player ([00E188A8]+38h): +04h time, std::vector<Track*> at +68h/+6Ch.
- Entity track: +0F4h list of offset samples (iterator +100h/+104h), +108h list<Action*>
  (scratch iterator +114h/+118h), +11Ch entity, +120h name NativeString, +140h/+144h the run
  cursor, +14Ch parent state, +1D4h copied from [00E188A8]+19FCh. Camera track adds shake lists
  at +1DCh and +1E8h.
- MSVC list object: +0 proxy, +4 head node, +8 size; node {next, prev, value}.

## Action run bodies (action vtable slot 0, `__thiscall(ECX action, [ESP+4] entity)`, RET 4)

| Address | Action | Behaviour |
|---|---|---|
| 004650F0 | sSetTarget | With action+14h non-null: command 00E08F78 if entity IsKindOf(6) else 00E08EF8; stack target = inlined 00465080 (byte 1, word entity+174h, entity, vector 00F87574, 0.0f); 0077D600(entity, command, &target, 1). |
| 00464870 | cSetParent | Track of entity (0045C690, not null-tested) and track of action+14h; when the second exists 00458D50(track, Num1, parent, &{X, Y, Z}, player time). |
| 00464350 | cUnParent | 00458A70(track of entity, unchecked; Num1, player time). |
| 004648F0 | pForceRelease | Byte entity+9C0h+[00F876B8]*8 set -> 007BBBA0(entity). |
| 00465180 | ForceDepthCharge | Children (+48h, next +44h) with IsKindOf(1Eh), IsKindOf(24h), [child+3F4h]+80h == 8 and group child+38Ch (int) == Num1 or Num1 == 0: child vtable+1F0h(). |
| 00464910 | SetGunFire | IsKindOf(18h): value 1/-1 from Enum "OpenFire" (00425850) into +9B0h of each member entity+3D0h[i] (null past index 4), count entity+3CCh. Else IsKindOf(0Fh): entity+9B0h = 1/-1. |
| 004649B0 | TogglePeriscope | entity+1235h = Enum is "Out" (_stricmp; null Enum gives 0). |
| 00464390 | ForceArtilleryAngle | RET 4, no effect. |
| 00465210 | ForceTorpedo | Children with IsKindOf(1Eh) and IsKindOf(20h): 0072CF00(i, 0.0f, 0) for each barrel (+448h), then vtable+1D8h(0, 80.0f, 80.0f) (00CE5444). |
| 00465410 | SetGunFireTargetOnly | Non-null entity, IsKindOf(5), non-null [entity+6DCh]: its +59h = Enum is "TargetOnly" (00425850). |
| 00465450 | TurnGun | Children with IsKindOf(1Eh), IsKindOf(24h) and the group test: 0085ABA0(child, Num2, Num3). |
| 004643D0 | cFOV | RET 4, no effect. |
| 004643E0 | cFOV_Now | RET 4, no effect. |
| 004649F0 | DontMove | entity+62h = Enum is "Stay". |
| 00464AB0 | CameraShake | camera = "CameraPos" (_stricmp) or "CameraAndLookAt" (00425850); look-at = "LookAtPos" or "CameraAndLookAt". With a player and camera track (0045DCA0): 0045AF70(track, ActivateTime, X, Y, Z, max(Num1, 0), 1) and/or (..., 0). |
| 00464C40 | Binocular | With [[00E198C4]+84h]+20h and +70h non-null, Enum (00425850): "Binocular" -> +6Ch(1), +20h(1), +70h(0); "Periscope" -> +6Ch(1), +70h(1), +20h(0); "OFF" -> +20h(0), +70h(0); each vtable+34h. |
| 00464D70 | TakeOffHack | entity+64h = Num1. |
| 00464D80 | WingTipEffect | entity+96Ah = Enum is "On". |
| 00464DC0 | MoveToPoint | entity+748h = Enum is "Enable". |
| 00464E00 | WideBlackBars | Enum "Enable" -> 005CB0F0 else 005CB130, ECX = [[00E198C4]+A0h]. |
| 00464E50 | UseCatapult | Stack message: 0075B430(7Bh), +20h = float(Num1 / 180.0 * pi) (doubles 00CE3D20, 00CE3D28), the 00464710 fields; 0077C2A0(entity, &message, 2, 0). |
| 00464EE0 | HideEntity | IsKindOf(5): entity+68h = Enum is "Hide" (00425850). |

Group test (00465180, 00465450): FILD of child+38Ch compared with FLD Num1 (FUCOMIP), then
UCOMISS Num1 against 0.0f (00D7A218). Written as a double compare plus `num1 == 0.0f`.

Notes on the enum compares: the binary uses two forms. The inlined form tests the data
pointer at +30h and calls `_stricmp` (00BF7FBF); it is reproduced with `_stricmp`. The other
form calls 00425850 (`BSP_NativeString_EqualsCStringInsensitive`), which goes through the host.

## Movie player and track helpers

| Address | Behaviour |
|---|---|
| 0045C690 | First track whose +11Ch is the entity, else null. |
| 0045DCA0 | First track whose name copy (0045D840) is "CameraPosition 01" (00CE4DA4); each copy released through 00419CC0 + 00BD1510 (size + 1, 1). |
| 0045D840 | Name copy: out = {0, 0}; unless out is track+120h, 0041DD40(out, size, 1) and, for a non-empty name, memcpy (00BF7680). |
| 0045C560 | Every track's vtable+0Ch(player time). |
| 004561E0 | Scratch-iterates the action list calling each action's vtable+4 (stop). |
| 00456290 | Cursor = (track+108h, first node), track vtable+4, +14Ch = 0. |
| 004562D0 | Mean of the three floats at node +14h..+1Ch over the +0F4h list, divided by float(unsigned size). |
| 004567D0 | Finds the node holding an action, unlinks and frees it (not the head), size - 1, cursor = (container, next node). |
| 004569A0 | List clear then tail-jump to 004555F0. |
| 004555F0 | List clear, free the head, +4 = 0 (an STL _Tidy; reconstructed because 004569A0 tail-jumps into it). |
| 00459F40 | List clear for nodes carrying a NativeString at node +0Ch/+10h, releasing each string to the pool before freeing the node. |
| 00458050 | 38h-byte record assignment: float +0, NativeString +4 (0041DD40 + memcpy), floats +0Ch..+30h, dword +34h. |
| 00456710 | `__fastcall(ECX place, EDX source)`: 00455880(place, source) when place is non-null. |
| 00457160 | track+1D4h = [[00E188A8]+19FCh]; tail-jump to 00456DD0. |
| 0045AF70 | 0045A320 on track+1DCh (camera flag set) or track+1E8h with (time, x, y, z, amplitude); RET 18h. |
| 0045BD90 | 00459020(time), then 0045ACC0(). |
| 0045BE40 | 004569A0 on +1E8h then +1DCh, then 0045B220. |
| 00458A70 | cUnParent blend, RET 8. With a parent (+154h): entity vtable+ACh(0), +14Ch = 0; start = 004581E0(parent, t) + offset +1C8h; start velocity = 200 * (004581E0(parent, float(t - 0.2)) - start) into +174h and +180h; t2 = duration + t as a double; end = 004581E0(track, float(t2)) (read from the stack buffer), +190h = float(t2); end velocity from 004581E0(track, float(t2 - 0.2)) into +1ACh and +1B8h; +150h = duration, offset zeroed, +14Ch = 3. |
| 00458D50 | cSetParent blend, RET 10h. 00414DB0(entity) when entity+C8h is clear; start = entity +FCh..+104h, +158h = t; start velocity from 004581E0(track, float(t - 0.2)); end = 004581E0(parent, float(t2)) + offset; end velocity from 004581E0(parent, float(t2 - 0.2)) + offset; +154h parent, +150h duration, +1C8h offset, +14Ch = 1. |
| 0045B100 | Entity-track constructor (vtable 00CE4DB8, lists from 00455520/00455540, 004658D0(0.0f) on +158h and +190h, cursor = (+108h, head), zero floats +4..+0F0h, +148h). |
| 0045BD00 | Camera-track constructor: 0045B100, vtable 00CE4DCC, lists +1DCh/+1E8h from 004555D0, +1D4h = 0. |
| 0045B220 | Entity-track destructor body: 00459F40(+0F4h); 00465610 + free for each non-null action; free the +108h nodes; release the strings +194h, +15Ch, +120h; clear and free the +108h head; 00459F40(+0F4h) and free its head. |
| 0045BAD0 | Appends each +0F4h sample's leading float to a std::vector<float> (in place when there is capacity, else 0045A120). |
| 0045EFB0 | Track factory, RET 8: compares a by-value name with a "CameraPosition 01" NativeString (00435C40); 1F4h bytes + 0045BD00 on a match, else 1D4h bytes + 0045B100; releases the literal and the argument. |
| 0045D8A0 | Assigns a by-value NativeString to the name +120h and releases the argument. |
| 0045DB40 | Movie start: +60h = 1; 004CD0F0(game, 0, 0, 1); 005B9BA0(screen, 0.99f, -0.01f, ""); 005B9760(screen, 1.0f, 1.0f, ""); 005CB160(movie screen). |

The blend constants are the doubles 0.20000000298023224 (00CE3D10) and 200.0 (00CE4D70). The
velocity products use `FMUL ST(1),ST(0)` (DC form) for x and `FMUL ST(0),ST(1)` for y; the
bytes at 00458B93/00458B9F, 00458CC3/00458CCF, 00458E41/00458E4D and 00458F8A/00458F96 were
checked, and both forms multiply the component by 200.

## Other bodies in the range

| Address | Behaviour |
|---|---|
| 004646B0 | 0072CF00(i, 0.0f, 0) for each barrel i < gun+448h. |
| 00465000 / 00465020 / 00465040 / 00465060 | Cast: non-null object answering IsKindOf(24h / 20h / 5 / 8), else null. |
| 00465080 | Command target fill (vector 00F87574, word 0, entity, then byte/word from the entity or zero, float +14h); returns ECX; RET 8. |
| 00464F70 | The same stores as 00465080; EAX is left holding the entity argument. |
| 00464500 / 00464590 / 004647B0 | Session message IsKindOf: 46h; 49h or 46h; 7Bh, 49h or 46h. |
| 004645B0 | 0075B480(message, stream), 004290B0(stream, byte +1Ch). |
| 004645D0 | 0075B4C0(message, stream), 00428D70(stream+4, &message+1Ch). |
| 00464710 | 0075B430(7Bh); +18h word 0, +1Ah 0, +1Ch 0, +4 = 1, vtable 00CE53AC, +20h float. |
| 0045C470 | game+193Ch = 0; tail-jump to 004C3CB0. |
| 0045C510 | Close a non-null handle (+14h) through the import at 00CE225C, +14h = 0. |
| 0045CA40 | Writer destructor body: vtable 00CE4E04, close, vtable 00CE4DFC, +14h = 0. |
| 0045CAC0 | Stack writer, 009501B0 open of path.data (or the .bss empty string 00E18511), close on success, inlined destructor. |

## Uncertainties and simplifications

- The checked-iterator and vector-range tests that call 00BF6713 repeat the loop
  conditions on every path reconstructed here and are omitted.
- 00464910 writes through a null member for an index past 4, as the binary does.
- Vtable pointer stores are written as the image addresses (00CE53AC, 00CE4E04, 00CE4DFC).
- 00458050 copies floats through FLD/FSTP; a signalling NaN would be quietened by the binary
  and is copied unchanged here.

## Belongs to other lanes

None of the bodies here are per-frame unit, ship or plane behaviour; the run bodies are
mission-script actions applied once when the movie reaches their ActivateTime.

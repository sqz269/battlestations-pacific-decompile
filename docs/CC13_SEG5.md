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

## Camera shake, track searches and path evaluation (cc13-x87)

These bodies were skipped by the first pass because of their x87 code. Each was traced from
the bytes (`bsp.py disasm-raw`) with the x87 stack depth written down at every step, and every
DC/DE register form was checked with `tools/x87_dc_form_scan.py --forms dc,de,d8` (the scan
agrees with the trace at every site). Status: reconstructed, Win32 build-tested; not
ABI-compatible, not game-validated. Names are hypotheses.

**Precision model.** A value the image keeps in an x87 register is a `double` in the C++ and is
rounded to float exactly where the image stores it (FSTP dword). That is the 53- and 64-bit
result up to double rounding. A single operation on two floats followed by a float store is
written as a float operation; it is correctly rounded in every mode. The main thread runs at
24-bit precision control (docs/CC13_FPU_PRECISION.md). At 24 bits every register operation also
rounds its significand to 24 bits, so the "kept in ST" intermediates listed below can differ in
the last place. FCOMI/FCOMIP/COMISS branch senses were read from the flags: `JA`/`JB` are not
taken on an unordered compare and `JBE` is, so a NaN takes the path stated per row.

**Shake channel** (70h bytes, the value of a 78h-byte `std::list` node at +8): +00h start,
+04h end, +08h key spacing, +0Ch byte frozen, +10h/+14h/+18h amplitude, +1Ch five keys
{x, y, z, time}, +6Ch byte. Constants: 0.25f (00CE3868), -0.0f (00D7A208, the SUBSS minuend of
each `-amplitude`), 4.0 (00D7A328, double), 1000000.0 (00CE4C08, double), 0.1f widened to double
(00D7A3A0), 0.5 (00D7A280, double), 1.0f (00D7A24C).

| Address | C++ | Behaviour and rounding points |
|---|---|---|
| 00454970 | `shake_key_copy_00454970` | Four FLD/FSTP float copies; returns ECX. |
| 00455880 | `shake_channel_copy_construct_00455880` | Floats +0/+4/+8, byte +0Ch, floats +10h..+18h, the five keys through 00453ED0 (the CRT `vector copy constructor iterator`, size 10h, count 5, 00454970; called directly here five times since the copy cannot throw), byte +6Ch. 00456710 now calls it directly. |
| 00454A50 | `shake_channel_shift_and_redraw_keys_00454a50` | RET 0Ch. Shift > 0: keys 4 down to `shift` take key - shift; shift < 0: keys 0 up to 4 + shift. Backward (forward byte 0) from index - 1: the next key's time is copied to a double slot, `next - r * spacing` is kept in ST and stored (one rounding point), r = 00BD2F10(ECX 1, 0.25, 1.0); the key is expired when `time <= 0` (FLDZ/FCOMIP/JB; NaN is not expired); each component is 0 when frozen or expired, else 00BD2F10(1, -0.0f - amp, amp). Forward from index + 1: `r * spacing + previous time` kept in ST, stored; components drawn unless frozen. |
| 00454EF0 | `shake_channel_evaluate_00454ef0` | RET 8, returns out. rel = time - start (stored). Frozen or `0 > rel` (COMISS; NaN carries on): out = 00F87574. window = spacing * 4.0 kept in ST for both tests: `key0.time - window > rel` or `rel > key4.time + window` (both kept in ST) re-anchors key 2 (time = rel, three fresh draws, no frozen test) and redraws both sides (00454A50(2,1,0), (2,0,0)); `key0.time > rel` slides by three (00454A50(3,0,3)) and `rel > key4.time` the other way ((1,1,-3)), each followed by a recursive call into a local. Else i = first key from 1 with time > rel (5 when none, as for NaN); f = (rel - t[i-1]) / (t[i] - t[i-1]) with both differences kept in ST, stored; clamped by 00415690 (`bsp::unit_clamp_00415690`) to [0, 1]; out = float(prev * (1 - f)) + float(next * f), every product, `1 - f` and sum stored. |
| 00456420 | `shake_channel_construct_00456420` | RET 14h. end = time + 1000000.0 (kept in ST, stored). frozen = `0.1 > float((y*y + x*x) + z*z)` (sum kept in ST, stored, compared with the double); amplitude = 00F87574 when frozen, else (x, y, z); key 0 = (00F87574, time 0); 00454A50(0, 1, 0). |
| 004569E0 | `shake_list_evaluate_004569e0` | RET 8. Empty list: 00F87574. Sum starts at 00F87574; channels in start order until one starts after `time`. to_end = (end + 0.5) - time (kept in ST, stored), from_start = time - start (stored); nearer = to_end unless to_end > from_start (FCOMIP/JBE: NaN keeps to_end); ramp = nearer + nearer (stored). Skipped when `0 > ramp`; weight 1 when ramp > 1, else ramp, skipped unless ramp > 0 (NaN skipped). sum += float(offset * weight), products and sums stored. |
| 0045A320 | `shake_list_add_0045a320` | RET 14h. Builds the channel on the stack (00456420). A queued channel with \|time - start\| < 0.1, \|spacing difference\| < 0.1 and squared amplitude difference ((dy*dy + dx*dx) + dz*dz, kept in ST, stored) < 0.1 returns without inserting; the first channel starting after `time` gets the new node in front (00457F30, 00459AE0, links). Otherwise the last channel's end (+0Ch of the node) is set to the new start and the node is appended. |
| 0045ACC0 | `camera_track_update_shakes_0045acc0` | No-op when +1D4h is 0. Finds "CameraPosition 01" and "CameraLookat 01" through 00925180 on [[00E188A8]+19CCh] (strings built by 0041E870, released inline), time = [[00E188A8]+38h]+4. Each entity's position (00414DB0 first when +C8h is 0) plus 004569E0 on +1DCh (eye) / +1E8h (target), sums stored. up = (sin roll, cos roll, 0) from FCOS/FSIN of +148h, each stored as float (std::cos/std::sin of the widened float here). 00B63F10 then 00B71490 on +1D4h. FOV = 0045A5A0(time) when the player's +60h is 1 or 2, else 35.0f; radians = fov * pi(float, as double 00CE3D28) / 180.0 kept in ST, stored; 00B6FBB0. |
| 00455BC0 | `entity_track_find_sample_before_00455bc0` | RET 8, returns out. Iterator +100h/+104h over the +0F4h samples moves to the first sample with time (+8) > `time` (FCOMIP/JA: NaN never stops), then back one; 00BF6713 when that lands on the head (no earlier sample). |
| 00455CD0 | `entity_track_find_nearest_sample_00455cd0` | RET 4. best = 999999.875f (00CE4C00); \|float(time_n - time)\| (stored, FABS, stored); strict `best > d` (NaN never wins). Returns node + 8. With no hit, 00BF6713 and then the image reads [0 + 4] (kept as a volatile read). No callers. |
| 00455E30 | `entity_track_find_action_before_00455e30` | As 00455BC0 over +108h/+114h/+118h, comparing the action's +4. |
| 00455F50 | `entity_track_find_nearest_action_00455f50` | RET 4. Null for an empty list. best = 9999.0f (00CE4C04), the action with the smallest \|time_n - time\| below it. The result register starts as `MOV EBP,[ESP+1Ch]`, the bits of `time`, so with no hit those bits come back as the pointer. No callers. |
| 00454340 / 004552C0 / 004552F0 | `list_iterator_post_decrement_00454340` etc. | Checked list iterator post-decrement, dereference and post-increment, each test calling 00BF6713. |
| 00458160 | `entity_track_last_sample_time_00458160` | ST0 result: 0 for an empty list, else the time of a copy (00457FF0) of the last record, released inline. Exact as a float. |
| 004581E0 | `entity_track_eval_position_004581e0` | See below. 00458A70 and 00458D50 (seven call sites) now reach it directly. |
| 004564E0 | `action_list_find_before_004564e0` | RET 0Ch. As 00455E30 over a caller's list<Action*>; ECX unused. |
| 0045A5A0 | `camera_track_fov_at_0045a5a0` | See below. |

**004581E0** (`__thiscall(track, out, time, char exact)`, RET 0Ch). Four 38h-byte records are
built by 004658D0(0) in the order a (ESP+4Ch), b (+84h), c (+0F4h), d (+0BCh) and released in
the order d, c, b, a (00456D10 on the state-2 exit, inline pool returns elsewhere). Unless
`exact`: `time + 0.04 >= +190h` (0.04f widened, 00CE4D68; sum kept in ST; NaN skips) moves state
1 to 2 and clears state 3 (+154h = 0, +14Ch = 0). With a state and not `exact`: state 2 returns
the offset +1C8h; states 1 and 3 interpolate a = +158h to b = +190h (00458050), with c and d left
as constructed. Otherwise: one sample returns its position; `time > last` (00458160; FCOMI/JBE,
NaN searches) returns a copy of the last record's position (the report runs twice on an empty
list); else 00455BC0 gives a (the report runs twice more when no sample precedes `time`); with no next sample
the result is a's position, else the next sample gives b, the
previous one c (a when a is first) and the one after b gives d (b when b is last), through the
iterator helpers. Interpolation: u = ((time - a.t) / 100, kept in ST) / float((b.t - a.t) / 100),
stored. Rolls (+18h, degrees) become float((r / 180) * pi) each; +148h = float(0.5 * (2p1 +
(p2 - p0)u + (2p0 - 5p1 + 4p2 - p3)u^2 + (3p1 - p0 - 3p2 + p3)u^3)) evaluated entirely in ST in
the image's order. Position: cubic Hermite with h11 = u^3 - u^2, h10 = (u^3 - 2u^2) + u,
h01 = 3u^2 - 2u^3, h00 = (2u^3 - 3u^2) + 1, where u^2 = u*u, u^3 = u*u^2, 2u^2 = (2u)u,
3u^2 = (3u)u, 2u^3 = ((2u)u)u are kept in ST and each h is stored; the tangent terms are
(b - b[+1Ch]) * h11 and (a - a[+28h]) * h10 (differences and products stored); out =
((a*h00 + b*h01) + (a - a28)*h10) + (b - b1C)*h11, every product and sum stored.

**0045A5A0** (`__thiscall(track, time)`, RET 4, ST0 result from a stored float). A temporary
list<Action*> (head 004555B0) gets four 35-degree FOV actions (00466020) at 0, 0.01f
(00D7A238), now - 0.01 (now = [[00E188A8]+38h]+0; the double 00D7A358 subtracted in ST,
stored) and now, with every type-12h action of the track between the second and third and
every type-16h action twice when `time > ActivateTime` (FCOMIP/JBE: NaN skips). Fewer than five
entries: the first entry's FOV, the actions destroyed (00465610 x4) and the list cleared inline
(00BF65AC per node, then the head). Else `0.02 > time` (00CE4D88, double): the first entry's
FOV; `0.02 + time > last sample time` (sum kept in ST): the last entry's FOV; else 004564E0 gives
the node, and the FOV is its action's when it is first or last; otherwise a Catmull-Rom between
the node's action p1 and the next one p2 whose outer control values are read from the same two
nodes ([ESI+8] and [EBP+8], so (p1, p1, p2, p2)): u as in 004581E0 (100.0 double), the curve
kept entirely in ST and stored once. These exits destroy the actions and call 00456670.

## Uncertainties and simplifications

- cc13-x87: the double model above differs from the main thread's 24-bit mode in the last
  place where an intermediate is kept in ST. FCOS/FSIN are modelled by std::cos/std::sin of the
  widened float, rounded to float. 00455CD0 and 00455F50 have no callers in the image.

- The checked-iterator and vector-range tests that call 00BF6713 repeat the loop
  conditions on every path reconstructed by cc13-seg5 and are omitted; the cc13-x87 bodies call
  the hook wherever such a test can fire on a valid list.
- 00464910 writes through a null member for an index past 4, as the binary does.
- Vtable pointer stores are written as the image addresses (00CE53AC, 00CE4E04, 00CE4DFC).
- 00458050 copies floats through FLD/FSTP; a signalling NaN would be quietened by the binary
  and is copied unchanged here.

## Belongs to other lanes

None of the bodies here are per-frame unit, ship or plane behaviour; the run bodies are
mission-script actions applied once when the movie reaches their ActivateTime.

# cc13-bigB1: three large leftovers as whole bodies

Packet cc13-bigB1 (lead: team-lead), from the large-leftover list of `docs/CC13_SWEEPB10.md`.
Every name is a hypothesis, not a recovered symbol. Source `src/cc13_bigB1.cpp`, header
`include/bsp/cc13_bigB1.hpp` (`Cc13BigB1Host` extends `Cc13SweepB9Host`). Nothing is wired.
Status of each body: reconstructed, build-tested only (no fixture, no game run).

Method: each body was paged from the Ghidra listing and anchored on ESP after every push
(and on the SEH state stores for 0060ABD0); x87 operand direction comes from the bytes
(`tools/x87_dc_form_scan.py`), constants from the PE on disk. Both bodies run on the main
thread, so every x87 operation is modelled as rounding to float in the image's order (rule 4c;
an operation with a double constant is `pc24(double op double)`).

## 1. 007A91A0 `BSP_MovCam_Step`

`__thiscall(ECX = camera+170h, float dt)`, RET 4: slot +8h of the +170h sub-object vtable
00D04B1C. Every callee gets ECX = EBP-170h, so the C++ (`cc13_bigB1_movcam_step_007a91a0`)
takes the MovCam object and uses camera-relative offsets. No SEH frame, no 00BD2F10 draws.

DC forms: 007AA6DF is `DC C9` (ST1 = ST1*ST0), so the arrival radius is
`R = top^2 / accel * 0.5` and the 0.5 stays on the stack for the mode-3 turn radius
`0.5 * turn_top^2 / turn_accel`; the pseudocode's `local_98` is that R. 007AA8A4 is `DC E9`
(h - 100). The others are squares.

Order of the body:

1. Latch copy `+51Ch..+537h -> +500h..+51Bh` (REP MOVSD, 7 dwords), then the edge bytes
   `+51Ch..+51Fh` and `+534h..+536h` are cleared. Actions `+518h/+519h` call 007A84D0(1).
2. Action `+503h` once the timer `+494h <= 0` (a NaN timer skips) in modes 1/2: `+494h = 1`,
   007A8DA0 (`cc13_sweepB4_movcam_step_deck_walk_007a8da0`), and in mode 1 an even sub-mode
   counter `+388h` steps to odd. Otherwise, with a secondary target `+390h` and a look input
   (`max(|+50Ch|, |+510h|) > 0`), modes 1/2 call 00797070(0, 0).
3. `+500h` forces mode 1, else `+501h` mode 2, each followed by 007A5110(0)
   (`cc13_sweepB6_movcam_enter_mode_007a5110`).
4. `+51Ah` toggles the editor byte `+538h` (forced back to 0 outside mode 2) and shows or hides
   the overlay widget `[[+4D0h]+28h]` through vtable +34h.
5. Mode 1 sub-mode counter `+388h`: below 2, `+502h` adds 2, sets `+498h = 1`, and drops to 2
   when the 00794910 aim delta's 00414C60 length is under pi/4 (00CEDCD0); at 2 or more `+502h`
   resets it to 0. Without `+502h` the parity steps: even, `+498h <= 0`, a look input and no
   stick input -> `+3E4h = 0`, odd; odd, `+494h <= 0` and a stick input -> even.
6. `+538h` set: 007A01F0.
7. Interpolator list (`+4D4h` checked list, head `+4D8h`, size `+4DCh`, payload node+8h):
   clock `+14h += dt`, 007901D0(record, camera), and a record past `+10h + +0Ch` is freed, then
   its node unlinked and freed (two `operator delete` 00BF65AC calls, size - 1). The 00BF6713
   checked-iterator tests are kept at their points.
8. Clears `+3ACh..+3B4h`, `+3D4h/+3D8h`; `+494h -= dt`, `+498h -= dt`. Reference velocity
   `+3B8h` = target `+38Ch` vtable +34h unless there is no target, `[00E188A8]+634h` is set,
   the mode is 2, or mode 1 is in sub-mode >= 2 (then zero).
9. Reference turn rates `+3DCh/+3E0h`: with the look entity (the secondary target, else the
   target in mode 1 even or mode 4), 00795300's rows 1 and 0 dotted with `v - vref`, times
   `1/|d|` (`-row1/|d|` and `row0/|d|`); zero otherwise.
10. Modes (`+384h`):
    - 0 free: 00794D60(+510h, -+50Ch, 1, 1, 0), 00794B80(+514h * mover row +94h, 1, 1),
      007943D0(dt), 00790540.
    - 1 even (targeted): ratio `|d| / ([[target+538h]+A0h] * 4) + 0.25` capped at 1;
      00794910(out, target, [+4D0h]+54h) and 00794F70(-out); with a stick input the orbit
      direction `(-+504h)*row0 + (-+508h)*row1 + +514h*n` from 00795300, else `+514h*n`, both
      times the ratio, into 00794B80(.., 1, 1); then the orbit term
      `|q x n|^2 / |d|` (q = v - vref + a*dt, 004F9B30) along `n` is added to `+3ACh`.
    - 1 odd (free): 00794B50 toward `+390h` into 00794D60(.., 0, +390h), else the stick; then
      the forward steer as mode 0. Both halves end with 007943D0, 00790540 and
      0079DA90(+390h, 0.1).
    - 2 deck: overlay colour (vtable +50h) yellow `{1,1,0,1}`, or with the editor open green
      `{0,1,0,1}` / red `{1,0,0,1}` for 0078E2B0 on the vehicle camera 005CD820(+388h) element
      +48h at (+3C8h, +3C4h); a stick push over 0.5 latched in `+490h` opens the next vehicle
      camera when the target has more than one (0079F9E0 with the editor, else 007A87B0);
      007A0DD0 / 007A8C60 / 00794B50 toward `+390h` or the stick, divided by the zoom `+460h`,
      into 00794D60; 007943D0; the angles go to the map `[+4D0h]+70h` (007A4E10, 00418170);
      position = `+A4h`; 0079DA90.
    - 3/4/6 orbit: mode 6 first builds the point `+408h` (target frame row 2 * `+488h` *
      `[+4D0h]+54h` / zoom + target position, pushed `+488h` away from `+390h`, raised by
      `+48Ch`). Mode 3 turns to pitch -pi/2 by the stick and stores `+418h = -pi/2`,
      `+41Ch = sign(+50Ch) * turn radius` (0064A740); modes 4/6 turn toward the look entity
      (00794B50) and store the offset, or keep the stored angles. Mode 3 with input flies the
      mover basis (rows +74h/+84h/+94h), the throttle scaled down near the height bounds
      (200 -> 00415550, 2900 -> 00415510), clamps the stopping point to 100/3000 (vertical
      acceleration `+3B0h` from 00415510), writes the height to `[[00E198C4]+98h]+5Ch` and moves
      the reference `+3F0h` (00793CD0). Otherwise it approaches the reference with the speed cap
      `R*4` and, beyond R, bends an acceleration that opposes the velocity
      (`v.a < -(accel*top)/4`) across it at the same size (0042B220, 0042AEE0). Then 007943D0,
      00790540, 0079DA90(+390h, 0.1), and 0079D960 on the resolved reference with threshold R.
    - 5 path: 007972F0 with a knot path (`+474h`), else 007B04C0 + 004142A0 + 00794070; arrival
      0079D960(position, 1.0) once `|+46Ch - +468h|` is under the top speed.
    - 7 fixed: position `+4C4h`, look point `+4B8h`, each through 0042D7E0 + 00414D10 when its
      entity exists; 00794070, 00790540.
11. Tail: the sub-object's vtable +0Ch (`movcam_controller_vcall_0c`).

Calls reused directly: 007A8DA0 (sweepB4), 007A5110 (sweepB6), 007A8C60 (sweepB2), 00794B50
(sweepB), 00414C60, 004F9B30, 0042B220, 0042AEE0, 00415510, 00415550. The typed
reconstructions of 00794B80 / 00794D60 / 00794F70 / 007943D0 / 00795300 / 0079D960 / 0079DA90
take structs, so those calls go through hooks.

Uncertainty: 0042B2F0's ST0 arrives through a double hook, so `1/len` and `cs/len` round
double -> float (a double-rounding difference is possible in halfway cases). 007A84D0 and
007A87B0 have no reconstruction yet.

## 2. 0060ABD0 `BSP_HudPlaneEffectsScreen_Update`

`__thiscall(ECX = screen, float dt)`, RET 4; slot +20h of vtable 00CF43AC. SEH frame (handler
00C77557; state 0/1/2/4 around the three effect references, 3 around the stack hit record),
not modelled. C++ `cc13_bigB1_hud_plane_effects_update_0060abd0`.

`docs/CC13_FE_7.md` deferred this body pending an ownership decision with cc9; the lead assigned
it to this packet. It is reconstructed here and not wired; it does not touch cc9's host files.

**Shared RNG:** two draws of 00BD2F10 with ECX = 1 (stream 1), at 0060B0CF and 0060B101, each
`uniform(-0.01, 0.01)` added to the chase camera `+94h`'s `+3B4h` / `+3B8h` while the unit's
`+9F8h` is set.

DC forms: 0060B133 and 0060B1A2 (`DC C9`: the row-2 x term is `M8 * 5000` / `M8 * 50`, the
constant stays for y and z), 0060BCEC (square), 0060BF35 (`FADD ST0,ST0`, throttle + throttle).

Order of the body:

1. No unit `+B8h`: 006485A0 on `[00E198C4]+40h`, return. 007C3CB0(dt); with `+98h` and unit
   vtable +1C0h, 007C7800(+98h). Local pilot (00927F30(1)) with `[00E188A8]+19C4h`: the axis
   filters `+E8h/+ECh` are cleared and 0077C470(2, 0). No local input (007BB9A0): 00647300 on
   the unit's `+9D4h`, 004CC460(24h, unit), return. Flight states other than 7/6/3 (or 4 with
   `+904h` clear): with `+BF8h` and `+BF4h`, spectate `[[+BF4h]+4]+7Ch` if 00645160(.., 1)
   accepts it, else 006485A0; return.
2. Action DFh (004C43C0) without 0047B850, in cockpit view or with byte 00F88A32, toggles
   `+FCh` and calls 00608280. 006068A0. No unit: `+4 = 0`, return.
3. Unit `+5Dh` clear: `+7Ch -= dt`; effect `+6Ch` (definition `+68h`, created with the
   identity matrix under the unit's `+4A4h`, 00440490 + 008687C0 + 004845D0 + 00440A30) while
   00604A50 and `+9F8h`, stopped (00867B10, byte +9 = 1, 00484620(0)) when `+9F8h` is clear;
   effect `+70h` likewise (definition `+78h` when `[[unit+538h]+144h]`, else `+74h`); the two
   RNG draws; the sweep probe: the contact list on the stack (count `+C0h` = 0) gets the
   segment from 50 to 5000 ahead along the frame's row 2 (0042B570), 00470470, 0098B370 on the
   0042E630 index (a hit calls unit vtable +38h, ST0 dropped); speed `|v| * 2.412` above 200
   keeps effect `+64h` (definition `+60h`, created at translation (0, 0, 40) from 0047C240) and
   aims it along `-v` (+5Ch..+64h, +28h = 0), otherwise stops it; 004704B0. Unit `+5Dh` set:
   effect `+64h` stopped.
4. Free look on action A0h (held per 004C5090). Starting latches `+9Ch` and clears `+A0h/+A4h`.
   On release 0042AA00(+98h, 0, 0, 0) runs and the chase camera's `+3E8h/+3E4h` are cleared. While latched,
   the commanded target (`[unit+9D4h]` vtable +114h, 0071EB60, 00521EA0):
   - chase camera `+94h` without `+BDh`: a ship (kind 5) or carrier group (18h, replaced by its
     `+3D0h`) is tracked (00427EB0 x2, 00427C90, 0042B260, 00414E10, 0042D0D0, 00521370):
     `+3E8h = -pitch`, `+3E4h = yaw`; otherwise the hold times B0h/B1h accumulate into
     `+A0h/+A4h` (clamped to +-pi and +-pi/2 by 00415690) and A1h/A2h add `pi` / `pi/2`
     scaled offsets; an ancestor (00923810) floors the pitch at 0 and below 50 altitude at
     `-pi/2 * alt/50` (00415550).
   - otherwise (no chase camera, or `+BDh`), with `+98h` and unit vtable +1C0h: cockpit look
     with the tuning limits `+A0h` (yaw), `+A8h` (up), `+A4h` (down): toward the target as above,
     or from the hold times; clamped and sent through 0042AA00(+98h, yaw, pitch, 0).
5. Pilot command block on the stack (`[ESP0+4Ch]`, 18h bytes; field names are hypotheses):
   axes `[input+4]+20C4h/20F4h` (zeroed in free look when the 00E188D8 party is its own
   00927880) and `+1F14h/+1F44h` through the filters 006062B0 (`+E8h`, `+ECh`); 007B9140(0)
   (result unused); cockpit scales tuning `+680h/+67Ch`. The throttle latch 00E19694 (set at
   priority level >= 2, released when A3h reads exactly 0), 00A92050 / 00A92090 / 00606250
   select absolute or incremental throttle; a moved absolute throttle ends the `+10Ch` hold
   (0077C470(2, 1)). The rudder A4h (cut by the HUD state 006529E0 / 00608C00), a digital stick
   keeping only its larger axis, the throttle target in `[ESP0+18h]` (incremental
   `+9F0h^2 + delta*dt`, absolute `-0.5*thr + 0.5` or 00419010(-0.5, 0, 1, 1, -thr)), a
   negative target braking (`+10h`), then `+0Ch = sqrt` (00BF7030) and byte `+14h` = held AAh.
   Any of the actions AEh, AFh, A5h, A6h, A3h, AAh, A4h, A9h, A8h, A7h pressed by a non-pilot
   local player calls 0077C470(2, 1) and clears `+10Ch`. Pilot: 006082D0(block); else `+BCh`
   and block byte `+16h` are cleared. Pilot: the five clamps (00415690; the elevator floor from
   00419010(0, -1, -tuning+2B8h, 0, unit+C64h) below 5 altitude) and 007B8C90(block).
6. Bomb timer `+CCh` (00D7A218 0.0): on expiry with `+E4h`, a bomber (kind 10h) with a loaded
   bomb class (007B9510, +5Ch) and more than 0.7 rounds (007BC8C0) re-arms through 0042ECA0
   (`+D0h`) at 0.01; otherwise `+E4h` vtable +108h's +38h, with both HUD panels closed,
   pushes request 26h (004CC460), else the timer restarts from 00424C40 `+460h`.
7. 00609BD0([00E198C4]+68h, dt, +BDh, unit +C24h saved at entry).
8. FOV with a unit, `[00E188A8]+19FCh` and `dt > 0`: unit vtable +204h into the 16-slot ring
   `+1Ch` (index `+5Ch`); with `+9F8h` the mean, clamped (004155B0) to tuning `+B8h..+BCh`,
   picks the zoom through 00419010(+B8h, +C0h, +BCh, +C4h); `+84h` steps toward zoom * FOV
   (0042AC60), `+80h += (+84h - +80h) * dt`, 00B6FBB0, and the chase camera's
   `+384h = tan(+80h/2) / tan(fov/2)` (00BFA020 is `_CItan`, the other caller being math_tan).
   The image also computes `cos((mean - lo) * pi / (hi - lo))` into a slot that is overwritten
   before any read; the C++ computes the quotient and drops it.

Uncertainty: the hook return types of the ST0 callees (004C5070, 006062B0, 007BC8C0, unit
vtable +204h) are float; an extended-precision ST0 that is not a float would round once more
here. The pilot command block field names are guesses.

## 3. 00822C20 `BSP_UnitInstance_SEntityInit`

`__thiscall(ECX = unit)`, RET; slot A0h of the game-unit vtables (00CF9150, 00CFA818, 00CFB7D8,
00CFC470, 00D09718), called from BSP_SEntity_InitAll's slot call 00926110 and by the four
overrides 0074BEC0, 007593D0, 00853630, 00857C80. EBP frame aligned to 8, SEH (handler 00C917F1,
states 0..12h around the temporary references), not modelled. C++
`cc13_bigB1_unit_sentity_init_00822c20`. No 00BD2F10 draws; no x87 DC forms in the body.

The image inlines every retained-reference copy, assign and release: count at +4h through the
imports InterlockedIncrement 00CE221C / InterlockedDecrement 00CE2220, and a count reaching 0
calls vtable slot 0. The C++ keeps those calls at their points through the sweepB hooks
`interlocked_increment_00ce221c`, `interlocked_decrement_00ce2220` and
`refcounted_vcall_00_destroy`. The cached world matrix refresh (+C8h valid, +CCh from +74h and the
parent +3Ch through 00414DB0 / 00413920 / 004134F0, then +10Ch = 0) is inlined at each use and
modelled once as a local helper called at every point.

Order of the body:

1. `+BC4h = 0`, `+9C0h = class(+538h)+500h`, colour defaults `+1160h..+1168h = 0.5`,
   `+116Ch = 1.0`, `+9C4h = 1`; 00822B70(1); unit vtable +10h; 00955420.
2. Four effects under the node `+4A4h` (008687C0 with an identity matrix translated to
   `z = -A0h/2, 0, +A0h/2, -A0h/2`): class `+E8h -> +BACh`, `+108h -> +BB0h`, `+100h -> +BA4h`,
   `+FCh -> +BA8h`; each created one gets its "Sound" child (0041E870, 00866CD0, 006FBEB0 into
   `+BBCh/+BC0h/+BB4h/+BB8h`, the name returned to the sized pool 00419CC0 / 00BD1510).
3. `+1114h` resized (00822460) to the count of 30h-byte records at `[+354h]+1Ch`; 00818EA0;
   `+104Ch = -1`, `+1130h = 0`; unit vtable +230h; `+1174h = 0`.
4. Scene record `+C0h`: kind 1 reads the property bag `[+C0h]+8` (008F2260): ShipYardLaunch byte,
   StartSpeed (throttle `speed / 0080FC30` to 0080D9B0 on `+838h`, `0080FC30 * throttle` to
   0092D770 on `+1018h`), CamoColor `+115Ch`, CamoColorGun's R/G/B/A into `+1160h..+116Ch`
   (no null test on the sub-records) or the defaults; kind 2 copies the authored record's
   `+C4h` byte to `+58h`, `+11Ch` (launch byte) and `+128h` to `+115Ch`.
5. `+9D8h = +9DCh = 1.0`, `+9E4h/+9E5h = 0`, `+9E0h = 0`, `+1130h = launch ? 5 : 0`,
   `+10CCh = class+660h`, `+10C8h = unit`, 0081E350(+10B8h, class+664h); 0074F490(+10D4h, unit,
   (int)settings+408h).
6. With a scene record: the part list `[[+360h]+160h]+7Ch` (checked vector of 8-byte
   entries) is scanned with 00711C30 + 00814870 for a name starting with "waterclip" (value to
   `+1170h`; the temporary string is freed when its capacity is 10h or more). Kind 2: unit
   vtable +128h(authored +B8h) and the director `[+738h]` vtable +3Ch(authored +110h). Kind 1:
   skill (00927A80 -> vtable +128h); Navigator false clears `[+740h]+11h`; the director enables
   ArtilleryDirector, TorpedoDirector (an explicit false asks unit vtable +5Ch(8), the
   submarine class), AADirector, DCDirector, FireStance (negative -> 1), PlayerCommandsEnabled,
   TorpedoEvasion, ShipCollAvoid, LandCollAvoid (all default true) and NavigatorAllowMaxDepth
   (into `[+73Ch]+20h`) go into the 50h-byte message built by 007214C0, with `+4/+5` from
   director vtable +24h/+28h(stance), `+8 = 1FFh` when player commands are enabled, `+48h = 0`,
   `+4Ch = unit+980h`, sent through director vtable +3Ch.
7. A ship node (`[+30h]+4A4h`) copies class `+594h..+5A8h` to `+1054h..+1068h`, binds screws and
   rudders (008346E0) and runs `+310h` vtable +0Ch. A non-ship node runs 00834690 instead. `+1134h..+1148h = 0`,
   `+1134h = (int)settings+46Ch`, `+114Ch = +1158h = 0`.
8. Entity paths: per class path record (`+6CCh`, 10h bytes), a 19Ch block (00BF55BE + memset),
   0080D690, vtable +98h(unit, +30h, identity), its 0Ch-byte points through `path+170h` vtable
   +8h (checked indexing kept), pushed into `+1178h` (inline or 0081DA30) and started (vtable
   +A0h).
9. Units without `+5Dh` and with class records at `+6E0h` (50h bytes) or `+700h` (1Ch bytes):
   004A6830 on `[[00E188A8]+21D0h]`.
10. Effects: bow pair `+9F0h/+9F4h` from class `+5ACh` with class matrices `+5B0h/+5F0h` when
    `+5E0h..+5E8h` is not zero; wake `+9E8h` from `+630h` in the effect system
    `[[00E188A8]+19ECh]` (00868420): with a positive body-axis speed (0092D730) `+54h = speed`,
    `+50h = 1/(modifier * +9C0h)` (008E6430 kind 4 when 00E0C978 and `[00F88C30]+B8h`), placed
    `10*speed` and `9*speed` behind along the frame's row 2 and at the position (004842C0) and
    stepped by 0.1, 1.0 and 9.0 (00867790); stern wake `+9ECh` from `+634h` (`+50h` written
    without a null test, stepped by 10.0 at positive speed); four effects `+A00h..+A0Ch` from
    `+65Ch`; one burst per class point record (`+678h`, count `+67Ch`) at its translation, kept in
    `+A14h` (0074D780) and indexed into the first of the 20 controller lists
    (`[+1018h]+9Ch + k*20h`) whose class id list (`+148h + k*30h`) holds the record id.

Existing fragment models: `docs/CRUISE_SPEED_SETTING.md` (StartSpeed arm),
`docs/SCENE_START_SPEED_FLOAT_PAYLOAD_CC11.md`, `docs/SCENE_DIRECTOR_BOOL_PAYLOAD_CC11.md`,
`docs/TORPEDO_DIRECTOR_CLASS8_CC11.md` describe the same read points; the whole body here agrees
with their stated rules (divide-then-multiply StartSpeed, default-true director bytes, the
class-8 torpedo exception) but their host code was not compared line by line and was not
edited.

Uncertainty: the ST0 callees (0080FC30, 0092D730, 008E6430) come through double hooks; the
StartSpeed quotient and product round once to float as the image's FSTP does. The 50h-byte
message and 1Ch-byte string sizes are taken from the stack layout.

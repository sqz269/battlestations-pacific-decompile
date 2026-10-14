# Segment 42 session messages (cc13-seg42)

Range 00758F90-0076A970. Code: `include/bsp/session_message_cc13_seg42.hpp`,
`src/session_message_cc13_seg42.cpp`, namespace `bsp::seg42`. Status of every
function listed here: reconstructed and build-tested only. New C++ interfaces,
not ABI-compatible, not fixture-tested, not wired into a host, not
game-validated. No game behaviour changes: nothing in the game build calls this
file, so no `kXxxBound` switch is needed.

Class names follow the MT_* name table at 00E0AB68 that the Ghidra names cite.
They are hypotheses, not recovered symbols. Field names describe offset and
storage; where a meaning is given in a comment it is an inference from the
class name.

## Shared conventions

- Head: `NativeSessionMessageStorage` (native_session_message_base.hpp), 18h
  bytes: vtable +0, delivery +4, +8, +0Ch, type byte +10h, selected local slot
  record +14h.
- Entity header (`HeaderedMessage`): 12-bit id WORD +18h, relay byte +1Ah, the
  same fields 0075B480/0075B4C0 code. Category-49h classes add a byte +1Ch that
  every writer sends as a bool (`UnitMessage`).
- Construct: the native inlines 0075B430 (delivery 3, +8/+0Ch zero, base
  vtable 00D02C68, type, slot record game+18CCh[game+18ECh] when the index is
  0..7, else 0). The C++ calls `construct_native_session_message_0075b430` and
  then stores the header zeros, the final delivery and the class vtable. The
  native stores are in a slightly different order on some classes (for example
  00760680 stores +18h/+19h before +14h in one branch); every store lands on
  fresh storage before anything reads it, so the order is not observable.
- ConstructForRead: zeroes +8, +0Ch, +14h, the type byte, the id WORD and relay
  byte, then delivery and vtable. It does not read the session.
- IsType (slot +0Ch): `t == class || [t == category] || t == 46h`, RET 4 with
  ECX ignored. Four classes (33h, C2h, D7h, E5h/E6h) compare with
  `this->type_10` instead of a category. The single-type tests (2Ah..32h, DEh,
  E2h, E7h, E8h, 2Ch, 29h) are `XOR EAX,EAX / CMP / SETZ AL`.
- Write (slot +4): __thiscall(this, cursor) RET 4. Read (slot +8):
  __thiscall(this, stream) RET 4, bit readers get stream+4. Field order and
  widths follow the listings exactly; the C++ uses the existing bit-cursor
  helpers (00428C70.., 00428FF0.., 004293F0, 004295C0, 004294F0, 00429790,
  00429AC0, 00429F20, 00428E50, 00429140, 00429B00, 0042A480).
- Numeric floats use (zero 0, signed 1, scale [00D7A248] = FLT_MAX, 32 bits)
  unless noted. The native routes values and the scale through FLD/FSTP, which
  quiets a signalling NaN; the C++ passes raw bits (MINOR, NaN inputs only).
- Vtable stores go through `VtableBinding::vtable(image_address)`, a named host
  hook called at the point the native stores the constant. Most slot functions
  of these tables lie outside the segment.

## Reconstructed

ConstructForRead (vtable): 0075A2E0 FORMATION_JOIN (00D02D30), 0075A330
FORMATION_LEAVE (00D02D44), 0075A380 FORMATION_SET (00CED548, also +28h/+2Ch),
0075A4D0 KILL_BULLET (00D02D80, no +1Ch), 0075A550 GUN_SET_FIRE_MODE
(00CFDE3C), 0075A580 R_T_GUN_SET_CONTFIRING (00CFDE50), 0075A5C0 S_T_GUN_FIRE
(00CFDE8C), 0075A630 SHIP_SYNC (00D02D94, delivery 0), 0075A690
SHIP_HELMSMAN_CONTROL (00D02DA8, delivery 0, limits +28h/+30h = 2.0f from
00CE3958 and +2Ch/+34h = -2.0f from 00CE7D7C), 0075A840/0075A8B0/0075A920/
0075A990 PATH_SET/ADD/INSERT/DELETE (00D02DBC/DD0/DE4/DF8), 0075AAB0
PLANE_SYNC (00D02E0C, delivery 0, +44h = 1.0f from 00D7A24C; only the fields
the constructor stores are modelled), 0075AB40 PLANE_CONTROLFACTOR (00D02E20,
seven 1.0f at +20h..+38h), 0075ABE0 PLANE_HIT (00D02E34), 0075AC50
PLANE_PCMDLASTTIME (00D02E48), 0075AD30 PLANESQUADRON_ATTACKRUN (00CF3BB8),
0075ADE0 SUBMARINE_DIP_RISE (00CF61FC), 0075AE10 SUBMARINE_DIP_RISE_SYNC
(00D02E70), 0075AEA0 GRND_VEHICLE_SYNC (00CFFD4C, delivery 0), 0075AED0
LANDFORT_EFFECT (00CFF37C), 0075AF00 LANDINGSHIP_STARTLANDING (00CFF908),
0075B220 PLAYERSPAWNPOINT_SETACTIVE (00D02EC0, +20h = 1), 0075B2A0
MOVCAMNEW_ADDPOSITION (00D02ED4, no entity header), 00766F60 net-entity
create base (00D03720), 007676C0 PATH_CREATE (00D0377C), 007679D0
AIRFIELD_CREATE (00CF8EBC, sub-object vtable 00CF8EB8 at +110h), 00767A70
LANDFORT_CREATE (00CFF358, 00CFF354 at +110h), 007683C0
MNETTABLESYNCENTITY_SYNC (00D03824, delivery 0; head node from 006EDEA0 with
isnil +31h = 1 and left/parent/right = itself).

Construct (type, vtable, delivery 1 unless noted): 0075BB30 ENTITY_RELOCATE
(97h, 00D02F24), 0075C680 PLAYERINFO (E3h, 00D02FA8, +24h = -1), 0075E4D0
MOVCAM_BLACKBAR (34h, 00CF17F8), 0075EA30 MUSICCONTROL_SETLEVEL (2Ah),
0075EB40 BLACKOUT (2Bh), 0075ECB0..0075F540 MOVCAM_* (2Dh..32h), 0075F9D0
VOICECHANGE (E8h), 0075FB00 PLAYERUNLOCKUNIT (DDh, 00CEB138), 0075FB70
SERVERDLCUNLOCK (DEh, zeroes 0FFh WORDs), 0075FC80 FRAGILE_SETHP (D2h),
0075FE90 T_GUN_TURN_SYNC (AEh, delivery 0), 0075FF60 VEHICLE_GUN_CONTROL (79h,
delivery 0), 007601A0/00760200 VEHICLE_UNIT_LAUNCH/ADD_LAUNCHED (7Bh/7Ch),
00760260/00760390 VEHICLE_SET_PARTY/RACE (7Fh/80h), 007604C0
VEHICLE_SHIPYARD_LAUNCH (7Ah), 00760680 HITMARKER (E5h, delivery 0), 007607B0
HITWARNING (E6h, delivery 0, +18h = 3), 007608E0/00760A50/00760B70
SHIPREPAIR_ADDDAMAGE/BODYREPAIR/REPAIRPRIORITY (9Eh/9Fh/A0h), 00760E90
SHIP_LANDINGSHIPSLAUNCHED (95h), 00760FD0 SHIP_TORPEDOGENERATE_HACK (9Dh),
007611D0 SHIP_SET_TORPEDOSTOCK (96h), 00761310 SHIP_WRECKED (98h), 007613B0
SHIP_EXPLODETOPARTS (9Ah), 00761450 SHIP_EXPLODEONEPART (99h), 00761590
SHIP_SECTIONFAILUREEFX (9Bh), 00761F60 SHIP_LEAK (90h), 00762130 SHIP_SINK
(92h), 00762190 SHIP_LEAKCHEATWATER (91h), 00762220 SHIP_ADDTORQUE (93h),
007623D0 SHIP_AVOIDSIDE (8Fh), 007626E0 PLANE_SYNC_ACK (C2h, delivery 0),
00762880 PLANE_CHANGE_STATE (C3h), 00762A00 PLANE_TAKEOFF (C6h), 00762A60
PLANE_TOUCHDOWN (C5h), 00762B20 PLANE_DISAPPEAR (C7h), 00762C80
PLANE_DEADMEATROT (C8h), 00763140..007634A0 AIRBASE_* (81h..88h), 00763500
MOTHERSHIP_ELEVATOR_MOVE (87h), 00763600 BOMBPLATFORM_FIRE (B1h), 00763660
MULTIBOMBPLATFORM_GENERATE (B3h), 007636C0 CATAPULT_FIRE (B4h), 00763B40
PARATROOPER_LANDINGPOS (71h), 00763CB0..00763DD0 SHIPYARD_* (A7h..AAh),
00764220..007642E0 COMMANDBUILDING_SETOWNER/SETLEVEL/UPDATECAPTURE (D3h..D5h),
00764490 HIGHLVLAI_SPAWNBANN (D7h), 007645B0 SHIP_ERASEWRECK (9Ch), 007648B0
WATERMINE_ARM (B5h), 00764960 CAMERA (55h, delivery 0), 00765490
MANAGE_MESSAGES (E2h), 00765610 VOICEOVER (E7h), 00765940 MANAGE_DIALOGS
(E1h), 007663F0 COUNTDOWN (2Ch), 007665D0 METRICSSESSION (E9h), 00766660
COMMANDBUILDING_INFERIORLIST (D6h), 007666C0 SESSION_MANAGER (E4h, list head
from 004C1F50), 007673B0 VOICEDATA (29h, delivery 2).

IsType: 00759E00 (4Ah), 0075A310, 0075A360, 0075A660 (8Ch/48h), 0075A7D0,
0075AB10, 0075ABB0, 0075AC20, 0075AE50, 0075B330, 0075BB90, 0075EAB0,
0075EBC0, 0075ED30, 0075EE70, 0075F080, 0075F220, 0075F3D0, 0075F5C0,
0075FA50, 0075FBF0, 0075FF10, 0075FFD0, 007602C0, 007603F0, 00760520,
007606F0, 00760830, 00760940, 00760AB0, 00760C80 (8Dh/48h), 00760EF0,
00761030, 00761230, 00761370 (98h/49h), 00761410, 007614B0, 007615F0,
00761FC0, 007621F0, 00762280, 00762430, 00762750, 007628F0, 00762AD0
(C5h/49h), 00762B90, 00762CF0, 00763210, 00763BA0, 00764510, 00764610,
00764920, 007654F0, 00765670, 00766450, 00767450, 00768440 (E0h/48h).

Write/Read pairs: every class above with a writer in the segment. Notable
bodies:

- 0075BBD0/0075BC80 ENTITY_RELOCATE: unit header, bools +20h/+21h, then +24h
  float only when +20h, +2Ch float[3] only when +21h, then +28h signed 4.
- 0075B350/0075B380 MOVCAMNEW_ADDPOSITION: type, then the keyframe codec
  00791360 (ECX keyframe, cursor). The reader allocates 104h (00BF681B),
  runs the keyframe constructor 00799B00 on non-null storage and stores the
  result at +18h. 0075B380 then calls 007A2370 with the whole stream (not
  stream+4) and sets +1Ch. Hooks: `MovieKeyframeHooks`.
- 0075FA60/0075FAA0 VOICECHANGE: type, 64 bits from +20h/+24h, bool +18h.
- 0075FFF0/007600D0 VEHICLE_GUN_CONTROL: two angles use scale [00D032E4] =
  6.3f with 18 bits; +38h id only when +36h.
- 00760D80 SHIP_HELMSMAN_CONTROL read: each float is read into a stack
  temporary and clamped from the listing: `lo > v ? lo : (v > hi ? hi : v)`
  with lo/hi = +2Ch/+28h and +34h/+30h. An unordered v is kept (FCOMIP/JBE),
  which differs from the Ghidra pseudocode's NaN path.
- 00760340, 00760880, 00762800, 00760D80: read into the stack argument slot
  and then store. 00428D10 zeroes its output before reading, so the result
  equals a direct read.
- 00762770 PLANE_SYNC_ACK write: count = SAR of the byte span of the vector at
  00F87288/00F8728C (0 when the first pointer is null), capped at 7Fh, 7 bits;
  each element is 12 bits with the checked-iterator test that calls 00BF6713
  and continues. Context: `PlaneSyncAckSource`. The reader reads the count into
  +18h and that many 12-bit ids into +1Ch.., without a bound check.
- 00762E00/00762E90 PLANE_CONTROLFACTOR: three floats with (0, 0, 1.0f, 8).
- 00766460/007664E0 COUNTDOWN: stop flag +28h first; when set only the bool is
  sent; otherwise the owned string +18h and two floats follow.
- 00767460/00767490 VOICEDATA: 00429B00/0042A480 on +18h. The payload is the
  semantic `VoiceDataPayload` (std::vector), so `VoiceDataMessage` is not the
  native layout (native vector at +20h, begin +24h).
- 00767810 PLANE_CREATE write: 00765000 base writer through `CreateWriteHooks`,
  then +110h id 12, three floats, two signed 4, bytes 4 and 3, bool +12Ah,
  then a presence bool and +12Ch in 8 bits when it is non-negative (signed
  test). 00767AD0 TORPEDO_CREATE and 00767C30 PARATROOPER_CREATE run the
  BOMB_CREATE writer 006E0690 first; torpedo +124h uses [00CE3D08] = 100.0f
  with 12 bits.

Other:

- 0075B500 `build_transform_0075b500`: identity (1.0f from 00D7A24C),
  00467050(out, [+30h], [+34h], [+38h]) through a function-pointer hook, the
  translation from +24h..+2Ch, and the basis rows scaled by +104h when +100h
  is set, reloading the scale per row. The x87 products of two floats are
  exact before the float store, so SSE products give the same bits.
- 00762500 scalar deleting destructor of SHIP_SYNC: root vtable 00CE4974,
  `singleton_lifetime_free` when bit 0 is set (the base file's _free bridge).
- Destructors 0075B2D0 (keyframe->vtable[0](1) when +1Ch and +18h), 00765590,
  007656E0, 00766550 (inlined 0041DD20 on +18h), 007674C0 (frees the frame
  vector), 007668E0 and the identical 00767920/00767B40/00767C70/00767D80
  (free +C0h and +4Ch when non-null, zero +4Ch..+54h), 00767740 PATH_CREATE
  (frees +114h, zeroes +114h..+11Ch, then the 007668E0 body inlined), 00768470
  MNETTABLESYNCENTITY_SYNC (0041DD20 on +28h, 006EE960 erase of the whole map,
  free the head, zero +20h/+24h).
- 00763E70: the SHIPYARD_SPAWN target id +28h is looked up in the entity-id
  registry (split [00F89A10], signed compare; rows of 10h bytes at [00F89A54]
  from base [00F89A0C], else [00F89AA8] from [00F89A60]); returns row+0Ch != 0
  in AL. Context: `EntityIdTables`.
- 0076A7B0 `apply_server_info_0076a7b0`: copies the MT 05h fields into the
  session record: +128h/+12Ah from +18h/+19h, strcpy of +1Ah to +100h, +169h,
  +129h, +127h from +3Bh/+3Dh/+3Ch, +16Ch from +40h, +12Dh/+12Ch from
  +45h/+44h, +170h/+174h from +48h/+4Ch, 0Eh dwords +50h.. to +130h...

## Skipped

- 00759020, 007590E0, 007592A0, 00759330, 00766EF0 (MOTHERSHIP_CREATE destruct,
  write, construct, send, construct-for-read) and 00759920
  (LANDINGSHIP_CREATE construct-for-read): they derive from the SHIP_CREATE
  message (0074xxxx), whose layout and constructor are not reconstructed, and
  the mother-ship side belongs to the cc9 carrier lane. A faithful version
  needs SHIP_CREATE first.
- 00758FE0 BSP_MotherShip_DetachFromWorldLists and 007593D0
  BSP_MotherShip_InitAll: behaviour is hosted by cc9 in game_hosts_units.cpp
  (verify-only under the cc9 terms); not reconstructed or recorded here.

## Verification

- `cl /W4 /std:c++17` of the new file is clean; `scripts/build.ps1` builds
  it in bsp_core and both existing CTests pass.
- Sampled listings: 0075EAC0, 0075EAF0, 0075A310, 0075EAB0, 0075B330,
  0075BB30, 0075FA60, 0075B350, 0075B380, 00760D80, 00762770, 00764960,
  00763E70, 00766460, 0075B500, 007656E0, 00767A70, 007676C0, 00768470,
  00767810, 00767AD0, 00767C30. Other bodies were read from the pseudocode,
  whose call order and arguments match the samples.

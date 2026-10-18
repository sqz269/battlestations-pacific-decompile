# cc13-seg48U: undone rows of segments 48 and 40

This packet covers the cc13 lead tree's `local/cc13_undone_seg48_40.txt`: 141
functions with no reconstruction record and no tag. They lie in segment 48
(007D1E50-007F8390) and segment 40 (00728A90-00735EC0). The work runs under the
cc9 gameplay terms (`local/cc13_gameplay_terms.txt`).

Each row was checked against rule 1 with:
- `git grep` over src/include for the address;
- `bsp.py lookup` and `docs-for`;
- the export listing.

The address grep was not enough. It missed `unit_last_damage_state_index`
(`include/bsp/unit_parts.hpp:119-122`, `src/unit_parts.cpp:65`), which implements
0072AB10 and cites the address only in upper case. The first commit reconstructed
0072AB10 a second time, and cc13-seg48V removed that duplicate. Later rows are
grepped in every case form and under likely names.

Nothing is wired.

| Group | Rows | Outcome |
|---|---|---|
| In the struck ranges 007DF000-007DFFFF and 007EE000-007F2FFF | 55 | Not touched |
| Template bodies: list, tree and vector instantiations, `std::copy` into a back inserter (00732560), `std::operator+(const char*, const std::string&)` (007F3D40) | 27 | Tagged `stl_shape`, `reviewed`, evidence `sys6_shape` |
| No cc9 host, reconstructed | 26 | `src/cc13_seg48U.cpp`, build-tested (0072C170, 007E29A0, 007EC5A0 and 007F3E50 by cc13-seg48V) |
| Already implemented: 00730BD0 as table data (`src/unit_kind_query.cpp`), 0072AB10 (`src/unit_parts.cpp`) | 2 | Record only |
| Referenced by cc9 `game_hosts_*` files (verify-and-record) | 31 | Recorded with a verdict by cc13-seg48V (see "Verify-and-record") |

The template tags use the existing evidence id `sys6_shape`, because
`config/tags/evidence.json` was leased to cc13-seg53U. Every tagged row
calls only:
- other template rows;
- operator new 00BF55BE/00BF681B, `_free` 00BF6989 and operator delete 00BF65AC;
- the length throw;
- for 007F3D40, the std::string assign 00408720, append 004C90D0 and copy 00408120.

These are bind-only under the 2026-10-16/17 library ruling.

## Reconstructions

The code is in `include/bsp/cc13_seg48U.hpp` and `src/cc13_seg48U.cpp`. The host
is `Cc13Seg48UHost : Cc13Seg40Host`. It reuses the seg40/seg48 chain's hooks for
these, and adds one named hook for every other native call:
- the CRT;
- the sized string pool;
- the archive slots +4h/+8h/+0Ch/+10h;
- reference release;
- 00956C20 and 0072B6D0.

| Address | ABI | Body |
|---|---|---|
| 0072BC80 BSP_GunBot_SetSkillIndex | thiscall(bot, index), RET 4 | +34h = index |
| 0072BD10 BSP_GunBot_HasFireTarget | thiscall(bot), AL | byte +39h or +38h |
| 00731050 / 00731040 | thiscall(gun), EAX | +78h == 0 gives null; else +74h, or [[+74h]+34h] |
| 00729FA0 BSP_Gun_OnActivated | thiscall(gun) | 00440F50, then 00956C20(unit) or 00956BA0(unit, gun) by unit bytes +5Dh/+60h |
| 0072D7E0 BSP_GunBase_InitDevice | thiscall(gun) | 004426C0; when modes > 0: 0072B6D0(+3A8h, modes) and 0072D5B0(gun, 0, 1); +3F0h = +3Ch |
| 0072BE40 BSP_GunBot_ConstructMuzzleClass | thiscall(bot), EAX | 0072BBD0(bot, [00E19990]), vtables, the 00F87574 vector twice, +B4h = -1.0f |
| 00729670 BSP_GunShotDecision_LineOfFireClear | thiscall(decision, arg), RET 4, AL | pose refresh, then the query's vtable +4h(&gun position, owner, arg, range); SETZ |
| 00729700 BSP_Gun_ApplyDescriptorNodeFlag | thiscall(gun) | class bytes +84h/+85h set or clear flags 4/2 on the descriptor and its node chain |
| 00731AE0 BSP_GunShotCommand_ReadAndCreate | thiscall(reader), EAX | See the torpedo note |
| 007323E0 BSP_GunClassEffectRecord_Destruct | thiscall(record) | effect stop, container erase, six reference releases |
| 00729D30 BSP_Gun_RebuildBarrelBuffers | thiscall(gun) | per-record barrel grid build (integer only) |
| 0072D950 BSP_Gun_Destruct | thiscall(gun) | full member teardown, then 0087A410 |
| 0072C170 BSP_Gun_ComputeAimForTarget | ECX shooter, EDX device id; stack (target, a*, b*, speed, flag dword), RET 14h | Lua HitTarget's aim: a stepped ballistic flight, the miss and the target's motion folded in, then the spawn (see the x87 note) |
| 007D5AC0 BSP_Plane_CopyClassRecordLists | thiscall(plane) | the plane's effect records rebuilt from the class record list (00868420, identity matrix) |
| 007F35C0 BSP_PlaneSquadron_GetDisplayName | thiscall(squadron), EAX | a stored name, or the function-static "." + id + "x\|" + name |
| 007E29A0 BSP_ObserverSet_Remove | thiscall(set, entity), RET 4 | the entity's slot +4h, 006952A0 (EDX = set), then std::remove (007E1D30) over {+14h, +18h count}; game code, the stl_shape tag was a mistake |
| 007EC5A0 BSP_PlaneFxComponent_LoadFromClass | thiscall(component; plane, byte active), RET 8 | Dying/Falling effect slots, 007EB160 or 007EB0E0, the stream-1 speed spread, and one engine per class+140h through slot 0; see "007EC5A0" below |
| 007F3E50 BSP_PlaneSquadron_BuildDebugText | thiscall(squadron; dword unused, target*), RET 8 | builds two std::strings and two format buffers, then drops them; see "007F3E50" below |
| 00735030 BSP_MissionNarrative_Construct | thiscall(narrative), EAX | the 58h object at game+21E8h |
| 00733030 BSP_MissionNarrative_Reset | thiscall(narrative) | bytes +3Ch and +14h cleared |
| 00733C80 BSP_NarrativeEntry_CopyConstruct | thiscall(entry, source), RET 4 | vtable 00CFE960, two native strings, +14h |
| 007337A0 / 00734460 BSP_MissionNarrative_Write / Read | thiscall(narrative, archive), RET 4 | the "title", "status" and "titleList" sections |
| 00734B10 / 00734BF0 BSP_MissionNarrative_SendToParty / SendToPlayer | thiscall(narrative; party or slot, text), RET 8 | MISSIONINFO relay (00770BD0) or peer send (00770B50); local echo through 00734870 |

### Fidelity notes

- **Checked-iterator tests.** 0072D950, 007D5AC0, 007337A0 and 00734460 call
  00BF6713 `_invalid_parameter_noinfo` where the image does. These tests stay in
  the model and are not dropped.
- **00731AE0 null calls.** The image calls the result's vtable +4h even when
  the result is null. That happens when the type is neither string, or when
  operator new returns null. The call is kept.
- **0072D950 reference release.** The image releases the vector element's
  reference (+0Ch) without a null test, and so does the model.
- **00734460 "state" field.** "state" is read into the reader argument's own
  stack slot ([ESP+3Ch] at 007344E7, which is ESP0+4h). That slot no longer holds
  the reader pointer: 007344B9 `MOV [ESP+44h],ESP` overwrote it with ESP0-40h,
  the address of the {0, "title"} begin-key record. When the archive has no
  "state", the image stores that stale stack address to +18h. The value cannot
  be reproduced exactly. The model starts the local from its own address, which
  is likewise a non-zero stack address.
- **x87 float copies.** In 0072BE40 and 00729670, the FLD/FSTP float copies
  quiet a signalling NaN (`x87_float_round_trip`). These two functions do no
  other floating-point arithmetic.
- **007323E0 body end.** Ghidra's body ends at 00732505. The rest
  (00732505..0073255B, three more releases and the epilogue) comes from
  `bsp.py disasm-raw`.
- **SEH unwind states.** They release locals on a throw and are not modelled.

### Torpedo note: 00731AE0 (sent to team-lead as URGENT)

00731AE0 reads "type" from the save reader:
- **"flak":** it builds an 8-byte command with vtable 00CFE4F8 and +4h = 0.0f.
- **"torpedo":** it calls `007311B0(0.0f, -2.5f, null)`. -2.5f is [00D0C314].
  Then the command's vtable +4h (00731270) reads "heading" and "swimdepth".

The only caller is 0072F060 BSP_Gun_LoadSavedState, which is the save-load path.
So this does not run in a normal mission.

cc9's `game_hosts_gunnery` treats a commanded heading of 10000 (00D0C310) as "no
order". A torpedo order loaded from a save starts at heading 0.0 instead, and it
keeps 0.0 unless the save carries "heading".

RNG: there is no direct draw.

### Adoption notes at the callers

- **00729670.** `gameplay_seg40_59_cc13` (a cc13 file) takes 00729670 as a
  `std::function<bool()>`. The image also refreshes the gun pose (00414DB0)
  before the query whenever byte +C8h is clear. A host that supplies the
  predicate should do the same.
- **00731AE0.** Its caller 0072F060 is reconstructed in `cc13_front5`, which
  calls it through a hook. No cc9 host models it.

- **00735030 and the cc9 countdown model.** `game_hosts_script_orders.hpp`
  (packet cc9_lua_countdown) models the countdown object at [game+21E8h]. It
  states that 00735030 writes +4Ch..+54h and leaves +3Ch..+48h unwritten. The
  listing agrees: it writes +0h (vtable 00CFE9C0), +8h (the list head node that
  00733430 returns, with ECX = +4h), +0Ch, +10h, +2Ch..+38h and +4Ch..+54h. It
  writes nothing in +14h..+28h or +3Ch..+48h.
  The constructor does not run alone, though. At 004DFA8D the world construction
  calls 00733030 (Reset) straight after the construct at 004DFA75
  (`include/bsp/world_construct.hpp:205`). Reset clears bytes +3Ch and +14h, so
  those two are zeroed by the image. Only +40h..+48h rely on cc9's assumption
  that they start at zero, and that assumption is not checked here.
- **The countdown in saves.** The "status" section of 007337A0/00734460 saves
  and restores the countdown: +3Ch active, +40h "level", +44h "time", +48h
  "init", +4Ch "callback". The "title" section does the same for +14h..+2Ch.
  cc9's countdown model has no save or load counterpart.
- **00731040.** `game_hosts_lua.hpp` cites 00731040 as the class's first
  bullet. It is used in a comment only, with no call.

No cc9 `game_hosts_*` file calls any of the 24 bodies.

## Verify-and-record (cc13-seg48V)

The 31 rows that cc9 `game_hosts_*` files reference were compared against the
export listing or the disk bytes. Each has a record in
`config/reconstruction/*.jsonl` that points at the host file and gives the
verdict in its evidence. No cc9 file was edited, and nothing is wired.

| Verdict (record status) | Rows |
|---|---|
| Host matches the listing (`existing_host_implementation_verified_against_listing`) | 00729560, 0072AC20, 0072B4C0, 0072BBD0, 0072BD30, 00730A20, 00731EF0, 00732210, 00733BB0, 00733D50, 00734CE0 |
| Labelled host substitution, consistent with the listing (`existing_host_substitution_labelled_verified_against_listing`) | 007DEDB0, 007E2090, 007F3820, 0072C970, 0072DD20, 007311B0 |
| Facets spot-checked, no defect seen (`existing_implementation_spot_checked_no_defect_seen`) | 007D3E60, 007F4BA0, 0072F830, 00730ED0 (table data in `src/unit_kind_query.cpp`), 007313E0, 007327B0 |
| Deviation (`existing_implementation_minor_deviation`) | 0072CDD0, 00731E20 (a label only) |
| Cited by cc9 only, with no behaviour to compare (`existing_host_verified_ledger_only`) | 00729600, 00730F70 |
| Not modelled by cc9, or a stub (`host_reference_unverified`) | 007EB380, 0072E6D0 (facets checked), 00730B80, 00735450 |

### Renames

| Address | Old name | New name | Evidence |
|---|---|---|---|
| 0072C970 | BSP_Gun_TorpedoLaunchHeading | BSP_Gun_TorpedoSwimDepth | It returns a depth in ST0: 00852410's prediction, or [00D0C314] = -2.5. 007311B0 stores it as swimdepth (+8h). |
| 00729600 | BSP_GunEffectSet_Destruct | BSP_GunShotDecision_Destruct | The destructor of the gun+424h decision object; it deletes the +8h predicate that 00729560 installs. |
| 00731C80 | ..._CreateMultipleBombPlatform | BSP_NodeFactory_CreateRapidFixedSlaveGun | Factory table, below |
| 00730B80 | BSP_MultipleBombPlatform_Construct | BSP_RapidFixedSlaveGun_Construct | It sets class id 21h (MRFSGun) |
| 00731D50 | ..._CreateRapidFixedSlaveGun | BSP_NodeFactory_CreateRapidTurningGun | Factory table |
| 00730E80 | BSP_RapidFixedSlaveGun_Construct | BSP_RapidTurningGun_Construct | It sets class id 23h (MRTGun) |
| 00731E20 | ..._CreateRapidTurningGun | BSP_NodeFactory_CreateSingleTurningGun | Factory table |
| 006FDED0 | BSP_RapidTurningGun_Construct | BSP_SingleTurningGun_Construct | It sets class id 24h (MSTGun) |
| 006FE390 | ..._CreateSingleTurningGun | BSP_NodeFactory_CreateDepthChargeLauncher | Factory table |
| 006FDFC0 | BSP_SingleTurningGun_Construct | BSP_DepthChargeLauncher_Construct | It sets class id 27h |

**The factory table at 00CE4580..00CE46C0.** Each record's dwords end with
the creator, and the class name string comes after them. The decisive test is
the record's own GetName slot, which returns the string that follows it:
- 00442DF0 returns 00CE460C "MRFSGun";
- 00442E90 returns 00CE4640 "MRTGun";
- 00442F20 returns 00CE4674 "MSTGun";
- 00442F80 returns 00CE46A8 "MDepthChargeLauncher".

The earlier names paired each creator with the string BEFORE its record, so
they were one record off. The constructors' class-id stores confirm the new
pairing against `src/entity_class_ids.cpp`. The ledger, the reconstruction
names and Ghidra are updated; the old values are in
`local/ghidra-annotations-20261018T142725Z.json`.

C++ symbols and comments elsewhere still use the old names in
src/include/docs (for example `cc13_seg40`'s
`multiple_bomb_platform_construct_00730b80`). They are hypotheses and were not
renamed here.

### Adoption notes (cc9 hosts; nothing edited)

- **0072CDD0, the Landscape query start.** At 0072CE8E the image pushes EBX,
  which is argument 1, the caller's UNRAISED muzzle. That is the start of the
  0098ADD0 Landscape query. Only the 0098B130 unit query uses the copy raised
  by 5.0. The host's `line_of_fire_blocked_0072cdd0` passes the raised `from`
  to `landscape_segment_hit` (under kLandscapeSpatialAttachBound, ON).
  - The host's lines starting near land are therefore 5 m higher than the
    image's, so the host can miss a Landscape block that the image sees.
  - The caller (the kAaLineOfFireBound lambda near game_hosts_gunnery.cpp
    6646) passes the same muzzle, and no switch moves the raise.
- **00731E20 and the turning-gun labels.** game_hosts_gunnery.cpp 2357 and
  the kGunImmediateFireSlotBound text (293..300) call Type 24h
  "Rapid_Turning_Gun (MRTGun)" and 27h "Single_Turning_Gun (MSTGun)". In the
  image's own table, 24h is MSTGun and 27h is MDepthChargeLauncher, and MRTGun
  is 23h, created by 00731D50. The behaviour the host binds to each Type cites
  the right code (006FDF60 and 0084C5B0 for 24h; 006FDC50, 006FE0D0 and
  006FDCD0 for 27h), so only the labels are off. This was not re-run.
- **00730B80 and "a bomb platform is a Gun subclass".** game_hosts_gunnery.hpp
  1109/1128 and game_hosts_units.cpp 3608 cite 00730B80 calling
  BSP_Gun_Construct as the proof. 00730B80 is the MRFSGun constructor (class
  21h). The bomb platforms are MBombPlatform 25h and MMultipleBombPlatform 26h,
  created by 006E08F0 and 006E0970. `src/entity_class_ids.cpp` gives 25h the
  parent 20h, the gun family, so the conclusion may still hold. The citation is
  wrong, though, and 006E08F0/006E0970's constructors were not read here.
- **0072C970's kind-6 arm.** The image also predicts the depth when the target
  is kind 6 whose +738h object casts to a submarine (0072B9B0, slot +2Ch,
  00465060). The host's `torpedo_sub_vs_sub_launches` counter tests target
  kind 8 only. This is a counter only: the depth stays -2.5 under the labelled
  substitution.
- **0072AC20, a player order that never clears.** The image keeps one command
  at gun+41Ch. Every later 0072AC20 call replaces it, and the AI aim tick
  008FFF20 is one of its two callers. The host sets `player_order_heading` from
  message 79h and never resets it to 10000. A tube that the player once ordered
  and the AI later fires can therefore keep the player's heading where the
  image would carry the AI's order. The gyro block (kTorpedoGyroHeadingBound)
  overwrites `commanded_heading` only when the lead is non-zero, so this is not
  settled here.
- **007E2090, the noise draw.** The host replaces U(0, 0.05) by its midpoint
  (labelled). The image draws from 00BD2F10 on stream 1 for every candidate
  with a positive score and a noise argument above [00D7A218]. 00BD2F10 is
  process-wide, so the substitution also leaves every later draw on that
  stream unshifted.
- **00734CE0 in `src/cc13_seg40.cpp` (a cc13 file, not cc9).**
  `cc13_seg40_mission_narrative_destruct_00735080` (cc13_seg40.cpp:665-677) has
  two errors against 007350C0..007350E2:
  - it calls `narrative_list_clear_00734ce0(narrative)` where the image passes
    ECX = narrative+4 (`ADD ESI,4`);
  - it frees `[narrative+4]` and zeroes `+4h` where the image frees and zeroes
    `[ESI+4]` = narrative+8 (the head node 00733430 returns, which 00735030
    stores at +8h).

  Its comment calls the pseudocode's `param_1[2]` wrong, but the pseudocode
  was right. The file is not leased here and is reported to the lead.
- **0072DD20's kind 46h-in-44h arm.** When the gun's parent is kind 46h inside
  a kind-44h parent, the frame is re-expressed through 00413920, 0087FA50 and
  00414E10. No cc9 switch text mentions this arm, and its reach is unverified.

## 0072C170: the x87 aim for Lua HitTarget (cc13-seg48V)

The source is the disk-byte disassembly 0072C170..0072C692 (`bsp.py disasm-raw`).
Every x87 operand form and direction was read from the bytes, not from the
Ghidra listing.
- **Entry.** HitTarget (00892F70) loads ECX = Lua arg 0 (the shooter) and EDX =
  arg 1 (__ftol, the device id). It pushes the target, the two vector pointers,
  the speed (stored as a float at 008931CC) and a flag dword. Only AL of that
  dword was stored (008931B2); the upper bytes are stale and are passed through.
- **Constants.** All six doubles are float-exact: [00CE7630] 30.0,
  [00D7A358] 0.01f, [00CF9058] 9.81f, [00D7A280] 0.5, [00D7A210] 1.0 and
  [00CFE058] 0.033333335f. On the main thread (24-bit precision control),
  float arithmetic therefore reproduces each rounding, the extended exponent
  range aside. The one sqrt is `_CIsqrt` on a float sum, and std::sqrt on a
  float is correctly rounded either way.
- **Order kept.** The loop updates x, then y (first + vy*dt, then - drop), then
  z, vy and t. The exit test is FCOMI/JA, so a NaN leaves the loop. When
  a.y <= b.y, or the compare is unordered, the loop is skipped. The fraction is
  then (a.y - b.y) / (a.y - (a.y + 0.01)).
- **The height term of the miss** is `b.y - b.y` (0072C43D FSUBR ST0,ST0). It
  is 0, or NaN for a non-finite b.y; the model keeps the subtraction.
- **Two calls.** The target's vtable +34h is called twice (0072C3DB and
  0072C44F), and each answer is scaled by the flight time. The model calls the
  hook twice.
- **Owner and source.** A shooter that is not IsKindOf(0Fh) binds a null owner
  (0072C693). For a plane with +914h > 0, the +170h/+54h/+180h fields come from
  the squadron's first plane, [[shooter+9D4h]+3D0h].

Adoption note: `cc13_left55_seg56a`'s hook `gun_compute_aim_for_target_0072c170`
takes the flag as a `bool`, which drops the stale upper bytes that the image
passes into the spawn's fifth argument. Whether the spawn reads more than the
low byte was not checked.

## 007EC5A0: the plane effects load (cc13-seg48V)

`cc13_seg48U_plane_fx_component_load_from_class_007ec5a0` is transcribed from the
disk-byte disassembly 007EC5A0..007ECCBF (RET 8). It is unwired.

- The plane's slot +5Ch(10h) or +5Ch(16h) picks the "Big" names and +1Ch = 0.5f.
  Otherwise the "Small" names are used and +1Ch = 1.0f.
- The two lazy effect slots, class+F0h (Dying) and class+F8h (Falling), are filled
  by name through 00871BA0 and 00440580. Then 007E2300 binds component+74h and +CCh
  with tuning +6BCh and +6B8h.
- The speed spread repeats `rotor_speed_spread_007eca0e` (src/gameplay_seg48_cc13.cpp)
  inline, because that file is bsp_game and this one is bsp_core. It draws two
  U1 values and then c NextU32 values from stream 1.
- The engines go into component+8h. Each engine's slot 0 takes four arguments:
  00476B90 takes none, so the three values pushed before it belong to the slot. The
  call reads data[i], which is not the engine just pushed when the array was not
  empty on entry.
- Uncertainty: the vector's first dword (the checked-iterator proxy) is never
  written by the image. The model zeroes it.

## 007F3E50: the squadron debug text (cc13-seg48V)

`cc13_seg48U_plane_squadron_build_debug_text_007f3e50` is transcribed from the
disk-byte disassembly 007F3E50..007F44D6. It is unwired.

- Nothing references the body. Ghidra has no xrefs, and `scan-bytes '50 3e 7f 00'`
  finds no dword. The same scan finds 007EBF10 in its vtable, so the scan works.
- It builds two strings, the squadron line and the target line, plus the
  "pos" and "dist %.2f" buffers. Then it destroys them without passing them on, so
  the output sink was probably compiled out. The first stack argument is never read.
- The effects that remain come from the callees: the name slots, 00521EA0, 00414DB0
  (when byte +C8h is 0), 006F7DD0, and the heap traffic of the string temporaries.
  The model keeps their order.
- The template string bodies (004C90D0, 004D0860, 007F3D40, 007B7E70, 007B7CC0) are
  bind-only hooks. 007F3DF0 calls the existing cc13_seg48 reconstruction.

## Left open
- No body from the row list is left. Open notes are in the sections above.

## Files

- `include/bsp/cc13_seg48U.hpp` and `src/cc13_seg48U.cpp`, registered in
  `cmake/startup.cmake` (bsp_core).
- 28 function records in `config/reconstruction/00720000`, `00730000`,
  `007d0000`, `007e0000` and `007f0000.jsonl`: the 26 bodies plus 00730BD0 and 0072AB10.
- 31 verify-and-record records (cc13-seg48V) in `config/reconstruction/00720000`,
  `00730000`, `007d0000`, `007e0000` and `007f0000.jsonl`, and 10 renames in
  `config/names/006f0000`, `00720000` and `00730000.jsonl`.
- 27 tag rows in `config/tags/00720000`, `00730000`, `007e0000` and
  `007f0000.jsonl`.

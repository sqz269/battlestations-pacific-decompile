# cc13-seg48U: undone rows of segments 48 and 40

This packet covers the cc13 lead tree's `local/cc13_undone_seg48_40.txt`: 141
functions with no reconstruction record and no tag. They lie in segment 48
(007D1E50-007F8390) and segment 40 (00728A90-00735EC0). The work runs under the
cc9 gameplay terms (`local/cc13_gameplay_terms.txt`).

Each row was checked against rule 1 with:
- `git grep` over src/include;
- `bsp.py lookup` and `docs-for`;
- the export listing.

Nothing is wired.

| Group | Rows | Outcome |
|---|---|---|
| In the struck ranges 007DF000-007DFFFF and 007EE000-007F2FFF | 55 | Not touched |
| Template bodies: list, tree and vector instantiations, `std::copy` into a back inserter (00732560), `std::operator+(const char*, const std::string&)` (007F3D40) | 28 | Tagged `stl_shape`, `reviewed`, evidence `sys6_shape` |
| No cc9 host, reconstructed | 23 | `src/cc13_seg48U.cpp`, build-tested |
| No cc9 host, already implemented as table data: 00730BD0 | 1 | Record only (`src/unit_kind_query.cpp`) |
| No cc9 host, not done in this packet: 0072C170, 007F3E50, 007EC5A0 | 3 | Left for a follow-up |
| Referenced by cc9 `game_hosts_*` files (verify-and-record) | 31 | Not done in this packet (see "Left open") |

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
| 0072AB10 BSP_UnitInstance_LastDamageStateIndex | thiscall(unit), EAX | count of 10h records at [unit+360h]+16Ch, minus 1; 0 when empty |
| 00729FA0 BSP_Gun_OnActivated | thiscall(gun) | 00440F50, then 00956C20(unit) or 00956BA0(unit, gun) by unit bytes +5Dh/+60h |
| 0072D7E0 BSP_GunBase_InitDevice | thiscall(gun) | 004426C0; when modes > 0: 0072B6D0(+3A8h, modes) and 0072D5B0(gun, 0, 1); +3F0h = +3Ch |
| 0072BE40 BSP_GunBot_ConstructMuzzleClass | thiscall(bot), EAX | 0072BBD0(bot, [00E19990]), vtables, the 00F87574 vector twice, +B4h = -1.0f |
| 00729670 BSP_GunShotDecision_LineOfFireClear | thiscall(decision, arg), RET 4, AL | pose refresh, then the query's vtable +4h(&gun position, owner, arg, range); SETZ |
| 00729700 BSP_Gun_ApplyDescriptorNodeFlag | thiscall(gun) | class bytes +84h/+85h set or clear flags 4/2 on the descriptor and its node chain |
| 00731AE0 BSP_GunShotCommand_ReadAndCreate | thiscall(reader), EAX | See the torpedo note |
| 007323E0 BSP_GunClassEffectRecord_Destruct | thiscall(record) | effect stop, container erase, six reference releases |
| 00729D30 BSP_Gun_RebuildBarrelBuffers | thiscall(gun) | per-record barrel grid build (integer only) |
| 0072D950 BSP_Gun_Destruct | thiscall(gun) | full member teardown, then 0087A410 |
| 007D5AC0 BSP_Plane_CopyClassRecordLists | thiscall(plane) | the plane's effect records rebuilt from the class record list (00868420, identity matrix) |
| 007F35C0 BSP_PlaneSquadron_GetDisplayName | thiscall(squadron), EAX | a stored name, or the function-static "." + id + "x\|" + name |
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
  stack slot ([ESP+3Ch]). When the archive has no "state", that slot still holds
  the reader pointer, and the image stores it to +18h. The model starts the local
  from the reader pointer for the same reason.
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
  listing agrees: it writes +0h, +4h..+0Ch, +10h, +2Ch..+38h and +4Ch..+54h,
  and nothing in +14h..+28h or +3Ch..+48h. cc9 assumes those unwritten fields
  start at zero; that is its own assumption and is not checked here.
- **The countdown in saves.** The "status" section of 007337A0/00734460 saves
  and restores the countdown: +3Ch active, +40h "level", +44h "time", +48h
  "init", +4Ch "callback". The "title" section does the same for +14h..+2Ch.
  cc9's countdown model has no save or load counterpart.
- **00731040.** `game_hosts_lua.hpp` cites 00731040 as the class's first
  bullet. It is used in a comment only, with no call.

No cc9 `game_hosts_*` file calls any of the 23 bodies.

## Left open

- **0072C170 BSP_Gun_ComputeAimForTarget.** 384 instructions of x87 with a
  sqrt; its only caller is 00892F70, the Lua HitTarget binding.
- **007F3E50 BSP_PlaneSquadron_BuildDebugText.**
- **007EC5A0 BSP_PlaneFxComponent_LoadFromClass.**
- **The 31 cc9-hosted rows** (verify against the binary, record only):
  - segment 48: 007D3E60, 007DEDB0, 007E2090, 007EB380, 007F3820, 007F4BA0;
  - segment 40: 00729560, 00729600, 0072AC20, 0072B4C0, 0072BBD0, 0072BD30,
    0072C970, 0072CDD0, 0072DD20, 0072E6D0, 0072F830, 00730A20, 00730B80,
    00730ED0, 00730F70, 007311B0, 007313E0, 00731E20, 00731EF0, 00732210,
    007327B0, 00733BB0, 00733D50, 00734CE0, 00735450.

0072C970 (BSP_Gun_TorpedoLaunchHeading) and 007311B0 are on the torpedo path.
They should be checked first.

## Files

- `include/bsp/cc13_seg48U.hpp` and `src/cc13_seg48U.cpp`, registered in
  `cmake/startup.cmake` (bsp_core).
- 24 function records in `config/reconstruction/00720000`, `00730000`,
  `007d0000` and `007f0000.jsonl`: the 23 bodies plus 00730BD0.
- 28 tag rows in `config/tags/00720000`, `00730000`, `007e0000` and
  `007f0000.jsonl`.

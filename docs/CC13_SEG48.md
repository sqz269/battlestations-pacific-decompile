# cc13-seg48: segment 48 leftovers

Packet `cc13-seg48`. Segment 48 is `007D1E50-007F8390` (plane flight: gears, bay doors, wings, travel speed).
This pass takes the functions that cc13-gpr48 left (docs/CC13_GPR_48.md section 6): plane-unit list removal,
the plane neighbour and effects-component plumbing, squadron member broadcasts and ordnance loops, the
save-game archive leaves, the small session messages and the gun-platform destructor.

The struck sub-ranges `007DF000-007DFFFF` and `007EE000-007F2FFF` (live cc9 packets) were not leased, read
for reconstruction or recorded.

Code: `include/bsp/cc13_seg48.hpp`, `src/cc13_seg48.cpp`. Names are hypotheses, not recovered symbols.
The functions are new C++ interfaces over raw native blocks. A block is touched only at the offsets the
image reads or writes. Every call the image makes goes through a named `Cc13Seg48Host` method at the same
point, with the same arguments. Nothing is wired into the game, and nothing was run. Status:
build-tested only.

Evidence: Capstone `disasm-raw` over the disk bytes for every function with a register input, an
indirect call or a float test, plus the Ghidra pseudocode. Constants were read with `ghidra bytes`.

## 1. Adoption note (sent to team-lead as URGENT, level-bomb)

`007ED830` BSP_Unit_HasLevelBombOrdnance asks every squadron member. The host asks only the slot-0
plane.

- **Binary.** Bytes `007ED830-007ED876`. The loop `i < [sq+3CCh]` sets ECX = `[sq+3D0h+4i]` (EDI
  advances by 4 at `007ED85F`), pushes the loadout and calls `007B9500` at `007ED853` (kind 31h, a live
  rack query). It returns 1 at the first member that answers.
- **Host.** `src/game_hosts_script_orders.cpp:1556-1567` sets `has_level_bomb_ordnance` from
  `unit_ordnance()` of the ordered unit only, and does not carry the loadout argument. Its comment says
  "every self query runs on element 0"; for these loops that is not what the image does.
- **The same shape applies to** `007ED7E0` (2Ah), `007ED880` (2Fh), `007ED8D0` (2Bh), `007ED970`,
  `007EDA10` (2Ch), and to `007ED790`, `007ED920` and `007ED9C0`.
- **Divergence.** A squadron whose slot-0 plane has emptied its kind-31h racks (live mask
  `kRackLiveOrdnanceMaskBound`) while a wingman still carries: the image answers true, the host false.
  The answer feeds `007EE8F0` BSP_Unit_AttackCommandApplies.
- **Live in a normal mission:** yes. **Shared RNG 00BD2F10:** not touched.

No host was edited. The eight loops are reconstructed standalone (section 3.4).

## 2. Conventions

- **Reference release.** The inlined release `p = [slot]; if (p) { if (InterlockedDecrement(p+4) == 0)
  p->vt[0](); [slot] = 0; }` is `release_slot`. Slot 0 is called with no stack argument; the bytes at
  `007E237D-007E237F` show `mov ecx, esi; call eax`.
- **Null-mask tests.** `-(p != 0) & 00E186ECh` (and `00E17BF8h`, `00E19AB0h`) is non-zero exactly when
  `p` is non-null. It is written as `p != nullptr`.
- **Two `_free` thunks.** `00BF6989` and `00BF65AC` are kept apart. So are the two operator-new
  addresses, `00BF55BE` and `00BF681B`.
- **Archive references.** An archive reference is a three-dword object passed by value (the archive
  object is the third dword), and the callee destroys it: `00410630` for a reader, `004107B0` for a
  writer. A child reference is built by `00BD4FE0` (reader) or `00BD5010` (writer) as
  `__thiscall(ECX = parent, out, name word 0, key)`, RET 0Ch. The image builds it in the argument slot;
  here it is built in a local and copied.
- **Byte saves.** A byte save pushes a dword whose upper 24 bits are stale stack. It is modelled as
  zero-extended.

## 3. Functions

### 3.1 Plane units and records

| Address | Name | What the code does |
|---|---|---|
| 007D7720 | BSP_MPlaneBomber_Construct | `007CFD20(unit, arg, 0)`, then eight vtables (00D06638 .. 00D065B0), +C4h = 10h, +AACh = 7 |
| 007D7790 | BSP_MPlaneBomber_Sub_Slot0_Return7 | returns 7 |
| 007D7850 | BSP_MPlaneBomberClass_Create | new E94h (00BF55BE), memset before the null test, construct, `009553D0(unit, class)`, +354h = class |
| 007D7930 | BSP_MPlaneBomber_UnregisterFromWorldLists | `0074E610`, then find the node at [[+30h]+DCh] and `004837D0(list +D8h, node)` |
| 007D7A10 | BSP_OwnedPtrHolder_Destruct_00D06848 | vtable 00D06848; [+10h]->vt[0](1) when non-null |
| 007D7A40 | BSP_PilotPlan_ResetCommandTimer | [[+10h]+C8h] = 1.2f (00CE3814) when [+10h] is non-null |
| 007DD9B0 | BSP_MPlaneFighter_Construct | as 007D7720 with 00D06920 .. 00D06898, +C4h = 13h, +AACh = 0Ah |
| 007DDA20 | BSP_MPlaneFighter_Sub_Slot0_Return0A | returns 0Ah |
| 007DDAE0 | BSP_MPlaneFighterClass_Create | as 007D7850 with 007DD9B0 |
| 007DDBC0 | BSP_MPlaneFighter_UnregisterFromWorldLists | as 007D7930 on [[+30h]+100h] / +FCh |
| 007DDCB0 | BSP_Plane_KamikazeGatePredicate | 0 only when byte +BC8h = 0, vt[5Ch](17h) is true and byte +C24h = 0; else 1 |
| 007DDD60 | BSP_PartRecordRef_Construct | +8h = weight, +0h owner, +4h node, +0Ch = 0.0f, 16 floats of `00B6DB60(node)` to +10h |
| 007DDDE0 | BSP_Vector3_FillOne_007ddde0 | three 1.0f (00D7A24C) |
| 007DDE00 | BSP_PlaneSubRecord_InitDefaults_007dde00 | +0h = 1.0f, byte +4h = 0, +8h = 0 |
| 007DDEA0 | BSP_PlaneSubBlock_ResetTimer40 | +40h = 60.0f (00CEB4B0) |
| 007DDFB0 | BSP_SingletonF87440_DestructUnwind | [00F87440] = 0, vtable 00CE3818 |
| 007E2510 | BSP_PlaneRecord_InitZero_007e2510 | zeroes, +8h = value, +18h = 1.0f |

### 3.2 Engine pod arrays `{data, count, capacity}`

| Address | Name | What the code does |
|---|---|---|
| 007DE030 | BSP_PodArray_FreeBuffer_007de030 | `_free` (00BF6989) of data when non-null |
| 007DE0B0 | BSP_PtrArray_Grow_007de0b0 | capacity = 2c + 2, new (00BF55BE), copy, free old |
| 007DE6B0 | BSP_PtrArray_PushBack_007de6b0 | grow when full, guarded append |
| 007E0FE0 | BSP_PodArray4_PushBack_007e0fe0 | when full `007BD940(max(2c, 1))`, append |
| 007E1CF0 | BSP_PodArray4_PushBack_007e1cf0 | same body |
| 007EDD00 | BSP_PodArray_FreeBufferUnwind_007edd00 | free data and clear it |
| 007EDD70 | BSP_PtrArray_Reserve_007edd70 | checked reserve; the size saturates to FFFFFFFFh on overflow |
| 007EDDE0 | BSP_PtrArray_Grow_007edde0 | checked reserve of 2c + 2 |
| 007F5570 | BSP_PodArray_FreeBufferUnwind_007f5570 | free data (not cleared) |

### 3.3 Callback buffers, neighbours, effects

| Address | Name | What the code does |
|---|---|---|
| 007DE420 | BSP_CallbackBuffer_Destruct_007de420 | free [+14h], then `00695870(buffer)` |
| 007DE690 | BSP_CallbackOwner_Reset_007de690 | `00695830(owner)`, count +18h = 0 |
| 007DEC80 | BSP_CallbackBuffer_Destruct_007dec80 | same as 007DE420 |
| 007DECF0 | BSP_PlaneNeighbours_Destruct | vtable 00D06B88; `006CDD70(+9Ch)`; the buffers +5Ch, +3Ch, +1Ch; `00875B30` |
| 007E0F20 | BSP_PlaneNeighbours_AppendObserved | `00694A60(ECX observed, EDX buffer)`, push onto +14h |
| 007E2070 | BSP_PlaneNeighbours_ClearTargetObserver | `006952A0(ECX [+B0h], EDX +9Ch)` and clear |
| 007E1020 | BSP_RefPtr_Assign_007e1020 | release the old pointee, store without adding a reference |
| 007E2360 | BSP_RefPtr_ReleaseAt4_007e2360 | release +4h |
| 007F6FA0 / 007F7010 | BSP_RefPtr_ReleaseAt8_* | release +8h |
| 007E1B30 | BSP_EffectHolder_StopEffectC_007e1b30 | when +0Ch: `00867B10`, byte +9 = 1, release +0Ch and +10h |
| 007E2240 | BSP_EffectVector_Destruct_00D05AB8 | vtable 00D05AB8; 007E1B30; release +10h, +0Ch, +8h |
| 007E2300 | BSP_PlaneEngineBase_Bind | +4h owner; swap the counted +8h (increment the new one, then release the old one); +24h; +18h |
| 007E2550 | BSP_PlaneEffectSlot_Destruct_007e2550 | stop the +4h effect; release +4h, +0h |
| 007EB0E0 | BSP_PlaneFxComponent_StopAllEffects | pop the +F8h vector from the back, stopping and releasing each |
| 007EB350 | BSP_PlaneFxComponent_Restart | `007EAE30(+48h, 1)`, then `007EB160` |
| 007EC490 | BSP_PlaneFxComponent_Destruct | 007EB0E0; engine records; `004C9550(+F8h, 0)`; four 007E2240; `007C3000(+8h, 0)`; frees |
| 007DE760 | BSP_PlaneSoundSet_ForEachSlot18 | vt[18h](a, float b) on the +0Ch list (null-tested), then on the +18h list (not tested) |
| 007E1E20 | BSP_PlaneNeighbours_Construct | tick-subnode base and `00876020(owner+310h or null)`; three callback buffers; +88h..+98h (3, 2, 1, 400, tan 0.0873); three STREAM-1 draws, `00BD2F10` called at 007E1F94, 007E1FB9 and 007E1FE1, seeding +7Ch, +84h, +80h as U(0, P) + P |
| 007E2010 | BSP_PlaneNeighbours_AdvanceTimers | +7Ch, +84h and +80h += dt; refresh `007E11D0` when +7Ch >= +88h (ordered), then subtract |
| 007EAE30 | BSP_EffectHolder_EnsureStarted_007eae30 | restart or create the effect (`00866B70`, or `008687C0` with an identity matrix plus `004845D0`); then the "Sound" child (`00866CD0`, `006FBEB0`) |
| 007EB160 | BSP_PlaneFxComponent_StartClassLoops | when there are no live effects: one `008687C0` per class entry ([[plane+538h]+250h], stride 44h), appended by `0074D780` |
| 007EB270 | BSP_PlaneFxComponent_OnFlightState | f = flying & (the 007DDCB0 gate); every engine's vt[4h](f); the levels; when f, `007EAE30(+48h, 1)` and `007EB160` |

### 3.4 Squadrons (member count +3CCh, members from +3D0h)

| Address | Name | What the code does |
|---|---|---|
| 007ECF00 | BSP_CommandTargetPair_Construct | vtable 00CFDB90, `00721070` on +10h and +28h |
| 007ECF20 | BSP_CommandTargetPair_D086F4_Construct | then vtable 00D086F4 |
| 007ECFD0 | BSP_PlaneSquadron_GetAiCommandBlock | [+348h] |
| 007ED020 | BSP_PlaneSquadron_BroadcastSlot148 | `00927D20(sq, a, b)`, then members vt[148h](a, b) |
| 007ED070 | BSP_PlaneSquadron_BroadcastSlot14C | `00927E50(sq, a, b, c)`, then members vt[14Ch] |
| 007ED160 | BSP_PlaneSquadron_MemberIndexOf | index or -1 |
| 007ED1D0 | BSP_PlaneSquadron_BroadcastSlot40 | members vt[40h](a) |
| 007ED3E0 | BSP_PlaneSquadron_SetFlag361 | byte +361h = 1 |
| 007ED500 | BSP_PlaneSquadron_LeaderFirstOrdnance | `007C3F60([+3D0h])` when non-null, else 0 |
| 007ED580 | BSP_PlaneSquadron_CurrentCommandToken | `0071BE40([+348h])` |
| 007ED590 | BSP_PlaneSquadron_ResetTravelSpeed | +388h = 0.5f, +3A0h = [[+35Ch]+190h], +3ACh = 1, +38Fh = 0 |
| 007ED5D0 | BSP_PlaneSquadron_RefreshTravelSpeed | when +38Fh = 0: copy when 0.0 > +388h (COMISS/JBE; a NaN skips) and +3ACh = 0; then +3ACh = 0 |
| 007ED690 | BSP_PlaneSquadron_ReEquipAll | `007BCB30(member, type)` for each member; +3ECh = 1 |
| 007ED6E0 | BSP_PlaneSquadron_StowMembersInside | +408h = 1, then `007C2130(member)` for each member |
| 007ED740 | BSP_PlaneSquadron_AllFreeFlightOrParked | false at the first member whose `007B8BD0` is false; then count > 0 |
| 007ED790 .. 007EDA10 | the ordnance loops (section 1) | per member: `007B9140`, `007B9320`, `007B9500`, `007B93E0`, `007B93F0`, `007B9400`, `007B9480`, `007B94F0` |
| 007ED9C0 | BSP_PlaneSquadron_AnyReadyRocket | `007B9410(member, arg)` per member |
| 007EDAC0 | BSP_PlaneSquadron_LeaderFlagC24 | byte [[+3D0h]+C24h], no null test |
| 007F3500 | BSP_PlaneSquadron_CloneFrom | new 414h, memset, `007F2C60(block, word +1Ch)`, `0041DD40(copy+154h, strlen)`, memcpy of the resized length, +369h |
| 007F3A60 | BSP_PlaneSquadron_ReleaseControlledUnit | the controlled-unit hand-back (limbo screen `00565FB0`, `004CC460(34h, 0)` or the manager's vt[10h], `004C0890(null)`), then `0077D270` |

### 3.5 Session messages

| Address | Name | What the code does |
|---|---|---|
| 007EDB80 | BSP_SessionMessage_PLANESQUADRON_CREATE_ConstructForRead | `00766F60`, vtable 00D08740, new 40h (00BF681B) built as 007ECF20, +118h, byte +124h = 1 |
| 007EDC10 | BSP_SessionMessage_PLANESQUADRON_CREATE_Destruct | free [+118h] (00BF65AC) with no destructor, then `007668E0` |
| 007EDEB0 | BSP_SessionMessage_ENTITY_RELOCATE_ConstructHeading | `0075B430(97h)`, byte +20h = 1, +24h heading |
| 007EDF00 | BSP_SessionMessage_ENTITY_RELOCATE_ConstructPosition | `0075B430(97h)`, byte +21h = 1, +2Ch..+34h position |
| 007EDF50 | BSP_SessionMessage_UNITEXITED_Construct | `0075B430(50h)`, +20h unit, vtable 00D02CE0 |
| 007F7CF0 | BSP_SessionMessageByteVec3Word_WriteBits | byte +10h (8 bits), dwords +18h..+24h (32 bits) on the cursor argument |
| 007F7D40 | BSP_SessionMessageByteVec3Word_ReadBits | the same, with the cursor at stream+4h |

### 3.6 Archives, strings, gun platform

| Address | Name | Keys / what the code does |
|---|---|---|
| 007DE880 / 007DE900 | BSP_ActuatorChannel_Load/SaveToArchive | "targst" byte +1h, "curst" +4h |
| 007E0740 / 007E07E0 | BSP_PlaneLerpValue_Load/SaveToArchive | "currentValue", "rate", "targetValue" |
| 007E08B0 / 007E0960 | BSP_PlaneLerpPair_Load/SaveToArchive | children "volume" (+20h), "pitch" (+14h) |
| 007E0A10 / 007E0AB0 | BSP_PlaneTimedFlag_Load/SaveToArchive | "value" byte +4h, "timeOut" +0h, "timer" +8h |
| 007E0B80 / 007E0D20 | BSP_PlaneStateBlock_Load/SaveToArchive | "running", child "flying", "RPM", "startStopTimer", four sound children, then slot +4h(0, "engines") and slot +8h() |
| 007D9B70 / 007DB430 | BSP_PlaneFlightController_Save/LoadFromArchive | "V" (type 5, &+18h), "Om" (&+24h), "Turbo" byte +4h; the load then calls `007D9C10` and `007D9C80` |
| 007F3DF0 | BSP_DebugString_FromInt | `00467310(buf, "%d", v)`, empty-string init, `00408720(out, buf, strlen)` |
| 007F7300 | BSP_GunPlatform_Destruct | vtable 00D08BB0; frees +8Ch (cleared), +3Ch, +2Ch (00BF65AC); the list +1Ch; `00487110(+10h)`; `00BD30F0` |
| 007F7110 | BSP_GunPlatform_Construct | counted base; two list heads (`004870F0`, `007F6D30`); `004134F0(+4Ch, identity)`; pushes one window {0, -3.14141846f, 3.14141846f, 0, 0} onto +3Ch |
| 007F6CA0 | BSP_GunPlatform_HasOnlySeedWindow | 0 when the window count is > 1; else the first window's flag bit, inverted |
| 007F5580 | BSP_PodArray20_Insert_007f5580 | insert at `where` (grows to 2c + 1); returns the slot |
| 007F5800 / 007F6A50 | BSP_PodArray20_Grow / PushBack | 2c + 2 growth of the 14h-byte elements |
| 007F74B0 | BSP_PlayerRecord_IsTalking | clears +8Ch when it is more than 10 s ahead of game+64Ch; true while +8Ch + 1.0 > now |
| 007F81C0 | BSP_NoseArt_SetSelection | for the three selections that changed: store, release +104h+4i |
| 007F8210 | BSP_Session_ReceiveMessageE3 | mode 1: `007F81C0` on the sender's record, then `0076A9F0(game+1EF0h, msg, 0)`; mode 2: `007F81C0` only |

## 4. Existing implementations, verified and recorded

- `007ECFE0` BSP_SensorCategory_Delegated: `include/bsp/sensor_tables.hpp:322`. The bytes
  `007ECFE0-007ECFFC` read `[ECX+1ECh]`; when it is non-null they tail-jump through the vtable slot +4h of
  `ptr+1E4h`, and otherwise return 6 (`unclassified`). ECX is the squadron's +1E4h sub-object, so +1ECh is
  squadron+3D0h, as the host's comment says. It matches.
- `007ED650` BSP_Squadron_ReloadBombPlatforms: `src/air_operations.cpp:217`. The bytes call `007C1D80`
  per member (`007ED667`), then set byte +3ECh = 1 (`007ED67C`). It matches.
- `007DAFD0` BSP_PlaneFlightController_ApplyForceAtPoint: `water_angular_kick_007dafd0`,
  `src/plane_water_law.cpp:292`. Checked against the bytes `007DAFD0-007DB1BB` (`__thiscall(ECX = ctl,
  body point, world dir, accel, step, limit)`, RET 14h):
  - the transform (`0042D0D0`), the cross product (`004F9B30`, EDX = dir) and the sum
    `(a0^2 + a1^2) + a2^2` all match;
  - the 1e-10 test (FCOMI/JBE), the 0.001 fallback to unit+674h (FCOMIP/JBE) and the normalisation
    (`0042B260`) match;
  - the per-axis rule (c < 0 or NaN: add when w > limit*c; otherwise add when w < limit*c) is
    equivalent to the host's: a NaN c is a no-op on both sides.

  One step is not in the host function: the image first refreshes the unit's pose through
  `00414DB0(unit)` when byte unit+C8h is 0 (`007DAFE5`). The host takes the pose as an input. It
  matches on that condition.

## 5. Left (not reconstructed in this pass)

- **Too big or string-heavy:**
  - `007D1E50`, the plane class constructor (Ghidra has no function there);
  - `007D5AC0`, checked-iterator STL plus effect creation;
  - `007D3E60`, `007F35C0`, `007F3E50`, `007F4BA0`, `007EB380`, `007EC5A0`;
  - `007F3820`, the rename (string search, substring and concatenation);
  - `007DC830`, the free-flight step: x87 law with interpolations, which the host only counts.
- **Plane neighbours (x87):** `007E2090`, `007DEDB0`.
- **STL instantiations (out of scope; names stand):** `007F6D30` and `007F6F80` (std::list head node
  and constructor), `007F7070` (std::list clear), `007F3D40` (std::operator+(const char*, string)).
- **Segment 40** was not started.

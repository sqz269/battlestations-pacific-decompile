# cc13-seg40: segment 40 leftovers (first pass)

Packet `cc13-seg40`. Segment 40 is `00728A90-00735EC0` (weapon classes: barrels, torpedo, load time). This
first pass takes the small and medium functions that nothing in `src/` or `include/` implements: gun
session messages, gun visibility and event hooks, node factories, the narrative destructors and owned-pointer
helpers.

Code: `include/bsp/cc13_seg40.hpp`, `src/cc13_seg40.cpp`. `Cc13Seg40Host` extends `Cc13Seg48Host`
(docs/CC13_SEG48.md section 2 has the conventions: raw native blocks, a named hook for every call, null-mask
tests read as plain null tests). Names are hypotheses, not recovered symbols. Nothing is wired into the
game, and nothing was run. Status: build-tested only.

Evidence: Capstone `disasm-raw` over the disk bytes, plus the pseudocode. Constants were read with
`ghidra bytes`:

- pi as a float, `00D7A264` = 40490FDB;
- FLT_MAX, `00CFDBF8` = 7F7FFFFF;
- 256.0f, `00CF8B3C`;
- the doubles 2pi, -pi and +pi (each from a float), `00CE3828`, `00CE3D18`, `00CE3D28`.

No item touches plane level-bomb, goaway, torpedo AI or submarine AI. The torpedo shot command's read and
destructor are archive and teardown code.

## Functions

| Address | Name | What the code does |
|---|---|---|
| 007293D0 | BSP_TickSubnodeOwner_DestructBase | `00695870(owner ? owner+1Ch : null)`, then `00875B30(owner)` |
| 007294A0 | BSP_FreeOwnedBlock_007294a0 | free [holder] (00BF6989) and clear it |
| 007294C0 / 007359D0 / 007359F0 | BSP_OwnedPtr_Delete_* | [holder]->vt[0](1), clear |
| 0072B990 / 0072B9B0 | BSP_Entity_AsKind08 / AsKind06 | the entity when vt[5Ch](8 or 6) is true, else null |
| 0072BCE0 | BSP_GunBot_OnTargetDestroyed | when `00521EA0(this+1Ch)` is the destroyed entity: (this-1Ch)->vt[30h]() |
| 0072BDD0 | BSP_GunBot_SetFireTargetPoint | vt[30h](), then the point at +40h..+48h and the flag bytes |
| 00728BE0 / 00728C90 | BSP_SessionMessage_GUN_CREATE_Write / Read | entity-create base; bytes +111h, +110h; words +112h; the +116h-gated angles +118h/+11Ch (16 bits over pi); boolean +120h. The reader's cursor is stream+4h |
| 0072A1D0 | BSP_SessionMessage_GUN_CREATE_Construct | `00766FC0(ABh, a, gun, 0)`; barrel ids from gun+3A8h (stride 14h); when vt[5Ch](22h), gun+488h/+48Ch wrapped to [-pi, pi] by `00BF857A` fmod |
| 0072A190 | BSP_SessionMessage_GUN_CREATE_CreateEntity | `00443490(word +1Eh, 0)`->vt[28h](word +1Ch); entity +38Ch and +3B8h |
| 0072B640 | BSP_Gun_SendCreateMessage | builds the 124h-byte message on the stack, `00779FC0(gun, peer, &msg)`, inline destructor `007668E0` |
| 007290D0 | BSP_SessionMessage_S_T_GUN_FIRE_Construct | `0075B430(B0h)`; aim angles from gun+488h/+48Ch, each wrapped by `00605070`; the shot fields |
| 007291C0 / 007292C0 | BSP_SessionMessage_S_T_GUN_FIRE_Write / Read | header; angles (10 bits over pi); +28h in 5 bits; word +38h; the gated FLT_MAX floats +30h/+34h and +3Ch |
| 007297B0 | BSP_Gun_ApplyWreckVisibility | model 1.0 and MeshHolder children 0.0 when gun+358h is 0, else the reverse |
| 00729870 | BSP_Gun_ShowOnlyMeshHolder | model 0.0, MeshHolder children 1.0 |
| 00729FD0 / 0072A0D0 | FUN_00729fd0 / FUN_0072a0d0 | on the "destroyed" event, route a stack message of type 68h or 69h (+20h = 256.0f) through `0077C2A0(entity, &msg, 7, 0)` |
| 0072AA10 | BSP_GunEffectSet_SetMix | vt[20h](float) on the null-tested +0Ch list, then on the +18h list |
| 0072ABE0 | BSP_GunBarrelTable_Destruct | vtable 00CFDC0C; free and clear +0Ch and +10h |
| 0072B5A0 | BSP_Gun_ApplySetDestroyedFailure | on request vt[0Ch](68h): +3B8h = 1 and vt[1B4h](last index); then the stop-effects / category rebuild |
| 00730E80 | BSP_RapidFixedSlaveGun_Construct | `006FDDA0(gun, arg)`, six vtables, kind +C4h = 23h |
| 00730FB0 | BSP_GunAimError_Write | "type" = "flak" (type 0), "distErr" (+4h) |
| 00731220 | BSP_TorpedoShotCommand_Destruct | vtable 00CFE740, `004B7EF0(+0Ch)`, base vtable 00CFE2B8 |
| 00731270 | BSP_TorpedoShotCommand_Read | "heading" +4h, "swimdepth" +8h |
| 00731A50 | BSP_GunClass_ActivateModelAndBulletClasses | when +44h is 0: `0043EBD0(class, arg)`; per bullet record, `006EAFE0` then release |
| 00731C80 / 00731D50 | BSP_NodeFactory_CreateMultipleBombPlatform / CreateRapidFixedSlaveGun | new 480h / 4D0h, memset, `00730B80` / `00730E80`, class back pointers, name copy |
| 00733B10 | BSP_SessionMessage_MISSIONINFO_Construct | `0075B430(35h)`, vtable 00CFE96C, copies the text string unless it is the same object |
| 0072CAE0 | BSP_NativeVector14_Destruct_0072cae0 | `0072B6D0(0)`, free data |
| 0072CC60 | BSP_NativeParticlePopulationLock_Destruct | `0041CC80(+4h)`, clears the singleton [0108FF50] |
| 00733E70 / 00733F00 | BSP_Countdown/NarrativeCallbackHolder_Destruct | the Lua argument vector (`006B88E0`, two frees), then the name string back to the pool |
| 00733DC0 | BSP_NarrativeEntry_Destruct | the holder body (vtable 00CFE960, vector +14h, string +0Ch), then the string at +4h |
| 00735080 | BSP_MissionNarrative_Destruct | vtable 00CFE9C0; 00733F00(+38h); 00733E70(+10h); `00734CE0`; free [+4h]. The pseudocode shows +8h, but the bytes read +4h |

## Simplifications

- **Angle wrap (0072A1D0).** The x87 FADD/FSUB of the double 2pi rounds once to float at the game's
  24-bit precision. The C++ adds in double, then rounds to float, which can differ in the last float bit.
- **Stack messages.** The stack messages (00729FD0, 0072B640) are zeroed before construction because C++
  needs a value; the image does not zero them.
- **The "flak" text.** 00730FB0 passes the address of a local copy of "flak" rather than 00CFE738.
- **Byte arguments.** Byte arguments that the image pushes as dwords are passed zero-extended.

## Left in segment 40

- **Hosted or implemented elsewhere, not yet re-verified:** every address with a grep hit in `src/` or
  `include/`. This includes the gun-class Lua readers, the barrel and aim functions in
  `game_hosts_gunnery.cpp`, `cc13_seg33a/b`, `cc13_seg49` and `gun_aiming`.
- **Not read:**
  - `0072A3B0`, `0072A6D0`, `0072B6D0`: engine vector templates;
  - `00729D30`, `00731AE0`, `00731C10`, `007323E0`, `00732F50`;
  - `00733C80`, `007337A0`, `007342B0`, `007343D0`, `00734460`.
- **STL instantiations (names stand):** `00733430` / `00733AF0` (list head node and constructor),
  `00732560` (std::copy into a back inserter).

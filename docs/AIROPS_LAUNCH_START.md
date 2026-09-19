# The launch start, 006C7490, and what builds the squadron, 006C5050

Addresses: 006C7490, 006C5050, 006BF150, 00694A60, 006952A0, 00922F30, 004F0AD0, 008F3710,
008F3770, 006CC690, 006BF620.

## 1. A correction to this thread's own code first

`src/air_operations.cpp` wrote `block+38h` when it started a launch. **Neither 006CC690 nor
006C7490 writes that field.** 006CC690 only compares it at 006CC715, 006C7490 does not touch it,
and 006C5050 only tests it at 006C5078. The write was mine, and its effect would have been to make
a deck report a launch in progress for ever after the first call, so `IsReadyToSendPlanes` would
have answered false from then on: the opposite of what the packet was for. It is removed.

The name is weakened with it. `docs/AIROPS_LAUNCH_GATES.md` called block+38h *named* on the
strength of two agreeing sites. There are three readers and no writer, so the reading is an
interpretation, not a recovered meaning, and both documents now say so. Three routines agreeing on
how to treat a field is weaker evidence than one routine writing it.

## 2. 006C7490

`__thiscall(block, slot_index, flag)`, called from 006CC690 with the flag zero.

```
slot = slot_index * 58h + block[4Ch]              ESI holds it throughout
006C74C6  CALL 006C5050          squadron = (block, flag, slot_index + 1, class[70h],
                                             slot[8], slot[10h], slot[4Ch], slot[50h])
006C74CB  CMP [ESI+2Ch],2
006C74D6  MOV [ESI+0Ch],EAX      only on that arm: the requested count follows
006C74E0  MOV [ESI+2Ch],3
006C74E7  MOVSS [ESI+30h],XMM0   zero
006C74D9  CMP [ESI+34h],0
006C74F6  MOVSS [ESI+30h],XMM0   the 5.0 cooldown on that arm
006C74FB  MOV [ESI+34h],0
006C74FF  MOV ECX,[ESI+28h]      when it differs from the squadron:
006C750F  CALL 006952A0            unregister the old observer pair
                                   slot[28h] = squadron
006C751F  CALL 00694A60            register the new pair
006C7528  CALL 00922F30          enable the scene node
006C7530  CALL 006BF150
```

**slot+28h is filled here.** That is the field the Lua `slots` reader publishes as `squadron`, so
this routine is the whole reason the mission script's line after `LaunchSquadron` can resolve a
striker.

**State 3 is a fourth value** for slot+2Ch, after the 1, 2 and 5 the launch routines showed and the
6 `006CADD0` mode 1 writes for a `FakeAllocated` slot. What it means is not established; it is
recorded because this routine writes it.

The timer pair at slot+30h and slot+34h is the same one the scene loader writes: a set
launch-requested byte becomes the 5.0 cooldown from `00CE3850` and is cleared.

## 3. 006C5050 builds the squadron through the scene creator

It does not allocate an entity itself. It fills a scene property bag and hands it to
`004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen`, the same creator a `PlaneSquadronGen` scene row
uses. Every key below is a recovered string:

| bag key | from |
| --- | --- |
| `Type` | the class's +70h, passed in by 006C7490 |
| `WingCount` | slot+8h, the count |
| `Skill` | the owner's virtual at +12Ch |
| `Party` | the owner's +54h |
| `OwnerPlayer` | the argument from slot+50h, or the owner's +188h when that argument is 9 |
| `HomeBase` | a reference to the owner itself |
| `State` | 1 on the normal path, 7 when the flag argument is set |
| `VelocitySI`, `Invincible` | zero |
| `Equipment` | slot+10h, only when positive |
| `AutoAttackTarget` | the +174h of the object at slot+4Ch, when that is not null |

Two things fall out. **block+7Ch is the owning entity**, since every one of `Skill`, `Party`,
`OwnerPlayer` and `HomeBase` is read from it. And slot+10h now has a **fourth** name: it is
class+134h, the scene authors it as `Arm`, the Lua reader publishes it as `equipment`, and the
squadron bag carries it as `Equipment`. Four independent spellings of one field, which is as much
corroboration as this field is going to get.

006C5050 also refuses early, at 006C5078, when the flag argument is clear and block+38h is set.
That is the third reader of that field and the reason section 1 calls it an interpretation.

## 4. What this packet implemented, and what it did not

`bsp::air_ops_launch_start_006c7490` performs section 2's slot effects: the requested-count carry,
state 3, the timer pair. It does not create a squadron, so slot+28h stays zero and the Lua
`squadron` key stays absent.

**Creating it needs the units host, and that is unavoidable.** 006C5050 hands a property bag to
`004F0AD0`, and this process's counterpart, `create_plane_squadron_004f0ad0`, is driven from the
scene contents pass with a `SceneUnitCreationInputs` and a `SceneUnitCreatorBinding`. Making a unit
appear mid-mission and take a `thisTable` slot is a units-host and scene-host operation, not an
air-operations or Lua one. Per the packet's own instruction to say so rather than force it, it is
named here as the remaining step rather than attempted from these files.

What that step needs, concretely: a way to run the scene unit creator outside the scene pass, the
owner entity's party and skill to fill the bag with, and the new unit appended to the list
`attach_scene_entities_00928a00` walks so the script's `thisTable` lookup resolves.

## Uncertainty

* What state 3 means, and what state 6 means.
* What writes block+38h. Three routines read it and none writes it.
* The owner's +5Dh byte, which 006BF620 requires clear.
* slot+4Ch and slot+50h are passed to 006C5050 as the auto-attack target source and the owner
  player, and neither is written by anything this thread has read.
* `006BF150`, called at the end of 006C7490, was not read.

## Host methods

`bsp::air_ops_launch_start_006c7490`, section 2's slot effects only.

## Corrections

* **A defect in this thread's own committed code**, described in section 1: the write to block+38h
  was invented and is removed. It is on `main` as part of 605e213ed and this commit corrects it.
* **An overstated name**, also section 1: block+38h was called named on two agreeing readers. It is
  now provisional in both documents and in the header.
* No retraction of a native-behaviour claim beyond those two.

## no_ghidra_function

None.

## Validation

**Not run from this packet.** A USN04 run covering the deck, both gates and the stationary fix was
queued on the shared lock while this packet was written; its result belongs to the run, not here.
The build is clean and both ctest suites pass.

The specific thing the corrected code changes: before it, a second `IsReadyToSendPlanes` on the
same carrier would have answered false whatever the deck said. After it, readiness depends only on
the fields the native reads.

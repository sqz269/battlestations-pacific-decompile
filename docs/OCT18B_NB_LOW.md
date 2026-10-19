# Oct18b nB low: command descriptors, dummy/projectile leaves and director support

All 41 dispatched rows at or below `00721970` are accounted for: 31 complete native source bodies and 10 compiler deleting-destructor/adjustor classifications. No game run, host wiring, game installation change, new tests or independent compiler invocation was made. Compilation is pending the integrator's combined `./scripts/build.ps1` build and source registration. These are source reconstructions; runtime ABI compatibility and game validation are not claimed.

Names are descriptive hypotheses, not recovered symbols. Existing `entity_orders` command vtables establish the close-to-ship, move-on-path, Leave and disband getter families. Native ECX and observed RET cleanup are kept with MSVC Win32 `__fastcall` bridges (unused EDX where a `__thiscall` stack argument exists). Boolean results promise AL only; no-op return registers have no meaningful result contract. The generic `0071D550` tail dispatch is naked so the target's unknown argument/return ABI survives unchanged.

The existing `dummy_depth_seg34_argon` and `director_geommesh_seg38_39_argon` header/source pairs were extended under file leases. The command pair was held by `agent/oct18b-nA:oct18b-nA-order-files`, and the fish-school pair by `agent/oct18b-nC-ai:oct18b-nC-ai`; the two narrow extensions therefore own no duplicate class, parameter layout or host interface. The director pair includes the integrator's four pre-existing gun leaves from `09be907b6`, cherry-picked as `cdc148084`, plus the requested x87 forwarding correction for `00728080`; those four addresses are outside this packet's 41-row count.

`006FCBC0` reads the base bomb-create message from the stream object, then reads the two depth-charge floats through the cursor at stream+4. `007004F0` retains sequential FLD/FSTP copies rather than integer copies. `0071EA90` passes tagged archive arguments `{key-kind=0, literal}` and `{output-kind=1, u32*}` for followMode and startMode, correcting the decompiler's saved-stack-slot aliases. `00721970` forwards to the existing original native `00721890`; it does not implement a second state machine.

The 1,438-byte fish-school renderer `00708120` required full listing review: its decompiler falsely exposes one argument, while the epilogue is `RET 10h`. The true stack is `(context, float lod, unused word, u32 visibility)`. It updates the school after testing context+8's transform, then builds four vertices per 40-byte fish record, refreshes the transform, writes per-fish greyscale into packed or float color attributes, unlocks, and calls generated-model rendering with flags zero. Existing recovered native cross/normalize kernels are reused. Explicit x87 operations keep the float spill boundaries for direction scaling, quad addition, dot/square/shade computation and byte/255 conversion; CVTTSS2SI stays unclamped. The final unsigned visibility conversion keeps signed FILD plus the original 2^32 correction before its float spill. The original update/geometry/world-refresh/base-render dependencies remain native image bindings, not invented link stubs. This has not been rendered or compared in the game.

`reports/oct18b_nB_low.json` is the complete row-level status and direct-call-site record. Ghidra mutations use `ghidra_annotate.py --apply` under its write lock, preserving previous comments and recording old values under `local/`; refreshed exports are shared through the normal workspace resolver. Exact final annotation/export/build results are recorded in the report validation fields.

| Address | Symbol | Status | Original ABI |
| --- | --- | --- | --- |
| `006f8e60` | `CommandCloseToShip_Category_006f8e60` | native source; build pending | __fastcall(ECX unused), RET, EAX=3 |
| `006f8e70` | `CommandCloseToShip_RequiresTarget_006f8e70` | native source; build pending | __fastcall(ECX unused), RET, AL=1 |
| `006f8e80` | `CommandCloseToShip_Property10_006f8e80` | native source; build pending | __fastcall(ECX unused), RET, EAX=1 |
| `006f8f00` | `CommandMoveOnPath_Category_006f8f00` | native source; build pending | __fastcall(ECX unused), RET, EAX=3 |
| `006f8f10` | `CommandMoveOnPath_RequiresTarget_006f8f10` | native source; build pending | __fastcall(ECX unused), RET, AL=0 |
| `006f8f20` | `CommandMoveOnPath_Property10_006f8f20` | native source; build pending | __fastcall(ECX unused), RET, EAX=0 |
| `006f8f30` | `CommandMoveOnPath_CopyDefaultParameters_006f8f30` | native source; build pending | __thiscall(ECX unused, stack out[4]), RET 4, EAX=out |
| `006f9010` | `CommandLeave_Category_006f9010` | native source; build pending | __fastcall(ECX unused), RET, EAX=0 |
| `006f9020` | `CommandLeave_RequiresTarget_006f9020` | native source; build pending | __fastcall(ECX unused), RET, AL=0 |
| `006f9030` | `CommandLeave_Property10_006f9030` | native source; build pending | __fastcall(ECX unused), RET, EAX=0 |
| `006f90b0` | `CommandDisband_Category_006f90b0` | native source; build pending | __fastcall(ECX unused), RET, EAX=0 |
| `006f90c0` | `CommandDisband_RequiresTarget_006f90c0` | native source; build pending | __fastcall(ECX unused), RET, AL=0 |
| `006f90d0` | `CommandDisband_Property10_006f90d0` | native source; build pending | __fastcall(ECX unused), RET, EAX=0 |
| `006fca30` | `CG_DepthCharge_DeletingAdjustor_006fca30` | compiler support, out of scope | ECX subobject, stack deleting flags; adjusted tail dispatch, target cleanup |
| `006fcbc0` | `SessionMessage_DEPTHCHARGE_CREATE_Read_006fcbc0` | native source; build pending | __thiscall(ECX message, stack stream), RET 4, void |
| `006fe9d0` | `CG_DummyKamikazePlane_DeletingAdjustor_006fe9d0` | compiler support, out of scope | ECX subobject, stack deleting flags; adjusted tail dispatch, target cleanup |
| `006ff540` | `CG_Dummy2D_DeletingAdjustor_006ff540` | compiler support, out of scope | ECX subobject, stack deleting flags; adjusted tail dispatch, target cleanup |
| `006ff6d0` | `DummySubmarine_PlaceStepPose_006ff6d0` | native source; build pending | __thiscall(ECX self, stack float dt), RET 4, void |
| `006ff8a0` | `CG_DummySubmarine_DeletingAdjustor3E4_006ff8a0` | compiler support, out of scope | ECX subobject, stack deleting flags; adjusted tail dispatch, target cleanup |
| `006ff8c0` | `CG_DummySubmarine_DeletingAdjustor310_006ff8c0` | compiler support, out of scope | ECX subobject, stack deleting flags; adjusted tail dispatch, target cleanup |
| `00700210` | `DummyTargetVehicleClass_VSlot04_Zero_00700210` | native source; build pending | __fastcall(ECX unused), RET, EAX=0 |
| `00700220` | `DummyTargetVehicle_VSlot17C_NoOp_00700220` | native source; build pending | __thiscall(ECX unused, stack unused word), RET 4, no result |
| `00700230` | `DummyTargetVehicle_VSlot180_NoOp_00700230` | native source; build pending | __thiscall(ECX unused, stack unused word), RET 4, no result |
| `007004f0` | `DummyTarget_CopyVector734_007004f0` | native source; build pending | __thiscall(ECX self, stack float[3] out), RET 4, EAX=out |
| `00700aa0` | `CG_DummyTarget_DeletingAdjustor_00700aa0` | compiler support, out of scope | ECX subobject, stack deleting flags; adjusted tail dispatch, target cleanup |
| `00708120` | `FishSchoolModel_Render_00708120` | native source; build pending | __thiscall(ECX model, stack context,float lod,unused word,u32 visibility), RET 10h, void |
| `00714f70` | `CG_SingletonE19A90_ScalarDelete_00714f70` | compiler support, out of scope | __thiscall(ECX self, stack deleting flags), RET 4, EAX=self |
| `00714fb0` | `CG_SingletonE19B84_ScalarDelete_00714fb0` | compiler support, out of scope | __thiscall(ECX self, stack deleting flags), RET 4, EAX=self |
| `00714ff0` | `CG_SingletonE19B8C_ScalarDelete_00714ff0` | compiler support, out of scope | __thiscall(ECX self, stack deleting flags), RET 4, EAX=self |
| `0071bc40` | `DirectorPathBase_VSlot04_False_0071bc40` | native source; build pending | __fastcall(ECX unused), RET, AL=0 |
| `0071bc50` | `DirectorPathBase_VSlot08_Zero_0071bc50` | native source; build pending | __fastcall(ECX unused), RET, EAX=0 |
| `0071bc60` | `DirectorPathBase_VSlot0C_False_0071bc60` | native source; build pending | __fastcall(ECX unused), RET, AL=0 |
| `0071bc70` | `DirectorPathBase_VSlot10_MinusOne_0071bc70` | native source; build pending | __fastcall(ECX unused), RET, EAX=ffffffff |
| `0071bc80` | `DirectorPathBase_VSlot14_MinusOne_0071bc80` | native source; build pending | __fastcall(ECX unused), RET, EAX=ffffffff |
| `0071bc90` | `DirectorPathBase_VSlot18_NoOp_0071bc90` | native source; build pending | __thiscall(ECX unused, stack unused word), RET 4, no result |
| `0071bca0` | `DirectorPathBase_VSlot1C_NoOp_0071bca0` | native source; build pending | __fastcall(ECX unused), RET, no result |
| `0071bcb0` | `CG_DirectorPathBase_ScalarDelete_0071bcb0` | compiler support, out of scope | __thiscall(ECX self, stack deleting flags), RET 4, EAX=self |
| `0071d550` | `WeaponDirector_ForwardSlot18_0071d550` | native source; build pending | ECX self; MOV EAX,[ECX]; MOV EDX,[EAX+18h]; JMP EDX; target arguments/result/cleanup unchanged |
| `0071ea90` | `PathFollowParams_Read_0071ea90` | native source; build pending | __thiscall(ECX params, stack archive), RET 4, void |
| `00720f40` | `CommandController_VSlot_NoOp_00720f40` | native source; build pending | __thiscall(ECX unused, stack unused word), RET 4, no result |
| `00721970` | `CommandControllerStateMessage_Apply_00721970` | native source; build pending | __thiscall(ECX message, stack controller), tail JMP 00721890; target RET 4 |

Integrator acceptance: all pending build/recovery notes are closed by [OCT18B_NB_DELIVERY.md](OCT18B_NB_DELIVERY.md). The combined Win32 build and both existing CTests passed; no runtime ABI or game validation is claimed.

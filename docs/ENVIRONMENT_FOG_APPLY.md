# Environment fog application

`apply_environment_fog_0078d076` reconstructs exactly
`[0078D076,0078D180)` inside `0078CFF0`. It copies the actual environment's fog
fields to the current camera fog owner through the concrete `system_fog_owner`
setters. It does not reconstruct the preceding water/sky or clear-color work,
the following scaled-scalar update, or the whole parent function.

The parent native ABI is ECX=environment, stack=camera and scalar, `RET 8`.
Within this fragment EDI holds the environment and ESI holds the camera.
The new C++ function has a semantic interface; it is not a callable native
interior or an environment-object layout replacement.

## Fields and live owner

`EnvironmentFogFields` borrows the actual eleven-float region at environment
`+08..+30`, the float4 at `+34`, four directional float4 records at `+44`, and
the float4 at `+84`. These references are stable during the call; the referenced
values are read when each setter executes. No environment allocation, default
values, or field snapshot is introduced. Descriptive color names follow their
observed setter consumers and remain semantic hypotheses.

The camera argument is the **same live** `const SystemFogState*` slot used by
`CameraFrameState.fog_184`, passed by reference. Every nonnull slot value must
come from a live concrete `SystemFogOwner::fields_08`; standalone diagnostic
`SystemFogState` objects are not valid here. The core's
`system_fog_owner_from_state` maps that borrowed projection back to its actual
0x94-byte owner. No registry, alternate owner, or replacement fog state is used.

There are seventeen independent owner reads: one for the primary color, four
for directionals, eleven for scalar setters, and one for underwater color.
No temporary reference increment occurs. Each captured owner must survive its
write. Native code dereferences a null owner; the new interface instead reports
an error while retaining earlier mutations and floating-point effects. It does
not skip the write or supply a default owner.

## Exact update order

The fragment first calls `00B84C40` with environment `+34`, then calls
`00B84FA0` for directional indices 0,1,2,3 using environment `+44,+54,+64,+74`.
It reloads camera `+184` for every iteration. These setters use four forward
integer word copies, preserving raw NaN payloads and the core's overlap order.

| Native source load | Environment source | Fog destination | Setter |
| --- | --- | --- | --- |
| `0078D0A8` | `+08` | `+68` | `00B84D00` |
| `0078D0BA` | `+0C` | `+6C` | `00B84D10` |
| `0078D0CC` | `+10` | `+70` | `00B84D20` |
| `0078D0DE` | `+14` | `+78` | `00B84D40` |
| `0078D0F0` | `+18` | `+74` | `00B84D30` |
| `0078D102` | `+1C` | `+7C` | `00B84D50` |

Next, `0078D114` reloads the owner and `0078D121` calls underwater-color setter
`00B84C70` with the environment float4 at `+84`. The final scalar sequence is:

| Native source load | Environment source | Fog destination | Setter |
| --- | --- | --- | --- |
| `0078D126` | `+2C` | `+80` | `00B84D60` |
| `0078D138` | `+30` | `+84` | `00B84D80` |
| `0078D14A` | `+20` | `+88` | `00B84DC0` |
| `0078D15C` | `+28` | `+8C` | `00B84DE0` |
| `0078D16E` | `+24` | `+90` | `00B84E00` |

Each scalar uses `FLD source`, then reloads camera `+184`, then uses `FSTP` to
prepare the setter's float32 stack argument. The setter itself is a raw `MOVSS`
copy. `load_scalar_and_owner` preserves the three operations in one assembly
sequence and passes the resulting DWORD bits to the core setter. A normal C++
float temporary before the owner read could quiet a signaling NaN or spill at
the wrong point. No arithmetic, scale, clamp, normalization, mode gate or
floating-point control-mode change occurs in the owned fragment.

## Verification and integration

The configured Ghidra `bsp` project and `/battlestationspacific.exe` were verified
through `bsp.py ghidra` before the analysis batches. The 266-byte fragment and
both supporting setter regions exactly match the installed executable. The
audit records lengths, hashes, original annotations and an unapplied fragment
comment proposal. The worker performs no Ghidra mutation or shared-ledger edit.

The new source compiled standalone with MSVC Win32 `/W4 /WX /O2 /fp:strict`.
The ignored native comparison runs the copied fragment and original leaf
setters, relocating only calls and ending at `0078D180`. Two input cases compare
the complete 0x94-byte destination owner and x87 status: finite inputs match
with status `0000`; signaling NaNs, denormals, signed zeros and infinities match
with status `0003`. Reference counts and vtable words remain unchanged. The
host side links the concrete core worker's setter objects, not test stubs.
Dynamic owner replacement during an unmasked floating-point exception was not
runtime-tested; its read placement is established by the preserved assembly.

`scripts/build.ps1` was attempted. The inherited base checkout fails in
`render_tail.cpp` because its `ParticleClock` header lacks `owned_records`;
this is separate from these four new files. No shared fix was made. The primary
integrator owns that pending integration, persistent source registration, core
dependency integration, annotation/save/export, and the final whole build.
Only a dependency header copy under ignored `local/include` was used here.

There is no loaded environment factory, original object ABI, GPU, gameplay, or
full `0078CFF0` validation claim.

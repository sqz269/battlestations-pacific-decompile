# Additive qualified Dyn Root construction

`dyn_profile_construct_00c50310(profile,context,delete_partial_00bf65ac)` adds the recovered Root raw-allocation cleanup to the existing source constructor. It composes the already admitted qualified C44000 constructor and retains its catch across the profile head stores, actual publication and existing tail operation. The old two-argument Root API and both enclosing engine callers remain normal-only and unchanged.

Base: `5ec9904f55e48f5d3cd871444b510b5cb5e1532b`. This packet owns only `dyn_engine_runtime.hpp/.cpp` and these evidence files, plus a read-only address lease for `00C50310`. No scope/process files, caller propagation, CMake, guards, ledgers or Ghidra state were changed. The readiness input is `713c8776330c076604279ec18fa4baee982020e9`; the pending default-domain supplier/child-caller packet is not merged into this tree.

## API and genuine inputs

The new required argument has type `DynProfileRootPartialNodeDelete = void (__cdecl*)(void*)`. This separate alias avoids including `dyn_profile_scopes.hpp` back into its engine dependency. Its underlying flat function type is compatible with the profile-child deleter type; no captured-context thunk, native address cast or default free is supplied.

The caller must supply the same actual engine/profile memory, actual profile-publication cell, real root-name input and a genuine raw-storage deleter matching the **captured allocation's heap**. The provider must return normally. `context.memory.release` remains the distinct BF6989 vector service used by qualified C44000; an opaque custom release or matching-looking name does not prove flat BF65AC custody. A nonnull function pointer does not authenticate an arbitrary manually constructed context. The source default-domain supplier is separate; this packet does not qualify custom memory or bind a production caller.

The new overload rejects a null provider before reading caller profile/context data or invoking the profile allocator. It then allocates the actual `48h` raw node **outside** its acquired cleanup scope and captures that original address. A returned null follows the retained native branch: Root head/current become null, publication still selects the incoming profile and its `90h` tail is cleared. This is not a fabricated null-success allocator or fallback profile.

## Exact original cleanup span

Read-only inspection of the configured BSP project/program and fresh on-disk PE bytes establishes:

| Native epoch | Obligation |
| --- | --- |
| `00C50328` | Allocate `48h` through BF681B, before Root cleanup state. |
| `00C50330/00C50335` | Capture returned allocation; arm state `0`, before the null test branches to construction or head stores. |
| `00C50347` | Construct C44000 when nonnull. |
| `00C50355..00C50360` | Store Root head/current and zero the third profile word. |
| `00C50367/00C5036D` | Publish incoming profile, then call native BF79F0 for its `90h` tail. |
| `00C50378` | Restore the previous exception record; there is no earlier state demotion. |

Handler `00CC836E` loads FuncInfo `00E041D0` (magic `19930522h`, max state `1`), whose map `00E041C8` is `{-1,00CC8363}`. Action `00CC8363` loads captured `[EBP-10h]`, pushes it, calls `00BF65AC`, balances the argument and returns. It performs no head/global clear, unregister, current-node reload or rollback. With original entry `ESP=E`, captured allocation is `E-10h` and state is `E-4`; the funclet's reconstructed `EBP=E` selects that captured slot.

## New source and emitted behavior

After allocation, the new `try` calls real four-argument qualified C44000 with `context.memory`. Its existing constructor cleanup first runs A900's current-vector conditional release when required, then rethrows. The Root catch subsequently calls the required raw deleter with the original captured allocation and rethrows. No fully reconstructed Root class destructor or current-publication cleanup is substituted.

The catch remains around all three head stores, `*context.profile_slot_0109e9f8=&profile`, the existing `std::memset(profile.bytes+0xC,0,0x90)` and return. Unlike C50390's child append, Root does not disarm cleanup before the publication/tail source extent. If a covered failure occurs after publication, reached fields/publication remain and may reference the freed node; the caller's enclosing failure/lifetime policy remains required. A captured null is passed unchanged to the genuine deleter if a later covered C++ operation throws. There is no clear, retry, parent rollback or different-node free.

Fresh strict Win32 emitted code confirms the new public entry's relative offsets: null guard `+2Bh` before context load `+31h`; allocator call `+3Bh` under state `-1`; captured slot `[EBP-14h]` written `+40h`, state `0` armed `+43h`; qualified C44000 call `+55h`; head stores `+67h/+69h/+6Ch`, publication `+76h`, `_memset` call `+7Ch`, with no state demotion before FS restoration `+89h`. The generated catch loads only saved `[EBP-14h]`, calls provider `[EBP+10h]` and rethrows. The null-provider branch targets the separate `invalid_argument` block and does not enter captured cleanup. Source compiler slots are not the original register/frame ABI.

The actual source tail is still `std::memset`, emitted here as `_memset`; this is not proof of native BF79F0 feature-word selection, faults, SEH, original FH3 transport or secondary-cleanup behavior. The C++ catch's evidenced span does not grant those stronger guarantees.

## Compatibility and verification

Strict baseline and changed MSVC Win32 object compilations passed `/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX /c`. An independent COFF comparison finds all **16 pre-existing function/helper records** identical in machine bytes and normalized relocations, including both old C44000 APIs, the old Root API, C55EA0 and C55F50. Only anonymous namespace hashes are normalized. No context/storage fields or layouts were extended. Old enclosing callers do not select the new overload.

The report pins original body/map/action/handler bytes, exports, source snapshots, strict objects/listings and emitted-scope inspection. No new test, fixture, probe, game, DLL call or full build was run; root owns the common build and independent review before integration. No Ghidra mutation or vendor-body reconstruction was performed.

C55EA0/C55F50 allocation scopes and caller propagation remain separate work. Real profile/root producer custody, custom-heap qualification, whole-graph failure handling and original ABI/FH3/SEH/hardware-fault/secondary-cleanup/runtime proof remain unclosed. Existing math source and frozen gameplay evidence retain their prior exact scope. No new native/vendor-body, activation or gameplay credit is added.

[Implementation report](../reports/cc16_dyn_profile_root_qualified.json).

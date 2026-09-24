# Canonical application material process (CC10)

The application now explicitly initializes the existing native material and
parameter pools before renderer consumers. One `GameNativeMaterialProcess`
retains their actual storage/companions and the effect-construction serial.
It borrows the existing E188B4 allocator-list domain. No material compiler,
effect cache, staged B107F0 initializer or new owner registry is activated.

## Native storage and startup

| Native cell | Source owner and admitted aliases |
| --- | --- |
| F8D3A8, 4 bytes | One volatile effect serial; `NativeMaterialEffectConstructionAccess::next_serial_00f8d3a8` borrows its accessor |
| F8D3AC, 38h bytes | `NativeMaterialPoolStorage` and `NativeMaterialPool`; the same object implements `NativeMaterialSlotPool` |
| F8D3E4, 38h bytes | `NativeMaterialParameterPoolStorage` and `NativeMaterialParameterPool`; the same object implements `NativeMaterialParameterSlots` and supplies `NativeMaterialParameterAccess::parameter_slots` |

All three installed PE extents lie beyond `.data`'s file-backed size and inside
its virtual size; their source-process storage starts with the proven loader-zero
preimage. F8D3E4 is distinct from particle parameter pool F8D344 and hierarchy
pool 0109022C. The existing material-pass pool 0108FBF8 is also unchanged.

The original CRT table contains CE34F4=CD78B0, CE34F8=CD78D0,
CE34FC=CD78F0, CE3500=CD7910 and CE3504=CD7960. The application inserts
the two new calls after its existing particle-parameter CD78B0 call and before
its existing layout-tree CD7960 call. CD7910 remains separate work.

| Explicit startup | Existing constructor | Actual registered exit | Existing destructor |
| --- | --- | --- | --- |
| CD78D0, 22 bytes | B17FA0 | CE0BF0, 10-byte tail thunk | B180D0 |
| CD78F0, 22 bytes | B18340 | CE0C00, 10-byte tail thunk | B18470 |

Each entry binds the process's canonical companion, initializes its original
pool, then invokes real `std::atexit` through the existing raw wrapper. A
process mutex/attempt state preserves the original registration result on later
calls. A thrown attempt cannot retry; a nonzero registration status leaves the
initialized pool intact without a substitute callback or rollback. Accessors
require that explicit initialization returned; borrowing them performs no native
construction or reset.

B18D60 samples F8D3A8 at B18E60, writes the captured DWORD to effect+C0 at
B18E69 and increments the current process cell at B18E7B. This occurs after
the real error-texture acquisition and temporary-name release. The serial is
independent of pool initialization and has no per-application reset. Native
serial increments remain noninterlocked and require caller serialization.

## Lifetime

The shared allocator process is constructed first, followed by this process
owner, followed by the two native exit registrations. The source objects thus
remain alive through application drain and both callbacks. CRT reverse order
runs CE0C00/B18470 for parameters before CE0BF0/B180D0 for materials;
the process bookkeeping destructor adds no second native destruction.

These native pool callbacks free slabs/table backing, destroy their critical
sections and unlink their actual E188B4 elements. They do **not** traverse or
destroy live material/parameter payloads. Callers must finish all payload
retirement, queued uses, borrowed parameter sources and operation-frame
obligations before CRT pool destruction. Pool headers retain native stale
pointer/count/capacity values after freeing; this packet adds no clearing.

## Verification

Strict MSVC Win32 build and all three existing CTests pass. The report records
five instruction-aligned code blocks and the original six-word CRT table,
all matching live Ghidra and the installed PE. Final instruction sizes and
inclusive/exclusive endpoints are explicit. Four direct call rows and two
tail-jump rows pass the live call verifier. No Ghidra mutation, raw-function
rewrite, additional tracked test or unrelated startup entry was added.

One ignored diagnostic was rebuilt from current `src/game_main.cpp`, 70 current
application objects and three current libraries, using `/MD /fp:strict` and
`/MANIFEST:EMBED`. It ran through `tools/run_game.ps1` with its own settings/logs.
At the actual ready application it checked stable pool/accessor identities and
membership in the shared allocator list. Genuine B18D60 base-effect construction
through the existing texture cache consumed serial `0 -> 1`; a real pooled
B18900 material retained that effect. Both were admitted into the same existing
canonical registry and retired through their actual terminal providers. A raw
parameter slot held an actual pooled name, which was released before returning
the slot; this is an allocator-use check, not full parameter registration.

Repeated startup calls returned their original successful statuses while live
pool headers and serial remained byte-for-byte unchanged. After retiring the
payloads, one returned slab remained in each pool for the real exit callbacks.
Probe-only observers, registered around the two native registrations, observed
parameter storage restored to its base profile and unlinked while material
storage was still live/linked; the next observer saw both base profiles and both
elements unlinked with the process owner still alive. Neither observer performs
native cleanup. Application singleton drain returned, final device/API COM
counts were 0/0, and process exit was zero.

## Separate material/compiler composition

R100/R101 already retain actual pass/reflection/D3D shader and source-compilation
providers. Future effect loading must compose these with the real descriptor,
state/pass-cache, effect-owner and VFS domains, keeping each loader/compiler/
sampler operation and descriptor/name backing alive through its native uses.
The actual numeric secondary-pass and renderer-capability bridges are separate;
this packet does not modify their source or supply lookup/compiler callbacks.

The installed `downscale_depth.shfx`, `downscale4x4.shfx`,
`dummy_passtrough.shfx` and `blur5x5.shfx` each begin with a sampler lacking
`TextureSource`. Native B57B50 initialization therefore leaves source+14 at
its established zero. B3B280 reaches that first source0 texture reference
before its sampler-state setter can establish B-16. R98's unknown entry-word
boundary at B3B328 remains: cached shader bytecode does not avoid these calls.
Original private stack/exception ABI, cold full effect loading, full B107F0
execution and gameplay are not established. Bloom's fourth-DWORD backing and
distortion's cleanup-preimage requirements remain unchanged.

The report indexes `local/application_material_process_*`, the local prepare/
build scripts and bounded `local/material_effect_readiness_*` inputs for archive.

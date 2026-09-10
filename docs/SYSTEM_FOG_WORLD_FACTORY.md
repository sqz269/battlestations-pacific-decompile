# Shared fog factory during world construction

`initialize_world_fog_004df6a3` reconstructs `[004DF6A3,004DF7A5)` of
`004DE610`. It allocates one concrete `SystemFogOwner`, publishes it into the
actual optional receiver and actual camera, drops the creator reference, then
initializes the fields. It also produces the packed-color temporary consumed
by the original continuation. The new API owns no second fog state and returns
no additional owning reference.

This is a new MSVC Win32 C++ ABI. Native live inputs are ESI game, EBX
`InterlockedDecrement`, EDI=-1 and the parent stack/EH frame; the interior has
no native return instruction. It is not a drop-in entrypoint or whole game/world
constructor. Names are descriptive hypotheses, not recovered type symbols.

## Actual bindings

`WorldFogFactoryGameFields` holds references to the live game projection pointer
slots. Their contents are read at the corresponding native consumption points,
including after allocation and after the preceding owner release. Neither the
view nor its receiver wrapper takes an entry-time snapshot.

| Binding | Meaning |
| --- | --- |
| `receiver_19e8` | Optional borrowed `FogReceiverFields`, whose `fog_10` references the actual +10 counted pointer of the 40h D64518 receiver |
| `camera_19fc` | The same actual `CameraFrameState` used by existing consumers; publication changes its `fog_184` |
| `scene_record_05fc` | Actual selected scene-record pointer, read for the initial guard and reloaded for every directional copy |
| `actual_packed_color_temporary_18` | Reference to the caller's actual packed temporary; receives `00A5A3AB` |

D64518 is the receiver published to game+19E8 by constructor `00BBDFF0`. It is
distinct from the world object at game+19CC. Existing documentation calls it an
ocean/sky owner, but no RTTI/type name was recovered. The core function name
`set_system_fog_world_owner_00bbdf20` remains a compatible API name for this
actual receiver's +10 field.

All referenced objects and pointer slots must remain live through their native
accesses. The camera is required to be nonnull when consumed. Each existing
nonnull fog pointer must originate in the same concrete live `SystemFogOwner`.
If the scene guard is nonnull, every subsequent live record reload must provide
at least A60h readable bytes. There is no new null-camera fallback, repeated
scene null gate, source snapshot, or substituted default record.

## Reconstructed sequence

`allocate_system_fog_owner()` calls the existing real
`singleton_lifetime_allocate` service and its in-place initializer. The request
is native94h/host `sizeof(SystemFogOwner)`, starts with one reference, and
preserves raw directional allocation bytes. Host new-handler and `bad_alloc`
behavior propagate before the game pointer slots are read. The original
allocator returns a nonnull allocation or throws; its emitted null branch is
not a valid factory fallback because the later decrement unconditionally
accesses owner+4.

The implementation reads receiver+19E8 at `004DF6D0` and, when present, calls
the existing counted receiver setter. It then independently reads camera+19FC
at `004DF6E7` and calls the counted camera setter. Both publish new, retain new,
and release old using the actual core owner. At `004DF6F7` it releases the
creator's reference **before all following field writes**. The camera's retained
reference keeps the same object alive; a present receiver retains a second one.

The field operations are:

1. `FLDZ; FSTP m32`, passed as raw DWORD bits to scalar+68 setter `00B84D00`.
2. Another `FLDZ; FSTP m32`, passed to scalar+78 setter `00B84D40`.
3. `FLDZ; FDIV m64` with the verified 255.0 double from `00CE4B48`. While alpha
   remains on the x87 stack, write RGB words with exact bits
   `3F25A5A6,3F23A3A4,3F2BABAC`, preserve the packed temporary's byte-write order,
   spill alpha to the fourth word, and call raw color setter `00B84C40`.
4. Test the current scene pointer once. If present, reload it independently for
   indices 0..3 and call raw forward directional setter `00B84FA0` with
   record+A20+10*index. Null at the initial guard leaves the constructor's raw
   directional allocation preimage intact.

Normally the x87 spills produce positive zero. They remain actual x87
operations, so masked stack-fault output and status are preserved rather than
replaced by integer zero constants. Directional data never passes through a
C++ float conversion; signaling NaNs and overlapping source reads remain raw.

The packed temporary's native byte-store order is offsets +2,+1,+0,+3 with
values A5,A3,AB,00, yielding `00A5A3AB`. Beyond the owned range, `004DF7A5` loads
it and `004DF7B0` calls `00B6FE50`, which stores camera+190. This implementation
exposes the temporary without performing that out-of-scope camera write.

The scene-record directional source is record+990+90, exactly the region read
later by `copy_authored_fog_0078caa4`. That path fills the actual environment
and its genuinely constructed private B4 owner; the environment apply path
updates this same shared camera fog owner. No extra factory domain is needed.

## Allocation and lifetime evidence boundary

The original parent sets EH state24 around raw construction, whose unwind
record D8FE48 points to `00C6713D`. Raw bytes establish its true returning
extent `[00C6713D,00C67148)`: free parent allocation slot, `POP ECX; RET`.
The state returns to -1 before either publication. It does not roll back
published fog references or represent the creator's later decrement.

The implementation reuses the already integrated `noexcept` initializer and
real host allocator/refcount/destructor services. It does not replace native
MSVC 2005 handler globals, heap selection, exception objects, or SEH metadata.
Full receiver/camera disposal and scene-list/parser lifetimes remain outside
this fragment. The detailed native binding and lifetime evidence is in
`SYSTEM_FOG_WORLD_FACTORY_NEXT.md` and its discovery report.

## Validation

The new translation unit compiles directly under MSVC 19.51 Win32 with
`/std:c++17 /permissive- /EHsc /fp:strict /O2 /W4 /WX`. A single ignored native
fixture includes the exact owned source to compare its internal continuation;
production code contains no fixture hook or injectable allocator.

The comparison executes original `[004DF6D0,004DF7A5)` after supplying an owner
initialized through the existing core. Original native counted/field setters
execute normally. Eight absolute address operands are relocated, two IAT slots
are bound to the actual Win32 interlocked APIs, and the sole control-flow patch
is `004DF7A5: 8B -> C3` at the unowned continuation. No allocator, destructor,
or refcount callback is replaced. The allocation/initializer prefix and native
EH execution are not part of this differential claim.

Each native/host input has a real B00h allocation with its record base at +0
and its supplied owner constructed at +A1C. Thus record+A20 reads the owner's
actual +04 refcount, and record+A20..A5F lies entirely inside the allocation.
An incorrectly delayed creator release becomes observable in the copied
directional words. No pointer before its allocation is formed. The compared
regions include all owner bytes and untouched surrounding bytes. Afterward,
borrowed slots are retired, each supplied owner receives explicit flags0
destruction, and the enclosing allocation is freed through the real service.

The 64 comparisons cover receiver absent/present, scene absent/present, empty
and full masked x87 stacks, and all four rounding modes at 24-bit and 64-bit
precision. All match for full region bytes, slot identity, old/new refcounts,
packed output, x87 control word, full status word, tag byte and all eight 80-bit
register payloads. Code/data instruction pointers and opcode metadata are
excluded because the native and new C++ ABIs execute at different addresses.
Absent-scene inputs preserve directional preimages containing signaling NaNs,
subnormals and signed zero.

A separate public-API smoke path calls the unchanged production allocator,
checks shared identity/counts and authored words, then releases both published
references through the actual core and final free. It confirms that the packed
temporary is returned while camera+190 remains for the continuation.

All eight native seed checks match disk. `./scripts/build.ps1` on base `e9eb028`
passes both existing CTest checks. The new source is independently compiled
above; CMake registration remains with the integrator. Exact source, fixture,
binary and log hashes plus current Ghidra/disk byte matches are in the audit.
This is reconstructed, strictly compiled and bounded-fixture-tested; it is not
native ABI, allocator-exhaustion/SEH, or game/render validation.

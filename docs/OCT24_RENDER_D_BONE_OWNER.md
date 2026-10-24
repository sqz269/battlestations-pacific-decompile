# MatrixIndexedModel actual owner and lifetime

This packet supplies the genuine bone-bearing model producer required by the
installed Val `MatrixIndexedMesh`. It adds an actual `194h` pool slot and the
`B91220`/`D63590` model, rather than drawing that mesh through an ordinary
`D62DE8` model or supplying a dummy palette. The model lane owns hierarchy and
WeightMapNames materialization; the frame lane owns Render20 profile admission.

## Reused domains and interfaces

`NativeMatrixIndexedModelEnvironment` borrows the SAME `NativeModelEnvironment`
already used by the application. Its node runtime, scene association, raw name
pool, type counter, actual-owner registry, geometry/material owner domain and
constant cells are unchanged. Its separate `pool_01090390` has the original
`194h` slot stride and `+190h` slab-index word. `vtable_00d63590` must be the real
retained Original table alias, not a copied table. Four current type IDs come
from the retained Source type descriptor initialized with that application's
existing counter and `ModelTypeBootstrap` storage; unrelated domains are refused.

The owner embeds `NativeModelOwner base` as an interface to the SAME actual
base subobject. This creates no second node, reference count, scene association,
transform, name, light array or mesh field. `B91220` really calls `B75030`, which
temporarily stamps its base profile as in the image. The continuation then stamps
`D63590` and writes the three actual bone-header cells. Its payload ends at
`190h`; the pool index there is never initialized by a model operation.

Caller order is explicit:

1. Initialize the type with `initialize_native_matrix_indexed_model_type_00cd8570`
   using the existing `TypeIdCounterLifetime` and `ModelTypeBootstrap`.
2. Call `game::initialize_matrix_indexed_model_pool_once_00cd8550`, then borrow
   `game::matrix_indexed_model_pool_01090390`. The process storage uses the SAME
   `game_native_physical_pool_process().allocator_list_domain_00e188b4()`.
3. Allocate a raw slot, prepare its owner and call `construct_native_matrix_indexed_model_00b91220`
   with the original live name header and existing raw-node constants.
4. Reuse `set_native_model_geometry_00b75170(owner.base, ...)` for the actual
   retained mesh. Bind exactly one `NativeMatrixIndexedModelReference` in the
   existing actual-owner registry, as for other actual render owners.
5. Use existing `resize_native_model_bone_nodes_00b90e30` and
   `set_native_model_bone_node_00b90600` on the model's actual address. The
   `184h` header contains real pointers matched to installed WeightMapNames.
6. Keep those bone nodes alive through all model draws and model-reference
   retirement. Bone pointers are borrowed, with no implicit retain/release.

Do not construct `NativeModelReference` on this base or call its `B75290` scalar
delete: those implement the distinct ordinary-model terminal/pool contract.
This packet's final reference uses `B915A0` and returns only to `01090390`.

## Native evidence and original ABI

The existing `C:/Users/sqz269/bsp.gpr` process and
`/battlestationspacific.exe`, x86 Windows, were verified before this analysis.
The following bodies/listings and actual `D63590` table bytes were inspected.
Names remain descriptive hypotheses. New C++ interfaces are not binary ABI
replacements, and previous semantic cc13 helpers remain separate evidence.

| Address | Original contract and retained behavior |
| --- | --- |
| `00B91220` | ECX raw model, stack name, EAX same model, `RET 4`. Calls `B75030`; stores `D63590`, then zero at `184/188/18C`. Full 45-byte assembly inspected. |
| `00B91280` | ECX model, `RET`. B90630 resize-zero on `model+184`, free CURRENT data, then `B750C0`. FH3 state0 cleans the base if the array phase unwinds; do not retry a failing base destruction. |
| `00B915A0` | ECX model, stack flags byte, EAX original address, `RET 4`. Full destruction, then `B90150` on canonical pool `01090390` iff flags bit0. |
| `00B8FB50` | ECX block, stack block index, `RET 4`. `32C4h` slab, 32 slots, free stack at `3280`, 16-bit free count at `32C0`, stride `194`, per-slot index at `190`. |
| `00B90940` | ECX actual `38h` pool, `RET`. Publish allocator-list base links and `D63560`, initialize the real critical section at `+0C` and recursion `+24`, initialize table/count/capacity/current, reserve 32 pointer slots. Three constructor cleanup states retained. |
| `00B90AC0` | ECX pool, EAX slot, `RET`. Lock/increment recursion; append slab when current is -1; grow table `capacity*2+2`; pop free stack; find the next later nonempty slab; decrement/unlock. No catch/rollback unlock on allocation failure. |
| `00B90150` | ECX pool, stack slot, `RET 4`. Read block index at `+190`, signed slot-offset division by `194`, push 16-bit index, increment count, unsigned-min current, decrement/unlock. |
| `00B90A20` | ECX pool, `RET`, no internal lock. Free empty slabs, replace with last, renumber its 32 index words, retry same index, recompute first free slab. |
| `00B90090` | ECX pool, `RET`. Free all slabs/table, drain positive recursive locks, destroy critical section, unlink allocator-list base. No model destructor is implied. |
| `00CD8550` / `00CE0F20` | Construct canonical pool then register its real CRT destructor; callback destroys that pool. Source one-attempt wrapper preserves returned registration status, sticky thrown startup and no retry. It does not establish Original CRT-table position. |
| `00CD8570` | No arguments, `RET`. Sticky guard `01090341`, native name `D63574`, initialize existing Model via `B74F90`, capture model/node/root IDs, then increment existing counter before publishing own ID at `010903C8`. Full listing inspected. |
| `00B91250` | Stack token, boolean AL, `RET 4`; ignores receiver. Exactly four comparisons against live `010903C8..D4`, false otherwise. Assembly corrects the old decompiler's misleading base-address return. |
| `00D63590` | 23 actual DWORDs. `+00 BD30E0`, `+04 B915A0`, `+0C B91250`, `+18 B6F310`, `+20 B748E0`, `+38 B6DB10`, `+40 B6DBE0`, `+48 B6E8C0`, `+50 B6ED80`, `+54 B6EE10`, `+58 6EF890`. |

The raw pool specialization follows the already reconstructed ordinary-model
pool's actual storage, allocation boundary and explicit cleanup structure,
with the independently inspected bone-pool sizes, addresses and profile.
`cc13fx::MitBlockPool` was inspected but cannot supply this owner: it explicitly
models slot order only, using a separate semantic vector rather than actual
slabs/critical-section/storage. The normalized-identical `B90630` resize reuses
the existing `B1C770` pointer-array implementation through the established
`B90E30` adapter. This adds no library-body reconstruction credit.

## Lifetime and evidence limits

The final companion borrows actual `+04` and the existing generated-model/node
lifetime runtime. Logical retirement reuses `B6F310`, scene removal reuses the
current `B6EE10` leaf, and terminal zero validates `D63590`'s real `BD30E0` and
`B915A0` slots. After destruction/pool return it unbinds only the host lifetime
identity and invokes the caller's explicit companion-disposal callback. The
callback must also retire the actual-owner registry association. No field is
read after callback disposal. Existing noexcept callback boundaries remain.

The bone header is resized to zero and its current allocation freed before
base destruction. It does not release any bone node, clear the freed pointer,
or return an ordinary `188h` slot. Callback/context publication is host
bookkeeping only; base destruction restores its own current profile and
bindings before invoking outgoing operations. Interrupted lifetimes require
process retention, as in the existing native owner graph.

Clone `B912E0`, original hierarchy animation, sub-frame interpolation and a
complete GGame renderer are not implemented by this packet. The model lane's
authored transforms and real palette-node construction remain necessary. The
frame lane admits this actual profile to the existing `B748E0` body; it does
not substitute another vtable. Flat drawables with null `+A0` notification
owner need no raw B6DB10 receiver-profile admission change.

This packet has source/listing/ABI and diff-review evidence only. Primary
integration must register the new source, run the authorized Win32 build,
and perform actual draw and terminal/drain validation. No build or game run
was executed in this worker, and no build/fixture/ABI/gameplay parity is claimed.
The existing `reports/native_resource_instance_bones_as.json` call-site check
passed 31 rows with zero failures; that does not test this new lifetime path.
The naming tool preserved prior comments, recorded old values under the local
`ghidra-annotations-20261024T092305Z.json` log, saved the project, and affected
exports were refreshed. Sharded new fragments explicitly remain
`source_reconstructed_reviewed_not_build_tested`.

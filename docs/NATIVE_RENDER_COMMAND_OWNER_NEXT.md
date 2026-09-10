# Native render-command owner implementation packet

The next packet is the **complete actual 44h command lifetime**: placement
constructors `00B1F170` and `00B1F1F0`, shared initializer `00B1EDC0`, destructor
`00B1DDD0`, and scalar deleter `00B1E6B0`. These five bodies total **1,238 bytes**.
Main `0e117e4` now contains the actual pointer arrays, context, batch pool,
group lifetime, and model lifetime needed to implement them. Start with the
actual command storage and destructor/deleter, then add initialization with the
prepared canonical companion associations described below.

This packet supersedes the dependency frontier in `NATIVE_RENDER_COMMAND_NEXT.md`.
That earlier audit remains the baseline for queue publication and execution,
which are outside this packet. This discovery changes only this document and
`reports/native_render_command_owner_next.json`; it does not implement C++, alter
Ghidra, run a new build/fixture, or establish game/render validation.

## Exact entries and storage

| Entry, exclusive end | Bytes | Original ABI | Current Ghidra name |
| --- | ---: | --- | --- |
| `B1F170..B1F1F0` | 128 | ECX=command; scene, camera, target; EAX=same; RET12 | `FUN_00b1f170` |
| `B1F1F0..B1F274` | 132 | ECX=command; scene, camera, second owner, target; EAX=same; RET16 | `FUN_00b1f1f0` |
| `B1EDC0..B1EF57` | 407 | ECX=command; four owners; RET16 | `BSP_RenderCommand_InitializeContextAndBatches` |
| `B1DDD0..B1DFED` | 541 | ECX=command; RET | `BSP_RenderCommand_Destroy` |
| `B1E6B0..B1E6CE` | 30 | ECX=command; flags; EAX=original address; RET4 | `CG_scalar_deleting_dtor_00b1e6b0` |

The command is **not reference-counted**. Its +04 is a retained scene/root
identity. Table `00D5E5E0` contains execution `00B1D950` at +00; +04 begins ASCII
`instanced - `, not a deleting slot. Do not derive the command from
`RenderCommandReference` or place a host vtable/count in its actual storage.

| Actual offset | Field |
| --- | --- |
| `00` | Native table identity `00D5E5E0` |
| `04` | Retained raw scene/root |
| `08` | Unwritten preimage DWORD |
| `0C`, `10` | Two raw actual `NativeRenderBatchStorage*` |
| `14`, `18` | Diagnostic length and buffer |
| `1C`, `20`, `24` | Metadata DWORDs |
| `28` | Raw actual `NativeRenderContextStorage*` |
| `2C..37` | `NativeRenderPointerArrayStorage`, owned indexed groups |
| `38..43` | Same header, borrowed ordered groups |

Use a POD-like `NativeRenderCommandStorage` with size/offset assertions and no
default member initialization or implicit native destructor. Placement construction
starts its typed lifetime without erasing the allocation preimage. Both native
constructors initialize table, scene, diagnostic, metadata, context and the two
array headers. They never write +08; +0C/+10 retain their preimages until each
batch acquisition is published. `B1F170` passes camera twice and consequently
retains its same actual +04 twice in two different context fields.

## Smallest complete host API

Use five address-named functions over actual command storage and one explicit
environment. The environment carries the shared `NativeRenderBatchLifetime`,
`SizedStoragePool` for Pool A, `NativeRenderActualOwners`,
`NativeRenderGroupModels`, and prepared context/batch associations. A host owner
may track phase outside the 44h allocation, but must not provide implicit retained
rollback or run native destruction twice.

The first implementation stage can provide `destroy_native_render_command_00b1ddd0`
and `delete_native_render_command_00b1e6b0` for already constructed actual storage.
Both mandatory batches, scene and context release through the same canonical
`NativeRenderActualOwners` contract. Add a concrete **`NativeRenderBatchReference`**
over `NativeRenderBatchStorage::references_04`: it does not initialize or retain
that count. At zero it validates the current `D5E5AC` profile, including virtual0
`B55680` and deleting +4 `B1C630`, calls the existing
`NativeRenderBatchLifetime::recycle_zero_reference_00b55680`, then retires its host
association. It must not read native storage after pool return or access its
companion after the retirement callback. Its implicit host destructor must not
recycle the native batch again.

The initializer then accepts explicit prepared, nonthrowing `bind_context` and
`bind_batch` associations. They install and return concrete canonical
`NativeRenderContextReference` / `NativeRenderBatchReference` companions for the
exact new raw identities and same actual +04 atomics. This is association work,
not a substitute destruction callback. All companion storage and lookup capacity
must be prepared **before entering the reconstructed native operation**; binding
must not allocate, retain, release, invoke arbitrary owner behavior, or throw in
the valid domain. Validate each returned identity/count association. Unsupported
current profiles have no fallback.

Bind the context after its native placement initialization and before command+28
publication; bind each acquired batch before its corresponding +0C/+10 publication.
These added host associations introduce no native ownership transition. Keep
them attached to surviving objects if the native constructor later fails. Do not
make them local RAII owners that recycle batches or destroy context on unwind.

A context companion and its owner lookup dependencies must outlive the command
whenever another owner holds a context reference. The existing
`NativeRenderContextReference` performs current `D5E5C4` virtual0 `BD30E0` /
deleting +4 `B1D570`, including native deletion and companion retirement.
Directly deleting a context while leaving that canonical companion bound is
invalid. Likewise, direct batch recycling must not bypass an existing companion's
retirement. Caller association/lifetime synchronization must prevent slot reuse
from observing stale bindings.

## Initialization and teardown order

`B1EDC0` allocates **18h bytes**, initializes the six-word context and publishes
command+28. It then assigns command scene+04, context camera+08, second owner+0C,
and target+14, in order. For each unequal identity, publish incoming first,
increment incoming actual+04 if nonnull, then decrement the captured previous
owner and invoke its current virtual0 only at zero. Reload command+28 before
each later context field and before storing the borrowed command at context+10.
An equal-identity assignment does nothing. There is no initializer-owned EH frame.

Obtain the actual batch singleton twice and acquire from each then-current
pool+04. Publish +0C followed by +10 without extra retains. A cached first pool
across both acquisitions changes the native callback boundary.

The default diagnostic is **`"X"`**, read from `00CE9A38`. If current length is
not one, allocate two bytes through the same Pool A service, then reload current
old buffer/length and return the old buffer with length+1 if nonnull. Publish new
buffer, then length1, then NUL at byte1. Finally reload current buffer and length
for the copy from the original default data. If length was already one, skip
allocation and termination but still copy when the current buffer is nonnull.
Use alignment1 semantics. `NativeString::release_to` clears fields and is not the
native diagnostic destructor's dead-storage behavior.

`B1DDD0` first stores `D5E5E0`, then releases required batch+0C and +10 through
their captured actual counts and current zero callbacks. There are no null guards
and these fields are not cleared. Read each later field after preceding callbacks.

Walk actual indexed group cells. Capture the current cell's group; when nonnull,
call the complete `destroy_native_render_group_00b1d760`, ordinary-free that
captured group, then clear the **original addressed cell**. Advance the old
cursor by four bytes and recompute the endpoint from current command data/count.
Do not replace this with a vector iterator, cached endpoint, or delete-before-
destroy shortcut. Callback mutation must preserve the cursor-addressed storage
and a reachable endpoint, as required by the native algorithm.

Release/clear scene+04, then release/clear context+28. Destroy ordered pointer
storage before indexed storage, then return the current diagnostic buffer.
Counts become zero; array data/capacity and diagnostic fields remain dangling
native bytes. Table, +08, batch fields and metadata remain. Ordinary free in the
scalar deleter occurs only after successful destruction and only for flags bit0;
the function returns the original address even if freed.

## EH states and failure boundaries

All three framed functions have three unwind entries. The constructor stores
state0 after scalar fields, then state2 before calling the initializer; their
nonthrowing array initialization has no separately observed state1 store.

| Owner entry | State0 → -1, diagnostic+14 | State1 → 0, indexed+2C | State2 → 1, ordered+38 | Handler / FuncInfo / map |
| --- | --- | --- | --- | --- |
| `B1F170` | `CBCB90 → 41DD20` | `CBCB9B → B1D260` | `CBCBA6 → B1D1F0` | `CBCBB1 / DF506C / DF5054` |
| `B1F1F0` | `CBCBC0 → 41DD20` | `CBCBCB → B1D260` | `CBCBD6 → B1D1F0` | `CBCBE1 / DF50A8 / DF5090` |
| `B1DDD0` | `CBCAB0 → 41DD20` | `CBCABB → B1D260` | `CBCAC6 → B1D1F0` | `CBCAD1 / DF4F58 / DF4F40` |

Constructor failure at state2 cleans ordered array, indexed array, diagnostic.
It does **not** release the scene, context or acquired batches. A host companion
factory cannot silently introduce their rollback. Failure during the destructor's
batch/group/scene/context phase invokes the same three storage cleanups, without
releasing the remaining owners or freeing the command.

Normal destructor state transitions happen before each cleanup: state1 at
`B1DE9F` before ordered storage, state0 at `B1DF32` before indexed storage, and
state-1 at `B1DFBF` before diagnostic return. Disable each cleanup guard before
running that action, so a throwing action does not repeat itself. Valid nonnegative
array headers make resize0 allocation-free; preserve that existing domain rather
than inventing corruption repair. Native exception dispatch was not executed in
this discovery; the map and action bytes were verified and decoded.

## Dependencies and explicit limits

The concrete group destructor uses `NativeRenderActualOwners` for its binding,
`NativeRenderGroupModels` for the actual model slots, and the same Pool A. Models
now resolve to `NativeModelReference`, invoke real `B6DFA0` unlink and current
virtual18 logical release, then perform native model/node destruction and the
canonical 188h pool return when actual +04 reaches zero. A nonempty group is now
a supported implementation dependency; it must appear in acceptance evidence.

Camera companions are available; the raw address is
`&camera_owner.storage.node`, not the host storage view. Arbitrary scene/root,
second-owner, target and group-binding profiles still require real canonical raw
owner associations. `SceneResource` alone is a host scene projection, not a raw
scene+04 overlay. The surface owner implements real native destruction/pool
return, but has no universal `RenderCommandReference` companion and does not
establish texture-target ownership. Do not classify these named dependencies as
complete by providing empty resolvers or zero callbacks.

The existing `SizedStoragePool` is the shared Pool A semantic service, not proof
that its original global allocation layout is reconstructed. Pass the same
service used by current groups/owners and preserve native allocate/return timing;
do not create a private command pool. Actual scalar and pointer storage continue
through the existing shared `singleton_lifetime_allocate/free` boundary.

`B1D950` execution, `B72190` scene gathering, `B1E990` upload/preparation, actual
queue singleton `F8D440`, queue creators, renderer/system/job services and queue
held-context execution remain separate packets. None needs a no-op substitute
to complete the five command lifetime functions above.

## Evidence and implementation acceptance

The guarded CLI checked project `bsp` (`C:/Users/sqz269/bsp.gpr`), program
`/battlestationspacific.exe`, x86 language and image base for each live query.
Fifteen bounded live spans, **1,583 bytes**, match the installed executable
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
They include all five complete bodies, three EH action/handler ranges, three
unwind maps/FuncInfo records, command/context/batch table words, and default
diagnostic data. The report records individual hashes and current source hashes.

Current `B1DDD0` already reaches the full return at `B1DFEC`. Only `B1E6B0`
still has a three-byte missing listing at `B1E6C5..B1E6C8`: verified bytes
`83 C4 04`, `ADD ESP,4`, after free call `B1E6C0`. Clear that call-site override
under the Ghidra write lock, disassemble the gap and refresh the export; do not
change `_free` globally. No repair was applied by this discovery.

Use one focused original-byte fixture, extended through both constructors,
initializer and complete teardown. Compare the 44h preimage, real context and
owner counts, duplicate-camera retain, two actual pool acquisitions, Pool A
diagnostic, and nonempty group with concrete binding/models. Include shared
context survival and callback mutation of later owner fields or group endpoint
where valid. Compare native release/free/clear ordering and dead bytes. Only
minimal host failure cases are needed for the three EH states. Strict Win32
build and fixture equivalence establish their own axes; they do not establish
original ABI compatibility, native exception dispatch or game/render behavior.

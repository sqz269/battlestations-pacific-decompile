# Native camera render-command boundary

`00B1F1F0` is the placement constructor for the existing `44h` two-batch render
command. It does not allocate storage and the command is not reference-counted.
Its single-entry table `00D5E5E0` contains `00B1D950`, the already reconstructed
execution fragment. The queue owns the command pointer and directly calls
`00B1DDD0` followed by `_free` in its control-zero path. The camera instead has
an independently retained reference in a separate `18h` command context.

The next implementation packet can close **native batch lifetime and its free
list**. Full command creation remains dependent on actual command/context/group
backing, actual scene/camera/target retention, and shared queue/job owners.
Existing host `RenderCommandReference` counts and `RenderCommandBatch` deletion
are not the native owner counts or pooled storage lifetime.

This is discovery only. No C++, shared metadata, ledger, Ghidra annotation,
runtime execution, or game validation changed in this packet.

## Concrete storage and ABI

| Owner | Native layout |
| --- | --- |
| Command, `44h` | `+00` table `D5E5E0`; `+04` retained scene/root; `+08` unwritten; `+0C/+10` two retained batches; `+14/+18` native diagnostic string; `+1C/+20/+24` three metadata DWORDs; `+28` retained context; `+2C/+30/+34` indexed groups data/count/capacity; `+38/+3C/+40` ordered groups data/count/capacity. |
| Context, `18h` | `+00` table `D5E5C4`; `+04` count one; `+08` render camera; `+0C` second retained input owner; `+10` borrowed command backpointer; `+14` retained target. In the shadow caller `+0C` is the active source camera. |
| Batch, `18h` | `+00` table `D5E5AC`; actual count `+04`; preparation mode `+08`, untouched by construction; borrowed entry pointer array at `+0C/+10/+14`. |
| Batch pool, `10h` | `+00` table `D5E5DC`; dead-slot free-list data/count/capacity at `+04/+08/+0C`; actual publication `0108FE8C`. |
| Batch lock owner, `8h` | `+00` table `D5E5D4`; owned `1Ch` tracked critical section at `+04`; actual publication `0109DBBC`. |
| Queue, `34h` | Table `D5E5F4`; command data/count/capacity `+14/+18/+1C`; control `+20`; retained current context `+30`; singleton `00F8D440`. Other constructor fields must also be preserved by a later owner implementation. |

Constructor `00B1F1F0` is 132 bytes, `[00B1F1F0,00B1F274)`, thiscall with ECX
storage and four owner-pointer arguments, returns that storage in EAX, RET16.
It writes its table, zeros `+04`, `+14..+28`, and both group arrays, then calls
`00B1EDC0`. It leaves `+08` untouched; batches `+0C/+10` are initialized later
by that helper. It does not memset the allocation or create a count at `+04`.

`00B1EDC0` is 407 bytes, thiscall ECX command, the same four arguments, RET16.
It allocates the `18h` context, installs base then concrete tables, sets count
one, zeros the four context fields, and publishes it at command `+28`. It then
assigns command `+04`, context `+08`, `+0C`, `+14` in that order. Each assignment
publishes the new pointer, increments its actual `+04`, decrements the captured
old owner's actual `+04`, and calls the old owner's current virtual zero slot
only when the decrement returns zero. Equal identity skips both counts. Later
context fields are reached through freshly loaded command `+28`.

After setting context `+10` to the command, it obtains the live pool twice and
acquires each batch through pool `+04`, storing the results at `+0C` then `+10`.
It initializes the diagnostic string to `"X"` through the actual sized string
pool. Existing `NativeString`, `PooledStringStorage`, and `SizedStoragePool`
provide reusable string operations; `RenderCommandDiagnosticString` is a
different host ownership representation.

Context `00B1D120` releases and clears `+08`, `+0C`, `+14` in order, then runs
base `00BD30F0`. Its table routes zero release through `00BD30E0` to deleting
destructor `00B1D570(flags=1)`. It never unconditionally destroys the camera:
the camera's current count and concrete terminal virtual operation decide that.
The command backpointer is borrowed and remains untouched. Complete command
destruction still requires its two valid batches and the actual generated-group
lifetimes; existing fragment contracts are in `RENDER_COMMAND_QUEUE.md`.

## Ready batch lifetime and pool

| Entry | Complete bytes | Original ABI and effect |
| --- | ---: | --- |
| `00B51D20` | 33 | ECX batch, EAX same batch, RET. Base table `CEB130`, count one, table `D62064`, zero entry data/count/capacity; preserve mode `+08`. Acquire then writes `D5E5AC`. |
| `00B51D50` | 113 | ECX batch, RET. Table `D62064`, resize entry array to zero twice in observed order, free its pointer storage, then base `BD30F0`. Borrowed entries are not destroyed. |
| `00B1C630` | 30 | ECX batch, stack flags, RET4; body above, free physical batch only for flags bit one; EAX original address even after free. |
| `00B55680` | 30 | ECX batch, RET. Capture current virtual `+04`, invoke with zero, then get the **current** pool and append the dead raw slot. This is concrete table `D5E5AC` slot zero. |
| `00B1D5B0` | 201 | ECX actual pool free-list subobject `pool+04`, EAX acquired batch, RET. Under the actual lock, pop the last dead slot or allocate `18h`, then reconstruct as above. Popped cells are not cleared. |
| `00B555E0` | 155 | ECX same free list, stack dead-slot pointer, RET4. Under the same actual lock, grow if full, publish pointer, increment count. No retention or construction. |
| `00B1C830` | 95 | ECX free list, stack signed capacity, RET4. Clamp requested capacity to at least one, grow only, allocate four bytes per slot, copy pointers, free old array, publish new pointer then capacity. |
| `00B1CE50` | 80 | ECX free list, stack signed count, RET4. Reserve when needed, zero newly exposed cells, decrement count while shrinking, publish requested count. No batch destruction. |
| `00B1D8A0` | 61 | ECX free list, RET. Raw-free every dead slot in ascending index order with live list/count reloads, resize zero, free pointer array. Does not rerun batch destruction. |
| `00B1E870` | 178 | No inputs, EAX current pool, RET. Double-checked lazy `10h` allocation under captured lifetime-manager lock; zero free-list fields, publish, register actual pool. |
| `00B1E930` | 49 | ECX pool, stack flags, RET4. Clear publication first, table `CE3818`, drain dead slots, free owner for flags bit one; return original pointer. |
| `00B1CD90` | 189 | No inputs, EAX current lock owner, RET. Double-checked lazy `8h` construction and manager registration. |
| `00B1C9D0` | 69 | ECX lock owner, EAX same owner, RET. Table `D5E5D4`, create real tracked section, publish `+04`. |
| `00B1D530` | 55 | ECX lock owner, stack flags, RET4. Destroy/drain tracked section, clear publication, table `CE3818`, free for flags bit one; return original pointer. |

The batch table is `{B55680, B1C630, B55550, B51DF0}`: return-on-zero, deleting
destructor, execute, prepare. Returning a batch does not keep a reference to a
live object. The pool holds destroyed allocation addresses and reconstruction
resets count and entries while preserving the mode preimage. A pool drain
therefore frees allocations directly. Checked native input scope requires valid
nonnegative pointer-array counts/capacities and valid allocated cells; native
signed growth and unchecked allocation/overflow paths must not be silently
reinterpreted as a different collection contract.

`random_threads.hpp` already provides exact-layout `TrackedCriticalSection`,
`critical_section_create_00bd1860`, and
`critical_section_destroy_owned_0041cc80`. The shared
`SingletonLifetimeDomain` supplies actual manager publication/registration and
its captured optional lock. Acquire/return capture the batch lock owner's
current `+04`, call the OS enter operation, increment actual `+18`, then decrement
before OS leave. Use these actual two locks and live publications; a private
mutex or an unrelated singleton domain would break the contract.

## Exception and missing-continuation evidence

The saved exports stop early after some wrongly no-return `_free` annotations.
Verified native bytes restore the loop at `B1D8BB`, free-list publication at
`B1C881`, base destruction/SEH restoration at `B51D9E`, and stack cleanup in
deleting destructors. The audit records complete spans through their returns;
no source was reconstructed from the truncated pseudocode alone.

| Function / FuncInfo | Established unwind actions |
| --- | --- |
| `B1F1F0 / DF50A8`, map `DF5090` | State2: `CBCBD6` destroys array `+38` through `B1D1F0`; state1: `CBCBCB` destroys array `+2C` through `B1D260`; state0: `CBCBC0` returns diagnostic storage through `41DD20`. No scene/context/batch release occurs in this constructor's unwind map. |
| `B1D5B0 / DF4E64`, map `DF4E4C` | State0 `CBC9E0` unwinds captured lock. Reused-slot state1 invokes placement-delete `00401130` through `CBC9E8`; that entry is a verified RET, then state0. Fresh-allocation state2 `CBC9F9` frees storage, then state0. |
| `B1E870 / DF4FFC`, map `DF4FF4` | Only state0 unlocks captured manager section. Publication/registration failure does not imply an invented pool rollback. |
| `B1CD90 / DF4DFC`, map `DF4DEC` | State1 frees in-progress `8h` allocation, then state0 unlocks captured manager section. |
| `B1C9D0 / DF4D68`, map `DF4D60` | State0 calls `B1C3A0`: clear actual `0109DBBC` publication and install `CE3818`. It does not destroy an unpublished section. |
| `B51D50 / DF8A70`, map `DF8A60` | State1 destroys entry array via `B51D00`; state0 invokes `BD30F0`. Normal body also includes the complete return after free. |
| `B555E0 / DF8E50`, map `DF8E48` | State0 unwinds the captured batch lock. |
| `B1F3C0 / DF5154`, map `DF5144` | State1 frees in-progress command allocation, then state0 returns by-value input label storage. After constructor success it is state0; no blanket complete-command rollback is present. |

No native exception was executed in this discovery. These maps constrain the
next implementation and prevent replacing staged cleanup with unconditional
camera/command destruction.

## Shadow creation, queue and jobs

The bounded original caller span is `[00A8EB7C,00A8EC8E)`. After four calls to
`A8BD20`, `A8EA20` loops over cascade cameras. Each iteration allocates `44h` and
passes `(shadow+34, cascade_camera, shadow+30, cascade_target)` to `B1F1F0`.
`shadow+30` is the active source camera; `shadow+34` is the borrowed outer scene
association copied from light `+A4` by the shadow constructor. The command/context
retain their supplied owners independently of the shadow's borrowed fields.

The caller sets metadata `{0,0x62,0x1E}` through `B1BF50`, label `"SHBU"` through
`B1D910`, then calls actual queue getter `004C11F0` and
`B1EB80(command,false)`. The false flag defers gathering. `B1EB80` is 86 bytes,
thiscall ECX queue, stack command and byte-valued flag, RET8; true instead calls
`B72190(command+04, command+28)` and `B1E990` after insertion.

Each deferred command is submitted to the actual job pool from `004C1130`
through its `+04` interface, virtual `+04`, with the concrete job from `A8E090`.
After four submissions the caller invokes that interface's virtual `+08` with
mode zero. The job singleton is the real `8h` owner at `00E18AD4`, primary table
`D5B570`, execution `A8AE50`, and secondary deleting table `D5B56C` at owner `+04`.
The lifetime manager registers that secondary pointer. Its `A8DDB0` thunk
adjusts ECX by minus four before `A8DDE0` clears publication and conditionally
frees the original owner. It has two table words, not a refcount field.

`A8AE50` is 38 bytes: ECX job interface is unused, stack command pointer, RET4.
It obtains context from `B1BF30`, obtains command scene from `B1BF20`, calls
scene gather `B72190`, then group upload `B1E990`. The pushed context survives
the scene getter; that getter does not consume a hidden argument. `B72190`
captures scene `+18` and context camera, follows the actual root chain `+0C`
and each live next `+3C`, tests camera mask `+19C` against node `+48`, and invokes
actual node virtual `+20`. It is not a fixed empty callback.

Later queue execution temporarily retains each current context, executes the
command, releases/clears the current context, and destroys/frees the raw command
when control is zero. `B1D950` has a separate two-batch preparation job service
and wait mode one when both batch counts exceed fifty and synchronization is
disabled. The shadow gather job/wait-zero and execution prepare job/wait-one are
different operations. Existing frame, system-prefix, grouping, and queue
fragments provide behavior, but actual owner/view dispatch must connect them.

Queue construction is also not a safe zeroed placeholder: `B1F280` reads the
unwritten preimage of `+20` before setting it to zero and calls the current
renderer virtual `+11C` if it was two. Queue owner destruction, renderer binding,
job-pool service ownership, and both job singleton registrations remain distinct
follow-up dependencies. No immediate or fake job implementation satisfies them.

## Proposed implementation packet

Add only `native_render_batch_lifetime.hpp/.cpp`, a focused document and audit.
Expose actual `18h` batch, `10h` pool and `8h` lock storage; bind live pool/lock
publication references and the same lifetime domain in an external companion.
Provide concrete get-pool/get-lock, acquire, return-on-zero, batch deletion and
pool/lock deletion operations at the addresses above. Use the existing tracked
section and raw pointer/count/capacity arrays; do not add a separate reference
count, entry vector, free list, or camera owner. Preserve all mode `+08` preimages.

One focused lifecycle check can establish empty acquire, count1-to-zero,
entry-array release before pooling, reacquisition of the same dead address,
mode retention, and final pool drain without double destruction. Command,
context, scene/target dispatch, queue, jobs, and existing host command structs
stay outside this implementation packet. A later command factory must bind the
actual retained owners and integrate real grouping/queue backing before it can
claim shadow-command creation is complete.

Evidence comprises 75 complete function/helper/EH code spans, one bounded caller
span, and 27 data spans: 5,856 bytes matched the installed executable or PE zero
fill. Every live read verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`; the binary hash, spans, proposed packet and exact
local evidence files are in `reports/native_camera_command_next.json`.

# Next native render-command packet

The next ready implementation packet is **the actual three-DWORD pointer arrays**:
`B1C660`, `B1C7C0`, `B1CBE0`, `B1D1F0`, `B1D260`, `B1C6C0`, and `B1CC80`.
These seven complete functions remove a concrete storage prerequisite for native
command/group ownership without claiming those pointed objects are reconstructed.

The next command-specific construction packet is complete `00B1F1F0`, its
three-argument sibling `00B1F170`, and shared initializer `00B1EDC0`, operating on
one actual `0x44`-byte command. It can use the completed context and batch owners,
actual incoming reference words, and shared string-pool service after the storage
and binding contracts below are connected. It is distinct from complete command
lifetime or actual queue publication. General `00B1DDD0` destruction still requires
the native `0x4C` group and concrete retained-owner/model terminal bindings.
Empty groups or a no-op group destructor do not close that boundary.

This is read-only discovery from context commit `9e64eef`, with current main
camera/surface source inspected separately. No C++, shared Ghidra annotations,
ledgers, or build registration changed. All names remain hypotheses and new host
interfaces remain distinct from the original ABI.

## Construction and ownership

The command is **not reference-counted**. Its only table entry at `00D5E5E0` is
`00B1D950`, command execution. Command+04 is a retained scene/root identity, not
its own count. The `00B1E6B0` scalar deleter exists as a separate function; current
Ghidra xrefs find no reference to it, and the known queues call the destructor
then scalar-free directly. Do not add a fictitious deleting slot at table+04.

| Offset | Actual `0x44` command field |
| --- | --- |
| `00` | Table `00D5E5E0` |
| `04` | Raw retained command scene/root |
| `08` | Unwritten constructor preimage |
| `0C`, `10` | Two actual pooled `NativeRenderBatchStorage` pointers |
| `14`, `18` | Native diagnostic length and buffer, eight bytes total |
| `1C`, `20`, `24` | Three metadata DWORDs, initially zero |
| `28` | Raw retained `NativeRenderContextStorage` pointer |
| `2C`, `30`, `34` | Indexed group pointers, signed count, signed capacity |
| `38`, `3C`, `40` | Ordered borrowed group pointers, signed count, signed capacity |

| Entry; exclusive end | Bytes | Original ABI / result |
| --- | ---: | --- |
| `B1F1F0..B1F274` | 132 | ECX=44h storage; four owner pointers on stack; EAX=same storage; RET16 |
| `B1F170..B1F1F0` | 128 | ECX=44h storage; scene, camera, target; EAX=same; RET12; passes camera twice |
| `B1EDC0..B1EF57` | 407 | ECX=command; scene, camera, second owner, target; RET16 |
| `B1DDD0..B1DFED` | 541 | ECX=command; RET; preserves table `D5E5E0` and selected dead-storage bytes |
| `B1E6B0..B1E6CE` | 30 | ECX=command; flags; EAX=original address including after free; RET4 |

The placement constructors initialize table, scene, diagnostic, metadata,
context, and both pointer-array headers, then call `B1EDC0`. They leave command+08
unwritten; +0C/+10 are not written until batch acquisition succeeds. `B1F170`
retains the same supplied camera separately in context+08 and context+0C.

`B1EDC0` allocates **18h bytes for the context**, not 44h for the command. Its
inline stores `B1EDD3..B1EDF2` are the previously recovered placement fragment.
Afterward it publishes command+28 and assigns command+04, context+08, context+0C,
context+14 in order. Each unequal assignment publishes the incoming raw identity,
increments its actual+04, then decrements the captured previous owner's actual+04
and invokes its current virtual0 only at zero. The context pointer at command+28
is reloaded for each later field. Context+10 then receives the borrowed command.

It obtains the current batch pool twice and acquires a slot from actual pool+04
each time, publishing results to command+0C then+10 without an additional retain.
The diagnostic becomes length1 with a two-byte allocation, byte1=NUL and byte0
copied from `00CE9A38`: the exact default is **`"X"`**. Existing length1 skips
allocation; a nonnull current buffer still receives the copy. Old nonnull storage
is returned with old length+1 before publishing the replacement. The actual
Pool A call convention includes alignment1. A private `std::pmr` string or a new
per-command pool is not the shared native storage contract.

The construction packet must preserve native staged cleanup. `B1F170` and
`B1F1F0` have three unwind states: diagnostic at+14, indexed array at+2C, ordered
array at+38. At a throw from the initializer, state2 runs ordered, indexed, then
diagnostic cleanup. There is **no scene/context/batch release in these constructor
maps**. Adding unconditional retained-owner rollback would change the native
failure behavior. The source helper can use explicit required pool/owner
dependencies, but host companion allocation must not silently introduce a new
publication or ownership rollback policy.

## Ready storage support

The assigned next packet is exactly the first seven rows. The final two rows
are separately identified EH/lifetime support and do not expand that packet.

| Entry; exclusive end | Bytes | Behavior |
| --- | ---: | --- |
| `B1C660..B1C6BF` | 95 | Group pointers: minimum1 grow-only reserve, copy raw pointers, free, publish data then capacity |
| `B1C7C0..B1C810` | 80 | Group pointers: reserve if needed, zero new cells, shrink count only |
| `B1CBE0..B1CC19` | 57 | Append through a pointer-to-pointer input; grow to max(1,2*capacity), publish cell then increment count |
| `B1D1F0..B1D207` | 23 | Ordered group-array destructor: resize0, free data, preserve dangling pointer/capacity |
| `B1D260..B1D277` | 23 | Indexed group-array destructor, same algorithm but distinct entry |
| `B1C6C0..B1C71F` | 95 | Command pointers: same reserve algorithm, actual queue+14 header |
| `B1CC80..B1CCD0` | 80 | Command pointers: same resize algorithm; no pointed-command destruction |
| `B1D590..B1D5A7` | 23 | Command-pointer array destructor: resize0, scalar-free its data; no pointed-command destruction |
| `41DD20..41DD3D` | 29 | String destructor: return nonnull buffer with length+1 and alignment1; do not clear header |

Reserve/resize/append use ECX=array header and one stack argument, RET4. Array
and string destruction use ECX=header and RET. Existing host vectors and
`RenderCommandPointerArray` do not overlay these actual headers. The shared
scalar/pointer allocators are available; use the existing same Pool A service
for string returns. `NativeString::release_to` clears fields, so it is not the
native destructor's dead-byte behavior. Valid count/capacity/spans and four-byte
allocation products remain preconditions rather than corruption recovery.

For all seven functions, use the caller's actual `{data00,count04,capacity08}`
storage, four-byte raw pointer cells, and the existing shared pointer allocator:
native reserve calls `BF55BE`, free calls `BF6989`. Reserve copies only current
live cells and leaves excess capacity unwritten. It reloads count/data in the
copy loop and publishes new data before capacity, after returning old storage.
The native minimum is1, not the batch entry array's minimum256. Resize preserves
stale cells when shrinking, initializes only newly exposed cells, and never
releases a pointed object. The two destructor entries resize0 then free pointer
storage while retaining its dangling data/capacity words. Append obtains and
dereferences its pointer-to-pointer input **after** any reserve; do not capture
the pointed value before the allocation boundary. The input pointer itself must
remain valid through that boundary. Count/capacity must be nonnegative and valid,
allocation products and capacity doubling must fit signed32, and callback
mutation must preserve addressed storage. No extra vector or count may be added.

## Complete teardown frontier

`B1DDD0` releases the required batch+0C then batch+10 through each captured actual
count/current virtual0, without null guards or clearing these two fields. It
walks the actual indexed group array; for each nonnull group it calls `B1D760`,
scalar-frees that group, then clears the originally addressed cell. The cursor
advances from its prior address while the end is recalculated from the current
array pointer/count after callbacks. It then releases/clears scene+04 and
context+28, destroys ordered storage before indexed storage, and returns the
diagnostic buffer. Group-array counts become zero, but data/capacity and the
diagnostic header remain native dangling bytes. The scalar wrapper tests only
flags bit0 after successful destruction; exceptions prevent scalar free.

`B1DDD0`'s EH states likewise clean diagnostic/indexed/ordered storage; they do
not release the remaining batches, groups, scene, or context. Original free
continuations, including `B1E6C5`, are stack cleanup rather than early returns.

Remaining prerequisites for a complete general destructor are concrete:

- Actual `0x4C` group storage and `B1D6F0/B1D760`, with the same raw binding at+00,
  two model identities at+1C/+20, and source pointer arrays+24/+30. Current
  `OwnedInstanceGroup` embeds shared pointers and host model/output objects.
- The group's binding must have its real actual+04/current-zero callback.
  Models must perform `B6DFA0` unlink followed by their real virtual+18 logical
  release and final owner destruction. Existing typed generated-model lifetime
  objects do not establish every original raw model allocation/geometry count.
- Command scene/root and context second-owner/target need canonical companions
  for their actual raw identities. The correct camera raw address is
  `&camera_owner.storage.node`; `camera_owner.storage` itself is a host view of
  references. `SceneResource` is a host scene view, not a raw scene+04 overlay.
- Native batch recycling is implemented, but generic use through
  `NativeRenderActualOwners` still needs a stable batch companion or explicit
  verified current-profile dispatch. A no-op resolver is never sufficient.
- The surface owner now has actual+04 and real native destruction/pool return,
  but its reusable `RenderCommandReference` binding and the exact target subtype
  must be supplied. Surface ownership does not establish texture2D ownership.

## Real queue publication and renderer dependencies

Actual queue singleton publication is `00F8D440`. `004C11F0` allocates34h,
constructs `B1F280`, publishes at `004C1271`, then obtains the lifetime manager
again and registers the current published pointer. It holds the captured actual
manager section, decrements depth before OS leave, and reloads publication after
unlock for its return. Use that same publication/domain; a private host queue
does not satisfy this path.

`B1EB80` inserts the raw command into actual queue+14/+18/+1C before any optional
work. False skips gathering; true calls `B72190` with ECX=command+04 and stack
context+28, then `B1E990`. Creators `B1F3C0` and `B1F4D0` allocate44h, construct,
copy a by-value diagnostic, publish the raw command to this list, and gather/
upload. Neither the placement constructor nor `B1EDC0` performs this queue
publication. The creators' failure maps do not provide complete command rollback
after constructor success. Shadow jobs and renderer preparation jobs remain
separate real services, not synchronous no-op substitutes.

`B1EBE0` publishes command context at **actual queue+30 before retaining it** and
releasing the previous context. It reloads the command for current virtual0
execution, then releases the current queue+30 and clears that slot after callback.
Control+20 is reloaded; zero destroys/frees the current indexed command. The list
is resized to zero only when the final current control remains zero. The existing
typed queue fragment is not that actual singleton/header publication.

Other complete native disposal routes are `B1E6D0..B1E718` (execute last command,
reload the current last pointer, destroy/free it, then decrement current count)
and `B1E720..B1E760` (destroy/free live indexed commands in ascending order, then
resize0). They preserve stale cells. Both have returning continuations after free
that the live pseudocode currently misses.

Queue construction does not permit a zero-filled placeholder: `B1F280` reads
the unwritten +20 preimage and, if it is2, calls the **current renderer** from
`00F8D394`, virtual+11C, before storing0. For concrete table `00D5F0A8`, the
verified slot at `00D5F1C4` is `00B28A90`: request worker stop when present, disable
optional synchronization, clear pipeline bindings, then Sleep100. Current worker
notes refine the stop acknowledgment, but this is still a required real renderer
operation and binding, not merely a mode counter or harmless empty callback.

The queue owns another array at+24/+28/+2C, with **20-byte rows**, each beginning
with a native string. `B1E7F0` destroys trailing row strings; growth uses `B1DB30`.
Those are distinct from the command-pointer list. Queue destructor `B1F330`
executes pending commands, destroys row storage, destroys command-pointer storage,
clears actual `F8D440`, installs `CE3818`, restores EH, then returns. Its complete
range is **`B1F330..B1F3BA`**, 138 bytes: current Ghidra incorrectly ends at
`B1F37A`, immediately after the first free. Its deleting wrapper is
`B1F6B0..B1F6CE`, RET4 and original EAX identity. Queue EH cleans row storage,
command-pointer storage, then base/unpublication `B1C3C0`.
The row-array EH wrapper `B1F150..B1F167` calls `B1E7F0(0)` then frees its data;
the command-array wrapper is `B1D590`. Both also have hidden free continuations.

## Evidence and acceptance boundary

The guarded CLI reverified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 48 selected exact spans, totaling 4,291 bytes,
match the installed binary SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report records every raw hash, complete end, relevant ABI, EH map, old
Ghidra bound, and source prerequisite. Undefined tails were decoded from these
verified original bytes; no shared disassembly or annotation was mutated.

For the next construction implementation, a focused original-byte sequence must
show the same 44h preimage, context identities/counts, both actual pool acquisitions,
`"X"` allocation, duplicate-camera retention, and native failure cleanup boundaries.
After group/owner dependencies close, extend that same sequence through the real
command destructor/deleter. Native queue publication and rendering require their
own remaining dependencies above. This discovery ran no new build, differential
execution, exception dispatch, game, or visual validation.
